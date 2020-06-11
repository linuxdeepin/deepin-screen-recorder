/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     Zheng Youge<youge.zheng@deepin.com>
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
#ifndef VOICE_RECORD_PROCESS_H
#define VOICE_RECORD_PROCESS_H
#include "settings.h"
#include <QProcess>
#include <QThread>
#include <QRect>
#include <proc/readproc.h>
#include <proc/sysinfo.h>
#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDateTime>
#include <QTimer>
class VoiceRecordProcess : public QThread
{
    Q_OBJECT

public:
    VoiceRecordProcess(QObject *parent = nullptr);

    QString generateRecordingFilepath();
    QString getRecordingFilepath();

    void exitRecord();
    void pauseRecord();
    void renderLevel(const QAudioBuffer &buffer);
    void resumeRecord();
    void startRecord();
    void stopRecord();
protected:
    void run();
private:
    QAudioProbe *audioProbe;
    QAudioRecorder *audioRecorder;
    QString recordPath;
    QDateTime lastUpdateTime;
    QTimer *tickerTimer;
    float recordingTime;
};
#endif // VOICE_RECORDER_PROCESS_H
