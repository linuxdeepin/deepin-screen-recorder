// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "delaytime.h"
#include <QDebug>
DelayTime::DelayTime(int msec)
{
    m_delayTime = msec;
    m_stopFlag = false;
    m_isForceToExitApp = false;
}

DelayTime::~DelayTime()
{

}

void DelayTime::stop()
{
    m_stopFlag = true;
    this->quit();
    this->wait();
    qDebug() << __FUNCTION__ << __LINE__ << "停止延时";
}

void DelayTime::setForceToExitApp(bool isForceToExitApp)
{
    m_isForceToExitApp = isForceToExitApp;
}

void DelayTime::forceToExitApp()
{
    qInfo() << __FUNCTION__ << __LINE__ << "已超时强制退出截图录屏";
    _Exit(0);
}

void DelayTime::run()
{
    qDebug() << __FUNCTION__ << __LINE__ << "延时开始...";
    int delayTime = m_delayTime;
    while (!m_stopFlag && delayTime > 0) {
        emit progress(delayTime);
        msleep(1);
        delayTime--;
    }
    //只有延时完成才可以执行任务
    if (delayTime == 0) {
        emit doWork();
        qDebug() << __FUNCTION__ << __LINE__ << "已延时" << m_delayTime << "ms";
    }
    if (m_isForceToExitApp) {
        forceToExitApp();
    }
}
