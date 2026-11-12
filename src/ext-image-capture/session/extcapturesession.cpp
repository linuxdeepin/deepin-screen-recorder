// Copyright (C) 2017-2026 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extcapturesession.h"
#include "../dmaframegpuconverter.h"
#include "../frame/extcaptureframe.h"
#include "../../protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h"
#include "../../protocols/ext-image-copy-capture/wayland-ext-image-copy-capture-v1-client-protocol.h"
#include "../../protocols/linux-dmabuf/qwayland-linux-dmabuf-unstable-v1.h"
#include "../../protocols/linux-dmabuf/wayland-linux-dmabuf-unstable-v1-client-protocol.h"

#include <QDebug>
#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <qpa/qplatformnativeinterface.h>
#include <QPointer>
#include "../utils/log.h"

// Wayland 相关
#include <wayland-client.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// DMA-BUF / GBM
#include <drm/drm_fourcc.h>
#include <gbm.h>

// Wayland 格式定义
#include <wayland-client.h>

namespace {

struct DmaBufFormatInfo {
    uint32_t format = 0;
    QList<uint64_t> modifiers;
};

QList<uint64_t> orderedModifiers(const QList<uint64_t> &modifiers)
{
    QList<uint64_t> ordered;
    if (modifiers.contains(DRM_FORMAT_MOD_LINEAR))
        ordered.append(DRM_FORMAT_MOD_LINEAR);
    for (uint64_t modifier : modifiers) {
        if (modifier != DRM_FORMAT_MOD_LINEAR)
            ordered.append(modifier);
    }
    return ordered;
}

QStringList drmDeviceCandidates(const QByteArray &deviceData)
{
    dev_t target = 0;
    const bool hasTarget = deviceData.size() >= static_cast<int>(sizeof(target));
    if (!hasTarget)
        return {};
    std::memcpy(&target, deviceData.constData(), sizeof(target));

    QString exactMatch;
    QStringList renderNodes;
    QStringList cardNodes;

    const QDir driDir(QStringLiteral("/dev/dri"));
    const QStringList entries = driDir.entryList(
            {QStringLiteral("renderD*"), QStringLiteral("card*")},
            QDir::System | QDir::Files, QDir::Name);

    for (const QString &name : entries) {
        const QString path = driDir.absoluteFilePath(name);
        struct stat st {};
        if (hasTarget && ::stat(QFile::encodeName(path).constData(), &st) == 0
                && st.st_rdev == target) {
            exactMatch = path;
        }
        if (name.startsWith(QStringLiteral("renderD")))
            renderNodes.append(path);
        else
            cardNodes.append(path);
    }

    QStringList candidates;
    if (!exactMatch.isEmpty())
        candidates.append(exactMatch);
    candidates.append(renderNodes);
    candidates.append(cardNodes);
    candidates.removeDuplicates();
    return candidates;
}

bool probeDmaBufferPlan(const QString &devicePath, uint32_t format, uint64_t modifier,
                        const QSize &size, uint64_t *selectedModifier, QString *error)
{
    if (selectedModifier)
        *selectedModifier = 0;
    if (error)
        error->clear();

    if (devicePath.isEmpty() || format == 0 || size.isEmpty()) {
        if (error)
            *error = QStringLiteral("DMA-BUF 探测参数无效");
        return false;
    }

    const int drmFd = ::open(QFile::encodeName(devicePath).constData(), O_RDWR | O_CLOEXEC);
    if (drmFd < 0) {
        if (error)
            *error = QStringLiteral("打开 DRM 节点 %1 失败：%2")
                             .arg(devicePath,
                                  QString::fromLocal8Bit(strerror(errno)));
        return false;
    }

    gbm_device *gbmDevice = gbm_create_device(drmFd);
    if (!gbmDevice) {
        if (error)
            *error = QStringLiteral("创建 GBM 设备失败：%1").arg(devicePath);
        ::close(drmFd);
        return false;
    }

    const bool implicitModifier = modifier == DRM_FORMAT_MOD_INVALID;
    gbm_bo *bo = nullptr;
    if (implicitModifier) {
        // DRM_FORMAT_MOD_INVALID 表示由驱动选择隐式布局。GBM 不接受把
        // INVALID 当作显式 modifier 传入，必须使用无 modifier 的创建接口。
        bo = gbm_bo_create(gbmDevice,
                           static_cast<uint32_t>(size.width()),
                           static_cast<uint32_t>(size.height()),
                           format, GBM_BO_USE_RENDERING);
    } else {
        uint64_t modifiers[] = {modifier};
        bo = gbm_bo_create_with_modifiers2(gbmDevice,
                                           static_cast<uint32_t>(size.width()),
                                           static_cast<uint32_t>(size.height()),
                                           format, modifiers, 1,
                                           GBM_BO_USE_RENDERING);
    }
    if (!bo) {
        if (error)
            *error = QStringLiteral("无法分配 DMA-BUF：format=0x%1 modifier=0x%2")
                             .arg(format, 0, 16)
                             .arg(modifier, 0, 16);
        gbm_device_destroy(gbmDevice);
        ::close(drmFd);
        return false;
    }

    const uint64_t actualModifier = gbm_bo_get_modifier(bo);
    const uint32_t stride = gbm_bo_get_stride_for_plane(bo, 0);
    const uint32_t offset = gbm_bo_get_offset(bo, 0);
    // 部分 Mesa/amdgpu 版本在显式请求 LINEAR 后，gbm_bo_get_modifier()
    // 仍返回 DRM_FORMAT_MOD_INVALID。此时 BO 是按 LINEAR 创建的，应该以
    // 协商请求的 modifier 为准，而不是把结果误判成不可用。
    // 隐式模式下驱动可以返回任意实际 modifier，包括仍然返回 INVALID。
    // 显式模式则必须匹配协商结果；LINEAR 被旧版 Mesa 报告成 INVALID 是
    // 已知兼容行为，此时 BO 仍按 LINEAR 分配，解码也应按 LINEAR 处理。
    const bool modifierMatches = implicitModifier
            || actualModifier == modifier
            || (actualModifier == DRM_FORMAT_MOD_INVALID
                && modifier == DRM_FORMAT_MOD_LINEAR);
    const uint64_t decodeModifier = (actualModifier == DRM_FORMAT_MOD_INVALID
                                     && !implicitModifier
                                     && modifier == DRM_FORMAT_MOD_LINEAR)
            ? DRM_FORMAT_MOD_LINEAR
            : actualModifier;
    bool usable = modifierMatches
            && gbm_bo_get_format(bo) == format
            && gbm_bo_get_plane_count(bo) >= 1
            && stride >= static_cast<uint32_t>(size.width()) * 4u;
    if (!usable && error) {
        *error = QStringLiteral("DMA-BUF 分配结果不匹配：期望 format=0x%1 modifier=0x%2，"
                                "实际 format=0x%3 modifier=0x%4 stride=%5")
                         .arg(format, 0, 16)
                         .arg(modifier, 0, 16)
                         .arg(gbm_bo_get_format(bo), 0, 16)
                         .arg(actualModifier, 0, 16)
                         .arg(stride);
    }

    int planeFd = -1;
    if (usable) {
        planeFd = gbm_bo_get_fd_for_plane(bo, 0);
        usable = planeFd >= 0;
        if (!usable && error)
            *error = QStringLiteral("导出 DMA-BUF plane fd 失败");
    }

    // 非 LINEAR modifier 不能交给 CPU 按行读取。这里用与录制线程相同的
    // EGL 导入路径做一次真实探测，避免协商阶段误选不可消费的 DMA-BUF。
    if (usable && decodeModifier != DRM_FORMAT_MOD_LINEAR) {
        DmaFrameGpuConverter converter(gbmDevice);
        QString gpuError;
        usable = converter.canImport(planeFd,
                                     static_cast<int>(size.width()),
                                     static_cast<int>(size.height()),
                                     static_cast<int>(stride), offset, format,
                                     decodeModifier, &gpuError);
        if (!usable && error) {
            *error = QStringLiteral("EGL 无法导入 DMA-BUF：format=0x%1 modifier=0x%2，%3")
                             .arg(format, 0, 16)
                             .arg(decodeModifier, 0, 16)
                             .arg(gpuError);
        }
    } else if (usable && decodeModifier == DRM_FORMAT_MOD_LINEAR) {
        // LINEAR 映射失败时仍可尝试 GPU 解布局，不能直接把整条 DMA 链路
        // 判死。只有 CPU 与 EGL 都无法消费时才由调用方回退 SHM。
        uint32_t mapStride = 0;
        void *mapHandle = nullptr;
        void *mapped = gbm_bo_map(bo, 0, 0, gbm_bo_get_width(bo), gbm_bo_get_height(bo),
                                  GBM_BO_TRANSFER_READ, &mapStride, &mapHandle);
        const bool cpuReadable = mapped
                && mapStride >= static_cast<uint32_t>(size.width()) * 4u;
        if (mapped)
            gbm_bo_unmap(bo, mapHandle);
        if (!cpuReadable) {
            DmaFrameGpuConverter converter(gbmDevice);
            QString gpuError;
            usable = converter.canImport(planeFd,
                                         static_cast<int>(size.width()),
                                         static_cast<int>(size.height()),
                                         static_cast<int>(stride), offset, format,
                                         decodeModifier, &gpuError);
            if (!usable && error) {
                *error = QStringLiteral("LINEAR DMA-BUF 的 CPU/EGL 消费均失败：%1")
                                 .arg(gpuError);
            }
        }
    }

    if (planeFd >= 0)
        ::close(planeFd);
    gbm_bo_destroy(bo);
    gbm_device_destroy(gbmDevice);
    ::close(drmFd);
    if (usable && selectedModifier)
        *selectedModifier = decodeModifier;
    return usable;
}

} // namespace

class ExtCaptureSession::Private : public QtWayland::ext_image_copy_capture_session_v1
{
public:
    Private(ExtCaptureSession *q) : q_ptr(q) {}

    ExtCaptureSession *q_ptr;
    SessionState state = Uninitialized;
    CaptureConfig config;
    
    QList<uint32_t> shmFormats;        // 共享内存格式
    QList<DmaBufFormatInfo> dmabufFormats; // DMA缓冲区格式及其 modifier 约束
    QByteArray dmabufDevice;           // composer 指定的 DMA 设备 dev_t
    
    bool constraintsReceived = false;
    bool preferDmaBuffer = true;
    ExtCaptureFrame *currentFrame = nullptr;
    
    // Wayland 全局对象
    wl_shm *waylandShm = nullptr;
    wl_registry *registry = nullptr;
    QtWayland::zwp_linux_dmabuf_v1 *linuxDmabuf = nullptr;
    
    // 初始化 Wayland 全局对象
    void initializeWaylandGlobals() {
#ifndef ENABLE_UNIT_TEST
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
#endif
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

bool ExtCaptureSession::initialize(void *manager,
                                   void *imageSource,
                                   bool paintCursors,
                                   bool preferDmaBuffer)
{
#ifdef ENABLE_UNIT_TEST
    Q_UNUSED(manager)
    Q_UNUSED(imageSource)
    Q_UNUSED(paintCursors)
    Q_UNUSED(preferDmaBuffer)
    return false;
#else
    // qCWarning(dsrApp) << "ExtCaptureSession::initialize: *** INITIALIZING SESSION *** paintCursors =" << paintCursors;
    
    if (d->state != Uninitialized) {
        qCWarning(dsrApp) << "Session already initialized";
        return false;
    }

    setState(Initializing);
    d->preferDmaBuffer = preferDmaBuffer;
    
    // 初始化 Wayland 全局对象（包括 wl_shm）
    d->initializeWaylandGlobals();

    auto *managerPrivate = static_cast<QtWayland::ext_image_copy_capture_manager_v1*>(manager);
    if (!managerPrivate) {
        qCWarning(dsrApp) << "Invalid manager provided";
        setState(Error);
        return false;
    }

    try {
        // 创建捕获会话。输出源是否真正排除光标由合成器实现决定；
        // TreeLand 0.11 的 output source 即使用 options=0 仍可能带软件光标。
        const uint32_t options = paintCursors
                ? QtWayland::ext_image_copy_capture_manager_v1::options_paint_cursors
                : 0;
        
        // 使用传入的 imageSource
        auto *source = reinterpret_cast<ext_image_capture_source_v1*>(imageSource);
        if (!source) {
            qCWarning(dsrApp) << "Invalid image sourcea provided";
            setState(Error);
            return false;
        }
        
        // qCWarning(dsrApp) << "ExtCaptureSession: Creating capture session with source:" << source << "options:" << options;
        auto *session = managerPrivate->create_session(source, options);
        
        if (!session) {
            qCWarning(dsrApp) << "Failed to create capture session";
            setState(Error);
            return false;
        }

        // 初始化协议绑定
        d->init(session);
        d->config.paintCursors = paintCursors;

        // qCWarning(dsrApp) << "ExtCaptureSession: Capture session initialized, waiting for constraints...";
        // qCWarning(dsrApp) << "ExtCaptureSession: Current state:" << d->state;
        return true;

    } catch (const std::exception &e) {
        qCWarning(dsrApp) << "Exception during session initialization:" << e.what();
        setState(Error);
        return false;
    }
#endif
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
        qCWarning(dsrApp) << "Session not ready for frame creation, current state:" << d->state;
        return nullptr;
    }

    if (d->currentFrame) {
        qCWarning(dsrApp) << "Frame already exists, destroy previous frame first";
        return nullptr;
    }

#ifdef ENABLE_UNIT_TEST
    // 测试桩：Wayland 帧创建与 DMA 信号路径不在单元测试覆盖
    return nullptr;
#else
    try {
        auto *frame = d->create_frame();
        if (!frame) {
            qCWarning(dsrApp) << "Failed to create frame";
            return nullptr;
        }

        d->currentFrame = new ExtCaptureFrame(this);
        if (!d->currentFrame->initialize(frame, d->config)) {
            delete d->currentFrame;
            d->currentFrame = nullptr;
            return nullptr;
        }

        setState(Capturing);
        // qCWarning(dsrApp) << "ExtCaptureSession::createFrame: State set to Capturing, currentFrame:" << d->currentFrame;
        
        // 连接帧信号
        connect(d->currentFrame, &ExtCaptureFrame::ready,
                this, [this]() {
                    // qCWarning(dsrApp) << "ExtCaptureSession: Frame ready, emitting frameReady and resetting to Ready state";
                    // 使用QPointer防止在信号处理过程中对象被外部删除导致悬空指针
                    QPointer<ExtCaptureFrame> frameGuard = d->currentFrame;

                    // 在发射任何信号前缓存DMA相关数据，避免在emit后解引用悬空对象
                    int dmaFd = -1;
                    void *gbmBo = nullptr;
                    size_t dataSize = 0;
                    int width = 0;
                    int height = 0;
                    int stride = 0;
                    uint64_t ts = 0;
                    bool isDma = false;
                    if (frameGuard) {
                        isDma = frameGuard->isDmaBuffer();
                        if (isDma) {
                            const FrameData &frameData = frameGuard->frameData();
                            dmaFd = frameGuard->getDmaBufferFd();
                            gbmBo = frameGuard->getGbmBufferObject();
                            dataSize = frameData.size;
                            width = frameData.dimensions.width();
                            height = frameData.dimensions.height();
                            stride = frameData.stride;
                            ts = frameData.timestamp;
                        }
                    }

                    // 若是DMA帧，先发射dmaFrameReady，避免frameReady的接收方同步删除对象后再访问
                    if (isDma) {
                        // qCWarning(dsrApp) << "ExtCaptureSession: *** DMA BUFFER FRAME *** fd:" << dmaFd;
                        emit dmaFrameReady(dmaFd, gbmBo, dataSize, width, height, stride, ts);
                    }

                    // 发射通用frameReady信号（最后发射，且不再在其后解引用frame）
                    if (frameGuard) {
                        emit frameReady(frameGuard);
                    }

                    // 安全地释放当前帧：异步删除，清空指针，避免重入期间悬空
                    if (d->currentFrame) {
                        d->currentFrame->deleteLater();
                        d->currentFrame = nullptr;
                    }
                    setState(Ready);
                });
                
        connect(d->currentFrame, &ExtCaptureFrame::failed,
                this, [this](const QString &error) {
                    // qCWarning(dsrApp) << "ExtCaptureSession: Frame failed:" << error << ", resetting to Ready state";
                    emit this->error(error);
                    if (d->currentFrame) {
                        delete d->currentFrame;  // 同步删除，避免时序问题
                        d->currentFrame = nullptr;
                    }
                    setState(Ready);
                });

        return d->currentFrame;

    } catch (const std::exception &e) {
        // qWarning() << "Exception during frame creation:" << e.what();
        emit error(QString("Frame creation failed: %1").arg(e.what()));
        return nullptr;
    }
#endif
}

void ExtCaptureSession::cancelFrame()
{
    if (!d->currentFrame) {
        if (d->state == Capturing)
            setState(Ready);
        return;
    }

    ExtCaptureFrame *frame = d->currentFrame;
    d->currentFrame = nullptr;
    disconnect(frame, nullptr, this, nullptr);
    delete frame;
    setState(Ready);

    qCWarning(dsrApp) << "ExtCaptureSession: cancelled pending capture frame";
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
    for (const DmaBufFormatInfo &info : d->dmabufFormats)
        formats.append(info.format);
    return formats;
}

void ExtCaptureSession::handleBufferSize(uint32_t width, uint32_t height)
{
    d->config.bufferSize = QSize(width, height);
    // qCWarning(dsrApp) << "ExtCaptureSession: Buffer size received:" << width << "x" << height;
}

void ExtCaptureSession::handleShmFormat(uint32_t format)
{
    if (!d->shmFormats.contains(format)) {
        d->shmFormats.append(format);
        // qCWarning(dsrApp) << "ExtCaptureSession: SHM format supported:" << format;
    }
}

void ExtCaptureSession::handleDmabufDevice(const QByteArray &device)
{
    d->dmabufDevice = device;
    qCDebug(dsrApp) << "DMA-BUF device set";
}

void ExtCaptureSession::handleDmabufFormat(uint32_t format, const QList<uint64_t> &modifiers)
{
    for (DmaBufFormatInfo &info : d->dmabufFormats) {
        if (info.format == format) {
            for (uint64_t modifier : modifiers) {
                if (!info.modifiers.contains(modifier))
                    info.modifiers.append(modifier);
            }
            return;
        }
    }

    DmaBufFormatInfo info;
    info.format = format;
    info.modifiers = modifiers;
    d->dmabufFormats.append(info);
    QStringList modifierText;
    for (uint64_t modifier : modifiers)
        modifierText.append(QStringLiteral("0x%1").arg(modifier, 0, 16));
    qCWarning(dsrApp) << "DMA-BUF format supported:"
                      << QStringLiteral("format=0x%1").arg(format, 0, 16)
                      << "modifiers:" << modifierText.join(QLatin1Char(','));
}

void ExtCaptureSession::handleDone()
{
    d->constraintsReceived = true;
    selectCapturePlan();
    if (d->state != Error) setState(Ready);
    if (d->state != Error) emit ready();
    // qCWarning(dsrApp) << "ExtCaptureSession: Session constraints received, ready for capture";
}

void ExtCaptureSession::handleStopped()
{
    setState(Stopped);
    emit stopped();
    qCDebug(dsrApp) << "Session stopped by compositor";
}

void ExtCaptureSession::setState(SessionState newState)
{
    if (d->state != newState) {
        d->state = newState;
        qCDebug(dsrApp) << "Session state changed to:" << newState;
    }
}

void ExtCaptureSession::selectCapturePlan()
{
    // DMA-BUF 是录制链路的首选 transport。这里遍历合成器广告的
    // format/modifier，使用与录制线程一致的 GBM + EGL 消费能力做一次性
    // 探测。只有所有候选都无法分配或消费时才回退 SHM。
    d->config.useDmaBuffer = false;
    d->config.dmaDevicePath.clear();
    d->config.dmaModifier = 0;
    d->config.dmaDecodeModifier = 0;
    d->config.dmaImplicitModifier = false;

    QString dmaFailure;
    if (d->preferDmaBuffer && d->dmabufFormats.isEmpty()) {
        dmaFailure = QStringLiteral("合成器未广告 DMA-BUF 格式");
    } else if (d->preferDmaBuffer) {
        static const uint32_t preferredDmaFormats[] = {
            DRM_FORMAT_XBGR8888,
            DRM_FORMAT_ABGR8888,
            DRM_FORMAT_XRGB8888,
            DRM_FORMAT_ARGB8888
        };

        // 已知的 32-bit 格式优先；其余广告格式仍交给 EGL 尝试，避免
        // 因为客户端偏好列表过窄而错误地判定“只能 SHM”。
        QList<uint32_t> formatCandidates;
        for (uint32_t format : preferredDmaFormats) {
            for (const DmaBufFormatInfo &info : d->dmabufFormats) {
                if (info.format == format && !formatCandidates.contains(format))
                    formatCandidates.append(format);
            }
        }
        for (const DmaBufFormatInfo &info : d->dmabufFormats) {
            if (!formatCandidates.contains(info.format))
                formatCandidates.append(info.format);
        }

        const QStringList devices = drmDeviceCandidates(d->dmabufDevice);
        if (devices.isEmpty()) {
            dmaFailure = QStringLiteral("合成器未提供可用的 DRM 节点");
        }

        for (uint32_t format : formatCandidates) {
            const DmaBufFormatInfo *formatInfo = nullptr;
            for (const DmaBufFormatInfo &info : d->dmabufFormats) {
                if (info.format == format) {
                    formatInfo = &info;
                    break;
                }
            }
            if (!formatInfo || formatInfo->modifiers.isEmpty())
                continue;

            for (uint64_t modifier : orderedModifiers(formatInfo->modifiers)) {
                for (const QString &device : devices) {
                    uint64_t selectedModifier = 0;
                    QString probeError;
                    if (probeDmaBufferPlan(device, format, modifier, d->config.bufferSize,
                                           &selectedModifier, &probeError)) {
                        d->config.format = format;
                        d->config.useDmaBuffer = true;
                        d->config.dmaDevicePath = device;
                        d->config.dmaModifier = modifier;
                        d->config.dmaDecodeModifier = selectedModifier;
                        d->config.dmaImplicitModifier = (modifier == DRM_FORMAT_MOD_INVALID);
                        qCWarning(dsrApp)
                                << "ExtCaptureSession: capture transport=DMA"
                                << "format=" + QStringLiteral("0x%1").arg(format, 0, 16)
                                << "alloc-modifier=" + QStringLiteral("0x%1").arg(modifier, 0, 16)
                                << "decode-modifier=" + QStringLiteral("0x%1").arg(selectedModifier, 0, 16)
                                << "implicit-modifier=" << d->config.dmaImplicitModifier
                                << "device=" + device
                                << "size=" << d->config.bufferSize;
                        return;
                    }
                    if (!probeError.isEmpty()) {
                        dmaFailure = QStringLiteral("format=0x%1 modifier=0x%2 device=%3：%4")
                                .arg(format, 0, 16)
                                .arg(modifier, 0, 16)
                                .arg(device, probeError);
                        qCDebug(dsrApp) << "ExtCaptureSession: DMA candidate rejected:"
                                        << dmaFailure;
                    }
                }
            }
        }

        if (dmaFailure.isEmpty())
            dmaFailure = QStringLiteral("没有可用的 DMA-BUF format/modifier 组合");
        qCWarning(dsrApp) << "ExtCaptureSession: capture transport=DMA unavailable,"
                          << "falling back to SHM, reason:" << dmaFailure;
    }

    if (!d->shmFormats.isEmpty()) {
        // Still-image capture deliberately uses wl_shm. DMA-BUF remains the
        // recording path, where frames are consumed as transparent handles.
        // Pick a format with a known QImage mapping instead of trusting the
        // arbitrary order in which the compositor advertised constraints.
        const uint32_t preferredFormats[] = {
            WL_SHM_FORMAT_XRGB8888,
            WL_SHM_FORMAT_ARGB8888,
            WL_SHM_FORMAT_XBGR8888,
            WL_SHM_FORMAT_ABGR8888
        };
        d->config.format = d->shmFormats.first();
        for (uint32_t format : preferredFormats) {
            if (d->shmFormats.contains(format)) {
                d->config.format = format;
                break;
            }
        }
        qCWarning(dsrApp) << "ExtCaptureSession: capture transport=SHM"
                          << "format=" + QStringLiteral("0x%1").arg(d->config.format, 0, 16)
                          << "size:" << d->config.bufferSize
                          << "reason:" << (d->preferDmaBuffer
                                                   ? dmaFailure
                                                   : QStringLiteral("调用方未请求 DMA-BUF"));
    } else {
        qCWarning(dsrApp) << "*** NO SUPPORTED FORMATS FOUND ***";
        setState(Error);
        emit error("No supported pixel formats");
    }
}

wl_shm* ExtCaptureSession::getWaylandShm() const
{
#ifdef ENABLE_UNIT_TEST
    return nullptr;
#else
    return d->waylandShm;
#endif
}

void* ExtCaptureSession::getLinuxDmabuf() const
{
#ifdef ENABLE_UNIT_TEST
    return nullptr;
#else
    return d->linuxDmabuf ? d->linuxDmabuf->object() : nullptr;
#endif
}

// Wayland registry listener 实现
void ExtCaptureSession::Private::registryGlobal(void *data, wl_registry *registry, 
                                               uint32_t id, const char *interface, uint32_t version)
{
#ifdef ENABLE_UNIT_TEST
    Q_UNUSED(data)
    Q_UNUSED(registry)
    Q_UNUSED(id)
    Q_UNUSED(interface)
    Q_UNUSED(version)
#else
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
#endif
}

void ExtCaptureSession::Private::registryGlobalRemove(void *data, wl_registry *registry, uint32_t id)
{
#ifdef ENABLE_UNIT_TEST
    Q_UNUSED(data)
    Q_UNUSED(registry)
    Q_UNUSED(id)
#else
    Q_UNUSED(data)
    Q_UNUSED(registry) 
    Q_UNUSED(id)
    // 不需要处理移除事件
#endif
}

const wl_registry_listener ExtCaptureSession::Private::s_registryListener = {
    ExtCaptureSession::Private::registryGlobal,
    ExtCaptureSession::Private::registryGlobalRemove
};
