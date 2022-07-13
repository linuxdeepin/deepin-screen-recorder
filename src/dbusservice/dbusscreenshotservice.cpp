/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbusscreenshotservice.h"
#include "utils/eventlogutils.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QDebug>

/*
 * Implementation of adaptor class DBusScreenshotService
 */

DBusScreenshotService::DBusScreenshotService(Screenshot *parent)
    : QDBusAbstractAdaptor(parent)
    , m_singleInstance(false)
{
    // constructor
    Q_UNUSED(parent);
    setAutoRelaySignals(true);
}

DBusScreenshotService::~DBusScreenshotService()
{
    // destructor
}

void DBusScreenshotService::setSingleInstance(bool instance)
{
    m_singleInstance = instance;
}

void DBusScreenshotService::StartScreenshot()
{
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"mode", 1},
        {"startup_mode", "B6"}
    };
    EventLogUtils::get().writeLogs(obj);

    if (!m_singleInstance)
        parent()->startScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::DelayScreenshot(qlonglong in0)
{
    qDebug() << "DBus screenshot service! delay screenshot";
    // handle method call com.deepin.Screenshot.DelayScreenshot
    if (!m_singleInstance)
        parent()->delayScreenshot(in0);
    m_singleInstance = true;
}

void DBusScreenshotService::NoNotifyScreenshot()
{
    qDebug() << "DBus screenshot service! nonofiy screenshot";
    // handle method call com.deepin.Screenshot.NoNotify
    if (!m_singleInstance)
        parent()->noNotifyScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::OcrScreenshot()
{
    qDebug() << "DBus screenshot service! OcrScreenshot";
    // handle method call com.deepin.Screenshot.NoNotify
    if (!m_singleInstance)
        parent()->OcrScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::ScrollScreenshot()
{
    qDebug() << "DBus screenshot service! ScrollScreenshot";
    // handle method call com.deepin.Screenshot.NoNotify
    if (!m_singleInstance)
        parent()->ScrollScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::TopWindowScreenshot()
{
    qDebug() << "DBus screenshot service! topWindow screenshot";
    // handle method call com.deepin.Screenshot.TopWindow
    if (!m_singleInstance)
        parent()->topWindowScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::FullscreenScreenshot()
{
    qDebug() << "DBus screenshot service! Fullscreen screenshot";
    // handle method call com.deepin.Screenshot.Fullscreenshot
    if (!m_singleInstance)
        parent()->fullscreenScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::SavePathScreenshot(const QString &in0)
{
    qDebug() << "DBus screenshot service! SavePath screenshot";
    // handle method call com.deepin.Screenshot.SavePath
    if (!m_singleInstance)
        parent()->savePathScreenshot(in0);
    m_singleInstance = true;
}
void DBusScreenshotService::StartScreenshotFor3rd(const QString &in0)
{
    qDebug() << "DBus screenshot service! startScreenshotFor3rd";
    if (!m_singleInstance)
        parent()->startScreenshotFor3rd(in0);
    m_singleInstance = true;
}
