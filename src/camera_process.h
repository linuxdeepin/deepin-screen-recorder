// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAMERA_PROCESS_H
#define CAMERA_PROCESS_H

#include <QObject>
#include <DWidget>


//Qt6
#if (QT_VERSION_MAJOR == 5)
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCameraInfo>
#elif (QT_VERSION_MAJOR == 6)
#include <QMediaCaptureSession>
#include <QImageCapture>
#include <QVideoWidget>
#endif

#include <QCamera>
#include <QList>
#include <QDebug>
#include <QImage>
#include <QVideoWidget>

class CameraProcess : public QObject
{
    Q_OBJECT
public:
    explicit CameraProcess(QObject *parent = nullptr);
    ~CameraProcess();
    //static bool checkCameraAvailability();

signals:

public slots:
    void ShowTheCapture();
    //void displayImage(int, QImage image);

private:
#if (QT_VERSION_MAJOR == 5)
    QCamera *camera; // 摄像头
    QCameraViewfinder *viewfinder; // 摄像头取景器部件
    QCameraImageCapture *imageCapture; // 截图部件
#elif (QT_VERSION_MAJOR == 6)
    QCamera *camera;
    QVideoWidget *viewfinder; // 摄像头取景器部件
    QImageCapture *imageCapture; // 截图部件
    QMediaCaptureSession *captureSession;
#endif
};

#endif // CAMERA_PROCESS_H
