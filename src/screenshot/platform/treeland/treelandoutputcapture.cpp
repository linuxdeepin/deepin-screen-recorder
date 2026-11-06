// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandoutputcapture.h"

#include "../../../protocols/ext-image-copy-capture/wayland-ext-image-capture-source-v1-client-protocol.h"
#include "../../../protocols/ext-image-copy-capture/wayland-ext-image-copy-capture-v1-client-protocol.h"
#include "../../../utils/log.h"

#include <QElapsedTimer>
#include <QList>
#include <QSet>
#include <QTransform>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <wayland-client.h>

namespace Dsr::Screenshot {

namespace {

constexpr int ProtocolTimeoutMs = 1500;
// A fresh capture session always copies its first frame immediately. That
// normally completes in a few milliseconds on TreeLand, so this timeout only
// catches a compositor/protocol failure rather than a slow path.
constexpr int FrameTimeoutMs = 1500;

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
    default:
        return QImage::Format_Invalid;
    }
}

QImage applyTransform(const QImage &image, uint32_t transform)
{
    switch (transform) {
    case WL_OUTPUT_TRANSFORM_90:
        return image.transformed(QTransform().rotate(90));
    case WL_OUTPUT_TRANSFORM_180:
        return image.transformed(QTransform().rotate(180));
    case WL_OUTPUT_TRANSFORM_270:
        return image.transformed(QTransform().rotate(270));
    case WL_OUTPUT_TRANSFORM_FLIPPED:
        return image.mirrored(true, false);
    case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        return image.transformed(QTransform().rotate(90)).mirrored(true, false);
    case WL_OUTPUT_TRANSFORM_FLIPPED_180:
        return image.mirrored(true, false).mirrored(false, true);
    case WL_OUTPUT_TRANSFORM_FLIPPED_270:
        return image.transformed(QTransform().rotate(270)).mirrored(true, false);
    default:
        return image;
    }
}

bool dispatchUntil(wl_display *display,
                   const std::function<bool()> &condition,
                   int timeoutMs,
                   QString *error)
{
    if (!display) {
        if (error)
            *error = QStringLiteral("Wayland display 不可用");
        return false;
    }

    const int fd = wl_display_get_fd(display);
    QElapsedTimer elapsed;
    elapsed.start();

    while (!condition()) {
        while (wl_display_prepare_read(display) != 0) {
            if (wl_display_dispatch_pending(display) < 0) {
                if (error)
                    *error = QStringLiteral("分发 Wayland 事件失败");
                return false;
            }
            if (condition())
                return true;
        }
        if (condition()) {
            wl_display_cancel_read(display);
            return true;
        }

        if (wl_display_flush(display) < 0 && errno != EAGAIN) {
            wl_display_cancel_read(display);
            if (error) {
                *error = QStringLiteral("发送 Wayland 请求失败：%1")
                                 .arg(QString::fromLocal8Bit(std::strerror(errno)));
            }
            return false;
        }

        const int remaining = timeoutMs - static_cast<int>(elapsed.elapsed());
        if (remaining <= 0) {
            wl_display_cancel_read(display);
            if (error)
                *error = QStringLiteral("等待 Wayland 事件超时");
            return false;
        }

        pollfd pollFd { fd, POLLIN, 0 };
        const int result = poll(&pollFd, 1, remaining);
        if (result < 0) {
            wl_display_cancel_read(display);
            if (errno == EINTR)
                continue;
            if (error) {
                *error = QStringLiteral("等待 Wayland 事件失败：%1")
                                 .arg(QString::fromLocal8Bit(std::strerror(errno)));
            }
            return false;
        }
        if (result == 0) {
            wl_display_cancel_read(display);
            if (error)
                *error = QStringLiteral("等待 Wayland 事件超时");
            return false;
        }
        if (wl_display_read_events(display) < 0) {
            if (error)
                *error = QStringLiteral("读取 Wayland 事件失败");
            return false;
        }
    }
    return true;
}

} // namespace

class TreelandOutputCapture::Private
{
public:
    struct OutputInfo
    {
        wl_output *output = nullptr;
        QString name;
        QRect geometry;
        bool done = false;
    };

    struct Session
    {
        ext_image_capture_source_v1 *source = nullptr;
        ext_image_copy_capture_session_v1 *session = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t format = 0;
        QSet<uint32_t> shmFormats;
        int constraintVersion = 0;
        bool ready = false;
        bool stopped = false;
    };

    struct Frame
    {
        ext_image_copy_capture_frame_v1 *frame = nullptr;
        wl_buffer *buffer = nullptr;
        wl_shm_pool *pool = nullptr;
        void *data = nullptr;
        size_t size = 0;
        int fd = -1;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t stride = 0;
        uint32_t format = 0;
        uint32_t transform = WL_OUTPUT_TRANSFORM_NORMAL;
        quint64 timestampNs = 0;
        bool ready = false;
        bool failed = false;
        uint32_t failureReason = 0;
    };

    explicit Private(TreelandOutputCapture *q)
        : q(q)
    {
    }

    bool initialize(QString *error);
    bool selectOutput(const QString &outputName, const QRect &outputGeometry, QString *error);
    Session *createSession(QString *error);
    void destroySession(Session *session);
    bool waitForConstraints(Session *session, QString *error);
    bool selectShmFormat(Session *session, QString *error);
    bool captureFromSession(Session *session,
                            QImage *image,
                            quint64 *timestampNs,
                            QString *error);
    void stop();

    static void handleOutputGeometry(void *data,
                                     wl_output *output,
                                     int32_t x,
                                     int32_t y,
                                     int32_t physicalWidth,
                                     int32_t physicalHeight,
                                     int32_t subpixel,
                                     const char *make,
                                     const char *model,
                                     int32_t transform);
    static void handleOutputMode(void *data,
                                 wl_output *output,
                                 uint32_t flags,
                                 int32_t width,
                                 int32_t height,
                                 int32_t refresh);
    static void handleOutputDone(void *data, wl_output *output);
    static void handleOutputScale(void *data, wl_output *output, int32_t factor);
    static void handleOutputName(void *data, wl_output *output, const char *name);
    static void handleOutputDescription(void *data, wl_output *output, const char *description);

    static void handleSessionBufferSize(void *data,
                                        ext_image_copy_capture_session_v1 *session,
                                        uint32_t width,
                                        uint32_t height);
    static void handleSessionShmFormat(void *data,
                                       ext_image_copy_capture_session_v1 *session,
                                       uint32_t format);
    static void handleSessionDmabufDevice(void *data,
                                          ext_image_copy_capture_session_v1 *session,
                                          wl_array *device);
    static void handleSessionDmabufFormat(void *data,
                                           ext_image_copy_capture_session_v1 *session,
                                           uint32_t format,
                                           wl_array *modifiers);
    static void handleSessionDone(void *data, ext_image_copy_capture_session_v1 *session);
    static void handleSessionStopped(void *data, ext_image_copy_capture_session_v1 *session);
    static void handleFrameTransform(void *data,
                                     ext_image_copy_capture_frame_v1 *frame,
                                     uint32_t transform);
    static void handleFrameDamage(void *data,
                                  ext_image_copy_capture_frame_v1 *frame,
                                  int32_t x,
                                  int32_t y,
                                  int32_t width,
                                  int32_t height);
    static void handleFramePresentationTime(void *data,
                                            ext_image_copy_capture_frame_v1 *frame,
                                            uint32_t tvSecHi,
                                            uint32_t tvSecLo,
                                            uint32_t tvNsec);
    static void handleFrameReady(void *data, ext_image_copy_capture_frame_v1 *frame);
    static void handleFrameFailed(void *data,
                                  ext_image_copy_capture_frame_v1 *frame,
                                  uint32_t reason);
    static void registryGlobal(void *data,
                               wl_registry *registry,
                               uint32_t name,
                               const char *interface,
                               uint32_t version);
    static void registryGlobalRemove(void *data, wl_registry *registry, uint32_t name);

    TreelandOutputCapture *q = nullptr;
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_shm *shm = nullptr;
    ext_output_image_capture_source_manager_v1 *sourceManager = nullptr;
    ext_image_copy_capture_manager_v1 *captureManager = nullptr;
    QList<OutputInfo> outputs;
    OutputInfo *selectedOutput = nullptr;
};

const wl_output_listener kOutputListener = {
    TreelandOutputCapture::Private::handleOutputGeometry,
    TreelandOutputCapture::Private::handleOutputMode,
    TreelandOutputCapture::Private::handleOutputDone,
    TreelandOutputCapture::Private::handleOutputScale,
    TreelandOutputCapture::Private::handleOutputName,
    TreelandOutputCapture::Private::handleOutputDescription,
};

const ext_image_copy_capture_session_v1_listener kSessionListener = {
    TreelandOutputCapture::Private::handleSessionBufferSize,
    TreelandOutputCapture::Private::handleSessionShmFormat,
    TreelandOutputCapture::Private::handleSessionDmabufDevice,
    TreelandOutputCapture::Private::handleSessionDmabufFormat,
    TreelandOutputCapture::Private::handleSessionDone,
    TreelandOutputCapture::Private::handleSessionStopped,
};

const ext_image_copy_capture_frame_v1_listener kFrameListener = {
    TreelandOutputCapture::Private::handleFrameTransform,
    TreelandOutputCapture::Private::handleFrameDamage,
    TreelandOutputCapture::Private::handleFramePresentationTime,
    TreelandOutputCapture::Private::handleFrameReady,
    TreelandOutputCapture::Private::handleFrameFailed,
};

const wl_registry_listener kRegistryListener = {
    TreelandOutputCapture::Private::registryGlobal,
    TreelandOutputCapture::Private::registryGlobalRemove,
};

bool TreelandOutputCapture::Private::initialize(QString *error)
{
    stop();
    display = wl_display_connect(nullptr);
    if (!display) {
        if (error)
            *error = QStringLiteral("无法连接 TreeLand Wayland display");
        return false;
    }

    registry = wl_display_get_registry(display);
    if (!registry) {
        if (error)
            *error = QStringLiteral("无法创建 TreeLand registry");
        stop();
        return false;
    }
    wl_registry_add_listener(registry, &kRegistryListener, this);

    for (int i = 0; i < 3; ++i) {
        if (wl_display_roundtrip(display) < 0) {
            if (error)
                *error = QStringLiteral("读取 TreeLand 输出截图协议失败");
            stop();
            return false;
        }
    }

    if (!shm || !sourceManager || !captureManager || outputs.isEmpty()) {
        QStringList missing;
        if (!shm)
            missing << QStringLiteral("wl_shm");
        if (!sourceManager)
            missing << QStringLiteral("ext_output_image_capture_source_manager_v1");
        if (!captureManager)
            missing << QStringLiteral("ext_image_copy_capture_manager_v1");
        if (outputs.isEmpty())
            missing << QStringLiteral("wl_output");
        if (error) {
            *error = QStringLiteral("当前 TreeLand 不支持输出截图：%1")
                             .arg(missing.join(QStringLiteral(", ")));
        }
        stop();
        return false;
    }

    qCInfo(dsrApp) << "TreeLand output capture: protocols ready, outputs"
                   << outputs.size();
    return true;
}

bool TreelandOutputCapture::Private::selectOutput(const QString &outputName,
                                                   const QRect &outputGeometry,
                                                   QString *error)
{
    selectedOutput = nullptr;

    for (OutputInfo &output : outputs) {
        if (!outputName.isEmpty() && output.name == outputName) {
            selectedOutput = &output;
            break;
        }
    }
    if (!selectedOutput && outputGeometry.isValid()) {
        for (OutputInfo &output : outputs) {
            if (output.geometry == outputGeometry) {
                selectedOutput = &output;
                break;
            }
        }
    }
    if (!selectedOutput && outputs.size() == 1)
        selectedOutput = &outputs.first();

    if (!selectedOutput) {
        if (error) {
            QStringList names;
            for (const OutputInfo &output : std::as_const(outputs)) {
                names << (output.name.isEmpty()
                                  ? QStringLiteral("<未命名>")
                                  : output.name);
            }
            *error = QStringLiteral("找不到输出 %1（可用输出：%2）")
                             .arg(outputName.isEmpty() ? QStringLiteral("<未命名>") : outputName,
                                  names.join(QStringLiteral(", ")));
        }
        return false;
    }

    qCInfo(dsrApp) << "TreeLand output capture: selected output"
                   << selectedOutput->name << selectedOutput->geometry;
    return true;
}

TreelandOutputCapture::Private::Session *
TreelandOutputCapture::Private::createSession(QString *error)
{
    if (!sourceManager || !captureManager || !selectedOutput || !selectedOutput->output) {
        if (error)
            *error = QStringLiteral("TreeLand 输出截图协议对象无效");
        return nullptr;
    }

    auto *newSession = new Session;
    newSession->source = ext_output_image_capture_source_manager_v1_create_source(
            sourceManager, selectedOutput->output);
    if (!newSession->source) {
        if (error)
            *error = QStringLiteral("无法创建 TreeLand 输出捕获源");
        delete newSession;
        return nullptr;
    }

    newSession->session = ext_image_copy_capture_manager_v1_create_session(
            captureManager, newSession->source, 0);
    if (!newSession->session) {
        if (error)
            *error = QStringLiteral("无法创建 TreeLand 输出捕获会话");
        destroySession(newSession);
        return nullptr;
    }
    ext_image_copy_capture_session_v1_add_listener(
            newSession->session, &kSessionListener, newSession);

    if (!waitForConstraints(newSession, error)
        || !selectShmFormat(newSession, error)) {
        destroySession(newSession);
        return nullptr;
    }
    return newSession;
}

void TreelandOutputCapture::Private::destroySession(Session *target)
{
    if (!target)
        return;
    if (target->session) {
        ext_image_copy_capture_session_v1_destroy(target->session);
        target->session = nullptr;
    }
    if (target->source) {
        ext_image_capture_source_v1_destroy(target->source);
        target->source = nullptr;
    }
    delete target;
}

bool TreelandOutputCapture::Private::waitForConstraints(Session *target, QString *error)
{
    if (!target || !target->session)
        return false;

    QString dispatchError;
    if (!dispatchUntil(display, [target]() {
            return target->ready || target->stopped;
        }, ProtocolTimeoutMs, &dispatchError)) {
        if (error) {
            *error = dispatchError.isEmpty()
                    ? QStringLiteral("等待输出截图约束超时")
                    : QStringLiteral("等待输出截图约束超时：%1").arg(dispatchError);
        }
        return false;
    }
    if (target->stopped) {
        if (error)
            *error = QStringLiteral("TreeLand 输出捕获会话已被合成器终止");
        return false;
    }
    if (target->width == 0 || target->height == 0 || target->shmFormats.isEmpty()) {
        if (error)
            *error = QStringLiteral("TreeLand 输出捕获约束无效");
        return false;
    }
    return true;
}

bool TreelandOutputCapture::Private::selectShmFormat(Session *target, QString *error)
{
    static constexpr uint32_t kPreferredFormats[] = {
        WL_SHM_FORMAT_XRGB8888,
        WL_SHM_FORMAT_ARGB8888,
        WL_SHM_FORMAT_XBGR8888,
        WL_SHM_FORMAT_ABGR8888,
    };

    for (uint32_t format : kPreferredFormats) {
        if (target->shmFormats.contains(format)) {
            target->format = format;
            return true;
        }
    }
    if (error)
        *error = QStringLiteral("TreeLand 输出捕获没有可用的共享内存格式");
    return false;
}

bool TreelandOutputCapture::Private::captureFromSession(Session *target,
                                                         QImage *image,
                                                         quint64 *timestampNs,
                                                         QString *error)
{
    if (!target || !target->session || !image || !timestampNs) {
        if (error)
            *error = QStringLiteral("TreeLand 输出捕获会话无效");
        return false;
    }

    const QImage::Format imageFormat = imageFormatForShm(target->format);
    if (imageFormat == QImage::Format_Invalid) {
        if (error)
            *error = QStringLiteral("TreeLand 输出捕获返回了不支持的格式");
        return false;
    }

    Frame state;
    state.width = target->width;
    state.height = target->height;
    state.stride = state.width * 4;
    state.format = target->format;
    state.size = static_cast<size_t>(state.stride) * state.height;
    state.fd = memfd_create("treeland-output-scroll-capture", MFD_CLOEXEC);
    if (state.fd < 0 || ftruncate(state.fd, static_cast<off_t>(state.size)) < 0) {
        if (error)
            *error = QStringLiteral("创建输出截图共享内存失败：%1")
                             .arg(QString::fromLocal8Bit(std::strerror(errno)));
        return false;
    }
    state.data = mmap(nullptr, state.size, PROT_READ | PROT_WRITE, MAP_SHARED, state.fd, 0);
    if (state.data == MAP_FAILED) {
        state.data = nullptr;
        close(state.fd);
        if (error)
            *error = QStringLiteral("映射输出截图共享内存失败");
        return false;
    }

    state.pool = wl_shm_create_pool(shm, state.fd, static_cast<int>(state.size));
    state.buffer = state.pool
            ? wl_shm_pool_create_buffer(state.pool,
                                        0,
                                        static_cast<int>(state.width),
                                        static_cast<int>(state.height),
                                        static_cast<int>(state.stride),
                                        state.format)
            : nullptr;
    if (!state.buffer) {
        if (state.pool)
            wl_shm_pool_destroy(state.pool);
        munmap(state.data, state.size);
        close(state.fd);
        if (error)
            *error = QStringLiteral("创建输出截图 wl_buffer 失败");
        return false;
    }

    state.frame = ext_image_copy_capture_session_v1_create_frame(target->session);
    if (!state.frame) {
        wl_buffer_destroy(state.buffer);
        wl_shm_pool_destroy(state.pool);
        munmap(state.data, state.size);
        close(state.fd);
        if (error)
            *error = QStringLiteral("创建输出截图帧失败");
        return false;
    }
    ext_image_copy_capture_frame_v1_add_listener(state.frame, &kFrameListener, &state);
    ext_image_copy_capture_frame_v1_attach_buffer(state.frame, state.buffer);
    ext_image_copy_capture_frame_v1_damage_buffer(state.frame,
                                                  0,
                                                  0,
                                                  static_cast<int32_t>(state.width),
                                                  static_cast<int32_t>(state.height));
    ext_image_copy_capture_frame_v1_capture(state.frame);

    QString dispatchError;
    const bool dispatched = dispatchUntil(display, [&state]() {
        return state.ready || state.failed;
    }, FrameTimeoutMs, &dispatchError);

    bool success = false;
    if (!dispatched) {
        if (error)
            *error = dispatchError.isEmpty()
                    ? QStringLiteral("等待输出截图帧超时")
                    : QStringLiteral("等待输出截图帧超时：%1").arg(dispatchError);
    } else if (state.failed) {
        if (state.failureReason == EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_BUFFER_CONSTRAINTS) {
            if (error)
                *error = QStringLiteral("TreeLand 输出截图缓冲区约束已变化");
        } else if (state.failureReason == EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_STOPPED) {
            if (error)
                *error = QStringLiteral("TreeLand 输出捕获会话已停止");
        } else {
            if (error)
                *error = QStringLiteral("TreeLand 输出截图帧失败，reason=%1")
                                 .arg(state.failureReason);
        }
    } else {
        const QImage source(static_cast<const uchar *>(state.data),
                            static_cast<int>(state.width),
                            static_cast<int>(state.height),
                            static_cast<int>(state.stride),
                            imageFormat);
        *image = applyTransform(source.copy(), state.transform);
        *timestampNs = state.timestampNs;
        success = !image->isNull();
        if (!success && error)
            *error = QStringLiteral("输出截图帧为空");
    }

    ext_image_copy_capture_frame_v1_destroy(state.frame);
    wl_buffer_destroy(state.buffer);
    wl_shm_pool_destroy(state.pool);
    munmap(state.data, state.size);
    close(state.fd);
    return success;
}

void TreelandOutputCapture::Private::stop()
{
    selectedOutput = nullptr;

    for (OutputInfo &output : outputs) {
        if (output.output) {
            wl_output_release(output.output);
            output.output = nullptr;
        }
    }
    outputs.clear();

    if (sourceManager) {
        ext_output_image_capture_source_manager_v1_destroy(sourceManager);
        sourceManager = nullptr;
    }
    if (captureManager) {
        ext_image_copy_capture_manager_v1_destroy(captureManager);
        captureManager = nullptr;
    }
    if (shm) {
        wl_shm_destroy(shm);
        shm = nullptr;
    }
    if (registry) {
        wl_registry_destroy(registry);
        registry = nullptr;
    }
    if (display) {
        wl_display_flush(display);
        wl_display_disconnect(display);
        display = nullptr;
    }
}

void TreelandOutputCapture::Private::handleOutputGeometry(
        void *data,
        wl_output *output,
        int32_t x,
        int32_t y,
        int32_t,
        int32_t,
        int32_t,
        const char *,
        const char *,
        int32_t)
{
    auto *self = static_cast<Private *>(data);
    for (OutputInfo &info : self->outputs) {
        if (info.output != output)
            continue;
        const QSize size = info.geometry.size();
        info.geometry = QRect(x, y, size.width(), size.height());
        break;
    }
}

void TreelandOutputCapture::Private::handleOutputMode(void *data,
                                                       wl_output *output,
                                                       uint32_t,
                                                       int32_t width,
                                                       int32_t height,
                                                       int32_t)
{
    auto *self = static_cast<Private *>(data);
    for (OutputInfo &info : self->outputs) {
        if (info.output != output)
            continue;
        info.geometry.setSize(QSize(width, height));
        break;
    }
}

void TreelandOutputCapture::Private::handleOutputDone(void *data, wl_output *output)
{
    auto *self = static_cast<Private *>(data);
    for (OutputInfo &info : self->outputs) {
        if (info.output == output) {
            info.done = true;
            break;
        }
    }
}

void TreelandOutputCapture::Private::handleOutputScale(void *, wl_output *, int32_t)
{
}

void TreelandOutputCapture::Private::handleOutputName(void *data,
                                                       wl_output *output,
                                                       const char *name)
{
    auto *self = static_cast<Private *>(data);
    for (OutputInfo &info : self->outputs) {
        if (info.output == output) {
            info.name = QString::fromUtf8(name ? name : "");
            break;
        }
    }
}

void TreelandOutputCapture::Private::handleOutputDescription(void *, wl_output *, const char *)
{
}

void TreelandOutputCapture::Private::handleSessionBufferSize(
        void *data,
        ext_image_copy_capture_session_v1 *,
        uint32_t width,
        uint32_t height)
{
    auto *target = static_cast<Session *>(data);
    target->width = width;
    target->height = height;
    target->ready = false;
}

void TreelandOutputCapture::Private::handleSessionShmFormat(
        void *data,
        ext_image_copy_capture_session_v1 *,
        uint32_t format)
{
    static_cast<Session *>(data)->shmFormats.insert(format);
}

void TreelandOutputCapture::Private::handleSessionDmabufDevice(
        void *,
        ext_image_copy_capture_session_v1 *,
        wl_array *)
{
}

void TreelandOutputCapture::Private::handleSessionDmabufFormat(
        void *,
        ext_image_copy_capture_session_v1 *,
        uint32_t,
        wl_array *)
{
}

void TreelandOutputCapture::Private::handleSessionDone(
        void *data,
        ext_image_copy_capture_session_v1 *)
{
    auto *target = static_cast<Session *>(data);
    target->ready = true;
    ++target->constraintVersion;
}

void TreelandOutputCapture::Private::handleSessionStopped(
        void *data,
        ext_image_copy_capture_session_v1 *)
{
    static_cast<Session *>(data)->stopped = true;
}

void TreelandOutputCapture::Private::handleFrameTransform(
        void *data,
        ext_image_copy_capture_frame_v1 *,
        uint32_t transform)
{
    static_cast<Frame *>(data)->transform = transform;
}

void TreelandOutputCapture::Private::handleFrameDamage(
        void *,
        ext_image_copy_capture_frame_v1 *,
        int32_t,
        int32_t,
        int32_t,
        int32_t)
{
}

void TreelandOutputCapture::Private::handleFramePresentationTime(
        void *data,
        ext_image_copy_capture_frame_v1 *,
        uint32_t tvSecHi,
        uint32_t tvSecLo,
        uint32_t tvNsec)
{
    auto *frame = static_cast<Frame *>(data);
    frame->timestampNs = (static_cast<quint64>(tvSecHi) << 32 | tvSecLo)
            * 1000000000ULL + tvNsec;
}

void TreelandOutputCapture::Private::handleFrameReady(
        void *data,
        ext_image_copy_capture_frame_v1 *)
{
    static_cast<Frame *>(data)->ready = true;
}

void TreelandOutputCapture::Private::handleFrameFailed(
        void *data,
        ext_image_copy_capture_frame_v1 *,
        uint32_t reason)
{
    auto *frame = static_cast<Frame *>(data);
    frame->failed = true;
    frame->failureReason = reason;
}

void TreelandOutputCapture::Private::registryGlobal(
        void *data,
        wl_registry *registry,
        uint32_t name,
        const char *interface,
        uint32_t version)
{
    auto *self = static_cast<Private *>(data);
    if (std::strcmp(interface, wl_shm_interface.name) == 0 && !self->shm) {
        self->shm = static_cast<wl_shm *>(
                wl_registry_bind(registry, name, &wl_shm_interface, 1));
    } else if (std::strcmp(interface, wl_output_interface.name) == 0) {
        auto *output = static_cast<wl_output *>(
                wl_registry_bind(registry, name, &wl_output_interface,
                                 qMin<uint32_t>(version, 4)));
        self->outputs.append({output, QString(), QRect(), false});
        wl_output_add_listener(output, &kOutputListener, self);
    } else if (std::strcmp(interface,
                           ext_output_image_capture_source_manager_v1_interface.name) == 0
               && !self->sourceManager) {
        self->sourceManager = static_cast<ext_output_image_capture_source_manager_v1 *>(
                wl_registry_bind(registry,
                                 name,
                                 &ext_output_image_capture_source_manager_v1_interface,
                                 qMin<uint32_t>(version, 1)));
    } else if (std::strcmp(interface, ext_image_copy_capture_manager_v1_interface.name) == 0
               && !self->captureManager) {
        self->captureManager = static_cast<ext_image_copy_capture_manager_v1 *>(
                wl_registry_bind(registry,
                                 name,
                                 &ext_image_copy_capture_manager_v1_interface,
                                 qMin<uint32_t>(version, 1)));
    }
}

void TreelandOutputCapture::Private::registryGlobalRemove(
        void *,
        wl_registry *,
        uint32_t)
{
}

TreelandOutputCapture::TreelandOutputCapture()
    : d(new Private(this))
{
}

TreelandOutputCapture::~TreelandOutputCapture()
{
    delete d;
    d = nullptr;
}

bool TreelandOutputCapture::start(const QString &outputName,
                                  const QRect &outputGeometry,
                                  QString *error)
{
    stop();
    if (!d->initialize(error))
        return false;
    if (!d->selectOutput(outputName, outputGeometry, error)) {
        d->stop();
        return false;
    }
    // Validate that the compositor can actually provide a capture session and
    // usable shm constraints. Frames themselves use one-shot sessions below.
    Private::Session *validationSession = d->createSession(error);
    if (!validationSession) {
        d->stop();
        return false;
    }
    d->destroySession(validationSession);
    wl_display_flush(d->display);
    return true;
}

bool TreelandOutputCapture::capture(QImage *image,
                                    quint64 *timestampNs,
                                    QString *error)
{
    if (!d || !d->display || !d->selectedOutput) {
        if (error)
            *error = QStringLiteral("TreeLand 输出捕获会话尚未启动");
        return false;
    }

    // ext-image-copy-capture explicitly allows the compositor to wait for an
    // arbitrary content change after the first frame of a session. A reused
    // session therefore stalls forever when scrolling pauses. A fresh session
    // makes every request the first frame, which is copied immediately and
    // remains stable even when the screen is static.
    Private::Session *oneShotSession = d->createSession(error);
    if (!oneShotSession) {
        wl_display_flush(d->display);
        return false;
    }
    const bool success = d->captureFromSession(oneShotSession,
                                               image,
                                               timestampNs,
                                               error);
    d->destroySession(oneShotSession);
    wl_display_flush(d->display);
    return success;
}

void TreelandOutputCapture::stop()
{
    if (d)
        d->stop();
}

} // namespace Dsr::Screenshot
