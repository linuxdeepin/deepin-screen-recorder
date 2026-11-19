// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qcameracontroller.h"

#include "../utils/log.h"

#include <QDebug>
#include <QCamera>
#include <QImage>
#include <QSize>

#if (QT_VERSION_MAJOR == 5)
#include <QAbstractVideoBuffer>
#include <QAbstractVideoSurface>
#include <QCameraInfo>
#include <QCameraViewfinderSettings>
#include <QVideoFrame>
#include <QVideoSurfaceFormat>
#elif (QT_VERSION_MAJOR == 6)
#include <QCameraDevice>
#include <QCameraFormat>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QVideoFrame>
#include <QVideoSink>
#endif

namespace {

#if (QT_VERSION_MAJOR == 5)
class CameraVideoSurface final : public QAbstractVideoSurface
{
public:
    explicit CameraVideoSurface(QObject *parent = nullptr)
        : QAbstractVideoSurface(parent)
    {
    }

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType) const override
    {
        if (handleType != QAbstractVideoBuffer::NoHandle)
            return {};

        return {
            QVideoFrame::Format_RGB32,
            QVideoFrame::Format_ARGB32,
            QVideoFrame::Format_RGB24,
            QVideoFrame::Format_YUV420P,
            QVideoFrame::Format_UYVY,
            QVideoFrame::Format_YUYV,
            QVideoFrame::Format_Jpeg
        };
    }

    bool present(const QVideoFrame &frame) override
    {
        if (!frame.isValid())
            return false;

        emit frameAvailable(frame);
        return true;
    }
};
#endif

#if (QT_VERSION_MAJOR == 6)
QCameraFormat preferredCameraFormat(const QCameraDevice &device)
{
    const QList<QCameraFormat> formats = device.videoFormats();
    if (formats.isEmpty())
        return {};

    QCameraFormat best;
    qint64 bestScore = -1;
    constexpr qreal preferredAspect = 16.0 / 9.0;

    for (const QCameraFormat &format : formats) {
        const QSize resolution = format.resolution();
        if (!resolution.isValid() || resolution.width() < 320 || resolution.height() < 240)
            continue;

        const qint64 area = static_cast<qint64>(resolution.width()) * resolution.height();
        const qreal aspect = static_cast<qreal>(resolution.width()) / resolution.height();
        const qreal fps = format.maxFrameRate();

        // 预览窗口最大只有 320x180，优先选择 720p 左右的 16:9 格式，
        // 既保证清晰度，也避免每一帧都进行过大的格式转换。
        qint64 score = area;
        if (resolution.width() <= 1280 && resolution.height() <= 720)
            score += 2000000000LL;
        else if (resolution.width() <= 1920 && resolution.height() <= 1080)
            score += 1000000000LL;

        if (qAbs(aspect - preferredAspect) < 0.03)
            score += 100000000LL;

        if (fps >= 24.0 && fps <= 30.5)
            score += 1000000LL;
        else if (fps > 60.0)
            score -= 1000000LL;

        if (score > bestScore) {
            bestScore = score;
            best = format;
        }
    }

    return best;
}
#endif

} // namespace

QtCameraController::QtCameraController(QObject *parent)
    : CameraController(parent)
{
}

QtCameraController::~QtCameraController()
{
    releaseCamera();
}

QList<CameraDeviceDescriptor> QtCameraController::availableDevices() const
{
    QList<CameraDeviceDescriptor> devices;

#if (QT_VERSION_MAJOR == 5)
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    devices.reserve(cameras.size());
    for (const QCameraInfo &camera : cameras) {
        devices.append({camera.deviceName(), camera.description(), camera.isDefault()});
    }
#elif (QT_VERSION_MAJOR == 6)
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    devices.reserve(cameras.size());
    for (const QCameraDevice &camera : cameras) {
        devices.append({QString::fromUtf8(camera.id()), camera.description(), camera.isDefault()});
    }
#endif

    return devices;
}

bool QtCameraController::start(const QString &deviceId)
{
    releaseCamera();
    m_currentDeviceId.clear();

#if (QT_VERSION_MAJOR == 5)
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.isEmpty()) {
        emit errorOccurred(tr("No camera device found"));
        return false;
    }

    QCameraInfo target = cameras.first();
    for (const QCameraInfo &camera : cameras) {
        if (camera.deviceName() == deviceId) {
            target = camera;
            break;
        }
    }

    m_camera = new QCamera(target, this);
    auto *surface = new CameraVideoSurface(this);
    m_videoSurface = surface;
    m_camera->setViewfinder(surface);
    connect(surface, &QAbstractVideoSurface::frameAvailable, this,
            [this](const QVideoFrame &frame) {
        QVideoFrame mappedFrame(frame);
        if (!mappedFrame.map(QAbstractVideoBuffer::ReadOnly))
            return;

        const QImage::Format imageFormat =
                QVideoFrame::imageFormatFromPixelFormat(mappedFrame.pixelFormat());
        if (imageFormat == QImage::Format_Invalid) {
            mappedFrame.unmap();
            return;
        }

        QImage image(mappedFrame.bits(),
                     mappedFrame.width(),
                     mappedFrame.height(),
                     mappedFrame.bytesPerLine(),
                     imageFormat);
        const QImage copy = image.copy();
        mappedFrame.unmap();
        if (!copy.isNull())
            emit frameReady(copy);
    });

    QCameraViewfinderSettings settings;
    settings.setResolution(640, 480);
    settings.setMinimumFrameRate(15.0);
    settings.setMaximumFrameRate(30.0);
    m_camera->setViewfinderSettings(settings);

    connect(m_camera,
            QOverload<QCamera::Error>::of(&QCamera::error),
            this,
            [this](QCamera::Error) { handleCameraError(); });

    m_currentDeviceId = target.deviceName();
#elif (QT_VERSION_MAJOR == 6)
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        emit errorOccurred(tr("No camera device found"));
        return false;
    }

    QCameraDevice target = cameras.first();
    for (const QCameraDevice &camera : cameras) {
        if (QString::fromUtf8(camera.id()) == deviceId) {
            target = camera;
            break;
        }
    }

    m_camera = new QCamera(target, this);
    m_captureSession = new QMediaCaptureSession(this);
    m_videoSink = new QVideoSink(this);

    const QCameraFormat format = preferredCameraFormat(target);
    if (!format.isNull())
        m_camera->setCameraFormat(format);

    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoSink(m_videoSink);

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this,
            [this](const QVideoFrame &frame) {
        const QImage image = frame.toImage();
        if (!image.isNull())
            emit frameReady(image);
    });

    connect(m_camera, &QCamera::errorOccurred, this,
            [this](QCamera::Error) { handleCameraError(); });

    m_currentDeviceId = QString::fromUtf8(target.id());
#endif

    qCDebug(dsrApp) << "Starting camera:" << m_currentDeviceId;
    m_camera->start();
    return true;
}

void QtCameraController::stop()
{
    releaseCamera();
}

bool QtCameraController::isActive() const
{
    return m_camera && m_camera->isActive();
}

QString QtCameraController::currentDeviceId() const
{
    return m_currentDeviceId;
}

void QtCameraController::releaseCamera()
{
#if (QT_VERSION_MAJOR == 6)
    // 先断开 capture session 对 camera 和 sink 的引用，再释放对象。
    if (m_captureSession) {
        m_captureSession->setVideoSink(nullptr);
        m_captureSession->setCamera(nullptr);
    }
#endif

    if (m_camera) {
        m_camera->stop();
#if (QT_VERSION_MAJOR == 5)
        m_camera->setViewfinder(nullptr);
#endif
        delete m_camera;
        m_camera = nullptr;
    }

#if (QT_VERSION_MAJOR == 5)
    delete m_videoSurface;
    m_videoSurface = nullptr;
#elif (QT_VERSION_MAJOR == 6)
    delete m_captureSession;
    m_captureSession = nullptr;
    delete m_videoSink;
    m_videoSink = nullptr;
#endif

    m_currentDeviceId.clear();
}

void QtCameraController::handleCameraError()
{
    const QString message = m_camera ? m_camera->errorString()
                                     : tr("Unknown camera error");
    qCWarning(dsrApp) << "Camera error:" << message;
    emit errorOccurred(message);
}
