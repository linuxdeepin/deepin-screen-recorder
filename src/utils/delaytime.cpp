// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "delaytime.h"
#include <QDebug>
DelayTime::DelayTime(int msec)
{
    qCDebug(dsrApp) << "DelayTime constructor called with msec: " << msec;
    m_delayTime = msec;
    m_stopFlag = false;
    m_isForceToExitApp = false;
}

DelayTime::~DelayTime()
{
    qCDebug(dsrApp) << "DelayTime destructor called.";
}

void DelayTime::stop()
{
    qCDebug(dsrApp) << "Stopping delay timer.";
    m_stopFlag = true;
    this->quit();
    this->wait();
    qCDebug(dsrApp) << "Delay timer stopped.";
}

void DelayTime::setForceToExitApp(bool isForceToExitApp)
{
    qCDebug(dsrApp) << "Setting force to exit app to: " << isForceToExitApp;
    m_isForceToExitApp = isForceToExitApp;
}

void DelayTime::forceToExitApp()
{
    qCDebug(dsrApp) << "Forcing application exit due to timeout.";
    _Exit(0);
}

void DelayTime::run()
{
    qCDebug(dsrApp) << "Delay timer started.";
    int delayTime = m_delayTime;
    while (!m_stopFlag && delayTime > 0) {
        qCDebug(dsrApp) << "Current delay time: " << delayTime << "ms.";
        emit progress(delayTime);
        msleep(1);
        delayTime--;
    }
    //只有延时完成才可以执行任务
    if (delayTime == 0) {
        qCDebug(dsrApp) << "Delay timer finished. Emitting doWork signal.";
        emit doWork();
        qCDebug(dsrApp) << "Delayed for " << m_delayTime << "ms.";
    }
    if (m_isForceToExitApp) {
        qCDebug(dsrApp) << "Force to exit app is true. Calling forceToExitApp.";
        forceToExitApp();
    }
    qCDebug(dsrApp) << "DelayTime run function finished.";
}
