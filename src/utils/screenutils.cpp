// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenutils.h"
#include "log.h"

#include <QApplication>
#include <QScreen>

ScreenUtils *ScreenUtils::m_screenUtils = nullptr;
ScreenUtils *ScreenUtils::instance(QPoint pos)
{
    qCDebug(dsrApp) << "Getting ScreenUtils instance for position:" << pos;
    if (!m_screenUtils) {
        m_screenUtils = new ScreenUtils(pos);
        qCDebug(dsrApp) << "New ScreenUtils instance created.";
    }

    return m_screenUtils;
}

ScreenUtils::ScreenUtils(QPoint pos, QObject *parent)
    : QObject(parent)
{
    qCDebug(dsrApp) << "Initializing ScreenUtils with position:" << pos;
    QList<QScreen *> screenList = qApp->screens();
    m_screenNum = qApp->desktop()->screenNumber(pos);
    m_rootWindowId = qApp->desktop()->screen(m_screenNum)->winId();
    m_primaryScreen = screenList[m_screenNum];
    if (m_screenNum != 0 && m_screenNum < screenList.length()) {
        m_backgroundRect = screenList[m_screenNum]->geometry();
        qCDebug(dsrApp) << "Background rect set to screen geometry for screen num:" << m_screenNum;
    } else {
        m_backgroundRect = qApp->primaryScreen()->geometry();
        qCDebug(dsrApp) << "Background rect set to primary screen geometry.";
    }

}

ScreenUtils::~ScreenUtils()
{
    qCDebug(dsrApp) << "ScreenUtils destructor called.";
}

int ScreenUtils::getScreenNum()
{
    qCDebug(dsrApp) << "Returning screen number:" << m_screenNum;
    return m_screenNum;
}

QRect ScreenUtils::backgroundRect()
{
    qCDebug(dsrApp) << "Returning background rect:" << m_backgroundRect;
    return m_backgroundRect;
}

WId ScreenUtils::rootWindowId()
{
    qCDebug(dsrApp) << "Returning root window ID:" << m_rootWindowId;
    return m_rootWindowId;
}

QScreen *ScreenUtils::primaryScreen()
{
    qCDebug(dsrApp) << "Returning primary screen pointer.";
    return m_primaryScreen;
}

