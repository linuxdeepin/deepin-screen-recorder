/*
 * Copyright © 2018 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 */

#include "waylandintegration.h"
#include "waylandintegration_p.h"
#include "screencaststream.h"

#include <QDBusArgument>
#include <QDBusMetaType>

#include <QEventLoop>
#include <QLoggingCategory>
#include <QThread>
#include <QTimer>

#include <QImage>

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/output.h>
#include <KWayland/Client/remote_access.h>

// system
#include <fcntl.h>
#include <unistd.h>


//=============

#include <KLocalizedString>

// KWayland
#include <KWayland/Client/fakeinput.h>

// system
#include <sys/mman.h>
#include <qdir.h>
#include "MainLoop.h"
#include "writeframetostreamthread.h"

#include <string.h>
#include <QMutexLocker>


Q_LOGGING_CATEGORY(XdgDesktopPortalKdeWaylandIntegration, "xdp-kde-wayland-integration")

Q_GLOBAL_STATIC(WaylandIntegration::WaylandIntegrationPrivate, globalWaylandIntegration)

void WaylandIntegration::init(int &argc, char **argv)
{
    globalWaylandIntegration->initWayland(argc,argv);
    globalWaylandIntegration->initDrm();

}

bool WaylandIntegration::isEGLInitialized()
{
    return globalWaylandIntegration->isEGLInitialized();
}

bool WaylandIntegration::startStreaming(const WaylandOutput &output)
{
    return globalWaylandIntegration->startStreaming(output);
}

void WaylandIntegration::stopStreaming()
{
    globalWaylandIntegration->stopStreaming();
}
bool WaylandIntegration::WaylandIntegrationPrivate::stopStreamMutex()
{
    //pthread_mutex_lock(&m_mtx_stream);
    bool returnVal = m_recordStreamObjPtr->stopStream();
    tempFps =0;
    tempType = RecordAudioType::NOS;
    tempCx =-1;
    tempCy = -1;
    tempCw = -1;
    tempCh = -1;
    pthread_cond_signal(&m_cond_stream);
    //pthread_mutex_unlock(&m_mtx_stream);
    //waylandFrame wf;
    //    while (getFrame(wf)) {
    //        //        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "write frame in thread------" ;
    //    }
    return returnVal;
}
QMap<quint32, WaylandIntegration::WaylandOutput> WaylandIntegration::screens()
{
    return globalWaylandIntegration->screens();
}

QVariant WaylandIntegration::streams()
{
    return globalWaylandIntegration->streams();
}

WaylandIntegration::WaylandIntegration * WaylandIntegration::waylandIntegration()
{
    return globalWaylandIntegration;
}

static const char * formatGLError(GLenum err)
{
    switch(err) {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return (QLatin1String("0x") + QString::number(err, 16)).toLocal8Bit().constData();
    }
}

// Thank you kscreen
void WaylandIntegration::WaylandOutput::setOutputType(const QString &type)
{
    const auto embedded = { QLatin1String("LVDS"),
                            QLatin1String("IDP"),
                            QLatin1String("EDP"),
                            QLatin1String("LCD") };

    for (const QLatin1String &pre : embedded) {
        if (type.toUpper().startsWith(pre)) {
            m_outputType = OutputType::Laptop;
            return;
        }
    }

    if (type.contains(QLatin1String("VGA")) || type.contains(QLatin1String("DVI")) || type.contains(QLatin1String("HDMI")) || type.contains(QLatin1String("Panel")) ||
            type.contains(QLatin1String("DisplayPort")) || type.startsWith(QLatin1String("DP")) || type.contains(QLatin1String("unknown"))) {
        m_outputType = OutputType::Monitor;
    } else if (type.contains(QLatin1String("TV"))) {
        m_outputType = OutputType::Television;
    } else {
        m_outputType = OutputType::Monitor;
    }
}

const QDBusArgument &operator >> (const QDBusArgument &arg, WaylandIntegration::WaylandIntegrationPrivate::Stream &stream)
{
    arg.beginStructure();
    arg >> stream.nodeId;

    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant map;
        arg.beginMapEntry();
        arg >> key >> map;
        arg.endMapEntry();
        stream.map.insert(key, map);
    }
    arg.endMap();
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator << (QDBusArgument &arg, const WaylandIntegration::WaylandIntegrationPrivate::Stream &stream)
{
    arg.beginStructure();
    arg << stream.nodeId;
    arg << stream.map;
    arg.endStructure();

    return arg;
}

Q_DECLARE_METATYPE(WaylandIntegration::WaylandIntegrationPrivate::Stream)
Q_DECLARE_METATYPE(WaylandIntegration::WaylandIntegrationPrivate::Streams)

WaylandIntegration::WaylandIntegrationPrivate::WaylandIntegrationPrivate()
    : WaylandIntegration()
    , m_eglInitialized(false)
    , m_registryInitialized(false)
    , m_connection(nullptr)
    , m_queue(nullptr)
    , m_registry(nullptr)
    , m_remoteAccessManager(nullptr)
{
    m_bInit = true;
    m_bufferSize = 30;
    m_ffmFrame = nullptr;
    qDBusRegisterMetaType<WaylandIntegrationPrivate::Stream>();
    qDBusRegisterMetaType<WaylandIntegrationPrivate::Streams>();
    m_recordStreamObjPtr = NULL;
    m_writeFrameThread = NULL;
    m_isStreamInit = false;
    m_isStreamObjCreat = false;
    m_recordTIme = -1;
}

WaylandIntegration::WaylandIntegrationPrivate::~WaylandIntegrationPrivate()
{
    for (int i=0;i<m_freeList.size();i++) {

        delete m_freeList[i];
    }
    for (int i=0;i<m_waylandList.size();i++) {
        delete m_waylandList[i]._frame;
    }
    if(nullptr != m_ffmFrame)
    {
        delete m_ffmFrame;
    }

    if (m_remoteAccessManager) {
        m_remoteAccessManager->destroy();
    }

    if (m_gbmDevice) {
        gbm_device_destroy(m_gbmDevice);
    }
    if(m_recordStreamObjPtr){
        delete m_recordStreamObjPtr;
        m_recordStreamObjPtr = NULL;
    }
    if(m_writeFrameThread){
        delete m_writeFrameThread;
        m_writeFrameThread = NULL;
    }
}

bool WaylandIntegration::WaylandIntegrationPrivate::isEGLInitialized() const
{
    return m_eglInitialized;
}

void WaylandIntegration::WaylandIntegrationPrivate::bindOutput(int outputName, int outputVersion)
{
    KWayland::Client::Output *output = new KWayland::Client::Output(this);
    output->setup(m_registry->bindOutput(outputName, outputVersion));
    m_bindOutputs << output;
}

bool WaylandIntegration::WaylandIntegrationPrivate::startStreaming(const WaylandOutput &output)
{
    m_stream = new ScreenCastStream(output.resolution());
    m_stream->init();

    connect(m_stream, &ScreenCastStream::startStreaming, this, [this, output] {
        m_streamingEnabled = true;
        bindOutput(output.waylandOutputName(), output.waylandOutputVersion());
    });

    connect(m_stream, &ScreenCastStream::stopStreaming, this, &WaylandIntegrationPrivate::stopStreaming);

    bool streamReady = false;
    QEventLoop loop;
    connect(m_stream, &ScreenCastStream::streamReady, this, [&loop, &streamReady] {
        loop.quit();
        streamReady = true;
    });

    // HACK wait for stream to be ready
    QTimer::singleShot(3000, &loop, &QEventLoop::quit);
    loop.exec();

    disconnect(m_stream, &ScreenCastStream::streamReady, this, nullptr);

    if (!streamReady) {
        if (m_stream) {
            delete m_stream;
            m_stream = nullptr;
        }
        return false;
    }

    // TODO support multiple outputs

    if (m_registry->hasInterface(KWayland::Client::Registry::Interface::RemoteAccessManager)) {
        KWayland::Client::Registry::AnnouncedInterface interface = m_registry->interface(KWayland::Client::Registry::Interface::RemoteAccessManager);
        if (!interface.name && !interface.version) {
            qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to start streaming: remote access manager interface is not initialized yet";
            return false;
        }
        m_remoteAccessManager = m_registry->createRemoteAccessManager(interface.name, interface.version);
        connect(m_remoteAccessManager, &KWayland::Client::RemoteAccessManager::bufferReady, this, [this] (const void *output, const KWayland::Client::RemoteBuffer * rbuf) {
            Q_UNUSED(output);
            connect(rbuf, &KWayland::Client::RemoteBuffer::parametersObtained, this, [this, rbuf] {
                processBuffer(rbuf);
            });
        });
        m_output = output.waylandOutputName();
        return true;
    }

    if (m_stream) {
        delete m_stream;
        m_stream = nullptr;
    }

    qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to start streaming: no remote access manager interface";
    return false;
}

void WaylandIntegration::WaylandIntegrationPrivate::stopStreaming()
{
    stopStreamMutex();
    if (m_streamingEnabled) {
        m_streamingEnabled = false;

        // First unbound outputs and destroy remote access manager so we no longer receive buffers
        if (m_remoteAccessManager) {
            m_remoteAccessManager->release();
            m_remoteAccessManager->destroy();
        }
        qDeleteAll(m_bindOutputs);
        m_bindOutputs.clear();

        if (m_stream) {
            delete m_stream;
            m_stream = nullptr;
        }
    }
}

QMap<quint32, WaylandIntegration::WaylandOutput> WaylandIntegration::WaylandIntegrationPrivate::screens()
{
    return m_outputMap;
}

QVariant WaylandIntegration::WaylandIntegrationPrivate::streams()
{
    Stream stream;
    stream.nodeId = m_stream->nodeId();
    stream.map = QVariantMap({{QLatin1String("size"), m_outputMap.value(m_output).resolution()}});
    return QVariant::fromValue<WaylandIntegrationPrivate::Streams>({stream});
}
//void *  writeFrameToStreamThread(void * object){
//    WaylandIntegration::WaylandIntegrationPrivate * pThis = (WaylandIntegration::WaylandIntegrationPrivate*)object;
//    while (pThis->m_isStreamObjCreat) {

//        WaylandIntegration::WaylandIntegrationPrivate::waylandFrame wf;
//        qDebug() << "=====================================================================================================";
//        //        bool isSeccess = pThis->getFrame(wf);
//        //        if(isSeccess){
//        //            QString name = QString::fromUtf8("test66655.jpg");
//        //            auto capture = new QImage(wf._frame, wf._width, wf._height, QImage::Format_RGB32);
//        //            //capture->save(name);
//        //            pThis->m_recordStreamObjPtr->addImage(capture);
//        //            delete  capture;
//        //        }

//        //        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "write frame in thread------" ;
//    }
//    return NULL;
//}
void *stopRecordStream(void* object){
    WaylandIntegration::WaylandIntegrationPrivate * pThis = (WaylandIntegration::WaylandIntegrationPrivate*)object;
    sleep(pThis->m_recordTIme);
    WaylandIntegration::stopStreaming();
    return NULL;
}
void cleanup_handler(void *arg)
{
    WaylandIntegration::WaylandIntegrationPrivate * pThis = (WaylandIntegration::WaylandIntegrationPrivate*)arg;
    printf("Cleanup handler of second thread./n");

    (void)pthread_mutex_unlock(&pThis->m_mtx_stream);
}
void * newStreamFuncOfMutxAuAndVidio(void * object){
    WaylandIntegration::WaylandIntegrationPrivate * pThis = (WaylandIntegration::WaylandIntegrationPrivate*)object;
    pthread_cleanup_push(cleanup_handler, object);
    if(pThis->m_recordStreamObjPtr){
        delete pThis->m_recordStreamObjPtr;
    }
    pThis->m_recordStreamObjPtr = new MainLoop();
    pThis->m_isStreamObjCreat = true;
    //    if(!pThis->m_isStreamInit){
    //        pThis->m_isStreamInit = true;
    //        StreamRecordAudioType recordType;
    //        if(pThis->tempType == RecordAudioType::MIC){
    //            recordType = StreamRecordAudioType::MIC;
    //        }else if(pThis->tempType == RecordAudioType::SYS){
    //            recordType = StreamRecordAudioType::SYS;
    //        }else if(pThis->tempType == RecordAudioType::MIC_SYS){
    //            recordType = StreamRecordAudioType::MIC_SYS;
    //        }else if(pThis->tempType == RecordAudioType::NOS){
    //            recordType = StreamRecordAudioType::NOS;
    //        }else{
    //             qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "startInit recordType error  recordType=%d"<< recordType;
    ////             return false;

    //        }
    //        qDebug() << pThis->argvList;
    ////        qDebug() << "------format="<<format;
    ////        if(tempCw < 0){
    ////            tempCw = width;
    ////        }
    ////        if(tempCh < 0){
    ////            tempCh = height;
    ////        }
    //        QByteArray cpath =pThis->tempFilePath.toLocal8Bit();
    //        pThis->m_streamPtr->startInit(width,height,tempFps,recordType,tempCx,tempCy,tempCw,tempCh,cpath.data());
    //    }
    //    while (1)
    //       {
    //           pthread_mutex_lock(&pThis->m_mtx_stream);
    //           while (pThis->m_streamPtr->m_frontImage == NULL)
    //           {
    //               pthread_cond_wait(&pThis->m_cond_stream, &pThis->m_mtx_stream);

    //           }
    //           printf("Got %d from front of queue/n", p->n_number);
    //           pthread_mutex_unlock(&pThis->m_mtx_stream);             //临界区数据操作完毕，释放互斥锁
    //       }
    pthread_mutex_lock(&pThis->m_mtx_stream);

    while(!pThis->m_recordStreamObjPtr->m_isOverFlage)
    {
        pthread_cond_wait(&pThis->m_cond_stream, &pThis->m_mtx_stream);

        //        printf("newStreamFuncOfMutxAuAndVidio isOver =/n",pThis->m_streamPtr->m_isOverFlage);

    }
    pThis->m_isStreamObjCreat = false;
    if(pThis->m_recordStreamObjPtr){
        delete pThis->m_recordStreamObjPtr;
        pThis->m_recordStreamObjPtr = NULL;
    }
    pthread_mutex_unlock(&pThis->m_mtx_stream);

    pthread_cleanup_pop(0);
    return NULL;
}
bool WaylandIntegration::WaylandIntegrationPrivate::recordStreamMutexInit(int fps,RecordAudioType type,int cx,int cy,int cw,int ch,QString outfile)
{
    //    m_CapVideoThread;
    if(m_recordStreamObjPtr==NULL){
        int rc = pthread_create(&m_initRecordStreamThread, NULL, newStreamFuncOfMutxAuAndVidio, (void *)this);
        if(rc==0){
            pthread_detach(m_initRecordStreamThread);
        }
    }else{
        printf("---streamInit is start not over!!/n");
        return false;
    }

    m_writeFrameThread = new WriteFrameToStreamThread(this);
    m_writeFrameThread->start();
#ifdef HH_DEBUG
    tempFps =fps;
    tempType = type;
    tempCx =cx;
    tempCy = cx;
    tempCw = cw;
    tempCh = ch;
    tempFilePath.clear();
    tempFilePath = outfile;
#endif


    return true;
}
void WaylandIntegration::WaylandIntegrationPrivate::steamMutexStopInit(){
    int rc = pthread_create(&m_stopRecordstreamThread2, NULL, stopRecordStream, (void *)this);
    pthread_detach(m_stopRecordstreamThread2);
}
// void WaylandIntegration::WaylandIntegrationPrivate::stopRecord()
// {
//     //通知录屏完成
//     QDBusInterface notification(QString::fromUtf8("com.deepin.ScreenRecorder"),
//                                QString::fromUtf8("/com/deepin/ScreenRecorder"),
//                                QString::fromUtf8("com.deepin.ScreenRecorder"),
//                                QDBusConnection::sessionBus());
//     QList<QVariant> arg;
//     notification.callWithArgumentList(QDBus::AutoDetect,QString::fromUtf8("stopRecord"),arg);



// }

void WaylandIntegration::WaylandIntegrationPrivate::initDrm()
{
    m_drmFd = open("/dev/dri/renderD128", O_RDWR);

    if (m_drmFd == -1) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Cannot open render node: " << strerror(errno);
        return;
    }

    //    m_gbmDevice = gbm_create_device(m_drmFd);

    //    if (!m_gbmDevice) {
    //        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Cannot create GBM device: " << strerror(errno);
    //    }

    initEGL();
}

void WaylandIntegration::WaylandIntegrationPrivate::initEGL()
{
    // Get the list of client extensions
    const char* clientExtensionsCString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    qCWarning(XdgDesktopPortalKdeWaylandIntegration) <<clientExtensionsCString;
    const QByteArray clientExtensionsString = QByteArray::fromRawData(clientExtensionsCString, qstrlen(clientExtensionsCString));
    if (clientExtensionsString.isEmpty()) {
        // If eglQueryString() returned NULL, the implementation doesn't support
        // EGL_EXT_client_extensions. Expect an EGL_BAD_DISPLAY error.
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "No client extensions defined! " << formatGLError(eglGetError());
        return;
    }

    m_egl.extensions = clientExtensionsString.split(' ');


    qDebug() << Q_FUNC_INFO << "EGL扩展模块：" << m_egl.extensions;


    //("EGL_EXT_client_extensions", "EGL_EXT_platform_base",
    //"EGL_KHR_client_get_all_proc_addresses", "EGL_KHR_platform_gbm",
    //"EGL_KHR_platform_wayland", "EGL_EXT_platform_wayland")

    // Use eglGetPlatformDisplayEXT() to get the display pointer
    // if the implementation supports it.
    if (!m_egl.extensions.contains(QByteArrayLiteral("EGL_EXT_platform_base")) ||
            //            !m_egl.extensions.contains(QByteArrayLiteral("EGL_MESA_platform_gbm"))) {
            !m_egl.extensions.contains(QByteArrayLiteral("EGL_EXT_platform_wayland"))) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "One of required EGL extensions is missing";
        return;
    }

    //    m_egl.display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA, m_gbmDevice, nullptr);
    //m_egl.display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, m_gbmDevice, nullptr);
    //m_egl.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    m_egl.display = eglGetDisplay((EGLNativeDisplayType)m_connection->display());

    qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "m_egl.display: " << m_egl.display;

    if (m_egl.display == EGL_NO_DISPLAY) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Error during obtaining EGL display: " << formatGLError(eglGetError());
        return;
    }

    EGLint major, minor;
    if (eglInitialize(m_egl.display, &major, &minor) == EGL_FALSE) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Error during eglInitialize: " << formatGLError(eglGetError());
        return;
    }
    qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "major: " << major<<"minor: " << minor;

    if (eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "bind OpenGL API failed";
        return;
    }


    m_egl.context = eglCreateContext(m_egl.display, nullptr, EGL_NO_CONTEXT, nullptr);
    qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "m_egl.context: " << m_egl.context;
    if (m_egl.context == EGL_NO_CONTEXT) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Couldn't create EGL context: " << formatGLError(eglGetError());
        return;
    }

    m_egl.create_image = (void *) eglGetProcAddress("eglCreateImageKHR");
    if (m_egl.create_image == nullptr) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Couldn't support eglCreateImageKHR";
        return;
    }

    m_egl.destroy_image = (void *) eglGetProcAddress("eglDestroyImageKHR");
    if (m_egl.destroy_image == nullptr) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Couldn't support eglDestroyImageKHR";
        return;
    }

    m_egl.image_target_texture_2d = (void *) eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (m_egl.image_target_texture_2d == nullptr) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Couldn't support glEGLImageTargetTexture2DOES";
        return;
    }

    qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Egl initialization succeeded";
    qCDebug(XdgDesktopPortalKdeWaylandIntegration) << QStringLiteral("EGL version: %1.%2").arg(major).arg(minor);

    m_eglInitialized = true;
}

void WaylandIntegration::WaylandIntegrationPrivate::initWayland(int &argc, char **argv)
{
    if(argc>8){
        // 程序名称，视频类型，视频宽，视频高，视频x坐标，视频y坐标，视频帧率，视频保存路径，音频类型, 录制时间单位:秒
        QString tempStr;
        if(!recordStreamMutexInit(0,RecordAudioType::NOS,0,0,0,0,tempStr)){
            return;
        }
        for(int i=0;i<argc;i++)
        {
            QString tempStr = QString::fromUtf8(argv[i]);
            argvList.append(tempStr);
            if(i==2){
                //录屏不支持奇数，转偶数
                int width =  tempStr.toInt()/2*2;
                tempCw = width;
            }else if(i==3){
                int height =  tempStr.toInt()/2*2;
                tempCh = height;
            }else if(i==4){
                int x =  tempStr.toInt();
                tempCx = x;
            }else if(i==5){
                int y =  tempStr.toInt();
                tempCy = y;
            }else if(i==6){
                int fps =  tempStr.toInt();
                tempFps =fps;
            }else if(i==7){
                tempFilePath.clear();
                tempFilePath = tempStr;
            }else if(i==8){
                //2 mic,3 sys,4 mic_sys,
                int type =  tempStr.toInt();
                if(type == 2){
                    tempType = RecordAudioType::MIC;
                }else if(type==3){
                    tempType = RecordAudioType::SYS;
                }else if(type==4){
                    tempType = RecordAudioType::MIC_SYS;
                }else{
                    tempType = RecordAudioType::NOS;
                }
            }else if(i==9){
                int recordTime =  tempStr.toInt();
                m_recordTIme = recordTime;
                if( m_recordTIme>0){
                    steamMutexStopInit();
                }
            }
        }
    }

    m_thread = new QThread(this);
    m_connection = new KWayland::Client::ConnectionThread;

    connect(m_connection, &KWayland::Client::ConnectionThread::connected, this, &WaylandIntegrationPrivate::setupRegistry, Qt::QueuedConnection);
    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied, this, [this] {
        if (m_queue) {
            delete m_queue;
            m_queue = nullptr;
        }

        m_connection->deleteLater();
        m_connection = nullptr;

        if (m_thread) {
            m_thread->quit();
            if (!m_thread->wait(3000)) {
                m_thread->terminate();
                m_thread->wait();
            }
            delete m_thread;
            m_thread = nullptr;
        }
    });
    connect(m_connection, &KWayland::Client::ConnectionThread::failed, this, [this] {
        m_thread->quit();
        m_thread->wait();
    });

    m_thread->start();
    m_connection->moveToThread(m_thread);
    m_connection->initConnection();
}

void WaylandIntegration::WaylandIntegrationPrivate::addOutput(quint32 name, quint32 version)
{
    KWayland::Client::Output *output = new KWayland::Client::Output(this);
    output->setup(m_registry->bindOutput(name, version));

    connect(output, &KWayland::Client::Output::changed, this, [this, name, version, output] () {
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Adding output:";
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    manufacturer: " << output->manufacturer();
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    model: " << output->model();
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    resolution: " << output->pixelSize();

        WaylandOutput portalOutput;
        portalOutput.setManufacturer(output->manufacturer());
        portalOutput.setModel(output->model());
        portalOutput.setOutputType(output->model());
        portalOutput.setResolution(output->pixelSize());
        portalOutput.setWaylandOutputName(name);
        portalOutput.setWaylandOutputVersion(version);

        if (m_registry->hasInterface(KWayland::Client::Registry::Interface::RemoteAccessManager)) {
            KWayland::Client::Registry::AnnouncedInterface interface = m_registry->interface(KWayland::Client::Registry::Interface::RemoteAccessManager);
            if (!interface.name && !interface.version) {
                qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to start streaming: remote access manager interface is not initialized yet";
                return ;
            }
            m_remoteAccessManager = m_registry->createRemoteAccessManager(interface.name, interface.version);
            connect(m_remoteAccessManager, &KWayland::Client::RemoteAccessManager::bufferReady, this, [this] (const void *output, const KWayland::Client::RemoteBuffer * rbuf) {
                Q_UNUSED(output);
                connect(rbuf, &KWayland::Client::RemoteBuffer::parametersObtained, this, [this, rbuf] {
                    processBuffer(rbuf);
                });
            });
            //            m_output = output.waylandOutputName();
            return ;
        }
        m_outputMap.insert(name, portalOutput);

        //        delete output;
    });
}

void WaylandIntegration::WaylandIntegrationPrivate::removeOutput(quint32 name)
{
    WaylandOutput output = m_outputMap.take(name);
    qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Removing output:";
    qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    manufacturer: " << output.manufacturer();
    qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    model: " << output.model();
}

void WaylandIntegration::WaylandIntegrationPrivate::processBuffer(const KWayland::Client::RemoteBuffer* rbuf)
{
    QScopedPointer<const KWayland::Client::RemoteBuffer> guard(rbuf);
    auto dma_fd = rbuf->fd();
    quint32 width = rbuf->width();
    quint32 height = rbuf->height();
    quint32 stride = rbuf->stride();
    quint32 format = rbuf->format();
    if(m_isStreamObjCreat){
        if(!m_isStreamInit){
            frameStartTime = av_gettime();
            m_isStreamInit = true;
            StreamRecordAudioType recordType;
            if(tempType == RecordAudioType::MIC){
                recordType = StreamRecordAudioType::MIC;
            }else if(tempType == RecordAudioType::SYS){
                recordType = StreamRecordAudioType::SYS;
            }else if(tempType == RecordAudioType::MIC_SYS){
                recordType = StreamRecordAudioType::MIC_SYS;
            }else if(tempType == RecordAudioType::NOS){
                recordType = StreamRecordAudioType::NOS;
            }else{
                qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "startInit recordType error  recordType=%d"<< recordType;
            }
            QByteArray cpath =tempFilePath.toLocal8Bit();
            m_recordStreamObjPtr->startInit(width,height,tempFps,recordType,tempCx,tempCy,tempCw,tempCh,cpath.data());
        }
    }else{
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "record stream obj not created or stop record!!!" ;
        return ;
    }
    if (m_lastFrameTime.isValid() &&m_lastFrameTime.msecsTo(QDateTime::currentDateTime()) < (1000 / m_stream->framerate())) {
        close(dma_fd);
        return;
    }
    unsigned char *mapData = (unsigned char *)mmap(0, stride * height, PROT_READ|PROT_WRITE, MAP_SHARED, dma_fd, 0);
    if (MAP_FAILED == mapData) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "dma fd " << dma_fd <<" mmap failed - ";
    }
    appendBuffer(mapData,width,height,stride,av_gettime()-frameStartTime);
    munmap(mapData,stride * height);
    close(dma_fd);
}

void WaylandIntegration::WaylandIntegrationPrivate::setupRegistry()
{
    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);

    connect(m_registry, &KWayland::Client::Registry::outputAnnounced, this, &WaylandIntegrationPrivate::addOutput);
    connect(m_registry, &KWayland::Client::Registry::outputRemoved, this, &WaylandIntegrationPrivate::removeOutput);

    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, this, [this] {
        m_registryInitialized = true;
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Registry initialized";
    });

    m_registry->create(m_connection);
    m_registry->setEventQueue(m_queue);
    m_registry->setup();
}

void WaylandIntegration::WaylandIntegrationPrivate::appendBuffer(unsigned char *frame, int width, int height, int stride, int64_t time)
{
    int size = height*stride;
    unsigned char *ch = nullptr;
    if(m_bInit)
    {
        m_bInit = false;
        m_width = width;
        m_height = height;
        m_stride = stride;
        m_ffmFrame = new unsigned char[size];
        for (int i=0;i< m_bufferSize;i++)
        {
            ch = new unsigned char[size];
            m_freeList.append(ch);
            //qDebug() << "创建内存空间";
        }
    }
    QMutexLocker locker(&m_mutex);
    if(m_waylandList.size() >= m_bufferSize){
        //先进先出
        //取队首
        waylandFrame wFrame = m_waylandList.first();
        memset(wFrame._frame,0,size);
        //拷贝当前帧
        memcpy(wFrame._frame,frame,size);
        wFrame._time = time;
        wFrame._width = width;
        wFrame._height = height;
        wFrame._stride = stride;
        //删队首
        m_waylandList.removeFirst();
        //存队尾
        m_waylandList.append(wFrame);
        //qDebug() << "环形缓冲区已满，删队首，存队尾";
    }
    else if(0 <= m_waylandList.size() < m_bufferSize)
    {
        if(m_freeList.size()>0)
        {
            waylandFrame wFrame;
            wFrame._time = time;
            wFrame._width = width;
            wFrame._height = height;
            wFrame._stride = stride;
            wFrame._frame = m_freeList.first();
            memset(wFrame._frame,0,size);
            memcpy(wFrame._frame,frame,size);
            m_waylandList.append(wFrame);
            //qDebug() << "环形缓冲区未满，存队尾";
            //空闲内存占用
            m_freeList.removeFirst();
        }
    }
}

bool WaylandIntegration::WaylandIntegrationPrivate::getFrame(waylandFrame &frame)
{
    QMutexLocker locker(&m_mutex);
    if(m_waylandList.size() <= 0 || nullptr == m_ffmFrame){
        return false;
    }
    else {
        int size = m_height*m_stride;
        //取队首，先进先出
        waylandFrame wFrame = m_waylandList.first();
        frame._width = wFrame._width;
        frame._height = wFrame._height;
        frame._stride = wFrame._stride;
        frame._time = wFrame._time;
        frame._frame = m_ffmFrame;
        memcpy(frame._frame,wFrame._frame,size);
        m_waylandList.removeFirst();
        m_freeList.append(wFrame._frame);
        //qDebug() << "获取视频帧";
        return true;
    }
}
