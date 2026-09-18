// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pinscreenshots.h"
#include "mainwindow.h"
#include <DWidgetUtil>
#include <QDebug>
#include "../utils/log.h"

PinScreenShots::PinScreenShots(QObject *parent) : QObject(parent)
{
    qCDebug(dsrApp) << "Initializing PinScreenShots";
    m_loadingCount = 0;
}

bool PinScreenShots::openFile(QString filePath)
{
    qDebug() << __FUNCTION__ << __LINE__ << filePath;
    bool bRet = false;
    MainWindow *win = new MainWindow();
    //增加判断，空图片不会启动
    bRet = win->openFile(filePath);
    if (bRet) {
        qCDebug(dsrApp) << "File opened successfully, showing window";
        win->show();
        if (m_loadingCount == 0) {
            qCDebug(dsrApp) << "First window, incrementing loading count";
            m_loadingCount++;
        }
    } else {
        qCWarning(dsrApp) << "Failed to open file:" << filePath;
    }
    return bRet;
}

void PinScreenShots::openImage(QImage image)
{
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        qDebug() << __FUNCTION__ << __LINE__ << image.size();
        MainWindow *win = new MainWindow();
        win->openImage(image);
        win->show();
        //第一次启动才居中
        if (m_loadingCount == 0) {
            qCDebug(dsrApp) << "First window, incrementing loading count";
            m_loadingCount++;
        }
    } else {
        qCWarning(dsrApp) << "Attempted to open invalid image";
    }
}

void PinScreenShots::openImageAndName(QImage image, QString imageName, QPoint point)
{
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        qCDebug(dsrApp) << "Opening image with name:" << imageName << "at position:" << point;
        MainWindow *win = new MainWindow();
        win->openImageAndName(image, imageName, point);
        qCDebug(dsrApp) << "Moving window to center and adjusting position for multi-screen";
        Dtk::Widget::moveToCenter(win);
        win->move(win->getShowPosition());
        win->show();
        //第一次启动才居中
        if (m_loadingCount == 0) {
            qCDebug(dsrApp) << "First window, incrementing loading count";
            m_loadingCount++;
        }
    } else {
        qCWarning(dsrApp) << "Attempted to open invalid image with name:" << imageName;
    }
}
