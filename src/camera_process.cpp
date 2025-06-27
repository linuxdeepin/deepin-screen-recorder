// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "camera_process.h"
#include "utils/log.h"

CameraProcess::CameraProcess(QObject *parent) : QObject(parent)
{
    qCDebug(dsrApp) << "Entering CameraProcess constructor.";
    camera = new QCamera; //摄像头
    qCDebug(dsrApp) << "QCamera instance created.";
    viewfinder = new QCameraViewfinder();
    qCDebug(dsrApp) << "QCameraViewfinder instance created.";
    imageCapture = new QCameraImageCapture(camera); //截图部件
    qCDebug(dsrApp) << "QCameraImageCapture instance created.";
    camera->setViewfinder(viewfinder);
    qCDebug(dsrApp) << "Viewfinder set for camera.";

//    QObject::connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(ShowTheCapture()));
//    QObject::connect(ui->SaveButton,SIGNAL(clicked()),this,SLOT(SavePicture()));
//    QObject::connect(imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(displayImage(int,QImage)));
//    imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
//    camera->setCaptureMode(QCamera::CaptureStillImage);
//    camera->setViewfinder(viewfinder);
//    camera->start(); //启动摄像头

    qCDebug(dsrApp) << "Exiting CameraProcess constructor.";
}

CameraProcess::~CameraProcess()
{
    qCDebug(dsrApp) << "Entering CameraProcess destructor.";
}

void CameraProcess::ShowTheCapture()
{
    qCDebug(dsrApp) << "Entering CameraProcess::ShowTheCapture.";
    //展示摄像头采集画面
    imageCapture->capture();
}
/*
 * never used
void CameraProcess::displayImage(int, QImage image)
{
    //QImage image1(ui->label->x(),ui->label->y(),QImage::Format_RGB888);
    // image=new QImage(220,220,QImage::Format_RGB888);
    // image.setPixel(,Format_RGB888);
    // QColor color(QImage::Format_RGB888);
    // image.fill(color);
    //QImage image2;
    //QImage::Format format =  image1.format();
    //qDebug() << (int)format;
    image = image.convertToFormat(QImage::Format_RGB888);
//    ui->label->setPixmap(QPixmap::fromImage(image));
    //QImage::Format format =  image.format();
    //qDebug() << (int)format;
}

bool CameraProcess::checkCameraAvailability()
{
    if(QCameraInfo::availableCameras().count() > 0)
        return true;
    else
        return false;
}
*/
