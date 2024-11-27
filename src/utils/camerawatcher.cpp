// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "camerawatcher.h"

#include <QThread>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QDebug>

CameraWatcher::CameraWatcher(QObject *parent)
    : QObject(parent)
    , m_coulduse(true)
{
    //m_isRecoding = false;
    m_watchTimer = new QTimer(this);
    connect(m_watchTimer, &QTimer::timeout, this, &CameraWatcher::slotCameraWatcher); //新增定时器检测摄像头
}

CameraWatcher::~CameraWatcher()
{

}

void CameraWatcher::setWatch(const bool isWatcher)
{
    if (isWatcher) {
        m_watchTimer->start(1000);
    } else {
        m_watchTimer->stop();
    }
}

// 设置Coulduse的值
void CameraWatcher::setCoulduseValue(bool value)
{
    m_coulduse = value;
}

// 将原有的run方法替换为slotCameraWatcher，解决截图录屏退出时缓慢的问题
void CameraWatcher::slotCameraWatcher()
{
    bool couldUse = false;
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (!cameras.isEmpty()) {
        couldUse = true;
    }

    if (couldUse != m_coulduse) {
        m_coulduse = couldUse;
        emit sigCameraState(couldUse);
    }
}

