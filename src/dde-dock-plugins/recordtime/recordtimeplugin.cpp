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
    m_timeWidget = nullptr;
    m_serviceWatcher = nullptr;
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

    if (nullptr != m_serviceWatcher) {
        qCDebug(dsrApp) << "Deleting service watcher";
        m_serviceWatcher->deleteLater();
        m_serviceWatcher = nullptr;
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
    m_timer->start(600);
    connect(m_timer, &QTimer::timeout, this, &RecordTimePlugin::refresh);

    bool widgetEnabled = m_timeWidget->enabled();
    qCInfo(dsrApp) << "RecordTimePlugin::onStart() - m_timeWidget->enabled() returns:" << widgetEnabled;
    if (widgetEnabled) {
        qInfo() << "load plugin";
        qCDebug(dsrApp) << "Time widget enabled, loading plugin.";
        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemAdded(this, pluginName());
        m_bshow = true;
        m_timeWidget->start();
    } else {
        qCWarning(dsrApp) << "Time widget is NOT enabled, plugin will NOT be loaded!";
    }
    qCDebug(dsrApp) << "onStart method finished.";
}

void RecordTimePlugin::onStop()
{
    qCDebug(dsrApp) << "onStop method called.";
    // m_timeWidget 可能已被前一次 onStop->clear() 置空。
    // 例如主程序先正常停录、随后退出，watcher 会再触发一次进来——
    // 此时直接解引用会崩，先保护再判 enabled。
    if (m_timeWidget && m_timeWidget->enabled()) {
        qInfo() << "unload plugin";
        qCDebug(dsrApp) << "Time widget enabled, unloading plugin.";
        m_proxyInter->itemRemoved(this, pluginName());
        m_bshow = false;
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
        // 首次进入录屏状态时，安装服务监听器以便主进程崩溃时及时收尾。
        // 主进程正常停止会主动通过 DBus 调用 onStop，无需 watcher 介入。
        if (nullptr == m_serviceWatcher) {
            qCDebug(dsrApp) << "Installing QDBusServiceWatcher for main recorder process";
            m_serviceWatcher = new QDBusServiceWatcher(
                "com.deepin.ScreenRecorder",
                QDBusConnection::sessionBus(),
                QDBusServiceWatcher::WatchForUnregistration,
                this);
            connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered,
                    this, [this](const QString &) {
                qCInfo(dsrApp) << "main recorder service unregistered, stopping plugin";
                onStop();
            });
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
