// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAMERA_PROCESS_H
#define CAMERA_PROCESS_H

#include <QObject>
#include <DWidget>
#include <QMediaCaptureSession>
#include <QImageCapture>
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

signals:

public slots:
    void ShowTheCapture();

private:
    QCamera *camera;
    QVideoWidget *viewfinder;
    QImageCapture *imageCapture;
    QMediaCaptureSession *captureSession;
};

#endif // CAMERA_PROCESS_H
