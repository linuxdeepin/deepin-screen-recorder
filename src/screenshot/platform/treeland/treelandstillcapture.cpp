// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandstillcapture.h"

#include "../../../ext-image-capture/frame/extcaptureframe.h"
#include "../../../ext-image-capture/manager/extcapturemanager.h"
#include "../../../ext-image-capture/session/extcapturesession.h"

#include <QDebug>
#include <QEventLoop>
#include <QScreen>
#include <QTransform>
#include <QTimer>
#include <wayland-client.h>

namespace Dsr::Screenshot {

TreelandStillCapture::TreelandStillCapture(QObject *parent)
    : QObject(parent)
    , m_manager(new ExtCaptureManager(this))
{
}

TreelandStillCapture::~TreelandStillCapture() = default;

QImage TreelandStillCapture::capture(QScreen *screen, QString *error)
{
    if (!screen) {
        if (error)
            *error = QStringLiteral("没有可用的输出屏幕");
        return QImage();
    }
    if (!waitForManager(error))
        return QImage();

    ExtCaptureSession *session = m_manager->createScreenCaptureSession(screen, false, false);
    if (!session) {
        if (error)
            *error = QStringLiteral("创建 ext-image-copy-capture 会话失败");
        return QImage();
    }

    if (session->state() != ExtCaptureSession::Ready) {
        QEventLoop readyLoop;
        QTimer readyTimer;
        readyTimer.setSingleShot(true);
        QString sessionError;
        connect(&readyTimer, &QTimer::timeout, &readyLoop, &QEventLoop::quit);
        connect(session, &ExtCaptureSession::ready, &readyLoop, &QEventLoop::quit);
        connect(session, &ExtCaptureSession::error, &readyLoop, [&](const QString &message) {
            sessionError = message;
            readyLoop.quit();
        });
        readyTimer.start(3000);
        readyLoop.exec();

        if (session->state() != ExtCaptureSession::Ready) {
            if (error) {
                *error = sessionError.isEmpty()
                        ? QStringLiteral("等待截图协议约束超时")
                        : sessionError;
            }
            session->stop();
            return QImage();
        }
    }

    QImage image;
    QString frameError;
    QEventLoop frameLoop;
    QTimer frameTimer;
    frameTimer.setSingleShot(true);
    connect(&frameTimer, &QTimer::timeout, &frameLoop, &QEventLoop::quit);
    connect(session, &ExtCaptureSession::frameReady, &frameLoop, [&](ExtCaptureFrame *frame) {
        if (!frame || frame->isDmaBuffer()) {
            frameError = QStringLiteral("静态截图收到了不支持的 DMA-BUF 帧");
            frameLoop.quit();
            return;
        }

        const FrameData &data = frame->frameData();
        const QImage::Format format = imageFormatForShm(data.format);
        if (!data.data || data.dimensions.isEmpty() || format == QImage::Format_Invalid) {
            frameError = QStringLiteral("截图帧格式无效：%1").arg(data.format);
            frameLoop.quit();
            return;
        }

        QImage wrapped(static_cast<const uchar *>(data.data),
                       data.dimensions.width(),
                       data.dimensions.height(),
                       static_cast<int>(data.stride),
                       format);
        image = wrapped.copy();

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
        frameLoop.quit();
    });
    connect(session, &ExtCaptureSession::error, &frameLoop, [&](const QString &message) {
        frameError = message;
        frameLoop.quit();
    });

    frameTimer.start(3000);
    ExtCaptureFrame *frame = session->createFrame();
    if (!frame) {
        if (error)
            *error = QStringLiteral("创建 ext-image-copy-capture 帧失败");
        session->stop();
        return QImage();
    }
    if (!frame->capture()) {
        if (error)
            *error = frameError.isEmpty()
                    ? QStringLiteral("提交 ext-image-copy-capture 抓帧请求失败")
                    : frameError;
        session->stop();
        return QImage();
    }
    frameLoop.exec();
    session->stop();

    if (image.isNull() && error) {
        *error = frameError.isEmpty()
                ? QStringLiteral("等待截图帧超时")
                : frameError;
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
