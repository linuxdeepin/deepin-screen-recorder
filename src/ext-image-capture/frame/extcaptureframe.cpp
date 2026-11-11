// Copyright (C) 2017-2026 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extcaptureframe.h"
#include "../session/extcapturesession.h"
#include "../../protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h"

#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <cstring>
#include "../utils/log.h"

// Wayland 相关
#include <wayland-client.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <drm/drm_fourcc.h>

// DMA-BUF相关
#include <gbm.h>
#include <sys/ioctl.h>
#include "../../protocols/ext-image-copy-capture/wayland-ext-image-copy-capture-v1-client-protocol.h"
#include "../../protocols/linux-dmabuf/wayland-linux-dmabuf-unstable-v1-client-protocol.h"

class ExtCaptureFrame::Private : public QtWayland::ext_image_copy_capture_frame_v1
{
public:
    Private(ExtCaptureFrame *q, ExtCaptureSession *session) : q_ptr(q), session(session) {}

    ExtCaptureFrame *q_ptr;
    ExtCaptureSession *session;
    FrameState state = Uninitialized;
    CaptureConfig config;
    FrameData frameData;
    
    wl_buffer *buffer = nullptr;
    int bufferFd = -1;
    void *mappedData = nullptr;
    size_t bufferSize = 0;
    
    bool bufferMapped = false;
    
    // DMA-BUF相关
    struct gbm_bo *bo = nullptr;
    struct gbm_device *gbmDevice = nullptr;

protected:
    // 重写协议事件处理函数
    void ext_image_copy_capture_frame_v1_transform(uint32_t transform) override {
        q_ptr->handleTransform(transform);
    }

    void ext_image_copy_capture_frame_v1_damage(int32_t x, int32_t y, int32_t width, int32_t height) override {
        q_ptr->handleDamage(x, y, width, height);
    }

    void ext_image_copy_capture_frame_v1_presentation_time(uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec) override {
        q_ptr->handlePresentationTime(tv_sec_hi, tv_sec_lo, tv_nsec);
    }

    void ext_image_copy_capture_frame_v1_ready() override final {
        // qCWarning(dsrApp) << "Private::ext_image_copy_capture_frame_v1_ready: *** VIRTUAL FUNCTION CALLED *** this=" << this << "q_ptr=" << q_ptr;
        if (q_ptr) {
            q_ptr->handleReady();
        } else {
            // qCCritical(dsrApp) << "Private::ext_image_copy_capture_frame_v1_ready: q_ptr is NULL!";
        }
    }

    void ext_image_copy_capture_frame_v1_failed(uint32_t reason) override final {
        // qCWarning(dsrApp) << "Private::ext_image_copy_capture_frame_v1_failed: *** VIRTUAL FUNCTION CALLED *** reason:" << reason << "this=" << this << "q_ptr=" << q_ptr;
        if (q_ptr) {
            q_ptr->handleFailed(reason);
        } else {
            // qCCritical(dsrApp) << "Private::ext_image_copy_capture_frame_v1_failed: q_ptr is NULL!";
        }
    }
};

ExtCaptureFrame::ExtCaptureFrame(QObject *parent)
    : QObject(parent)
    , d(new Private(this, qobject_cast<ExtCaptureSession*>(parent)))
{
}

ExtCaptureFrame::~ExtCaptureFrame()
{
    // 清理DMA-BUF或SHM资源
    if (d->config.useDmaBuffer) {
        cleanupDmaBuffer();
    } else {
        if (d->mappedData && d->bufferFd >= 0) {
            munmap(d->mappedData, d->bufferSize);
        }
        
        if (d->bufferFd >= 0) {
            close(d->bufferFd);
        }
    }
    
    if (d->buffer) {
        wl_buffer_destroy(d->buffer);
    }
    
    if (d->isInitialized()) {
        d->destroy();
    }
    
    delete d;
}

bool ExtCaptureFrame::initialize(void *frame, const CaptureConfig &config)
{
    if (d->state != Uninitialized) {
        qCWarning(dsrApp) << "Frame already initialized";
        return false;
    }

    auto *frameObj = static_cast<ext_image_copy_capture_frame_v1*>(frame);
    if (!frameObj) {
        qCWarning(dsrApp) << "Invalid frame object";
        return false;
    }

#ifndef ENABLE_UNIT_TEST
    d->config = config;
    d->frameData.dimensions = config.bufferSize;
    d->frameData.format = config.format;

    // 初始化协议绑定
    d->init(frameObj);
    // qCWarning(dsrApp) << "ExtCaptureFrame::initialize: Protocol binding initialized, listener registered for frame:" << frameObj;

    // qCWarning(dsrApp) << "ExtCaptureFrame::initialize: Qt Wayland listener already registered";

    // 创建缓冲区
    if (!createBuffer()) {
        qCWarning(dsrApp) << "Failed to create buffer";
        return false;
    }

    setState(Attached);
    // qDebug() << "Frame initialized with size:" << config.bufferSize;
    return true;
#else
    Q_UNUSED(config)
    return false; // 单测桩：Wayland 协议绑定 + createBuffer 需真实 compositor，跳过
#endif
}

ExtCaptureFrame::FrameState ExtCaptureFrame::state() const
{
    return d->state;
}

bool ExtCaptureFrame::capture(bool fullDamage)
{
    // qCWarning(dsrApp) << "ExtCaptureFrame::capture: Starting capture, current state:" << d->state;
    
    if (d->state < Attached) {
        // qCWarning(dsrApp) << "ExtCaptureFrame::capture: Frame not ready for capture, state:" << d->state;
        return false;
    }

#ifndef ENABLE_UNIT_TEST
    try {
        // 附加缓冲区
        // qCWarning(dsrApp) << "ExtCaptureFrame::capture: Attaching buffer:" << d->buffer;
        d->attach_buffer(d->buffer);

        // 设置损坏区域
        if (fullDamage) {
            d->damage_buffer(0, 0, d->config.bufferSize.width(), d->config.bufferSize.height());
        } else {
            // 如果有特定的损坏区域，可以在这里设置
            // 目前简单起见，总是使用全损坏
            d->damage_buffer(0, 0, d->config.bufferSize.width(), d->config.bufferSize.height());
        }

        setState(Damaged);
        // qCWarning(dsrApp) << "ExtCaptureFrame::capture: State set to Damaged, starting capture...";

        // 开始捕获
        d->capture();
        setState(Capturing);

    // 确保 Wayland 事件被处理
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    if (native) {
        struct wl_display *display = static_cast<struct wl_display *>(
            native->nativeResourceForWindow("display", nullptr));
        if (display) {
            // qCWarning(dsrApp) << "ExtCaptureFrame::capture: Flushing Wayland display...";
            wl_display_flush(display);

        // 强制处理待处理的事件
        // qCWarning(dsrApp) << "ExtCaptureFrame::capture: Dispatching pending events...";
        // 直接处理待处理的事件，不阻塞
        wl_display_dispatch_pending(display);
        }
    }

        // qCWarning(dsrApp) << "ExtCaptureFrame::capture: Frame capture started successfully, state set to Capturing";
        return true;

    } catch (const std::exception &e) {
        // qCWarning(dsrApp) << "ExtCaptureFrame::capture: Exception during capture:" << e.what();
        setState(Failed);
        emit failed(QString("Capture failed: %1").arg(e.what()));
        return false;
    }
#else
    Q_UNUSED(fullDamage)
    return false; // 单测桩：Wayland attach_buffer/capture 需真实 compositor，跳过
#endif
}

const FrameData& ExtCaptureFrame::frameData() const
{
    return d->frameData;
}

void* ExtCaptureFrame::mapBuffer()
{
    if (!d->mappedData || d->bufferMapped) {
        return d->mappedData;
    }
    
    d->bufferMapped = true;
    d->frameData.data = d->mappedData;
    d->frameData.size = d->bufferSize;
    
    return d->mappedData;
}

void ExtCaptureFrame::unmapBuffer()
{
    if (d->bufferMapped) {
        d->bufferMapped = false;
        d->frameData.data = nullptr;
    }
}

size_t ExtCaptureFrame::copyFrameData(void *dest, size_t maxSize) const
{
    if (!d->mappedData || d->state != Ready) {
        return 0;
    }
    
    size_t copySize = std::min(d->bufferSize, maxSize);
    std::memcpy(dest, d->mappedData, copySize);
    return copySize;
}

QSize ExtCaptureFrame::size() const
{
    return d->frameData.dimensions;
}

uint32_t ExtCaptureFrame::pixelFormat() const
{
    return d->frameData.format;
}

uint64_t ExtCaptureFrame::timestamp() const
{
    return d->frameData.timestamp;
}

bool ExtCaptureFrame::isDmaBuffer() const
{
    return d->config.useDmaBuffer;
}

int ExtCaptureFrame::getDmaBufferFd() const
{
    return d->config.useDmaBuffer ? d->bufferFd : -1;
}

void* ExtCaptureFrame::getGbmBufferObject() const
{
    return d->config.useDmaBuffer ? d->bo : nullptr;
}

void ExtCaptureFrame::handleTransform(uint32_t transform)
{
    d->frameData.transform = static_cast<FrameTransform>(transform);
    qCDebug(dsrApp) << "Frame transform:" << transform;
}

void ExtCaptureFrame::handleDamage(int32_t x, int32_t y, int32_t width, int32_t height)
{
    QRect damageRect(x, y, width, height);
    d->frameData.damageRegions.append(damageRect);
    qCDebug(dsrApp) << "Frame damage:" << damageRect;
}

void ExtCaptureFrame::handlePresentationTime(uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec)
{
    // 组合时间戳
    uint64_t seconds = (static_cast<uint64_t>(tv_sec_hi) << 32) | tv_sec_lo;
    d->frameData.timestamp = seconds * 1000000000ULL + tv_nsec;
    qCDebug(dsrApp) << "Frame presentation time:" << d->frameData.timestamp;
}

void ExtCaptureFrame::handleReady()
{
    // qCWarning(dsrApp) << "ExtCaptureFrame::handleReady: *** FRAME READY EVENT RECEIVED ***";
    setState(Ready);
    
    // 映射缓冲区数据
    mapBuffer();
    
    // qCWarning(dsrApp) << "ExtCaptureFrame::handleReady: Frame ready, size:" << d->bufferSize << ", emitting ready signal";
    emit ready();
}

void ExtCaptureFrame::handleFailed(uint32_t reason)
{
    // qCWarning(dsrApp) << "ExtCaptureFrame::handleFailed: *** FRAME FAILED EVENT RECEIVED *** reason:" << reason;
    setState(Failed);
    QString errorMsg = failureReasonToString(reason);
    // qCWarning(dsrApp) << "ExtCaptureFrame::handleFailed: Frame capture failed:" << errorMsg;
    emit failed(errorMsg);
}

bool ExtCaptureFrame::createBuffer()
{
#ifdef ENABLE_UNIT_TEST
    return false; // 单测桩：Wayland/SHM 缓冲区需真实 Wayland，跳过
#else
    // 根据配置选择缓冲区类型
    if (d->config.useDmaBuffer) {
        // qCWarning(dsrApp) << "ExtCaptureFrame: Creating DMA Buffer";
        return createDmaBuffer();
    } else {
        // qCWarning(dsrApp) << "ExtCaptureFrame: Creating SHM Buffer";
        return createShmBuffer();
    }
#endif
}

bool ExtCaptureFrame::createShmBuffer()
{
#ifdef ENABLE_UNIT_TEST
    return false; // 单测桩：SHM 缓冲区需真实 Wayland，跳过
#else
    // createBuffer() calls this as a fallback from DMA-BUF. Keep the frame's
    // advertised buffer type in sync so consumers take the SHM path.
    d->config.useDmaBuffer = false;

    // 计算缓冲区大小（假设RGBA格式，4字节/像素）
    uint32_t width = d->config.bufferSize.width();
    uint32_t height = d->config.bufferSize.height();
    uint32_t stride = width * 4;  // RGBA
    d->bufferSize = stride * height;
    
    d->frameData.stride = stride;
    
    // 创建共享内存文件
    d->bufferFd = memfd_create("ext-capture-buffer", MFD_CLOEXEC);
    if (d->bufferFd < 0) {
        qCWarning(dsrApp) << "Failed to create memfd";
        return false;
    }
    
    if (ftruncate(d->bufferFd, d->bufferSize) < 0) {
        qCWarning(dsrApp) << "Failed to resize buffer";
        close(d->bufferFd);
        d->bufferFd = -1;
        return false;
    }
    
    // 映射内存
    d->mappedData = mmap(nullptr, d->bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, d->bufferFd, 0);
    if (d->mappedData == MAP_FAILED) {
        qCWarning(dsrApp) << "Failed to map buffer";
        close(d->bufferFd);
        d->bufferFd = -1;
        return false;
    }
    
    // 使用标准的 Wayland 方式获取 wl_shm
    auto *nativeInterface = QGuiApplication::platformNativeInterface();
    auto *wlDisplay = static_cast<wl_display*>(
        nativeInterface->nativeResourceForIntegration("display"));
    
    if (!wlDisplay) {
        qCWarning(dsrApp) << "Failed to get Wayland display";
        return false;
    }
    
    // 通过 session 获取全局 wl_shm 对象 (session 应该已经绑定了全局对象)
    if (!d->session) {
        qCWarning(dsrApp) << "Session is null, cannot get shm";
        return false;
    }
    
    auto *shm = d->session->getWaylandShm();
    if (!shm) {
        qCWarning(dsrApp) << "Failed to get Wayland shm from session";
        return false;
    }
    
    auto *pool = wl_shm_create_pool(shm, d->bufferFd, d->bufferSize);
    if (!pool) {
        qCWarning(dsrApp) << "Failed to create shm pool";
        return false;
    }
    
    d->buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, d->config.format);
    wl_shm_pool_destroy(pool);
    
    if (!d->buffer) {
        qCWarning(dsrApp) << "Failed to create buffer";
        return false;
    }
    
    qCDebug(dsrApp) << "Buffer created:" << width << "x" << height << "stride:" << stride << "size:" << d->bufferSize;
    return true;
#endif
}

void ExtCaptureFrame::setState(FrameState newState)
{
    if (d->state != newState) {
        d->state = newState;
        qCDebug(dsrApp) << "Frame state changed to:" << newState;
    }
}

bool ExtCaptureFrame::createDmaBuffer()
{
#ifdef ENABLE_UNIT_TEST
    return false; // 单测桩：DRM/GBM/DMA-BUF 需真实设备，跳过
#else
    const uint32_t width = static_cast<uint32_t>(d->config.bufferSize.width());
    const uint32_t height = static_cast<uint32_t>(d->config.bufferSize.height());
    const uint32_t format = d->config.format;

    if (d->config.dmaDevicePath.isEmpty() || format == 0 || width == 0 || height == 0) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: incomplete DMA plan";
        return false;
    }

    // 节点、格式和 LINEAR modifier 已由 ExtCaptureSession 在约束阶段探测，
    // 这里只执行方案，不再逐帧尝试其他节点或临时回退 SHM。
    const int drmFd = open(QFile::encodeName(d->config.dmaDevicePath).constData(),
                           O_RDWR | O_CLOEXEC);
    if (drmFd < 0) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: cannot open DRM node"
                          << d->config.dmaDevicePath << "errno:" << errno;
        return false;
    }

    d->gbmDevice = gbm_create_device(drmFd);
    if (!d->gbmDevice) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: gbm_create_device failed";
        close(drmFd);
        return false;
    }

    // 录制链路按 LINEAR 行布局读取 DMA-BUF。必须显式请求 LINEAR，并在
    // 分配后检查驱动实际返回的 modifier；DMA-BUF 本身不等于 LINEAR。
    uint64_t modifiers[] = { DRM_FORMAT_MOD_LINEAR };
    d->bo = gbm_bo_create_with_modifiers2(d->gbmDevice, width, height, format,
                                          modifiers, 1, GBM_BO_USE_RENDERING);

    if (!d->bo) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: failed to allocate LINEAR GBM BO";
        cleanupDmaBuffer();
        close(drmFd);
        return false;
    }

    const uint64_t modifier = gbm_bo_get_modifier(d->bo);
    if (modifier != DRM_FORMAT_MOD_LINEAR || modifier != d->config.dmaModifier) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: driver returned non-LINEAR BO"
                          << "modifier:" << Qt::hex << modifier;
        cleanupDmaBuffer();
        close(drmFd);
        return false;
    }

    if (gbm_bo_get_format(d->bo) != format || gbm_bo_get_plane_count(d->bo) < 1) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: BO format/plane mismatch";
        cleanupDmaBuffer();
        close(drmFd);
        return false;
    }

    const uint32_t stride = gbm_bo_get_stride_for_plane(d->bo, 0);
    const uint32_t offset = gbm_bo_get_offset(d->bo, 0);
    const int planeFd = gbm_bo_get_fd_for_plane(d->bo, 0);
    if (planeFd < 0) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: failed to export plane fd";
        cleanupDmaBuffer();
        close(drmFd);
        return false;
    }

    d->frameData.stride = stride;
    d->bufferSize = stride * height;
    d->frameData.size = d->bufferSize;
    d->bufferFd = planeFd;

    qCDebug(dsrApp) << "ExtCaptureFrame: DMA-LINEAR BO created"
                    << "device:" << d->config.dmaDevicePath
                    << "stride:" << stride
                    << "offset:" << offset
                    << "modifier:" << Qt::hex << modifier
                    << "fd:" << planeFd;

    if (!d->session) {
        cleanupDmaBuffer();
        close(drmFd);
        return false;
    }

    auto *linuxDmabuf = static_cast<struct zwp_linux_dmabuf_v1*>(d->session->getLinuxDmabuf());
    if (!linuxDmabuf) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: linux-dmabuf unavailable";
        cleanupDmaBuffer();
        close(drmFd);
        return false;
    }

    auto *params = zwp_linux_dmabuf_v1_create_params(linuxDmabuf);
    if (!params) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: create_params failed";
        cleanupDmaBuffer();
        close(drmFd);
        return false;
    }

    zwp_linux_buffer_params_v1_add(params, planeFd, 0, offset, stride,
                                   modifier >> 32, modifier & 0xffffffff);

    d->buffer = zwp_linux_buffer_params_v1_create_immed(params, width, height, format, 0);
    zwp_linux_buffer_params_v1_destroy(params);

    if (!d->buffer) {
        qCWarning(dsrApp) << "ExtCaptureFrame::createDmaBuffer: create_immed failed";
        cleanupDmaBuffer();
        close(drmFd);
        return false;
    }

    // GBM 设备持有自身 fd 的引用，创建完成后可以关闭本地打开的描述符。
    close(drmFd);

    return true;
#endif
}

void ExtCaptureFrame::cleanupDmaBuffer()
{
#ifndef ENABLE_UNIT_TEST
    if (d->mappedData && d->mappedData != MAP_FAILED) {
        munmap(d->mappedData, d->bufferSize);
        d->mappedData = nullptr;
    }
    
    if (d->bufferFd >= 0) {
        close(d->bufferFd);
        d->bufferFd = -1;
    }
    
    if (d->bo) {
        gbm_bo_destroy(d->bo);
        d->bo = nullptr;
    }
    
    if (d->gbmDevice) {
        gbm_device_destroy(d->gbmDevice);
        d->gbmDevice = nullptr;
    }
#endif
}

QString ExtCaptureFrame::failureReasonToString(uint32_t reason)
{
    switch (reason) {
    case 0:  // failure_reason_unknown
        return "Unknown error";
    case 1:  // failure_reason_buffer_constraints
        return "Buffer constraints mismatch";
    case 2:  // failure_reason_stopped
        return "Session stopped";
    default:
        return QString("Unknown failure reason: %1").arg(reason);
    }
}
