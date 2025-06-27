// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devnummonitor.h"
#include "../utils/log.h"

#include <QDebug>

#if (QT_VERSION_MAJOR == 5)
#include <QCameraInfo>
#elif (QT_VERSION_MAJOR == 6)
#include <QMediaDevices>
#endif

#include <QString>
extern "C"
{
#include "v4l2_devices.h"
#include "camview.h"
}

DevNumMonitor::DevNumMonitor()
{
    qCDebug(dsrApp) << "Entry: DevNumMonitor::DevNumMonitor";
    m_noDevice = false;
    m_pTimer = nullptr;
    m_canUse = true;
    init();
    qCDebug(dsrApp) << "Exit: DevNumMonitor::DevNumMonitor";
}

/**
 * @brief 之前的代码timer 在子线程里面创建，timeOutSlot在主线程运行，修改为在主线程创建timer，无需开启一个挂起的线程。
 * 注意： timeOutSlot函数会修改摄像机管理链表内存、摄像机链表未加锁，并且发送信号的几个处理函数都将使用到摄像机链表，所以timeOutSlot必须在主线中使用
 */
void DevNumMonitor::init()
{
    qCDebug(dsrApp) << "Entry: DevNumMonitor::init";
    m_pTimer = new QTimer;
    m_pTimer->setInterval(500);
    connect(m_pTimer, &QTimer::timeout, this, &DevNumMonitor::timeOutSlot);
    qCDebug(dsrApp) << "Exit: DevNumMonitor::init";
}

//void DevNumMonitor::run()
//{
//    m_pTimer = new QTimer;
//    m_pTimer->setInterval(500);
//    connect(m_pTimer, &QTimer::timeout, this, &DevNumMonitor::timeOutSlot);
//    QObject::connect(this, &QThread::destroyed, m_pTimer, &QTimer::deleteLater);
//    m_pTimer->start();
//    this->exec();
//    qCDebug(dsrApp) << "Start monitoring the number of devices!";
//}

void DevNumMonitor::timeOutSlot()
{
    qCDebug(dsrApp) << "Entry: DevNumMonitor::timeOutSlot";
    check_device_list_events(get_v4l2_device_handler());
    if (get_device_list()->num_devices < 1) {
        qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - No devices found.";
        //没有设备发送信号
        if (!m_noDevice) {
            qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - Emitting existDevice(false).";
            emit existDevice(false);
            m_noDevice = true;
        }
        //qCDebug(dsrApp) << "There is no camera connected!";
    } else {
        qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - Devices found. Number of devices:" << get_device_list()->num_devices;
        //v23特有处理，由于QCameraInfo::availableCameras().count()在v23上无法获取正确结果故更换为此方式
        int isExistAvailableCameras = 0;
        //qCDebug(dsrApp) << "m_canUse: " << m_canUse;
        if (m_canUse) {
            qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - m_canUse is true. Checking available cameras.";
            for (int i = 0; i < get_device_list()->num_devices; i++) {
                qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - Checking device:" << get_device_list()->list_devices[i].device << "at index:" << i;
                int ret = camInit(get_device_list()->list_devices[i].device);
                camUnInit();
                if (ret == 0) {
                    qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - Device is available:" << get_device_list()->list_devices[i].device;
                    isExistAvailableCameras++;
                    m_availableCamera = get_device_list()->list_devices[i].device;
                }
            }
        }
        if (m_canUse && isExistAvailableCameras == 0) {
            qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - m_canUse is true and no available cameras found. Emitting existDevice(false).";
            emit existDevice(false);
            m_noDevice = true;
            qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - There is no camera free!";
        } else {
            qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - Available cameras found. Emitting existDevice(true).";
            m_noDevice = false;
            emit existDevice(true);
            //qCDebug(dsrApp) << "There are " << isExistAvailableCameras << " camera connected and free!";
        }
    }
    qCDebug(dsrApp) << "Exit: DevNumMonitor::timeOutSlot";
}
void DevNumMonitor::setWatch(const bool isWatcher)
{
    qCDebug(dsrApp) << "Entry: DevNumMonitor::setWatch - isWatcher:" << isWatcher;
    if (isWatcher) {
        qCDebug(dsrApp) << "Log: DevNumMonitor::setWatch - Starting timer.";
        m_pTimer->start(1000);
    } else {
        qCDebug(dsrApp) << "Log: DevNumMonitor::setWatch - Stopping timer.";
        m_pTimer->stop();
    }
    qCDebug(dsrApp) << "Exit: DevNumMonitor::setWatch";
}

void DevNumMonitor::setCanUse(bool canUse)
{
    qCDebug(dsrApp) << "Entry: DevNumMonitor::setCanUse - canUse:" << canUse;
    m_canUse = canUse;
    qCDebug(dsrApp) << "Exit: DevNumMonitor::setCanUse";
}

QString DevNumMonitor::availableCamera()
{
    qCDebug(dsrApp) << "Entry: DevNumMonitor::availableCamera";
    qCDebug(dsrApp) << "Exit: DevNumMonitor::availableCamera - Available camera:" << m_availableCamera;
    return m_availableCamera;
}
DevNumMonitor::~DevNumMonitor()
{
    qCDebug(dsrApp) << "Entry: DevNumMonitor::~DevNumMonitor";
    if (nullptr != m_pTimer) {
        qCDebug(dsrApp) << "Log: DevNumMonitor::~DevNumMonitor - Stopping and deleting timer.";
        m_pTimer->stop();
        m_pTimer->deleteLater();
        m_pTimer = nullptr;
    }
    qCDebug(dsrApp) << "Exit: DevNumMonitor::~DevNumMonitor";
}
