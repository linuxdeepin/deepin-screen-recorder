// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandstillcapture.h"

#include "../../../ext-image-capture/frame/extcaptureframe.h"
#include "../../../ext-image-capture/manager/extcapturemanager.h"
#include "../../../ext-image-capture/session/extcapturesession.h"

#include <QDebug>
#include <QEventLoop>
#include <QHash>
#include <QScreen>
#include <QSet>
#include <QTimer>
#include <QTransform>
#include <wayland-client.h>

#include <utility>

namespace Dsr::Screenshot {

TreelandStillCapture::TreelandStillCapture(QObject *parent)
    : QObject(parent)
    , m_manager(new ExtCaptureManager(this))
{
}

TreelandStillCapture::~TreelandStillCapture() = default;

QList<TreelandStillCapture::CapturedOutput> TreelandStillCapture::capture(
        const QList<QScreen *> &screens, QString *error)
{
    if (error)
        error->clear();
    if (screens.isEmpty()) {
        if (error)
            *error = QStringLiteral("没有可用的输出屏幕");
        return {};
    }
    if (!waitForManager(error))
        return {};

    constexpr int captureTimeoutMs = 3000;
    QList<ExtCaptureSession *> sessions;
    sessions.reserve(screens.size());

    auto stopSessions = [&sessions]() {
        for (ExtCaptureSession *session : std::as_const(sessions)) {
            session->stop();
            session->deleteLater();
        }
    };

    // Match the snap-target demo: create every output session before waiting
    // for any individual session to become ready.
    for (QScreen *screen : screens) {
        ExtCaptureSession *session = m_manager->createScreenCaptureSession(screen, false, false);
        if (!session) {
            if (error) {
                *error = QStringLiteral("为输出 %1 创建 ext-image-copy-capture 会话失败")
                                 .arg(screen ? screen->name() : QStringLiteral("<null>"));
            }
            stopSessions();
            return {};
        }
        sessions.append(session);
    }

    QSet<ExtCaptureSession *> pendingConstraints;
    QEventLoop constraintsLoop;
    QTimer constraintsTimer;
    QString batchError;
    constraintsTimer.setSingleShot(true);
    connect(&constraintsTimer, &QTimer::timeout, &constraintsLoop, &QEventLoop::quit);

    for (int i = 0; i < sessions.size(); ++i) {
        ExtCaptureSession *session = sessions.at(i);
        if (session->state() == ExtCaptureSession::Ready)
            continue;
        if (session->state() == ExtCaptureSession::Error
                || session->state() == ExtCaptureSession::Stopped) {
            batchError = QStringLiteral("输出 %1 截图会话未能就绪").arg(screens.at(i)->name());
            break;
        }

        pendingConstraints.insert(session);
        connect(session, &ExtCaptureSession::ready, &constraintsLoop, [&, session]() {
            pendingConstraints.remove(session);
            if (pendingConstraints.isEmpty())
                constraintsLoop.quit();
        });
        connect(session, &ExtCaptureSession::error, &constraintsLoop,
                [&, i](const QString &message) {
                    if (batchError.isEmpty()) {
                        batchError = QStringLiteral("输出 %1 截图会话准备失败：%2")
                                             .arg(screens.at(i)->name(), message);
                    }
                    constraintsLoop.quit();
                });
    }

    if (batchError.isEmpty() && !pendingConstraints.isEmpty()) {
        constraintsTimer.start(captureTimeoutMs);
        constraintsLoop.exec();
    }

    if (batchError.isEmpty() && !pendingConstraints.isEmpty())
        batchError = QStringLiteral("等待多屏截图协议约束超时");
    if (!batchError.isEmpty()) {
        if (error)
            *error = batchError;
        stopSessions();
        return {};
    }

    QSet<ExtCaptureSession *> pendingFrames;
    QHash<ExtCaptureSession *, QImage> images;
    QHash<ExtCaptureSession *, quint64> timestamps;
    QEventLoop frameLoop;
    QTimer frameTimer;
    frameTimer.setSingleShot(true);
    connect(&frameTimer, &QTimer::timeout, &frameLoop, &QEventLoop::quit);

    for (int i = 0; i < sessions.size(); ++i) {
        ExtCaptureSession *session = sessions.at(i);
        pendingFrames.insert(session);
        connect(session, &ExtCaptureSession::frameReady, &frameLoop,
                [&, session, i](ExtCaptureFrame *frame) {
                    QString imageError;
                    const QImage image = imageFromFrame(frame, &imageError);
                    if (image.isNull()) {
                        if (batchError.isEmpty()) {
                            batchError = QStringLiteral("抓取输出 %1 失败：%2")
                                                 .arg(screens.at(i)->name(), imageError);
                        }
                        frameLoop.quit();
                        return;
                    }

                    images.insert(session, image);
                    timestamps.insert(session, frame->timestamp());
                    pendingFrames.remove(session);
                    if (pendingFrames.isEmpty())
                        frameLoop.quit();
                });
        connect(session, &ExtCaptureSession::error, &frameLoop,
                [&, i](const QString &message) {
                    if (batchError.isEmpty()) {
                        batchError = QStringLiteral("抓取输出 %1 失败：%2")
                                             .arg(screens.at(i)->name(), message);
                    }
                    frameLoop.quit();
                });
    }

    // All frame requests are submitted before the loop waits for any of them.
    for (int i = 0; i < sessions.size() && batchError.isEmpty(); ++i) {
        ExtCaptureFrame *frame = sessions.at(i)->createFrame();
        if (!frame) {
            batchError = QStringLiteral("输出 %1 创建截图帧失败").arg(screens.at(i)->name());
            break;
        }
        if (!frame->capture()) {
            batchError = QStringLiteral("输出 %1 提交截图帧请求失败").arg(screens.at(i)->name());
            break;
        }
    }

    if (batchError.isEmpty() && !pendingFrames.isEmpty()) {
        frameTimer.start(captureTimeoutMs);
        frameLoop.exec();
    }
    if (batchError.isEmpty() && !pendingFrames.isEmpty())
        batchError = QStringLiteral("等待多屏截图帧超时");

    if (!batchError.isEmpty()) {
        if (error)
            *error = batchError;
        stopSessions();
        return {};
    }

    QList<CapturedOutput> outputs;
    outputs.reserve(screens.size());
    for (int i = 0; i < screens.size(); ++i) {
        ExtCaptureSession *session = sessions.at(i);
        outputs.append({screens.at(i), images.value(session), timestamps.value(session)});
    }

    stopSessions();
    return outputs;
}

QImage TreelandStillCapture::imageFromFrame(ExtCaptureFrame *frame, QString *error)
{
    if (!frame) {
        if (error)
            *error = QStringLiteral("截图帧对象无效");
        return {};
    }
    if (frame->isDmaBuffer()) {
        if (error)
            *error = QStringLiteral("静态截图收到了不支持的 DMA-BUF 帧");
        return {};
    }

    const FrameData &data = frame->frameData();
    const QImage::Format format = imageFormatForShm(data.format);
    if (!data.data || data.dimensions.isEmpty() || format == QImage::Format_Invalid) {
        if (error)
            *error = QStringLiteral("截图帧格式无效：%1").arg(data.format);
        return {};
    }

    QImage image(static_cast<const uchar *>(data.data),
                 data.dimensions.width(),
                 data.dimensions.height(),
                 static_cast<int>(data.stride),
                 format);
    image = image.copy();

    switch (data.transform) {
    case FrameTransform::Rotate90:
        image = image.transformed(QTransform().rotate(90));
        break;
    case FrameTransform::Rotate180:
        image = image.transformed(QTransform().rotate(180));
        break;
    case FrameTransform::Rotate270:
        image = image.transformed(QTransform().rotate(270));
        break;
    default:
        break;
    }
    return image;
}

bool TreelandStillCapture::waitForManager(QString *error)
{
    if (m_manager->isProtocolAvailable())
        return true;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(m_manager, &ExtCaptureManager::protocolAvailable, &loop, &QEventLoop::quit);
    timer.start(3000);
    loop.exec();

    if (m_manager->isProtocolAvailable())
        return true;
    if (error)
        *error = QStringLiteral("ext-image-copy-capture 协议不可用");
    return false;
}

QImage::Format TreelandStillCapture::imageFormatForShm(uint32_t format)
{
    switch (format) {
    case WL_SHM_FORMAT_ARGB8888:
        return QImage::Format_ARGB32;
    case WL_SHM_FORMAT_XRGB8888:
        return QImage::Format_RGB32;
    case WL_SHM_FORMAT_XBGR8888:
        return QImage::Format_RGBX8888;
    case WL_SHM_FORMAT_ABGR8888:
        return QImage::Format_RGBA8888;
    default:
        return QImage::Format_Invalid;
    }
}

} // namespace Dsr::Screenshot
