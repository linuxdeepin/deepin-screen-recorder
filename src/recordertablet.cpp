// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordertablet.h"

#include "utils/log.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

const QString RecorderTablet::RESOURCES_PATH = "/usr/share/deepin-screen-recorder/tablet_resources";

RecorderTablet::RecorderTablet(QObject *parent) : QObject(parent)
{
    qCDebug(dsrApp) << "RecorderTablet constructor called.";

    QDBusConnection sb = QDBusConnection::sessionBus();
    bool result;
    result = sb.registerService("com.deepin.TabletScreenRecorder");
    if (!result) {
        qCWarning(dsrApp) << "Failed to register DBus service:" << sb.lastError().message();
    } else {
        qCDebug(dsrApp) << "DBus service 'com.deepin.TabletScreenRecorder' registered successfully.";
    }

    result = sb.registerObject("/com/deepin/TabletScreenRecorder",
                               "com.deepin.TabletScreenRecorder", this,
                               QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportScriptableSlots);
    if (!result) {
        qCWarning(dsrApp) << "Failed to register DBus object:" << sb.lastError().message();
    } else {
        qCDebug(dsrApp) << "DBus object '/com/deepin/TabletScreenRecorder' registered successfully.";
    }
}

RecorderTablet::~RecorderTablet()
{
    qCDebug(dsrApp) << "RecorderTablet destructor called.";
    if (nullptr != m_changeTimer) {
        qCDebug(dsrApp) << "Deleting m_changeTimer.";
        delete m_changeTimer;
        m_changeTimer = nullptr;
    }
    if (nullptr != m_statusBarTimer) {
        qCDebug(dsrApp) << "Deleting m_statusBarTimer.";
        delete m_statusBarTimer;
        m_statusBarTimer = nullptr;
    }
}

void RecorderTablet::start()
{
    qCDebug(dsrApp) << "RecorderTablet start() called.";
    // 拷贝json文件到指定目录
    /*
    QString dirPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    dirPath += "/Applications/";
    QDir dir;
    if (!dir.exists(dirPath)) {
        dir.mkdir(dirPath);
    }
    QFile::copy(RESOURCES_PATH + "/recorder.json", dirPath + "recorder.json");
    */
    // 通知开始录屏
    sendRecorderState(true);
    qCDebug(dsrApp) << "Sent recorder state to true.";

    showCountdownCounter = 3;
    m_changeTimer = new QTimer(this);
    qCDebug(dsrApp) << "Countdown timer created and connected.";
    connect(m_changeTimer, SIGNAL(timeout()), this, SLOT(update()));
    m_changeTimer->start(1000);
    qCDebug(dsrApp) << "Countdown timer started.";
}

void RecorderTablet::stop()
{
    qCDebug(dsrApp) << "RecorderTablet stop() called.";
    if (m_statusBarTimer) {
        m_statusBarTimer->stop();
        qCDebug(dsrApp) << "Status bar timer stopped.";
    }
    // 下拉面板图标设置为非录屏状态
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/TabletScreenRecorder", "com.deepin.TabletScreenRecorder", "DropDownIconChange");
    msg << QString("%1/%2").arg(RESOURCES_PATH).arg("fast-icon_recording_normal.svg");
    QDBusConnection::sessionBus().send(msg);
    qCDebug(dsrApp) << "Sent DBus signal to change dropdown icon to normal.";
    // 删除json文件
    // QFile::remove(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/Applications/recorder.json");

    // 通知停止录屏
    sendRecorderState(false);
    qCDebug(dsrApp) << "Sent recorder state to false.";
}

void RecorderTablet::update()
{
    qCDebug(dsrApp) << "RecorderTablet update() called.";

    //下拉面板发送dbus信号， 倒计时图片
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/TabletScreenRecorder", "com.deepin.TabletScreenRecorder", "DropDownIconChange");
    msg << QString("%1/counting%2.svg").arg(RESOURCES_PATH).arg(showCountdownCounter);
    QDBusConnection::sessionBus().send(msg);
    qCDebug(dsrApp) << "Sent DBus signal to change dropdown icon to countdown image:" << showCountdownCounter << ".";

    showCountdownCounter--;
    if (showCountdownCounter <= 0) {
        qCDebug(dsrApp) << "Countdown finished, stopping timer and emitting finished signal.";
        m_changeTimer->stop();
        emit finished();
    }
}

void RecorderTablet::startStatusBar()
{
    qCDebug(dsrApp) << "RecorderTablet startStatusBar() called.";
    // 下拉面板图标设置为录屏状态
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/TabletScreenRecorder", "com.deepin.TabletScreenRecorder", "DropDownIconChange");
    msg << QString("%1/%2").arg(RESOURCES_PATH).arg("fast-icon_recording_active.svg");
    QDBusConnection::sessionBus().send(msg);
    qCDebug(dsrApp) << "Sent DBus signal to change dropdown icon to active recording icon.";

    m_statusBarTimer = new QTimer(this);
    qCDebug(dsrApp) << "Status bar timer created and connected.";
    connect(m_statusBarTimer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));
    m_statusBarTimer->start(800);
    qCDebug(dsrApp) << "Status bar timer started.";
}

const QString RecorderTablet::getRecorderNormalIcon()
{
    qCDebug(dsrApp) << "RecorderTablet getRecorderNormalIcon() called.";
    return RecorderTablet::RESOURCES_PATH + "/fast-icon_recording_normal.svg";
}

void RecorderTablet::updateStatusBar()
{
    qCDebug(dsrApp) << "RecorderTablet updateStatusBar() called.";
    //状态栏发送dbus信号
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/TabletScreenRecorder", "com.deepin.TabletScreenRecorder", "StatusBarIconChange");
    msg << QString("%1/recording%2.svg").arg(RESOURCES_PATH).arg(flashTrayIconCounter % 2);
    QDBusConnection::sessionBus().send(msg);
    qCDebug(dsrApp) << "Sent DBus signal to change status bar icon.";
    flashTrayIconCounter++;
}

void RecorderTablet::sendRecorderState(const bool state)
{
    qCDebug(dsrApp) << "RecorderTablet sendRecorderState() called with state:" << state << ".";
    QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/ScreenRecorder", "com.deepin.ScreenRecorder", "RecorderState");
    msg << state;
    QDBusConnection::sessionBus().send(msg);
    qCDebug(dsrApp) << "Sent DBus signal 'RecorderState' with state:" << state << ".";
}
