// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VOICEVOLUMEWATCHER_H
#define VOICEVOLUMEWATCHER_H

#include <QTimer>

#include <com_deepin_daemon_audio.h>
#include <com_deepin_daemon_audio_source.h>
#include <com_deepin_daemon_audio_sink.h>

#include "audioutils.h"

// audio service monitor
class voiceVolumeWatcher : public QObject
{
    Q_OBJECT

public:
    explicit voiceVolumeWatcher(QObject *parent = nullptr);
    ~voiceVolumeWatcher();

    // monitor audio change
    void setWatch(const bool isWatcher);
    // changed the original run() method to a timer for quick exit
    Q_SLOT void slotVoiceVolumeWatcher();
    // returns whether there is a system sound card or not
    bool getystemAudioState();

    Q_SIGNAL void sigRecodeState(bool couldUse);

protected:
    void onCardsChanged(const QString &value);
    void initAvailInputPorts(const QString &cards);
    bool isMicrophoneAvail(const QString &activePort) const;

    // For V20 or older
    void initV20DeviceWatcher();

private:
    // For v23 or later
    AudioUtils *m_audioUtils{nullptr};

    // For V20 or older
    // Audio interface com.deepin.daemon.Audio
    QScopedPointer<com::deepin::daemon::Audio> m_audioInterface;
    // Audio Source (Input Source)
    QScopedPointer<com::deepin::daemon::audio::Source> m_defaultSource;

    struct Port
    {
        QString portId;
        QString portName;
        QString cardName;
        int available{1};
        int cardId;
        bool isActive{false};

        bool isInputPort() const;
        bool isLoopback() const;
    };
    friend QDebug &operator<<(QDebug &out, const Port &port);

    // All available input ports.except loopback port.
    QMap<QString, Port> m_availableInputPorts;
    bool m_coulduse{false};

    QTimer *m_watchTimer{nullptr};     // microphone detection timer
    bool m_isExistSystemAudio{false};  // system sound card exists ? false by default

    bool m_version2{false};  // Use version 2.0 if system edition greater or equal V23
};

#endif  // VOICEVOLUMEWATCHER_H
