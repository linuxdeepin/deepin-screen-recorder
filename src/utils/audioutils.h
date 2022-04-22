/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
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
