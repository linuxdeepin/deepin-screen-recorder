// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUDIOUTILS_H
#define AUDIOUTILS_H
#include <QObject>
#include <QProcess>
class AudioUtils: public QObject
{
    Q_OBJECT
public:
    enum DefaultAudioType {
        Source = 0,     //输入音频
        Sink,        //输出音频
        NoneType  //无声音
    };
public:
    explicit AudioUtils(QObject *parent = nullptr);
    //bool isSystemAudioOutput();
    //bool isMicrophoneOutput();
    //void setupMicrophoneOutput();
    //void setupSystemAudioOutput();
    //void setupAudioSink(QString sink);
    //bool canVirtualCardOutput();
    //static bool canMicrophoneInput();
    //QString currentAudioSink();
    //QString currentAudioSource();
    QString currentAudioChannel();

    /**
     * @brief 获取默认输出或输入设备名称
     * @param mode: 0:获取输出音频设备 1:获取输入音频设备
     * @return 设备名称
     */
    QString getDefaultDeviceName(DefaultAudioType mode);


private:
    //void initProcess();

private:
    //QProcess *process;

};

#endif // AUDIOUTILS_H
