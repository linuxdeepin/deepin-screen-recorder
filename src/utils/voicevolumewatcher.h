// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VOICEVOLUMEWATCHER_H
#define VOICEVOLUMEWATCHER_H

#include <com_deepin_daemon_audio.h>
#include <com_deepin_daemon_audio_source.h>
#include <com_deepin_daemon_audio_sink.h>

#include <QObject>
#include <QTimer>

class voiceVolumeWatcher : public QObject
{
    Q_OBJECT
public:
    explicit voiceVolumeWatcher(QObject *parent = nullptr);
    ~voiceVolumeWatcher();
    void setWatch(const bool isWatcher);
    void setIsRecoding(bool value);
    // 将原来的run()方法改为定时器的槽函数，便于截图快速退出
    // 取消之前的线程方式，采用定时器监测
    void slotvoiceVolumeWatcher();
    /**
     * @brief 返回当前是否有系统声卡
     * @return
     */
    bool getystemAudioState();
protected:
    void initDeviceWatcher();
    void onCardsChanged(const QString &value);
    void initAvailInputPorts(const QString &cards);
    void initConnections();
    void onDefaultSourceChanaged(const QDBusObjectPath &value);
    bool isMicrophoneAvail(const QString &activePort) const;

signals:
    void sigRecodeState(bool couldUse);
    /* void inputSourceChanged(const QString &name); */

private:

    //音频服务名
    const QString m_serviceName {"com.deepin.daemon.Audio"};

    /**
     * @brief 音频接口对象
     * 对应为com.deepin.daemon.Audio
     */
    QScopedPointer<com::deepin::daemon::Audio> m_audioInterface;
    /**
     * @brief 音频源(输入源)
     */
    QScopedPointer<com::deepin::daemon::audio::Source> m_defaultSource;
    struct Port {
        QString  portId;
        QString portName;
        QString cardName;
        int     available {1};
        int     cardId;
        bool    isActive {false};

        bool isInputPort() const;
        bool isLoopback() const;
    };
    friend QDebug &operator << (QDebug &out, const Port &port);

    //All available input ports.except loopback port.
    QMap<QString, Port> m_availableInputPorts;
    //bool m_isRecoding;
    bool m_coulduse;
    QTimer *m_watchTimer = nullptr; //新增麦克风定时检测

    /**
     * @brief 是否存在系统声卡
     */
    bool m_isExistSystemAudio ;
};


#endif // VOICEVOLUMEWATCHER_H
