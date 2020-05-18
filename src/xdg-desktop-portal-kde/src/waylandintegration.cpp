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
    pthread_mutex_lock(&m_mtx_stream);
    bool returnVal = m_recordStreamObjPtr->stopStream();
    tempFps =0;
    tempType = RecordAudioType::NOS;
    tempCx =-1;
    tempCy = -1;
    tempCw = -1;
    tempCh = -1;
    pthread_cond_signal(&m_cond_stream);
    pthread_mutex_unlock(&m_mtx_stream);
    waylandFrame wf;
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

    qDebug() << "test:++++++++++++++++++++++++++++++++++++++++++++++++++++:111111111111111111111111";
    m_bInit = true;
    m_bufferSize = 30;
    cacheFrame = nullptr;
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
    for (int i=0;i<m_waylandList.size();i++) {

        delete m_waylandList[i];
    }
    if(nullptr != cacheFrame)
    {
        delete cacheFrame;
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
void *  writeFrameToStreamThread(void * object){
    WaylandIntegration::WaylandIntegrationPrivate * pThis = (WaylandIntegration::WaylandIntegrationPrivate*)object;
    while (pThis->m_isStreamObjCreat) {

        WaylandIntegration::WaylandIntegrationPrivate::waylandFrame wf;
        bool isSeccess = pThis->getFrame(wf);
        if(isSeccess){
            QString name = QString::fromUtf8("test66655.jpg");
            auto capture = new QImage(wf.frame, wf.width, wf.height, QImage::Format_RGB32);
            //capture->save(name);
            pThis->m_recordStreamObjPtr->addImage(capture);
            delete  capture;
        }

        //        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "write frame in thread------" ;
    }
    return NULL;
}
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

    qDebug() << "test:++++++++++++++++++++++++++++++++++++++++++++++++++++:22222222222222222222222222222222";

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
        // 0 2160 1440 0 0 24 ./33333h.mp4 2 10
        //    程序名称，视频类型，视频宽，视频高，视频x坐标，视频y坐标，视频帧率，视频保存路径，音频类型, 录制时间单位:秒
        QString tempStr;
        if(!recordStreamMutexInit(0,RecordAudioType::NOS,0,0,0,0,tempStr)){
            return;
        }
        for(int i=0;i<argc;i++)
        {
            QString tempStr = QString::fromUtf8(argv[i]);
            argvList.append(tempStr);
            if(i==2){
                int width =  tempStr.toInt();
                tempCw = width;
            }else if(i==3){
                int height =  tempStr.toInt();
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

int i = 0;

void WaylandIntegration::WaylandIntegrationPrivate::processBuffer(const KWayland::Client::RemoteBuffer* rbuf)
{
    QScopedPointer<const KWayland::Client::RemoteBuffer> guard(rbuf);
    QTime time;
    time.start();
    // auto gbmHandle = rbuf->fd();
    auto dma_fd = rbuf->fd();
    auto width = rbuf->width();
    auto height = rbuf->height();
    auto stride = rbuf->stride();
    auto format = rbuf->format();
    if(m_isStreamObjCreat){
        if(!m_isStreamInit){
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
                //             return false;

            }
            qDebug() << argvList;
            qDebug() << "------format="<<format;
            //        if(tempCw < 0){
            //            tempCw = width;
            //        }
            //        if(tempCh < 0){
            //            tempCh = height;
            //        }
            QByteArray cpath =tempFilePath.toLocal8Bit();
            m_recordStreamObjPtr->startInit(width,height,tempFps,recordType,tempCx,tempCy,tempCw,tempCh,cpath.data());
        }
    }else{
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "record stream obj not created or stop record!!!" ;
        return ;
    }

    //    qCDebug(XdgDesktopPortalKdeWaylandIntegration)
    //            << QStringLiteral("Incoming dma fd %1, %2x%3, stride %4, fourcc 0x%5").arg(dma_fd).arg(width).arg(height).arg(stride).arg(QString::number(format, 16))
    //            << format;

    //    if (!m_streamingEnabled) {
    //        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Streaming is disabled";
    //        close(gbmHandle);
    //        return;
    //    }

    if (m_lastFrameTime.isValid() &&
            m_lastFrameTime.msecsTo(QDateTime::currentDateTime()) < (1000 / m_stream->framerate())) {
        close(dma_fd);
        return;
    }

    // if (!gbm_device_is_format_supported(m_gbmDevice, format, GBM_BO_USE_SCANOUT)) {
    //     qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to process buffer: GBM format is not supported by device!";
    //     close(gbmHandle);
    //     return;
    // }

    // // import GBM buffer that was passed from KWin
    // gbm_import_fd_data importInfo = {gbmHandle, width, height, stride, format};
    // gbm_bo *imported = gbm_bo_import(m_gbmDevice, GBM_BO_IMPORT_FD, &importInfo, GBM_BO_USE_SCANOUT);
    // if (!imported) {
    //     qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to process buffer: Cannot import passed GBM fd - " << strerror(errno);
    //     close(gbmHandle);
    //     return;
    // }

    // unsigned int mapStride=0;
    // unsigned char * mapData=0;
    // unsigned char * mapbuf=0;
    // mapbuf = (unsigned char *)gbm_bo_map(imported,0,0,width,height,GBM_BO_TRANSFER_READ,&mapStride,&mapData);

    // unsigned char * mapData=0;
    unsigned char *mapData = (unsigned char *)mmap(0, stride * height, PROT_READ|PROT_WRITE, MAP_SHARED, dma_fd, 0);
    if (MAP_FAILED == mapData) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "dma fd " << dma_fd <<" mmap failed - ";
        // goto error;
    }

    //---------------------------------------------------------------------
    appendBuffer(mapData,(int)width,(int)height,(int)stride);



    //    qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "dma fd " << dma_fd <<" mmap success mapData: " << mapData;

    // save buffer to png
    // QString filename(QLatin1String("/home/wugang/dumpDmaFd.png"));
    // const QImage destImage = QImage(mapData, width, height, QImage::Format_RGB32);
    // destImage.save(filename);

    // qCDebug(XdgDesktopPortalKdeWaylandIntegration)
    //         << QStringLiteral("mmap fd %1, %2x%3, stride %4, mapData:%p").arg(dma_fd).arg(width).arg(height).arg(stride).arg(mapData);

    // bind context to render thread
    eglMakeCurrent(m_egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_egl.context);

    //const EGLint attribs[] = {
    //        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
    //        EGL_NONE
    //    };

    // create EGL image from imported BO
    // EGLImageKHR image = eglCreateImageKHR(m_egl.display, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, imported, 0);

    // const EGLint attribs[] = {
    //     EGL_WAYLAND_PLANE_WL, 0,
    //     EGL_NONE
    // };

    // EGLImageKHR image = eglCreateImageKHR(m_egl.display, EGL_NO_CONTEXT, EGL_WAYLAND_BUFFER_WL,
    //                                   (EGLClientBuffer)imported, attribs);


    EGLint attribs[30];
    unsigned int atti = 0;

    attribs[atti++] = EGL_WIDTH;
    attribs[atti++] = width;
    attribs[atti++] = EGL_HEIGHT;
    attribs[atti++] = height;
    attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
    attribs[atti++] = format;

    attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
    attribs[atti++] = dma_fd;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
    attribs[atti++] = 0;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
    attribs[atti++] = stride;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
    attribs[atti++] = 0;
    attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
    attribs[atti++] = 0;
    attribs[atti] = EGL_NONE;

    EGLImageKHR image = m_egl.create_image(m_egl.display, m_egl.context,
                                           EGL_LINUX_DMA_BUF_EXT, NULL, attribs);

    if (image == EGL_NO_IMAGE_KHR) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to process buffer: Error creating EGLImageKHR - " << formatGLError(glGetError());
        return;
    } else {
        //        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "success to create texture image for dma fd "<< dma_fd;
    }

    // We can already close gbm handle
    // gbm_bo_destroy(imported);
    close(dma_fd);

    // create GL 2D texture for framebuffer
    GLuint texture;
    glGenTextures(1, &texture);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glBindTexture(GL_TEXTURE_2D, texture);
    // glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_egl.image_target_texture_2d(GL_TEXTURE_EXTERNAL_OES, image);

    // bind framebuffer to copy pixels from
    // GLuint framebuffer;
    // glGenFramebuffers(1, &framebuffer);
    // glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    // const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // if (status != GL_FRAMEBUFFER_COMPLETE) {
    //     qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to process buffer: glCheckFramebufferStatus failed - " << formatGLError(glGetError());
    //     glDeleteTextures(1, &texture);
    //     glDeleteFramebuffers(1, &framebuffer);
    //     eglDestroyImageKHR(m_egl.display, image);
    //     return;
    // }

    // auto capture = new QImage(QSize(width, height), QImage::Format_RGBA8888);
    // glViewport(0, 0, width, height);
    // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, capture->bits());
    // glGetTexImage(GL_TEXTURE_EXTERNAL_OES, 0, GL_RGBA, GL_UNSIGNED_BYTE, capture->bits());
    //   QImage* m_image =  new QImage(1920,1080, QImage::Format_RGB32);
    //    m_image->fill(Qt::white);
    //    int rawDataCount = 0;
    //    for (int y = 0;y<1920;y++)
    //    {
    //        uint* line = (uint*)m_image->scanLine(y);
    //        for (int x=0;x<1080;x++)
    //        {
    //            WORD temp = cameraZero[rawDataCount++];
    //            BYTE value = temp >> 2;
    //            QColor grey(value,value,value);
    //            line[x] = qRgba(value,value,value,100);
    //        }
    //    }


    //    QString path =QDir::currentPath();


    //-----------------------------------------------------------------------------------------------
    //    获取缓存函数 int getNextBuffer(unsigned char *& mapData,int &width,int& height,int& stride);
    /***
 * int getNextBuffer(unsigned char *& mapData,int &width,int& height,int& stride){
 * mapData = ;
 * width = ;
 * height = ;
 * stride = ;
 * return 0; //成功返回0.
 * }
 *
 * */

    //    waylandFrame wf;
    //    getFrame(wf);
    //    QString name = QString::fromUtf8("test666.jpg");



    //    auto capture = new QImage(wf.frame, wf.width, wf.height, QImage::Format_RGB32);
    //    capture->save(name);

    //    QThread::sleep(10000);


    //    m_recordStreamObjPtr->addImage(capture);

    int elapsed = time.elapsed();
    qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "-----frame elapsed " << elapsed ;
    //-----------------------------------------------------------------------------------------------


    //    if (m_stream->recordFrame(capture->bits())) {
    //        m_lastFrameTime = QDateTime::currentDateTime();
    //    }

    glDeleteTextures(1, &texture);
    // glDeleteFramebuffers(1, &framebuffer);
    // eglDestroyImageKHR(m_egl.display, image);
    m_egl.destroy_image(m_egl.display, image);

    munmap(mapData,stride * height);
    //    delete capture;
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

void WaylandIntegration::WaylandIntegrationPrivate::appendBuffer(unsigned char *frame, int width, int height, int stride)
{
    int size = height*stride;
    unsigned char *ch = nullptr;
    if(m_bInit)
    {
        m_bInit = false;
        m_width = width;
        m_height = height;
        m_stride = stride;
        cacheFrame = new unsigned char[size];
        for (int i=0;i< m_bufferSize;i++)
        {
            ch = new unsigned char[size];
            m_catchList.append(ch);
            qDebug() << "创建内存空间";
        }
    }
    QMutexLocker locker(&m_mutex);
    if(m_waylandList.size() >= m_bufferSize){
        //先进先出
        //取队首
        ch = m_waylandList.first();
        memset(ch,0,size);
        //拷贝当前帧
        memcpy(ch,frame,size);
        //删队首
        m_waylandList.removeLast();
        //存队尾
        m_waylandList.append(ch);
        qDebug() << "环形缓冲区已满，删队首，存队尾";
    }
    else if(0 <= m_waylandList.size() < m_bufferSize)
    {
        if(m_catchList.size()>0)
        {
            ch = m_catchList.first();
            memset(ch,0,size);
            memcpy(ch,frame,size);
            m_waylandList.append(ch);
            qDebug() << "环形缓冲区未满，存队尾";
            m_catchList.removeFirst();
        }
    }
}


bool WaylandIntegration::WaylandIntegrationPrivate::getFrame(waylandFrame &f)
{
    QMutexLocker locker(&m_mutex);
    if(m_waylandList.size() <= 0 || nullptr == cacheFrame){
        return false;
    }
    else {
        int size = m_height*m_stride;
        //取队首，先进先出
        unsigned char* ch = m_waylandList.first();
        f.width = m_width;
        f.height = m_height;
        f.stride = m_stride;
        f.frame = cacheFrame;
        memcpy(f.frame,ch,size);
        m_waylandList.removeFirst();
        m_catchList.append(ch);
        qDebug() << "获取视频帧";
        return true;
    }
}
