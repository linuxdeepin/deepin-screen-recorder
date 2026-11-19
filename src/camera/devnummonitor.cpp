// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devnummonitor.h"
#include "../utils/log.h"
#include "cameraavailabilityprobe.h"

#include <QDebug>
#include <QString>

#if (QT_VERSION_MAJOR == 5)
#include <QCameraInfo>
#elif (QT_VERSION_MAJOR == 6)
#include <QMediaDevices>
#include <QCameraDevice>
#endif

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

void DevNumMonitor::timeOutSlot()
{
    qCDebug(dsrApp) << "Entry: DevNumMonitor::timeOutSlot";

#if (QT_VERSION_MAJOR == 5)
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
#elif (QT_VERSION_MAJOR == 6)
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
#endif

    if (cameras.isEmpty()) {
        m_availableCamera.clear();
        qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - No devices found.";
        //没有设备发送信号
        if (!m_noDevice) {
            qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - Emitting existDevice(false).";
            emit existDevice(false);
            m_noDevice = true;
        }
    } else {
        qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - Devices found. Number of devices:" << cameras.size();
        int isExistAvailableCameras = 0;
        if (m_canUse) {
            m_availableCamera.clear();
            qCDebug(dsrApp) << "Log: DevNumMonitor::timeOutSlot - m_canUse is true. Checking available cameras.";
#if (QT_VERSION_MAJOR == 5)
            for (const QCameraInfo &camera : cameras) {
                if (CameraAvailabilityProbe::isDeviceAvailable(camera.deviceName(),
                                                               camera.description())) {
                    ++isExistAvailableCameras;
                    m_availableCamera = camera.deviceName();
                    break;
                }
            }
#elif (QT_VERSION_MAJOR == 6)
            for (const QCameraDevice &camera : cameras) {
                const QString deviceId = QString::fromUtf8(camera.id());
                if (CameraAvailabilityProbe::isDeviceAvailable(deviceId,
                                                               camera.description())) {
                    ++isExistAvailableCameras;
                    m_availableCamera = deviceId;
                    break;
                }
            }
#endif
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

bool DevNumMonitor::isDevicePresent(const QString &device) const
{
    if (device.isEmpty())
        return false;

#if (QT_VERSION_MAJOR == 5)
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cam : cameras) {
        if (cam.deviceName() == device)
            return true;
    }
#elif (QT_VERSION_MAJOR == 6)
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const QCameraDevice &cam : cameras) {
        if (QString::fromUtf8(cam.id()) == device)
            return true;
    }
#endif
    return false;
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
