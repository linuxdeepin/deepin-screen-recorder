// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandfullscreengrabber.h"
#include "log.h"

#include "../protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h"
#include "../protocols/ext-image-copy-capture/qwayland-ext-image-capture-source-v1.h"

#include <private/qwaylandclientextension_p.h>
#include <private/qwaylandshmbackingstore_p.h>
#include <private/qguiapplication_p.h>
#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandintegration_p.h>

#include <QEventLoop>
#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <qpa/qplatformnativeinterface.h>

#include <wayland-client.h>

namespace {

QtWaylandClient::QWaylandDisplay *waylandDisplay()
{
    auto *integration = dynamic_cast<QtWaylandClient::QWaylandIntegration *>(
        QGuiApplicationPrivate::platformIntegration());
    return integration ? integration->display() : nullptr;
}

class OutputSourceManager
    : public QWaylandClientExtensionTemplate<OutputSourceManager>,
      public QtWayland::ext_output_image_capture_source_manager_v1
{
    Q_OBJECT
public:
    OutputSourceManager() : QWaylandClientExtensionTemplate<OutputSourceManager>(1) { }
};

class CopyCaptureManager
    : public QWaylandClientExtensionTemplate<CopyCaptureManager>,
      public QtWayland::ext_image_copy_capture_manager_v1
{
    Q_OBJECT
public:
    CopyCaptureManager() : QWaylandClientExtensionTemplate<CopyCaptureManager>(1) { }
};

class CaptureSession : public QObject, public QtWayland::ext_image_copy_capture_session_v1
{
    Q_OBJECT
public:
    CaptureSession(struct ::ext_image_copy_capture_session_v1 *object, QObject *parent = nullptr)
        : QObject(parent), QtWayland::ext_image_copy_capture_session_v1(object)
    {
    }
    ~CaptureSession() override
    {
        if (isInitialized())
            destroy();
    }

    QSize bufferSize;
    QList<uint32_t> shmFormats;

Q_SIGNALS:
    void done();
    void stopped();

protected:
    void ext_image_copy_capture_session_v1_buffer_size(uint32_t width, uint32_t height) override
    {
        bufferSize = QSize(static_cast<int>(width), static_cast<int>(height));
    }
    void ext_image_copy_capture_session_v1_shm_format(uint32_t format) override
    {
        shmFormats.append(format);
    }
    void ext_image_copy_capture_session_v1_dmabuf_device(wl_array *) override { }
    void ext_image_copy_capture_session_v1_dmabuf_format(uint32_t, wl_array *) override { }
    void ext_image_copy_capture_session_v1_done() override { Q_EMIT done(); }
    void ext_image_copy_capture_session_v1_stopped() override { Q_EMIT stopped(); }
};

class CaptureFrame : public QObject, public QtWayland::ext_image_copy_capture_frame_v1
{
    Q_OBJECT
public:
    CaptureFrame(struct ::ext_image_copy_capture_frame_v1 *object, QObject *parent = nullptr)
        : QObject(parent), QtWayland::ext_image_copy_capture_frame_v1(object)
    {
    }
    ~CaptureFrame() override
    {
        if (isInitialized())
            destroy();
    }

Q_SIGNALS:
    void ready();
    void failed(uint32_t reason);

protected:
    void ext_image_copy_capture_frame_v1_transform(uint32_t) override { }
    void ext_image_copy_capture_frame_v1_damage(int32_t, int32_t, int32_t, int32_t) override { }
    void ext_image_copy_capture_frame_v1_presentation_time(uint32_t, uint32_t, uint32_t) override { }
    void ext_image_copy_capture_frame_v1_ready() override { Q_EMIT ready(); }
    void ext_image_copy_capture_frame_v1_failed(uint32_t reason) override { Q_EMIT failed(reason); }
};

// Map an advertised wl_shm format to a wl_shm_format QWaylandShmBuffer can
// allocate and QImage can read directly.
bool isSupportedShmFormat(uint32_t format)
{
    switch (format) {
    case WL_SHM_FORMAT_ARGB8888:
    case WL_SHM_FORMAT_XRGB8888:
    case WL_SHM_FORMAT_ABGR8888:
    case WL_SHM_FORMAT_XBGR8888:
        return true;
    default:
        return false;
    }
}

} // namespace

class TreelandFullScreenGrabber::Private
{
public:
    OutputSourceManager sourceManager;
    CopyCaptureManager captureManager;
};

TreelandFullScreenGrabber::TreelandFullScreenGrabber(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

TreelandFullScreenGrabber::~TreelandFullScreenGrabber()
{
    delete d;
}

QImage TreelandFullScreenGrabber::grab(int timeoutMs)
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        qCWarning(dsrApp) << "TreelandFullScreenGrabber: no screens";
        return QImage();
    }

    // Wait for both protocol globals to bind.
    if (!d->sourceManager.isActive() || !d->captureManager.isActive()) {
        QEventLoop loop;
        QTimer deadline;
        deadline.setSingleShot(true);
        connect(&deadline, &QTimer::timeout, &loop, &QEventLoop::quit);
        auto checkActive = [&] {
            if (d->sourceManager.isActive() && d->captureManager.isActive())
                loop.quit();
        };
        connect(&d->sourceManager, &OutputSourceManager::activeChanged, &loop, checkActive);
        connect(&d->captureManager, &CopyCaptureManager::activeChanged, &loop, checkActive);
        deadline.start(timeoutMs);
        QTimer::singleShot(0, &loop, checkActive);
        loop.exec();
        if (!d->sourceManager.isActive() || !d->captureManager.isActive()) {
            qCWarning(dsrApp) << "TreelandFullScreenGrabber: ext-image-copy-capture globals"
                              << "not available (source:" << d->sourceManager.isActive()
                              << "capture:" << d->captureManager.isActive() << ")";
            return QImage();
        }
    }

    if (screens.size() == 1)
        return grabOutput(screens.first(), timeoutMs);

    // Composite multiple outputs onto a canvas covering the virtual desktop.
    const QRect virtualRect = screens.first()->virtualGeometry();
    const qreal ratio = qreal(QGuiApplication::primaryScreen()->devicePixelRatio());
    QImage canvas(virtualRect.size() * ratio, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::black);
    QPainter painter(&canvas);
    bool any = false;
    for (QScreen *screen : screens) {
        const QImage img = grabOutput(screen, timeoutMs);
        if (img.isNull()) {
            qCWarning(dsrApp) << "TreelandFullScreenGrabber: failed to grab" << screen->name();
            continue;
        }
        any = true;
        const QRect logical = screen->geometry().translated(-virtualRect.topLeft());
        painter.drawImage(QRectF(logical.topLeft() * ratio, logical.size() * ratio), img);
    }
    painter.end();
    return any ? canvas : QImage();
}

QImage TreelandFullScreenGrabber::grabOutput(QScreen *screen, int timeoutMs)
{
    auto *nativeInterface = QGuiApplication::platformNativeInterface();
    auto *output = static_cast<wl_output *>(nativeInterface->nativeResourceForScreen("output", screen));
    if (!output) {
        qCWarning(dsrApp) << "TreelandFullScreenGrabber: no wl_output for" << screen->name();
        return QImage();
    }

    auto *source = d->sourceManager.create_source(output);
    CaptureSession session(d->captureManager.create_session(source, 0));
    // The source can be destroyed as soon as the session holds it.
    ext_image_capture_source_v1_destroy(source);
    if (auto *display = waylandDisplay())
        wl_display_flush(display->wl_display());

    QImage result;
    QEventLoop loop;
    QTimer deadline;
    deadline.setSingleShot(true);
    connect(&deadline, &QTimer::timeout, &loop, [&loop] {
        qCWarning(dsrApp) << "TreelandFullScreenGrabber: timed out waiting for frame";
        loop.quit();
    });

    QtWaylandClient::QWaylandShmBuffer *buffer = nullptr;
    CaptureFrame *frame = nullptr;

    connect(&session, &CaptureSession::stopped, &loop, &QEventLoop::quit);
    connect(&session, &CaptureSession::done, &loop, [&] {
        if (frame)
            return; // constraint updates after the first done are ignored
        if (session.bufferSize.isEmpty()) {
            qCWarning(dsrApp) << "TreelandFullScreenGrabber: empty buffer size";
            loop.quit();
            return;
        }
        uint32_t shmFormat = 0;
        bool found = false;
        for (uint32_t format : session.shmFormats) {
            if (isSupportedShmFormat(format)) {
                shmFormat = format;
                found = true;
                break;
            }
        }
        if (!found) {
            qCWarning(dsrApp) << "TreelandFullScreenGrabber: no supported SHM format in"
                              << session.shmFormats;
            loop.quit();
            return;
        }

        buffer = new QtWaylandClient::QWaylandShmBuffer(
            waylandDisplay(),
            session.bufferSize,
            QtWaylandClient::QWaylandShm::formatFrom(static_cast<::wl_shm_format>(shmFormat)));
        frame = new CaptureFrame(session.create_frame(), &session);
        connect(frame, &CaptureFrame::failed, &loop, [&loop](uint32_t reason) {
            qCWarning(dsrApp) << "TreelandFullScreenGrabber: frame failed, reason" << reason;
            loop.quit();
        });
        connect(frame, &CaptureFrame::ready, &loop, [&] {
            result = buffer->image()->copy();
            loop.quit();
        });
        frame->attach_buffer(buffer->buffer());
        // Protocol: first capture in a session must damage the whole buffer.
        frame->damage_buffer(0, 0, session.bufferSize.width(), session.bufferSize.height());
        frame->capture();
        if (auto *display = waylandDisplay())
            wl_display_flush(display->wl_display());
    });

    deadline.start(timeoutMs);
    loop.exec();

    // `frame` is parented to `session`; only the buffer needs manual cleanup.
    delete buffer;

    if (result.isNull())
        qCWarning(dsrApp) << "TreelandFullScreenGrabber: grab failed for" << screen->name();
    else
        qCInfo(dsrApp) << "TreelandFullScreenGrabber: grabbed" << screen->name() << result.size();
    return result;
}

#include "treelandfullscreengrabber.moc"
