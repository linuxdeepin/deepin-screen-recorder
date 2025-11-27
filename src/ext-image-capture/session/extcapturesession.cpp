// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extcapturesession.h"
#include "../frame/extcaptureframe.h"
#include "../../protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h"
#include "../../protocols/ext-image-copy-capture/wayland-ext-image-copy-capture-v1-client-protocol.h"
#include "../../protocols/linux-dmabuf/qwayland-linux-dmabuf-unstable-v1.h"
#include "../../protocols/linux-dmabuf/wayland-linux-dmabuf-unstable-v1-client-protocol.h"

#include <QDebug>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include "../utils/log.h"

// Wayland 相关
#include <wayland-client.h>
#include <cstring>

// Wayland 格式定义
#include <wayland-client.h>

class ExtCaptureSession::Private : public QtWayland::ext_image_copy_capture_session_v1
{
public:
    Private(ExtCaptureSession *q) : q_ptr(q) {}

    ExtCaptureSession *q_ptr;
    SessionState state = Uninitialized;
    CaptureConfig config;
    
    QList<uint32_t> shmFormats;        // 共享内存格式
    QList<uint32_t> dmabufFormats;     // DMA缓冲区格式
    QByteArray dmabufDevice;           // DMA设备
    
    bool constraintsReceived = false;
    ExtCaptureFrame *currentFrame = nullptr;
    
    // Wayland 全局对象
    wl_shm *waylandShm = nullptr;
    wl_registry *registry = nullptr;
    QtWayland::zwp_linux_dmabuf_v1 *linuxDmabuf = nullptr;
    
    // 初始化 Wayland 全局对象
    void initializeWaylandGlobals() {
        auto *nativeInterface = QGuiApplication::platformNativeInterface();
        auto *wlDisplay = static_cast<wl_display*>(
            nativeInterface->nativeResourceForIntegration("display"));
        
        if (wlDisplay) {
            registry = wl_display_get_registry(wlDisplay);
            if (registry) {
                wl_registry_add_listener(registry, &s_registryListener, this);
                wl_display_roundtrip(wlDisplay);
            }
        }
    }
    
    // Wayland registry listener
    static const wl_registry_listener s_registryListener;
    
    // Registry listener 回调函数
    static void registryGlobal(void *data, wl_registry *registry, uint32_t id,
                              const char *interface, uint32_t version);
    static void registryGlobalRemove(void *data, wl_registry *registry, uint32_t id);

protected:
    // 重写协议事件处理函数
    void ext_image_copy_capture_session_v1_buffer_size(uint32_t width, uint32_t height) override {
        q_ptr->handleBufferSize(width, height);
    }

    void ext_image_copy_capture_session_v1_shm_format(uint32_t format) override {
        q_ptr->handleShmFormat(format);
    }

    void ext_image_copy_capture_session_v1_dmabuf_device(wl_array *device) override {
        QByteArray deviceData(static_cast<const char*>(device->data), device->size);
        q_ptr->handleDmabufDevice(deviceData);
    }

    void ext_image_copy_capture_session_v1_dmabuf_format(uint32_t format, wl_array *modifiers) override {
        QList<uint64_t> modifierList;
        uint64_t *modData = static_cast<uint64_t*>(modifiers->data);
        size_t count = modifiers->size / sizeof(uint64_t);
        for (size_t i = 0; i < count; ++i) {
            modifierList.append(modData[i]);
        }
        q_ptr->handleDmabufFormat(format, modifierList);
    }

    void ext_image_copy_capture_session_v1_done() override {
        q_ptr->handleDone();
    }

    void ext_image_copy_capture_session_v1_stopped() override {
        q_ptr->handleStopped();
    }
};

ExtCaptureSession::ExtCaptureSession(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

ExtCaptureSession::~ExtCaptureSession()
{
    if (d->state != Stopped && d->state != Uninitialized) {
        stop();
    }
    delete d;
}

bool ExtCaptureSession::initialize(void *manager, void *imageSource, bool paintCursors)
{
    qCWarning(dsrApp) << "ExtCaptureSession::initialize: *** INITIALIZING SESSION *** paintCursors =" << paintCursors;
    
    if (d->state != Uninitialized) {
        qWarning() << "Session already initialized";
        return false;
    }

    setState(Initializing);
    
    // 初始化 Wayland 全局对象（包括 wl_shm）
    d->initializeWaylandGlobals();

    auto *managerPrivate = static_cast<QtWayland::ext_image_copy_capture_manager_v1*>(manager);
    if (!managerPrivate) {
        qWarning() << "Invalid manager provided";
        setState(Error);
        return false;
    }

    try {
        // 创建捕获会话
        // uint32_t options = paintCursors ? EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_OPTIONS_PAINT_CURSORS : 0;
        uint32_t options = 0;
        qCWarning(dsrApp) << "ExtCaptureSession: paintCursors =" << paintCursors << "-> options =" << options << "(C enum value:" << EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_OPTIONS_PAINT_CURSORS << "Qt enum:" << QtWayland::ext_image_copy_capture_manager_v1::options_paint_cursors << ")";
        
        if (!paintCursors) {
            qCWarning(dsrApp) << "ExtCaptureSession: TESTING TreeLand options handling - using 0 (no cursors)";
        }
        
        // 使用传入的 imageSource
        auto *source = reinterpret_cast<ext_image_capture_source_v1*>(imageSource);
        if (!source) {
            qWarning() << "Invalid image sourcea provided";
            setState(Error);
            return false;
        }
        
        qCWarning(dsrApp) << "ExtCaptureSession: Creating capture session with source:" << source << "options:" << options;
        auto *session = managerPrivate->create_session(source, options);
        
        if (!session) {
            qWarning() << "Failed to create capture session";
            setState(Error);
            return false;
        }

        // 初始化协议绑定
        d->init(session);
        d->config.paintCursors = paintCursors;

        qCWarning(dsrApp) << "ExtCaptureSession: Capture session initialized, waiting for constraints...";
        qCWarning(dsrApp) << "ExtCaptureSession: Current state:" << d->state;
        return true;

    } catch (const std::exception &e) {
        qWarning() << "Exception during session initialization:" << e.what();
        setState(Error);
        return false;
    }
}

ExtCaptureSession::SessionState ExtCaptureSession::state() const
{
    return d->state;
}

const CaptureConfig& ExtCaptureSession::config() const
{
    return d->config;
}

ExtCaptureFrame* ExtCaptureSession::createFrame()
{
    if (d->state != Ready && d->state != Capturing) {
        qWarning() << "Session not ready for frame creation, current state:" << d->state;
        return nullptr;
    }

    if (d->currentFrame) {
        qWarning() << "Frame already exists, destroy previous frame first";
        return nullptr;
    }

    try {
        auto *frame = d->create_frame();
        if (!frame) {
            qWarning() << "Failed to create frame";
            return nullptr;
        }

        d->currentFrame = new ExtCaptureFrame(this);
        if (!d->currentFrame->initialize(frame, d->config)) {
            delete d->currentFrame;
            d->currentFrame = nullptr;
            return nullptr;
        }

        setState(Capturing);
        qCWarning(dsrApp) << "ExtCaptureSession::createFrame: State set to Capturing, currentFrame:" << d->currentFrame;
        
        // 连接帧信号
        connect(d->currentFrame, &ExtCaptureFrame::ready,
                this, [this]() {
                    qCWarning(dsrApp) << "ExtCaptureSession: Frame ready, emitting frameReady and resetting to Ready state";
                    
                    // 发射通用frameReady信号
                    emit frameReady(d->currentFrame);
                    
                    // 如果是DMA Buffer，同时发射dmaFrameReady信号
                    if (d->currentFrame->isDmaBuffer()) {
                        qCWarning(dsrApp) << "ExtCaptureSession: *** DMA BUFFER FRAME *** fd:" << d->currentFrame->getDmaBufferFd();
                        
                        const FrameData& frameData = d->currentFrame->frameData();
                        emit dmaFrameReady(d->currentFrame->getDmaBufferFd(), 
                                         d->currentFrame->getGbmBufferObject(),
                                         frameData.size, 
                                         frameData.dimensions.width(), 
                                         frameData.dimensions.height(),
                                         frameData.stride, 
                                         frameData.timestamp);
                    }
                    
                    // 立即清理帧，不依赖于接收方的状态
                    if (d->currentFrame) {
                        delete d->currentFrame;  // 同步删除，避免时序问题
                        d->currentFrame = nullptr;
                    }
                    setState(Ready);
                });
                
        connect(d->currentFrame, &ExtCaptureFrame::failed,
                this, [this](const QString &error) {
                    qCWarning(dsrApp) << "ExtCaptureSession: Frame failed:" << error << ", resetting to Ready state";
                    emit this->error(error);
                    if (d->currentFrame) {
                        delete d->currentFrame;  // 同步删除，避免时序问题
                        d->currentFrame = nullptr;
                    }
                    setState(Ready);
                });

        return d->currentFrame;

    } catch (const std::exception &e) {
        qWarning() << "Exception during frame creation:" << e.what();
        emit error(QString("Frame creation failed: %1").arg(e.what()));
        return nullptr;
    }
}

void ExtCaptureSession::stop()
{
    if (d->state == Stopped || d->state == Uninitialized) {
        return;
    }

    setState(Stopped);
    
    if (d->currentFrame) {
        d->currentFrame->deleteLater();
        d->currentFrame = nullptr;
    }

    if (d->isInitialized()) {
        d->destroy();
    }

    emit stopped();
}

QList<uint32_t> ExtCaptureSession::supportedFormats() const
{
    QList<uint32_t> formats = d->shmFormats;
    formats.append(d->dmabufFormats);
    return formats;
}

void ExtCaptureSession::handleBufferSize(uint32_t width, uint32_t height)
{
    d->config.bufferSize = QSize(width, height);
    qCWarning(dsrApp) << "ExtCaptureSession: Buffer size received:" << width << "x" << height;
}

void ExtCaptureSession::handleShmFormat(uint32_t format)
{
    if (!d->shmFormats.contains(format)) {
        d->shmFormats.append(format);
        qCWarning(dsrApp) << "ExtCaptureSession: SHM format supported:" << format;
    }
}

void ExtCaptureSession::handleDmabufDevice(const QByteArray &device)
{
    d->dmabufDevice = device;
    qDebug() << "DMA-BUF device set";
}

void ExtCaptureSession::handleDmabufFormat(uint32_t format, const QList<uint64_t> &modifiers)
{
    if (!d->dmabufFormats.contains(format)) {
        d->dmabufFormats.append(format);
        qDebug() << "DMA-BUF format supported:" << format << "with" << modifiers.size() << "modifiers";
    }
}

void ExtCaptureSession::handleDone()
{
    d->constraintsReceived = true;
    selectOptimalFormat();
    setState(Ready);
    emit ready();
    qCWarning(dsrApp) << "ExtCaptureSession: Session constraints received, ready for capture";
}

void ExtCaptureSession::handleStopped()
{
    setState(Stopped);
    emit stopped();
    qDebug() << "Session stopped by compositor";
}

void ExtCaptureSession::setState(SessionState newState)
{
    if (d->state != newState) {
        d->state = newState;
        qDebug() << "Session state changed to:" << newState;
    }
}

void ExtCaptureSession::selectOptimalFormat()
{
    // 优先选择DMA-BUF格式，然后是共享内存
    if (!d->dmabufFormats.isEmpty()) {
        d->config.format = d->dmabufFormats.first();
        d->config.useDmaBuffer = true;
        qCWarning(dsrApp) << "*** DMA-BUF FORMAT SELECTED ***";
        qCWarning(dsrApp) << "Selected DMA-BUF format:" << d->config.format;
        qCWarning(dsrApp) << "Available DMA-BUF formats count:" << d->dmabufFormats.size();
    } else if (!d->shmFormats.isEmpty()) {
        d->config.format = d->shmFormats.first();
        d->config.useDmaBuffer = false;
        qCWarning(dsrApp) << "*** SHM FORMAT SELECTED (DMA-BUF not available) ***";
        qCWarning(dsrApp) << "Selected SHM format:" << d->config.format;
        qCWarning(dsrApp) << "Available SHM formats count:" << d->shmFormats.size();
    } else {
        qCWarning(dsrApp) << "*** NO SUPPORTED FORMATS FOUND ***";
        setState(Error);
        emit error("No supported pixel formats");
    }
}

wl_shm* ExtCaptureSession::getWaylandShm() const
{
    return d->waylandShm;
}

void* ExtCaptureSession::getLinuxDmabuf() const
{
    return d->linuxDmabuf ? d->linuxDmabuf->object() : nullptr;
}

// Wayland registry listener 实现
void ExtCaptureSession::Private::registryGlobal(void *data, wl_registry *registry, 
                                               uint32_t id, const char *interface, uint32_t version)
{
    auto *priv = static_cast<ExtCaptureSession::Private*>(data);
    
    if (strcmp(interface, wl_shm_interface.name) == 0) {
        priv->waylandShm = static_cast<wl_shm*>(
            wl_registry_bind(registry, id, &wl_shm_interface, qMin(version, 1u)));
        qCWarning(dsrApp) << "Bound to wl_shm, version:" << version;
    } else if (strcmp(interface, zwp_linux_dmabuf_v1_interface.name) == 0) {
        priv->linuxDmabuf = new QtWayland::zwp_linux_dmabuf_v1();
        priv->linuxDmabuf->init(registry, id, qMin(version, 4u));
        qCWarning(dsrApp) << "Bound to zwp_linux_dmabuf_v1, version:" << version;
    }
}

void ExtCaptureSession::Private::registryGlobalRemove(void *data, wl_registry *registry, uint32_t id)
{
    Q_UNUSED(data)
    Q_UNUSED(registry) 
    Q_UNUSED(id)
    // 不需要处理移除事件
}

const wl_registry_listener ExtCaptureSession::Private::s_registryListener = {
    ExtCaptureSession::Private::registryGlobal,
    ExtCaptureSession::Private::registryGlobalRemove
};

