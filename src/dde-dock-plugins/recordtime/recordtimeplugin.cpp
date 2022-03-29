/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
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

#include <QDesktopWidget>
#include "recordtimeplugin.h"

RecordTimePlugin::RecordTimePlugin(QObject *parent)
    : QObject(parent)
    , m_bshow(false)
{
    m_timer = new QTimer(this);
    m_timeWidget = new TimeWidget();
    m_checkTimer = nullptr;
}

RecordTimePlugin::~RecordTimePlugin()
{
    if (nullptr != m_timer)
        m_timer->deleteLater();
    if (nullptr != m_timeWidget)
        m_timeWidget->deleteLater();
    if (nullptr != m_checkTimer) {
        m_checkTimer->stop();
        m_checkTimer->deleteLater();
        m_checkTimer = nullptr;
    }
}

const QString RecordTimePlugin::pluginName() const
{
    return QString("deepin-screen-recorder-plugin");
}

const QString RecordTimePlugin::pluginDisplayName() const
{
    return QString("deepin-screen-recorder");
}

void RecordTimePlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;
    m_dBusService = new DBusService(this);
    connect(m_dBusService, SIGNAL(start()), this, SLOT(onStart()));
    connect(m_dBusService, SIGNAL(stop()), this, SLOT(onStop()));
    connect(m_dBusService, SIGNAL(recording()), this, SLOT(onRecording()));
    connect(m_dBusService, SIGNAL(pause()), this, SLOT(onPause()));
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.ScreenRecorder.time")
            && sessionBus.registerObject("/com/deepin/ScreenRecorder/time", this, QDBusConnection::ExportAdaptors)) {
        qDebug() << "dbus service registration failed!";
    }

    m_timer->start(600);
    connect(m_timer, &QTimer::timeout, this, &RecordTimePlugin::refresh);
    //test
    //onStart();
}

QWidget *RecordTimePlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_timeWidget;
}

void RecordTimePlugin::onStart()
{
    if (m_timeWidget->enabled()) {
        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemAdded(this, pluginName());
        m_bshow = true;
        m_timeWidget->start();
    }
}

void RecordTimePlugin::onStop()
{
    if (m_timeWidget->enabled()) {
        m_proxyInter->itemRemoved(this, pluginName());
        m_bshow = false;
        if (nullptr != m_checkTimer) {
            m_checkTimer->stop();
            m_checkTimer->deleteLater();
            m_checkTimer = nullptr;
        }
        m_count = 0;
        m_nextCount = 0;
        //m_timeWidget->stop();
    }
}

//当托盘插件开始闪烁计数时才会执行
void RecordTimePlugin::onRecording()
{
    if (m_timeWidget->enabled() && m_bshow) {
        m_nextCount++;
        if (1 == m_nextCount) {
            m_checkTimer = new QTimer();
            connect(m_checkTimer, &QTimer::timeout, this, [ = ] {
                //说明录屏还在进行中
                if (m_count < m_nextCount)
                {
                    m_count = m_nextCount;
                }
                //说明录屏已经停止了
                else
                {
                    onStop();
                }
            });
            m_checkTimer->start(2000);
        }
    }
}

void RecordTimePlugin::onPause()
{
    if (m_timeWidget->enabled() && m_bshow) {
        m_timeWidget->stop();
    }
}

void RecordTimePlugin::refresh()
{
    QSize size = m_timeWidget->sizeHint();
    if (size.width() > m_timeWidget->width()
            && 1 != position()
            && 3 != position()
            && m_bshow) {
        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemAdded(this, pluginName());
    }
}








