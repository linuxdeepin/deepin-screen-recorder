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
#include "recordadmin.h"

#include <string.h>
#include <QMutexLocker>


Q_LOGGING_CATEGORY(XdgDesktopPortalKdeWaylandIntegration, "xdp-kde-wayland-integration")

//取消自动析构机制，此处后续还需优化
//Q_GLOBAL_STATIC(WaylandIntegration::WaylandIntegrationPrivate, globalWaylandIntegration)
static WaylandIntegration::WaylandIntegrationPrivate *globalWaylandIntegration = new  WaylandIntegration::WaylandIntegrationPrivate();

void WaylandIntegration::init(QStringList list)
{
    globalWaylandIntegration->initWayland(list);
}

bool WaylandIntegration::isEGLInitialized()
{
    return globalWaylandIntegration->isEGLInitialized();
}

void WaylandIntegration::stopStreaming()
{
    globalWaylandIntegration->stopVideoRecord();
    globalWaylandIntegration->stopStreaming();
}

bool WaylandIntegration::WaylandIntegrationPrivate::stopVideoRecord()
{
    return  m_recordAdmin->stopStream();
}

QMap<quint32, WaylandIntegration::WaylandOutput> WaylandIntegration::screens()
{
    return globalWaylandIntegration->screens();
}

WaylandIntegration::WaylandIntegration *WaylandIntegration::waylandIntegration()
{
    return globalWaylandIntegration;
}

// Thank you kscreen
void WaylandIntegration::WaylandOutput::setOutputType(const QString &type)
{
    const auto embedded = { QLatin1String("LVDS"),
                            QLatin1String("IDP"),
                            QLatin1String("EDP"),
                            QLatin1String("LCD")
                          };

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
    m_bufferSize = 60;
    m_ffmFrame = nullptr;
    qDBusRegisterMetaType<WaylandIntegrationPrivate::Stream>();
    qDBusRegisterMetaType<WaylandIntegrationPrivate::Streams>();
    m_recordAdmin = nullptr;
    //m_writeFrameThread = nullptr;
    m_bInitRecordAdmin = true;
    m_bGetFrame = true;
    //m_recordTIme = -1;
}

WaylandIntegration::WaylandIntegrationPrivate::~WaylandIntegrationPrivate()
{
    QMutexLocker locker(&m_mutex);
    for (int i = 0; i < m_freeList.size(); i++) {
        delete m_freeList[i];
    }
    m_freeList.clear();
    for (int i = 0; i < m_waylandList.size(); i++) {
        delete m_waylandList[i]._frame;
    }
    m_waylandList.clear();
    if (nullptr != m_ffmFrame) {
        delete m_ffmFrame;
        m_ffmFrame = nullptr;
    }
    if (nullptr != m_recordAdmin) {
        delete m_recordAdmin;
        m_recordAdmin = nullptr;
    }
}

bool WaylandIntegration::WaylandIntegrationPrivate::isEGLInitialized() const
{
    return m_eglInitialized;
}

void WaylandIntegration::WaylandIntegrationPrivate::bindOutput(int outputName, int outputVersion)
{
    KWayland::Client::Output *output = new KWayland::Client::Output(this);
    output->setup(m_registry->bindOutput(static_cast<uint32_t>(outputName), static_cast<uint32_t>(outputVersion)));
    m_bindOutputs << output;
}

void WaylandIntegration::WaylandIntegrationPrivate::stopStreaming()
{
    if (m_streamingEnabled) {
        m_streamingEnabled = false;

        // First unbound outputs and destroy remote access manager so we no longer receive buffers
        if (m_remoteAccessManager) {
            m_remoteAccessManager->release();
            m_remoteAccessManager->destroy();
        }
        qDeleteAll(m_bindOutputs);
        m_bindOutputs.clear();
        //        if (m_stream) {
        //            delete m_stream;
        //            m_stream = nullptr;
        //        }
    }
}

QMap<quint32, WaylandIntegration::WaylandOutput> WaylandIntegration::WaylandIntegrationPrivate::screens()
{
    return m_outputMap;
}

void WaylandIntegration::WaylandIntegrationPrivate::initWayland(QStringList list)
{
    m_recordAdmin = new RecordAdmin(list, this);
    //设置获取视频帧
    setBGetFrame(true);

    m_thread = new QThread(this);
    m_connection = new KWayland::Client::ConnectionThread;

    connect(m_connection, &KWayland::Client::ConnectionThread::connected, this, &WaylandIntegrationPrivate::setupRegistry, Qt::QueuedConnection);
    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied, this, [this] {
        if (m_queue)
        {
            delete m_queue;
            m_queue = nullptr;
        }

        m_connection->deleteLater();
        m_connection = nullptr;

        if (m_thread)
        {
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

    connect(output, &KWayland::Client::Output::changed, this, [this, name, version, output]() {
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Adding output:";
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    manufacturer: " << output->manufacturer();
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    model: " << output->model();
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "    resolution: " << output->pixelSize();

        WaylandOutput portalOutput;
        portalOutput.setManufacturer(output->manufacturer());
        portalOutput.setModel(output->model());
        portalOutput.setOutputType(output->model());
        portalOutput.setResolution(output->pixelSize());
        portalOutput.setWaylandOutputName(static_cast<int>(name));
        portalOutput.setWaylandOutputVersion(static_cast<int>(version));

        if (m_registry->hasInterface(KWayland::Client::Registry::Interface::RemoteAccessManager)) {
            KWayland::Client::Registry::AnnouncedInterface interface = m_registry->interface(KWayland::Client::Registry::Interface::RemoteAccessManager);
            if (!interface.name && !interface.version) {
                qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to start streaming: remote access manager interface is not initialized yet";
                return ;
            }
            m_remoteAccessManager = m_registry->createRemoteAccessManager(interface.name, interface.version);
            connect(m_remoteAccessManager, &KWayland::Client::RemoteAccessManager::bufferReady, this, [this](const void *output, const KWayland::Client::RemoteBuffer * rbuf) {
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

void WaylandIntegration::WaylandIntegrationPrivate::processBuffer(const KWayland::Client::RemoteBuffer *rbuf)
{
    //qDebug() << Q_FUNC_INFO;
    QScopedPointer<const KWayland::Client::RemoteBuffer> guard(rbuf);
    auto dma_fd = rbuf->fd();
    quint32 width = rbuf->width();
    quint32 height = rbuf->height();
    quint32 stride = rbuf->stride();
    //    if(!bGetFrame())
    //        return;
    if (m_bInitRecordAdmin) {
        m_bInitRecordAdmin = false;
        m_recordAdmin->init(static_cast<int>(width), static_cast<int>(height));
        frameStartTime = av_gettime();
    }
    unsigned char *mapData = static_cast<unsigned char *>(mmap(nullptr, stride * height, PROT_READ, MAP_SHARED, dma_fd, 0));
    if (MAP_FAILED == mapData) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "dma fd " << dma_fd << " mmap failed - ";
    }
    appendBuffer(mapData, static_cast<int>(width), static_cast<int>(height), static_cast<int>(stride), av_gettime() - frameStartTime);
    munmap(mapData, stride * height);
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
    if (!bGetFrame() || nullptr == frame || width <= 0 || height <= 0) {
        return;
    }

    int size = height * stride;
    unsigned char *ch = nullptr;
    if (m_bInit) {
        m_bInit = false;
        m_width = width;
        m_height = height;
        m_stride = stride;
        m_ffmFrame = new unsigned char[static_cast<unsigned long>(size)];
        for (int i = 0; i < m_bufferSize; i++) {
            ch = new unsigned char[static_cast<unsigned long>(size)];
            m_freeList.append(ch);
            //qDebug() << "创建内存空间";
        }
    }
    QMutexLocker locker(&m_mutex);
    if (m_waylandList.size() >= m_bufferSize) {
        //先进先出
        //取队首
        waylandFrame wFrame = m_waylandList.first();
        memset(wFrame._frame, 0, static_cast<size_t>(size));
        //拷贝当前帧
        memcpy(wFrame._frame, frame, static_cast<size_t>(size));
        wFrame._time = time;
        wFrame._width = width;
        wFrame._height = height;
        wFrame._stride = stride;
        wFrame._index = 0;
        //删队首
        m_waylandList.removeFirst();
        //存队尾
        m_waylandList.append(wFrame);
        //qDebug() << "环形缓冲区已满，删队首，存队尾";
    } else if (0 <= m_waylandList.size() &&  m_waylandList.size() < m_bufferSize) {
        if (m_freeList.size() > 0) {
            waylandFrame wFrame;
            wFrame._time = time;
            wFrame._width = width;
            wFrame._height = height;
            wFrame._stride = stride;
            wFrame._index = 0;
            //分配空闲内存
            wFrame._frame = m_freeList.first();
            memset(wFrame._frame, 0, static_cast<size_t>(size));
            //拷贝wayland推送的视频帧
            memcpy(wFrame._frame, frame, static_cast<size_t>(size));
            m_waylandList.append(wFrame);
            //qDebug() << "环形缓冲区未满，存队尾"
            //空闲内存占用，仅删除索引，不删除空间
            m_freeList.removeFirst();
        }
    }
}

int WaylandIntegration::WaylandIntegrationPrivate::frameIndex = 0;

bool WaylandIntegration::WaylandIntegrationPrivate::getFrame(waylandFrame &frame)
{
    QMutexLocker locker(&m_mutex);
    if (m_waylandList.size() <= 0 || nullptr == m_ffmFrame) {
        frame._width = 0;
        frame._height = 0;
        frame._frame = nullptr;
        return false;
    } else {
        int size = m_height * m_stride;
        //取队首，先进先出
        waylandFrame wFrame = m_waylandList.first();
        frame._width = wFrame._width;
        frame._height = wFrame._height;
        frame._stride = wFrame._stride;
        frame._time = wFrame._time;
        //m_ffmFrame 视频帧缓存
        frame._frame = m_ffmFrame;
        frame._index = frameIndex++;
        //拷贝到 m_ffmFrame 视频帧缓存
        memcpy(frame._frame, wFrame._frame, static_cast<size_t>(size));
        //删队首视频帧 waylandFrame，未删空闲内存 waylandFrame::_frame，只删索引，不删内存空间
        m_waylandList.removeFirst();
        //回收空闲内存，重复使用
        m_freeList.append(wFrame._frame);
        //qDebug() << "获取视频帧";
        return true;
    }
}

bool WaylandIntegration::WaylandIntegrationPrivate::isWriteVideo()
{
    QMutexLocker locker(&m_mutex);
    if (m_recordAdmin->m_writeFrameThread->bWriteFrame()) {
        return true;
    } else {
        return !m_waylandList.isEmpty();
    }
}

bool WaylandIntegration::WaylandIntegrationPrivate::bGetFrame()
{
    QMutexLocker locker(&m_bGetFrameMutex);
    return m_bGetFrame;
}

void WaylandIntegration::WaylandIntegrationPrivate::setBGetFrame(bool bGetFrame)
{
    QMutexLocker locker(&m_bGetFrameMutex);
    m_bGetFrame = bGetFrame;
}
