// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUDIOUTILS_H
#define AUDIOUTILS_H

#include <QDebug>
#include <QtDBus/QtDBus>
#include <QObject>
#include <QProcess>

//暂时使用这个头文件，后续修改
#include </usr/include/libdframeworkdbus-2.0/types/audioport.h>

/**
 * @brief AudioService 音频服务名 org.deepin.dde.Audio1
 */
const static QString AudioService = QStringLiteral("org.deepin.dde.Audio1");
/**
 * @brief AudioPath 服务地址 /org/deepin/dde/Audio1
 */
const static QString AudioPath = QStringLiteral("/org/deepin/dde/Audio1");
/**
 * @brief AudioInterface 接口名称 org.deepin.dde.Audio1
 */
const static QString AudioInterface = QStringLiteral("org.deepin.dde.Audio1");
/**
 * @brief SinkInterface 音频输出接口名称 org.deepin.dde.Audio1.Sink
 */
const static QString SinkInterface = QStringLiteral("org.deepin.dde.Audio1.Sink");
/**
 * @brief SourceInterface 音频输入接口名称 org.deepin.dde.Audio1.Source
 */
const static QString SourceInterface = QStringLiteral("org.deepin.dde.Audio1.Source");
/**
 * @brief PropertiesInterface 属性接口名称 org.freedesktop.DBus.Properties
 */
const static QString PropertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
/**
 * @brief PropertiesChanged 属性改变信号
 */
const static QString PropertiesChanged = QStringLiteral("PropertiesChanged");

/**
 * @brief The AudioUtils class 音频服务工具类
 * 获取音频服务的相关数据
 * 音频Dbus服务: org.deepin.dde.Audio1
 * 音频服务输入服务: org.deepin.dde.Audio1.SourceX  X---序号
 * 音频服务输出: org.deepin.dde.Audio1.SinkX  X---序号
 */
class AudioUtils : public QObject
{
    Q_OBJECT
public:
    enum DefaultAudioType {
        Source = 0,  //输入音频
        Sink,        //输出音频
        NoneType     //无声音
    };

public:
    explicit AudioUtils(QObject *parent = nullptr);

    /**
     * @brief currentAudioChannel 获取当前系统音频通道
     * @return
     */
    QString currentAudioChannel();

    /**
     * @brief 获取默认输出或输入设备名称
     * @param mode: 0:获取输出音频设备 1:获取输入音频设备
     * @return 设备名称
     */
    QString getDefaultDeviceName(DefaultAudioType mode);

    /**
     * @brief audioDBusInterface 音频dbus服务的接口
     * @return
     */
    QDBusInterface *audioDBusInterface();

    /**
     * @brief defaultSourceDBusInterface 音频dbus服务默认输入源的接口
     * @return
     */
    QDBusInterface *defaultSourceDBusInterface();

    /**
     * @brief defaultSourceDBusInterface 音频dbus服务默认输出源的接口
     * @return
     */
    QDBusInterface *defaultSinkDBusInterface();

    /**
     * @brief activePortSource 音频dbus服务默认输入源的活跃端口
     * @return
     */
    AudioPort defaultSourceActivePort();

    /**
     * @brief activePortSource 音频dbus服务默认输入源的音量
     * @return
     */
    double defaultSourceVolume();
    /**
     * @brief cards 音频dbus服务的声卡信息
     * @return
     */
    QString cards();

signals:
    /**
     * @brief defaultSourceChanaged 默认输入源被改变时发出
     */
    void defaultSourceChanaged();
    /**
     * @brief cardsChanged 声卡信息被改变时发出
     * @param cards
     */
    void cardsChanged(const QString &cards);

public slots:
    /**
     * @brief onDBusAudioPropertyChanged 音频dbus服务属性改变时触发
     * @param msg
     */
    void onDBusAudioPropertyChanged(QDBusMessage msg);

protected:
    /**
     * @brief initAudioDBusInterface 初始化音频dbus服务的接口
     */
    void initAudioDBusInterface();

    /**
     * @brief initDefaultSourceDBusInterface 初始化音频dbus服务默认输入源的接口
     */
    void initDefaultSourceDBusInterface();

    /**
     * @brief initDefaultSourceDBusInterface 初始化音频dbus服务默认输出源的接口
     */
    void initDefaultSinkDBusInterface();

    /**
     * @brief initConnections 初始化音频dbus服务属性改变链接
     */
    void initConnections();

    /**
     * @brief currentAudioChannel 获取当前系统音频通道
     * @return
     */
    QString currentAudioChannelV20Impl();

    /**
     * @brief 获取默认输出或输入设备名称
     * @param mode: 0:获取输出音频设备 1:获取输入音频设备
     * @return 设备名称
     */
    QString getDefaultDeviceNameV20Impl(DefaultAudioType mode);

private:
    /**
     * @brief 音频服务dbus接口
     * 对应为 (org.deepin.dde.Audio)
     */
    QDBusInterface *m_audioDBusInterface = nullptr;

    /**
     * @brief 音频源(默认音频输入源接口)
     * 对应为 (org.deepin.dde.Audio.Source)
     */
    QDBusInterface *m_defaultSourceDBusInterface = nullptr;

    /**
     * @brief 音频源(默认音频输入源接口)
     * 对应为 (org.deepin.dde.Audio.Sink)
     */
    QDBusInterface *m_defaultSinkDBusInterface = nullptr;

    /**
     * @brief 默认输入音频服务地址 org.deepin.dde.Audio1.SourceX  X---序号
     */
    QString m_defaultSourcePath;

    /**
     * @brief 默认输出音频服务地址 org.deepin.dde.Audio1.SinkX  X---序号
     */
    QString m_defaultSinkPath;

    /**
     * @brief m_audioCards 音频服务支持的声卡信息
     */
    QString m_audioCards;
};

#endif  // AUDIOUTILS_H
