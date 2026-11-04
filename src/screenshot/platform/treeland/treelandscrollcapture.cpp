// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandscrollcapture.h"

#include "../../../protocols/wlr-screencopy/wayland-wlr-screencopy-unstable-v1-client-protocol.h"
#include "../../../utils/log.h"
#include "treelandsnapproxy.h"

#include <QElapsedTimer>
#include <QEventLoop>
#include <QGuiApplication>
#include <QImage>
#include <QScreen>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <wayland-client.h>

namespace Dsr::Screenshot {

struct TreelandWaylandOutput
{
    wl_output *output = nullptr;
    QString name;
    bool done = false;
};

namespace {

constexpr int CaptureIntervalMs = 150;
constexpr int FrameTimeoutMs = 1000;

struct ScreencopyFrameState
{
    wl_display *display = nullptr;
    wl_shm *shm = nullptr;
    zwlr_screencopy_frame_v1 *frame = nullptr;
    wl_buffer *buffer = nullptr;
    void *data = nullptr;
    size_t dataSize = 0;
    uint32_t format = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t stride = 0;
    uint32_t flags = 0;
    uint32_t protocolVersion = 0;
    quint64 timestampNs = 0;
    bool bufferCreated = false;
    bool bufferDone = false;
    bool copySent = false;
    bool ready = false;
    bool failed = false;
    QString error;
};

QImage::Format imageFormatForShm(uint32_t format)
{
    switch (format) {
    case WL_SHM_FORMAT_ARGB8888:
        return QImage::Format_ARGB32;
    case WL_SHM_FORMAT_XRGB8888:
        return QImage::Format_RGB32;
    case WL_SHM_FORMAT_ABGR8888:
        return QImage::Format_RGBA8888;
    case WL_SHM_FORMAT_XBGR8888:
        return QImage::Format_RGBX8888;
    case WL_SHM_FORMAT_RGB565:
        return QImage::Format_RGB16;
    default:
        return QImage::Format_Invalid;
    }
}

bool createShmBuffer(ScreencopyFrameState *state)
{
    const QImage::Format imageFormat = imageFormatForShm(state->format);
    const int bytesPerPixel = imageFormat == QImage::Format_RGB16 ? 2 : 4;
    if (!state->shm || state->width == 0 || state->height == 0
            || state->stride < state->width * bytesPerPixel
            || imageFormat == QImage::Format_Invalid) {
        state->error = QStringLiteral("合成器返回了无效的共享内存帧参数");
        return false;
    }

    state->dataSize = static_cast<size_t>(state->stride) * state->height;
    const int fd = memfd_create("treeland-scroll-capture", MFD_CLOEXEC);
    if (fd < 0) {
        state->error = QStringLiteral("创建滚动截图共享内存失败：%1")
                               .arg(QString::fromLocal8Bit(std::strerror(errno)));
        return false;
    }
    if (ftruncate(fd, static_cast<off_t>(state->dataSize)) < 0) {
        state->error = QStringLiteral("调整滚动截图共享内存大小失败：%1")
                               .arg(QString::fromLocal8Bit(std::strerror(errno)));
        close(fd);
        return false;
    }

    state->data = mmap(nullptr,
                       state->dataSize,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED,
                       fd,
                       0);
    if (state->data == MAP_FAILED) {
        state->data = nullptr;
        state->error = QStringLiteral("映射滚动截图共享内存失败：%1")
                               .arg(QString::fromLocal8Bit(std::strerror(errno)));
        close(fd);
        return false;
    }

    wl_shm_pool *pool = wl_shm_create_pool(state->shm,
                                           fd,
                                           static_cast<int32_t>(state->dataSize));
    if (pool) {
        state->buffer = wl_shm_pool_create_buffer(pool,
                                                  0,
                                                  static_cast<int32_t>(state->width),
                                                  static_cast<int32_t>(state->height),
                                                  static_cast<int32_t>(state->stride),
                                                  state->format);
        wl_shm_pool_destroy(pool);
    }
    close(fd);

    if (!state->buffer) {
        state->error = QStringLiteral("创建滚动截图 wl_buffer 失败");
        munmap(state->data, state->dataSize);
        state->data = nullptr;
        state->dataSize = 0;
        return false;
    }
    return true;
}

void destroyFrameState(ScreencopyFrameState *state)
{
    if (state->frame) {
        zwlr_screencopy_frame_v1_destroy(state->frame);
        state->frame = nullptr;
    }
    if (state->buffer) {
        wl_buffer_destroy(state->buffer);
        state->buffer = nullptr;
    }
    if (state->data) {
        munmap(state->data, state->dataSize);
        state->data = nullptr;
        state->dataSize = 0;
    }
}

void handleFrameBuffer(void *data,
                       zwlr_screencopy_frame_v1 *frame,
                       uint32_t format,
                       uint32_t width,
                       uint32_t height,
                       uint32_t stride)
{
    auto *state = static_cast<ScreencopyFrameState *>(data);
    state->format = format;
    state->width = width;
    state->height = height;
    state->stride = stride;

    if (!createShmBuffer(state)) {
        state->failed = true;
        return;
    }
    state->bufferCreated = true;
    if (state->protocolVersion < 3) {
        zwlr_screencopy_frame_v1_copy(frame, state->buffer);
        state->copySent = true;
    }
}

void handleFrameFlags(void *data, zwlr_screencopy_frame_v1 *, uint32_t flags)
{
    static_cast<ScreencopyFrameState *>(data)->flags = flags;
}

void handleFrameReady(void *data,
                      zwlr_screencopy_frame_v1 *,
                      uint32_t tvSecHi,
    uint32_t tvSecLo,
    uint32_t tvNsec)
{
    auto *state = static_cast<ScreencopyFrameState *>(data);
    state->timestampNs = (static_cast<quint64>(tvSecHi) << 32 | tvSecLo)
            * 1000000000ULL + tvNsec;
    state->ready = true;
}

void handleFrameFailed(void *data, zwlr_screencopy_frame_v1 *)
{
    auto *state = static_cast<ScreencopyFrameState *>(data);
    state->failed = true;
    if (state->error.isEmpty())
        state->error = QStringLiteral("合成器未能完成滚动截图帧");
}

void handleFrameDamage(void *,
                       zwlr_screencopy_frame_v1 *,
                       uint32_t,
                       uint32_t,
                       uint32_t,
                       uint32_t)
{
}

void handleFrameLinuxDmabuf(void *,
                            zwlr_screencopy_frame_v1 *,
                            uint32_t,
                            uint32_t,
                            uint32_t)
{
}

void handleFrameBufferDone(void *data, zwlr_screencopy_frame_v1 *frame)
{
    auto *state = static_cast<ScreencopyFrameState *>(data);
    state->bufferDone = true;
    if (state->bufferCreated && !state->copySent) {
        zwlr_screencopy_frame_v1_copy(frame, state->buffer);
        state->copySent = true;
    }
}

const zwlr_screencopy_frame_v1_listener kFrameListener = {
    handleFrameBuffer,
    handleFrameFlags,
    handleFrameReady,
    handleFrameFailed,
    handleFrameDamage,
    handleFrameLinuxDmabuf,
    handleFrameBufferDone,
};

bool dispatchUntilFrameReady(ScreencopyFrameState *state,
                             int timeoutMs,
                             QString *error)
{
    wl_display *display = state->display;
    const int fd = wl_display_get_fd(display);
    QElapsedTimer elapsed;
    elapsed.start();

    while (!state->ready && !state->failed) {
        while (wl_display_prepare_read(display) != 0) {
            if (wl_display_dispatch_pending(display) < 0) {
                *error = QStringLiteral("分发滚动截图 Wayland 事件失败");
                return false;
            }
            if (state->ready || state->failed)
                break;
        }
        if (state->ready || state->failed)
            break;

        if (wl_display_flush(display) < 0 && errno != EAGAIN) {
            wl_display_cancel_read(display);
            *error = QStringLiteral("发送滚动截图 Wayland 请求失败：%1")
                             .arg(QString::fromLocal8Bit(std::strerror(errno)));
            return false;
        }

        const int remaining = timeoutMs - static_cast<int>(elapsed.elapsed());
        if (remaining <= 0) {
            wl_display_cancel_read(display);
            *error = QStringLiteral("等待滚动截图帧超时");
            return false;
        }

        pollfd pollFd { fd, POLLIN, 0 };
        const int result = poll(&pollFd, 1, remaining);
        if (result < 0) {
            wl_display_cancel_read(display);
            if (errno == EINTR)
                continue;
            *error = QStringLiteral("等待滚动截图帧失败：%1")
                             .arg(QString::fromLocal8Bit(std::strerror(errno)));
            return false;
        }
        if (result == 0) {
            wl_display_cancel_read(display);
            *error = QStringLiteral("等待滚动截图帧超时");
            return false;
        }
        if (wl_display_read_events(display) < 0) {
            *error = QStringLiteral("读取滚动截图 Wayland 事件失败");
            return false;
        }
    }

    if (state->failed) {
        *error = state->error.isEmpty()
                ? QStringLiteral("合成器未能完成滚动截图帧")
                : state->error;
        return false;
    }
    return true;
}

QImage imageFromFrameState(ScreencopyFrameState *state, QString *error)
{
    const QImage::Format format = imageFormatForShm(state->format);
    if (!state->data || state->width == 0 || state->height == 0
            || format == QImage::Format_Invalid) {
        *error = QStringLiteral("滚动截图帧格式无效");
        return {};
    }

    const QImage source(static_cast<const uchar *>(state->data),
                        static_cast<int>(state->width),
                        static_cast<int>(state->height),
                        static_cast<int>(state->stride),
                        format);
    if (source.isNull()) {
        *error = QStringLiteral("滚动截图帧数据无效");
        return {};
    }

    const int bytesPerLine = static_cast<int>(state->width) * source.depth() / 8;
    const bool inverted = state->flags & ZWLR_SCREENCOPY_FRAME_V1_FLAGS_Y_INVERT;
    QImage image(static_cast<int>(state->width),
                 static_cast<int>(state->height),
                 format);
    for (int y = 0; y < image.height(); ++y) {
        const int sourceY = inverted ? image.height() - 1 - y : y;
        std::memcpy(image.scanLine(y), source.constScanLine(sourceY), bytesPerLine);
    }
    return image;
}

} // namespace

namespace {

void handleOutputGeometry(void *,
                          wl_output *,
                          int32_t,
                          int32_t,
                          int32_t,
                          int32_t,
                          int32_t,
                          const char *,
                          const char *,
                          int32_t)
{
}

void handleOutputMode(void *, wl_output *, uint32_t, int32_t, int32_t, int32_t)
{
}

void handleOutputDone(void *data, wl_output *)
{
    static_cast<TreelandWaylandOutput *>(data)->done = true;
}

void handleOutputScale(void *, wl_output *, int32_t)
{
}

void handleOutputName(void *data, wl_output *, const char *name)
{
    static_cast<TreelandWaylandOutput *>(data)->name = QString::fromUtf8(name);
}

void handleOutputDescription(void *, wl_output *, const char *)
{
}

const wl_output_listener kOutputListener = {
    handleOutputGeometry,
    handleOutputMode,
    handleOutputDone,
    handleOutputScale,
    handleOutputName,
    handleOutputDescription,
};

const wl_registry_listener kRegistryListener = {
    TreelandScrollCapture::registryGlobal,
    TreelandScrollCapture::registryGlobalRemove,
};

} // namespace

TreelandScrollCapture::TreelandScrollCapture(QObject *parent)
    : IScrollCapture(parent)
    , m_captureTimer(new QTimer(this))
{
    m_captureTimer->setInterval(CaptureIntervalMs);
    connect(m_captureTimer, &QTimer::timeout,
            this, &TreelandScrollCapture::requestFrame);
}

void TreelandScrollCapture::setSnapSession(TreelandSnapSession *snapSession)
{
    m_snapSession = snapSession;
}

void TreelandScrollCapture::setFrozenFrames(const FrozenFrameSet &frames)
{
    m_referenceFrames = frames;
}

TreelandScrollCapture::~TreelandScrollCapture()
{
    stop();
}

bool TreelandScrollCapture::start(const QList<OutputDescriptor> &outputs,
                                  const QRect &globalSelection,
                                  QString *error)
{
    stop();
    m_stopping = false;
    m_started = false;
    m_frameInFlight = false;
    m_globalSelection = globalSelection.normalized();
    m_targetWindowRegion = m_snapSession
            ? m_snapSession->region().normalized()
            : QRect();
    m_targetProcessName = m_snapSession
            ? m_snapSession->processName().trimmed()
            : QString();

    if (m_globalSelection.isEmpty()) {
        if (error)
            *error = QStringLiteral("滚动截图选区为空");
        return false;
    }
    if (m_targetWindowRegion.isEmpty() || m_targetProcessName.isEmpty()) {
        if (error)
            *error = QStringLiteral(
                    "TreeLand 滚动截图需要选区位于一个应用窗口内");
        return false;
    }
    const QRect tolerantWindowRegion =
            m_targetWindowRegion.adjusted(-1, -1, 1, 1);
    if (!tolerantWindowRegion.contains(m_globalSelection)) {
        if (error)
            *error = QStringLiteral(
                    "滚动截图选区必须完整位于目标窗口内，不能跨窗口或包含桌面");
        return false;
    }

    QList<OutputDescriptor> matchingOutputs;
    for (const OutputDescriptor &output : outputs) {
        if (output.logicalGeometry.isValid()
                && output.logicalGeometry.intersects(m_globalSelection)) {
            matchingOutputs.append(output);
        }
    }
    if (matchingOutputs.size() != 1) {
        if (error) {
            *error = matchingOutputs.isEmpty()
                    ? QStringLiteral("滚动截图选区没有对应的输出")
                    : QStringLiteral("TreeLand 窗口滚动截图仅支持单输出");
        }
        return false;
    }

    m_captureOutput = matchingOutputs.first();
    if (!m_captureOutput.logicalGeometry.contains(m_globalSelection)) {
        if (error)
            *error = QStringLiteral("滚动截图选区超出了目标输出范围");
        return false;
    }

    m_captureScreen = nullptr;
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen && screen->name() == m_captureOutput.outputId) {
            m_captureScreen = screen;
            break;
        }
    }
    if (!m_captureScreen) {
        if (error)
            *error = QStringLiteral("找不到滚动截图输出 %1")
                             .arg(m_captureOutput.outputId);
        return false;
    }

    m_captureRegion = m_globalSelection.translated(
            -m_captureOutput.logicalGeometry.topLeft());
    m_captureRegion = m_captureRegion.intersected(
            QRect(QPoint(0, 0), m_captureOutput.logicalGeometry.size()));
    if (m_captureRegion.isEmpty()) {
        if (error)
            *error = QStringLiteral("滚动截图捕获区域为空");
        return false;
    }

    if (!initializeWayland(error))
        return false;

    m_output = findWaylandOutput(m_captureOutput.outputId);
    if (!m_output)
        m_output = findWaylandOutput(m_captureScreen->name());
    if (!m_output) {
        if (error)
            *error = QStringLiteral("无法在 Wayland 中定位滚动截图输出 %1")
                             .arg(m_captureOutput.outputId);
        cleanupWayland();
        return false;
    }

    m_started = true;
    m_captureTimer->start();
    QTimer::singleShot(0, this, &TreelandScrollCapture::requestFrame);
    return true;
}

void TreelandScrollCapture::requestFrame()
{
    if (m_stopping || !m_started || m_frameInFlight)
        return;

    m_frameInFlight = true;
    emit captureAboutToStart();
    // Give the overlay connection one event-loop turn to commit the hidden
    // preview/toolbar surfaces before this independent screencopy connection
    // asks the compositor for a frame.
    QEventLoop guardLoop;
    QTimer::singleShot(8, &guardLoop, &QEventLoop::quit);
    guardLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QImage image;
    quint64 timestampNs = 0;
    QString error;
    if (!captureFrame(&image, &timestampNs, &error)) {
        m_frameInFlight = false;
        emit captureFinished();
        emitFailure(error);
        return;
    }
    m_frameInFlight = false;
    emit captureFinished();
    if (m_stopping || image.isNull())
        return;

    ScrollFrame frame;
    frame.output = m_captureOutput;
    frame.globalSelection = m_globalSelection;
    frame.image = image;
    frame.timestampNs = timestampNs;
    emit frameReady(frame);
}

void TreelandScrollCapture::stop()
{
    m_stopping = true;
    m_started = false;
    m_frameInFlight = false;
    if (m_captureTimer)
        m_captureTimer->stop();
    cleanupWayland();
}

bool TreelandScrollCapture::initializeWayland(QString *error)
{
    cleanupWayland();
    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        if (error)
            *error = QStringLiteral("无法连接 TreeLand Wayland display");
        return false;
    }

    m_registry = wl_display_get_registry(m_display);
    if (!m_registry) {
        if (error)
            *error = QStringLiteral("无法创建滚动截图 Wayland registry");
        cleanupWayland();
        return false;
    }
    wl_registry_add_listener(m_registry, &kRegistryListener, this);

    // The first roundtrip binds the globals, the second delivers the initial
    // wl_output name/done events used to match the captured QScreen.
    for (int i = 0; i < 2; ++i) {
        if (wl_display_roundtrip(m_display) < 0) {
            if (error)
                *error = QStringLiteral("读取 TreeLand 滚动截图协议失败");
            cleanupWayland();
            return false;
        }
    }

    if (!m_shm || !m_screencopyManager) {
        if (error) {
            *error = QStringLiteral(
                    "当前 TreeLand 不支持 wlr-screencopy-unstable-v1，无法进行滚动截图");
        }
        cleanupWayland();
        return false;
    }
    return true;
}

void TreelandScrollCapture::cleanupWayland()
{
    if (m_screencopyManager) {
        zwlr_screencopy_manager_v1_destroy(m_screencopyManager);
        m_screencopyManager = nullptr;
    }
    m_screencopyVersion = 0;
    if (m_shm) {
        wl_shm_destroy(m_shm);
        m_shm = nullptr;
    }
    for (TreelandWaylandOutput *output : std::as_const(m_waylandOutputs)) {
        if (output && output->output)
            wl_output_destroy(output->output);
        delete output;
    }
    m_waylandOutputs.clear();
    m_output = nullptr;
    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }
    if (m_display) {
        wl_display_flush(m_display);
        wl_display_disconnect(m_display);
        m_display = nullptr;
    }
}

wl_output *TreelandScrollCapture::findWaylandOutput(const QString &outputId) const
{
    if (outputId.isEmpty())
        return nullptr;

    for (const TreelandWaylandOutput *entry : m_waylandOutputs) {
        if (entry && entry->output && entry->name == outputId)
            return entry->output;
    }
    if (m_waylandOutputs.size() == 1
            && m_waylandOutputs.first()->name.isEmpty()) {
        return m_waylandOutputs.first()->output;
    }
    return nullptr;
}

bool TreelandScrollCapture::captureFrame(QImage *image,
                                         quint64 *timestampNs,
                                         QString *error)
{
    if (!image || !timestampNs || !error)
        return false;
    *image = QImage();
    *timestampNs = 0;

    if (!m_screencopyManager || !m_output || !m_shm
            || !m_display || m_captureRegion.isEmpty()) {
        *error = QStringLiteral("TreeLand 滚动截图捕获环境无效");
        return false;
    }

    ScreencopyFrameState state;
    state.display = m_display;
    state.shm = m_shm;
    state.protocolVersion = m_screencopyVersion;
    // overlay_cursor is always 0: the compositor must not composite the
    // cursor into scrolling frames, otherwise every frame contributes a
    // cursor ghost to the stitched image.
    state.frame = zwlr_screencopy_manager_v1_capture_output_region(
            m_screencopyManager,
            0,
            m_output,
            m_captureRegion.x(),
            m_captureRegion.y(),
            m_captureRegion.width(),
            m_captureRegion.height());
    if (!state.frame) {
        *error = QStringLiteral("提交 TreeLand 滚动截图帧失败");
        return false;
    }
    zwlr_screencopy_frame_v1_add_listener(state.frame, &kFrameListener, &state);

    const bool captured = dispatchUntilFrameReady(&state, FrameTimeoutMs, error);
    if (!captured) {
        destroyFrameState(&state);
        wl_display_flush(m_display);
        return false;
    }

    *image = imageFromFrameState(&state, error);
    if (image->isNull()) {
        destroyFrameState(&state);
        wl_display_flush(m_display);
        return false;
    }
    *timestampNs = state.timestampNs;

    destroyFrameState(&state);
    wl_display_flush(m_display);
    return true;
}

void TreelandScrollCapture::emitFailure(const QString &reason)
{
    if (m_stopping)
        return;
    m_stopping = true;
    m_started = false;
    m_frameInFlight = false;
    if (m_captureTimer)
        m_captureTimer->stop();
    cleanupWayland();
    emit failed(reason);
}

void TreelandScrollCapture::registryGlobal(void *data,
                                           wl_registry *registry,
                                           uint32_t name,
                                           const char *interface,
                                           uint32_t version)
{
    auto *self = static_cast<TreelandScrollCapture *>(data);
    if (!self || !registry || !interface)
        return;

    if (std::strcmp(interface, wl_shm_interface.name) == 0) {
        if (!self->m_shm) {
            self->m_shm = static_cast<wl_shm *>(
                    wl_registry_bind(registry, name, &wl_shm_interface, 1));
        }
        return;
    }
    if (std::strcmp(interface, zwlr_screencopy_manager_v1_interface.name) == 0) {
        if (!self->m_screencopyManager) {
            self->m_screencopyManager = static_cast<zwlr_screencopy_manager_v1 *>(
                    wl_registry_bind(registry,
                                     name,
                                     &zwlr_screencopy_manager_v1_interface,
                                     qMin<uint32_t>(version, 3)));
            self->m_screencopyVersion = qMin<uint32_t>(version, 3);
        }
        return;
    }
    if (std::strcmp(interface, wl_output_interface.name) == 0) {
        auto *entry = new TreelandWaylandOutput;
        entry->output = static_cast<wl_output *>(
                wl_registry_bind(registry,
                                 name,
                                 &wl_output_interface,
                                 qMin<uint32_t>(version, 4)));
        if (!entry->output) {
            delete entry;
            return;
        }
        self->m_waylandOutputs.append(entry);
        wl_output_add_listener(entry->output, &kOutputListener, entry);
    }
}

void TreelandScrollCapture::registryGlobalRemove(void *,
                                                 wl_registry *,
                                                 uint32_t)
{
}

} // namespace Dsr::Screenshot
