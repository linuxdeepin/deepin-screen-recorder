// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusscreenshotservice.h"
#include "utils/eventlogutils.h"
#include "../utils/log.h"
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
    qCInfo(dsrApp) << "Starting screenshot via DBus service";
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B6"}
    };
    EventLogUtils::get().writeLogs(obj);
    qDebug() << "DBus screenshot service! start screenshot";
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new screenshot instance";
        parent()->startScreenshot();
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
}

void DBusScreenshotService::DelayScreenshot(qlonglong in0)
{
    qCInfo(dsrApp) << "Starting delayed screenshot with delay:" << in0;
    qDebug() << "DBus screenshot service! delay screenshot";
    // handle method call com.deepin.Screenshot.DelayScreenshot
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B2"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new delayed screenshot instance";
        parent()->delayScreenshot(in0);
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
}

void DBusScreenshotService::NoNotifyScreenshot()
{
    qCInfo(dsrApp) << "Starting no-notify screenshot";
    qDebug() << "DBus screenshot service! nonofiy screenshot";
    // handle method call com.deepin.Screenshot.NoNotify
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new no-notify screenshot instance";
        parent()->noNotifyScreenshot();
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
}

void DBusScreenshotService::OcrScreenshot()
{
    qCInfo(dsrApp) << "Starting OCR screenshot";
    qDebug() << "DBus screenshot service! OcrScreenshot";
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B4"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new OCR screenshot instance";
        parent()->OcrScreenshot();
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
}

void DBusScreenshotService::ScrollScreenshot()
{
    qCInfo(dsrApp) << "Starting scroll screenshot";
    qDebug() << "DBus screenshot service! ScrollScreenshot";
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B5"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new scroll screenshot instance";
        parent()->ScrollScreenshot();
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
}

void DBusScreenshotService::TopWindowScreenshot()
{
    qCInfo(dsrApp) << "Starting top window screenshot";
    qDebug() << "DBus screenshot service! topWindow screenshot";
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B3"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance)
        parent()->topWindowScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::FullscreenScreenshot()
{
    qDebug() << "DBus screenshot service! Fullscreen screenshot";
    // handle method call com.deepin.Screenshot.Fullscreenshot
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B1"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance)
        parent()->fullscreenScreenshot();
    m_singleInstance = true;
}

void DBusScreenshotService::SavePathScreenshot(const QString &in0)
{
    qCInfo(dsrApp) << "Starting save path screenshot with path:" << in0;
    qDebug() << "DBus screenshot service! SavePath screenshot";
    // handle method call com.deepin.Screenshot.SavePath
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new save path screenshot instance";
        parent()->savePathScreenshot(in0);
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
}

void DBusScreenshotService::StartScreenshotFor3rd(const QString &in0)
{
    qDebug() << "DBus screenshot service! startScreenshotFor3rd";
    if (!m_singleInstance)
        parent()->startScreenshotFor3rd(in0);
    m_singleInstance = true;
}

void DBusScreenshotService::FullScreenRecord(const QString &in0)
{

    if (!m_singleInstance)
        parent()->fullScreenRecord(in0);
    m_singleInstance = true;
}
