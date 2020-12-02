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

#include "utils/configsettings.h"
#include <QProcess>
#include <QThread>
#include <QRect>
#include <proc/readproc.h>
#include <proc/sysinfo.h>
#include "utils/desktopinfo.h"
#include "xgifrecord.h"


class RecordProcess : public QThread
{
    Q_OBJECT

    typedef std::map<int, proc_t> StoredProcType;

public:
    static const int RECORD_TYPE_VIDEO;
    static const int RECORD_TYPE_GIF;
    static const int RECORD_GIF_SLEEP_TIME;
    static const int RECORD_AUDIO_INPUT_MIC;
    static const int RECORD_AUDIO_INPUT_SYSTEMAUDIO;
    static const int RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO;

    static const int RECORD_FRAMERATE_5;
    static const int RECORD_FRAMERATE_10;
    static const int RECORD_FRAMERATE_20;
    static const int RECORD_FRAMERATE_24;
    static const int RECORD_FRAMERATE_30;

    explicit RecordProcess(QObject *parent = nullptr);
    ~RecordProcess();

    void setRecordInfo(const QRect &recordRect, const QString &filename);
    //void setRecordType(int recordType);
    //void setFrameRate(int framerate);
    void setRecordAudioInputType(int inputType);
    void startRecord();
    //void setIsZhaoXinPlatform(bool status);
    void stopRecord();
    void recordGIF();
    void recordVideo();
    void initProcess();
    int readSleepProcessPid();
protected:
    void run();

private:
    QProcess *process = nullptr;

    int recordType = 0;
    int recordAudioInputType = 0;

    QRect m_recordRect;

    QString savePath;
    QString saveBaseName;
    QString saveTempDir;
    QString saveDir;
    QString defaultSaveDir;
    QString saveAreaName;
    QString displayNumber;

    ConfigSettings *settings = nullptr;

    int byzanzProcessId = 0;
    int m_framerate;

    //QString lastAudioSink;
    //bool m_isZhaoxin = false;
    DesktopInfo m_info;
    //x11 gif 录屏模块
    XGifRecord *m_pXGifRecord = nullptr;
};

#endif //RECORDPROCESS_H
