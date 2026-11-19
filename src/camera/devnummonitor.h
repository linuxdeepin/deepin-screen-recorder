// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVNUMMONITOR_H
#define DEVNUMMONITOR_H

#include <QThread>
#include <QTimer>

#if (QT_VERSION_MAJOR == 5)
#include <QCameraInfo>
#elif (QT_VERSION_MAJOR == 6)
#include <QMediaDevices>
#include <QCameraDevice>
#endif

/**
* @brief DevNumMonitor　设备数目监视器
*/
class DevNumMonitor: public QObject
{
    Q_OBJECT
public:
    explicit DevNumMonitor();

    ~DevNumMonitor();


    void setWatch(const bool isWatcher);

    /**
     * @brief setCanUse 控制是否执行摄像头占用探测
     *
     * 录屏应用未打开摄像头时允许探测；使用摄像头期间应关闭探测，避免
     * 探测操作干扰当前采集。
     * @param canUse
     */
    void setCanUse(bool canUse);

    /**
     * @brief getAvailableCamera 获取可用的摄像头名称
     * @return
     */
    QString availableCamera();

    /**
     * @brief 判断指定设备节点当前是否仍在设备列表中
     *
     * 录制期间 m_canUse 为 false，timeOutSlot() 不会重新探测设备是否可用；
     * 该接口只检查设备节点是否仍被 Qt 枚举，用于发现当前选中的
     * 摄像头被拔出但还有其他摄像头在线的场景。
     */
    bool isDevicePresent(const QString &device) const;
signals:
    /**
     * @brief existDevice 存在摄像头设备信号
     * @param isExist: false:不存在 true:存在
     */
    void existDevice(bool isExist);

protected:
    /**
    * @brief run 运行
    */
    //void run();
    /**
    * @brief init 初始化
    */
    void init();
private slots:
    /**
    * @brief timeOutSlot 监视设备数目槽，根据相机数目执行相应的处理
    */
    void timeOutSlot();

private:
    QTimer             *m_pTimer;
    bool               m_noDevice;
    bool m_canUse;

    /**
     * @brief m_availableCamera 可用的摄像头名称
     */
    QString m_availableCamera = "";

};

#endif // DEVNUMMONITOR_H
