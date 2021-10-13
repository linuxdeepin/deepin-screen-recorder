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

#ifndef VOICEVOLUMEWATCHER_H
#define VOICEVOLUMEWATCHER_H

#include <com_deepin_daemon_audio.h>
#include <com_deepin_daemon_audio_source.h>

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
    const QString m_serviceName {"com.deepin.daemon.Audio"};

    QScopedPointer<com::deepin::daemon::Audio> m_audioInterface;
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
};


#endif // VOICEVOLUMEWATCHER_H
