/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
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

#ifndef RECORDPROCESS_H
#define RECORDPROCESS_H

#include "settings.h"
#include <QProcess>
#include <QThread>
#include <QRect>
#include <proc/readproc.h>
#include <proc/sysinfo.h>
#include "utils/desktopinfo.h"
#include "desktopportal.h"

class RecordProcess : public QThread
{
    Q_OBJECT

    typedef std::map<int, proc_t> StoredProcType;

public:
    static const int RECORD_TYPE_VIDEO;
    static const int RECORD_TYPE_GIF;
    static const int RECORD_TYPE_MP4;
    static const int RECORD_TYPE_MKV;
    static const int RECORD_GIF_SLEEP_TIME;
    static const int RECORD_AUDIO_INPUT_MIC;
    static const int RECORD_AUDIO_INPUT_SYSTEMAUDIO;
    static const int RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO;

    static const int RECORD_FRAMERATE_5;
    static const int RECORD_FRAMERATE_10;
    static const int RECORD_FRAMERATE_20;
    static const int RECORD_FRAMERATE_24;
    static const int RECORD_FRAMERATE_30;

    RecordProcess(QObject *parent = nullptr);

    void setRecordInfo(const QRect &recordRect, const QString &filename);
    void setRecordType(int recordType);
    void setFrameRate(int framerate);
    void setRecordAudioInputType(int inputType);
    void startRecord();
    void setIsZhaoXinPlatform(bool status);
    void stopRecord();
    void recordGIF();
    void recordVideo();
    void initProcess();
    int readSleepProcessPid();
    void WaylandRecord();
protected:
    void run();

private:
    QProcess *process;

    int recordType;
    int recordAudioInputType;

    QRect m_recordRect;

    QString savePath;
    QString saveBaseName;
    QString saveTempDir;
    QString saveDir;
    QString defaultSaveDir;
    QString saveAreaName;
    QString displayNumber;

    Settings *settings;

    int byzanzProcessId;
    int sleepProcessId;
    int m_framerate;

    QString lastAudioSink;
    bool m_isZhaoxin = false;
    DesktopInfo m_info;
    //wayland 录屏模块
    DesktopPortal *m_pDesktopPortal;
};

#endif //RECORDPROCESS_H
