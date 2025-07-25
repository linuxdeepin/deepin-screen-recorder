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
#include <QJsonDocument>
#include <QJsonObject>

/*
 * Implementation of adaptor class DBusScreenshotService
 */

DBusScreenshotService::DBusScreenshotService(Screenshot *parent)
    : QDBusAbstractAdaptor(parent)
    , m_singleInstance(false)
{
    qCDebug(dsrApp) << "DBusScreenshotService constructor called.";
    // constructor
    Q_UNUSED(parent);
    setAutoRelaySignals(true);
}

DBusScreenshotService::~DBusScreenshotService()
{
    qCDebug(dsrApp) << "DBusScreenshotService destructor called.";
    // destructor
}

void DBusScreenshotService::setSingleInstance(bool instance)
{
    qCDebug(dsrApp) << "setSingleInstance called with instance:" << instance;
    m_singleInstance = instance;
}

void DBusScreenshotService::StartScreenshot()
{
    qCDebug(dsrApp) << "StartScreenshot method called.";
    qCInfo(dsrApp) << "Starting screenshot via DBus service";
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B6"}
    };
    EventLogUtils::get().writeLogs(obj);
    qCDebug(dsrApp) << "DBus screenshot service! start screenshot";
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new screenshot instance";
        parent()->startScreenshot();
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
    qCDebug(dsrApp) << "StartScreenshot method finished.";
}

void DBusScreenshotService::DelayScreenshot(qlonglong in0)
{
    qCInfo(dsrApp) << "Starting delayed screenshot with delay:" << in0;
    qCDebug(dsrApp) << "DBus screenshot service! delay screenshot";
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
    qCDebug(dsrApp) << "DelayScreenshot method finished.";
}

void DBusScreenshotService::NoNotifyScreenshot()
{
    qCDebug(dsrApp) << "NoNotifyScreenshot method called.";
    qCInfo(dsrApp) << "Starting no-notify screenshot";
    qCDebug(dsrApp) << "DBus screenshot service! nonofiy screenshot";
    // handle method call com.deepin.Screenshot.NoNotify
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new no-notify screenshot instance";
        parent()->noNotifyScreenshot();
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
    qCDebug(dsrApp) << "NoNotifyScreenshot method finished.";
}

void DBusScreenshotService::OcrScreenshot()
{
    qCDebug(dsrApp) << "OcrScreenshot method called.";
    qCInfo(dsrApp) << "Starting OCR screenshot";
    qCDebug(dsrApp) << "DBus screenshot service! OcrScreenshot";
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
    qCDebug(dsrApp) << "OcrScreenshot method finished.";
}

void DBusScreenshotService::ScrollScreenshot()
{
    qCInfo(dsrApp) << "Starting scroll screenshot";
    qCDebug(dsrApp) << "DBus screenshot service! ScrollScreenshot";
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
    qCDebug(dsrApp) << "ScrollScreenshot method finished.";
}

void DBusScreenshotService::TopWindowScreenshot()
{
    qCInfo(dsrApp) << "Starting top window screenshot";
    qCDebug(dsrApp) << "DBus screenshot service! topWindow screenshot";
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B3"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new top window screenshot instance";
        parent()->topWindowScreenshot();
    }
    m_singleInstance = true;
    qCDebug(dsrApp) << "TopWindowScreenshot method finished.";
}

void DBusScreenshotService::FullscreenScreenshot()
{
    qCDebug(dsrApp) << "FullscreenScreenshot method called.";
    // handle method call com.deepin.Screenshot.Fullscreenshot
    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1},
        {"startup_mode", "B1"}
    };
    EventLogUtils::get().writeLogs(obj);
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new fullscreen screenshot instance";
        parent()->fullscreenScreenshot();
    }
    m_singleInstance = true;
    qCDebug(dsrApp) << "FullscreenScreenshot method finished.";
}

void DBusScreenshotService::SavePathScreenshot(const QString &in0)
{
    qCInfo(dsrApp) << "Starting save path screenshot with path:" << in0;
    qCDebug(dsrApp) << "DBus screenshot service! SavePath screenshot";
    // handle method call com.deepin.Screenshot.SavePath
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new save path screenshot instance";
        parent()->savePathScreenshot(in0);
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
    qCDebug(dsrApp) << "SavePathScreenshot method finished.";
}

void DBusScreenshotService::StartScreenshotFor3rd(const QString &in0)
{
    qCDebug(dsrApp) << "DBus screenshot service! startScreenshotFor3rd";
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new 3rd party screenshot instance";
        parent()->startScreenshotFor3rd(in0);
    }
    m_singleInstance = true;
    qCDebug(dsrApp) << "StartScreenshotFor3rd method finished.";
}

void DBusScreenshotService::FullScreenRecord(const QString &in0)
{
    qCDebug(dsrApp) << "FullScreenRecord method called with parameter:" << in0;
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new full screen record instance";
        parent()->fullScreenRecord(in0);
    }
    m_singleInstance = true;
    qCDebug(dsrApp) << "FullScreenRecord method finished.";
}

void DBusScreenshotService::CustomScreenshot(const QVariantMap &params)
{
    qCDebug(dsrApp) << "CustomScreenshot method called with QVariantMap parameters";
    
    // 解析参数
    bool showToolbar = params.value("showToolBar", true).toBool();
    bool showNotification = params.value("showNotification", true).toBool();
    
    qCDebug(dsrApp) << "Parsed parameters:" << "showToolbar:" << showToolbar << "showNotification:" << showNotification;
    
    if (!m_singleInstance) {
        qCDebug(dsrApp) << "Starting new custom screenshot instance";
        parent()->customScreenshot(showToolbar, showNotification);
    } else {
        qCDebug(dsrApp) << "Screenshot instance already running";
    }
    m_singleInstance = true;
    qCDebug(dsrApp) << "CustomScreenshot method finished.";
}
