// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "camerawatcher.h"
#include "log.h"

#include <QThread>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QDebug>

CameraWatcher::CameraWatcher(QObject *parent)
    : QObject(parent)
    , m_coulduse(true)
{
    qCDebug(dsrApp) << "CameraWatcher constructor called.";
    //m_isRecoding = false;
    m_watchTimer = new QTimer(this);
    connect(m_watchTimer, &QTimer::timeout, this, &CameraWatcher::slotCameraWatcher); //新增定时器检测摄像头
}

CameraWatcher::~CameraWatcher()
{
    qCDebug(dsrApp) << "CameraWatcher destructor called.";
}

void CameraWatcher::setWatch(const bool isWatcher)
{
    qCDebug(dsrApp) << "setWatch called with isWatcher:" << isWatcher;
    if (isWatcher) {
        m_watchTimer->start(1000);
        qCDebug(dsrApp) << "Watch timer started.";
    } else {
        m_watchTimer->stop();
        qCDebug(dsrApp) << "Watch timer stopped.";
    }
}

// 设置Coulduse的值
void CameraWatcher::setCoulduseValue(bool value)
{
    qCDebug(dsrApp) << "setCoulduseValue called with value:" << value;
    m_coulduse = value;
}

// 将原有的run方法替换为slotCameraWatcher，解决截图录屏退出时缓慢的问题
void CameraWatcher::slotCameraWatcher()
{
    qCDebug(dsrApp) << "slotCameraWatcher called, checking camera state.";
    bool couldUse = false;
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (!cameras.isEmpty()) {
        couldUse = true;
        qCDebug(dsrApp) << "Cameras detected, couldUse set to true.";
    }

    if (couldUse != m_coulduse) {
        m_coulduse = couldUse;
        qCInfo(dsrApp) << "Camera state changed, available:" << couldUse;
        emit sigCameraState(couldUse);
    }
}

