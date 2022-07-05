/*
* Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     shicetu <shicetu@uniontech.com>
*
* Maintainer: shicetu <shicetu@uniontech.com>
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

#include "devnummonitor.h"

#include <QDebug>
#include <QCameraInfo>
#include <QString>
extern "C"
{
#include "v4l2_devices.h"
#include "camview.h"
}

DevNumMonitor::DevNumMonitor()
{
    m_noDevice = false;
    m_pTimer = nullptr;
    m_canUse = true;
    init();
}

/**
 * @brief 之前的代码timer 在子线程里面创建，timeOutSlot在主线程运行，修改为在主线程创建timer，无需开启一个挂起的线程。
 * 注意： timeOutSlot函数会修改摄像机管理链表内存、摄像机链表未加锁，并且发送信号的几个处理函数都将使用到摄像机链表，所以timeOutSlot必须在主线中使用
 */
void DevNumMonitor::init()
{
    m_pTimer = new QTimer;
    m_pTimer->setInterval(500);
    connect(m_pTimer, &QTimer::timeout, this, &DevNumMonitor::timeOutSlot);
}

//void DevNumMonitor::run()
//{
//    m_pTimer = new QTimer;
//    m_pTimer->setInterval(500);
//    connect(m_pTimer, &QTimer::timeout, this, &DevNumMonitor::timeOutSlot);
//    QObject::connect(this, &QThread::destroyed, m_pTimer, &QTimer::deleteLater);
//    m_pTimer->start();
//    this->exec();
//    qDebug() << "Start monitoring the number of devices!";
//}

void DevNumMonitor::timeOutSlot()
{
    check_device_list_events(get_v4l2_device_handler());
    if (get_device_list()->num_devices < 1) {
        //没有设备发送信号
        if (!m_noDevice) {
            emit existDevice(false);
            m_noDevice = true;
        }
        qDebug() << "There is no camera connected!";
    } else {
        if (m_canUse && QCameraInfo::availableCameras().count() <= 0) {
            emit existDevice(false);
            m_noDevice = true;
            qDebug() << "There is no camera free!";
        } else {
            m_noDevice = false;
            emit existDevice(true);
            qDebug() << "There are " << get_device_list()->num_devices << " camera connected and free!";
        }
    }
}
void DevNumMonitor::setWatch(const bool isWatcher)
{
    if (isWatcher) {
        m_pTimer->start(1000);
    } else {
        m_pTimer->stop();
    }
}

void DevNumMonitor::setCanUse(bool canUse)
{
    m_canUse = canUse;
}
DevNumMonitor::~DevNumMonitor()
{
    if (nullptr != m_pTimer) {
        m_pTimer->stop();
        m_pTimer->deleteLater();
        m_pTimer = nullptr;
    }
}
