// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QCAMERACONTROLLER_H
#define QCAMERACONTROLLER_H

#include "cameracontroller.h"

#include <QString>

class QCamera;
#if (QT_VERSION_MAJOR == 5)
class QAbstractVideoSurface;
#elif (QT_VERSION_MAJOR == 6)
class QMediaCaptureSession;
class QVideoSink;
#endif

/**
 * @brief QtCameraController 基于 Qt Multimedia 的摄像头采集实现
 */
class QtCameraController : public CameraController
{
    Q_OBJECT
public:
    explicit QtCameraController(QObject *parent = nullptr);
    ~QtCameraController() override;

    QList<CameraDeviceDescriptor> availableDevices() const override;
    bool start(const QString &deviceId) override;
    void stop() override;
    bool isActive() const override;
    QString currentDeviceId() const override;

private:
    void releaseCamera();
    void handleCameraError();

    QCamera *m_camera = nullptr;
    QString m_currentDeviceId;

#if (QT_VERSION_MAJOR == 5)
    QAbstractVideoSurface *m_videoSurface = nullptr;
#elif (QT_VERSION_MAJOR == 6)
    QMediaCaptureSession *m_captureSession = nullptr;
    QVideoSink *m_videoSink = nullptr;
#endif
};

#endif // QCAMERACONTROLLER_H
