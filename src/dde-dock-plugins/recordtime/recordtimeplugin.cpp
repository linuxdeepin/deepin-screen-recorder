// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordtimeplugin.h"
#include "timewidget.h"
#include "../../utils/log.h"
#include <QWidget>

Q_LOGGING_CATEGORY(dsrApp, "record-time-plugin");

RecordTimePlugin::RecordTimePlugin(QObject *parent)
    : QObject(parent)
    , m_bshow(false)
{
    qCDebug(dsrApp) << "RecordTimePlugin constructor called.";
    m_timer = nullptr;
    m_checkTimer = nullptr;
    m_timeWidget = nullptr;
}

const QString RecordTimePlugin::pluginName() const
{
    qCDebug(dsrApp) << "pluginName method called.";
    return QString("deepin-screen-recorder-plugin");
}

const QString RecordTimePlugin::pluginDisplayName() const
{
    qCDebug(dsrApp) << "pluginDisplayName method called.";
    return QString("deepin-screen-recorder");
}

void RecordTimePlugin::init(PluginProxyInterface *proxyInter)
{
    qCInfo(dsrApp) << "Initializing plugin with proxy interface";
    m_proxyInter = proxyInter;
    m_dBusService = new DBusService(this);
    // Reset record time (save in file) when restart recording.
    connect(m_dBusService, &DBusService::start, this, [this](){ onStart(true); });
    connect(m_dBusService, SIGNAL(stop()), this, SLOT(onStop()));
    connect(m_dBusService, SIGNAL(recording()), this, SLOT(onRecording()));
    connect(m_dBusService, SIGNAL(pause()), this, SLOT(onPause()));
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.ScreenRecorder.time") &&
        sessionBus.registerObject("/com/deepin/ScreenRecorder/time", this, QDBusConnection::ExportAdaptors)) {
        qInfo() << "dbus service registration success!";
    } else {
        qWarning() << "dbus service registration failed!";
    }
    qCDebug(dsrApp) << "init method finished.";
}

bool RecordTimePlugin::pluginIsDisable()
{
    qCDebug(dsrApp) << "pluginIsDisable method called.";
    return m_proxyInter->getValue(this, "disabled", false).toBool();
}

void RecordTimePlugin::pluginStateSwitched()
{
    qCDebug(dsrApp) << "pluginStateSwitched method called.";
    const bool disabledNew = !pluginIsDisable();
    qCInfo(dsrApp) << "Plugin state switched, new disabled state:" << disabledNew;
    m_proxyInter->saveValue(this, "disabled", disabledNew);
    if (disabledNew) {
        qCDebug(dsrApp) << "Removing plugin item";
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        qCDebug(dsrApp) << "Adding plugin item";
        m_proxyInter->itemAdded(this, pluginName());
    }
    qCDebug(dsrApp) << "pluginStateSwitched method finished.";
}

QWidget *RecordTimePlugin::itemWidget(const QString &itemKey)
{
    qCDebug(dsrApp) << "itemWidget method called with itemKey:" << itemKey;
    Q_UNUSED(itemKey);
    return m_timeWidget;
}

void RecordTimePlugin::clear()
{
    qCDebug(dsrApp) << "clear method called.";
    qCInfo(dsrApp) << "Clearing plugin resources";
    m_timeWidget->clearSetting();

    if (nullptr != m_timer) {
        qCDebug(dsrApp) << "Stopping and deleting timer";
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    } else {
        qCDebug(dsrApp) << "Timer is already null, no action needed.";
    }

    if (nullptr != m_timeWidget) {
        qCDebug(dsrApp) << "Deleting time widget";
        m_timeWidget->deleteLater();
        m_timeWidget = nullptr;
    }

    if (nullptr != m_checkTimer) {
        qCDebug(dsrApp) << "Stopping and deleting check timer";
        m_checkTimer->stop();
        m_checkTimer->deleteLater();
        m_checkTimer = nullptr;
    }
    qCDebug(dsrApp) << "clear method finished.";
}

void RecordTimePlugin::onStart(bool resetTime)
{
    qCInfo(dsrApp) << "Starting record time plugin, reset time:" << resetTime;
    m_timer = new QTimer(this);
    m_timeWidget = new TimeWidget();
    if (resetTime) {
        qCDebug(dsrApp) << "Clearing time widget settings";
        m_timeWidget->clearSetting();
    }
    m_checkTimer = nullptr;
    m_timer->start(600);
    connect(m_timer, &QTimer::timeout, this, &RecordTimePlugin::refresh);

    if (m_timeWidget->enabled()) {
        qInfo() << "load plugin";
        qCDebug(dsrApp) << "Time widget enabled, loading plugin.";
        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemAdded(this, pluginName());
        m_bshow = true;
        m_timeWidget->start();
    }
    qCDebug(dsrApp) << "onStart method finished.";
}

void RecordTimePlugin::onStop()
{
    qCDebug(dsrApp) << "onStop method called.";
    if (m_timeWidget->enabled()) {
        qInfo() << "unload plugin";
        qCDebug(dsrApp) << "Time widget enabled, unloading plugin.";
        m_proxyInter->itemRemoved(this, pluginName());
        m_bshow = false;
        if (nullptr != m_checkTimer) {
            qCDebug(dsrApp) << "Stopping check timer";
            m_checkTimer->stop();
            m_checkTimer->deleteLater();
            m_checkTimer = nullptr;
        }
        m_count = 0;
        m_nextCount = 0;
        clear();
    }
    qInfo() << "stop record time";
    qCDebug(dsrApp) << "onStop method finished.";
}

// 当托盘插件开始闪烁计数时才会执行
void RecordTimePlugin::onRecording()
{
    qCDebug(dsrApp) << "Recording status update received";
    // 录屏过程中，killall dde-dock，恢复时重新初始化
    if (m_timeWidget == nullptr) {
        qCInfo(dsrApp) << "Time widget is null, reinitializing";
        onStart();
    }

    if (m_timeWidget->enabled() && m_bshow) {
        qCDebug(dsrApp) << "Time widget enabled and visible.";
        m_nextCount++;
        if (1 == m_nextCount) {
            qCDebug(dsrApp) << "Starting check timer for recording status";
            m_checkTimer = new QTimer();
            connect(m_checkTimer, &QTimer::timeout, this, [=] {
                // 说明录屏还在进行中
                if (m_count < m_nextCount) {
                    qCDebug(dsrApp) << "Recording in progress, updating count";
                    m_count = m_nextCount;
                }
                // 说明录屏已经停止了
                else {
                    qCDebug(dsrApp) << "Recording stopped, calling onStop";
                    onStop();
                }
            });
            m_checkTimer->start(2000);
        }
    }
    qCDebug(dsrApp) << "onRecording method finished.";
}

void RecordTimePlugin::onPause()
{
    qCDebug(dsrApp) << "onPause method called.";
    // Empty implementation to match legacy behavior
    if (m_timeWidget->enabled() && m_bshow) {
        qCInfo(dsrApp) << "Pausing record time widget";
        m_timeWidget->stop();
    }
    qCDebug(dsrApp) << "onPause method finished.";
}

void RecordTimePlugin::refresh()
{
    qCDebug(dsrApp) << "refresh method called.";
    QSize size = m_timeWidget->sizeHint();
    if (size.width() > m_timeWidget->width() && 1 != position() && 3 != position() && m_bshow) {
        qInfo() << "refresh plugin";
        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemAdded(this, pluginName());
    }
    qCDebug(dsrApp) << "refresh method finished.";
}

RecordTimePlugin::~RecordTimePlugin()
{
    qCDebug(dsrApp) << "RecordTimePlugin destructor called.";
}
