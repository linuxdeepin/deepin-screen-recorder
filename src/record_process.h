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

#ifndef RECORDPROCESS_H
#define RECORDPROCESS_H

#include "utils/configsettings.h"
#ifdef KF5_WAYLAND_FLAGE_ON
#include "waylandrecord/waylandintegration.h"
#endif
#include <QProcess>
#include <QObject>
#include <QRect>
#include <QTimer>

//不需要开启线程，用信号槽代替 process->waitForFinished(-1); 避免线程等待浪费系统资源
/**
 * @brief The RecordProcess class 录屏的控制类
 * 录屏分类两种情况：x11下的录屏 wayland下的录屏
 * x11录屏：通过使用ffmpeg命令进行录屏
 * wayland录屏：通过调用wayland的接口获取屏幕的的帧，进行裁剪后，通过ffmpeg的接口合成视频
 */
class RecordProcess  : public QObject
{
    Q_OBJECT

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

    explicit RecordProcess(QObject *parent = nullptr);
    ~RecordProcess();

    /**
     * @brief 设置录屏的基本信息
     * @param 录屏的范围
     * @param filename
     */
    void setRecordInfo(const QRect &recordRect, const QString &filename);
    /**
     * @brief 开始录屏
     */
    void startRecord();
    /**
     * @brief 停止录屏
     */
    void stopRecord();

private:
    /**
     * @brief x11协议下录制视频
     */
    void recordVideo();

    /**
     * @brief 开始录制wayland视频
     */
    void waylandRecord();

    /**
     * @brief 初始化进程
     */
    void initProcess();
public slots:
    /**
     * @brief onRecordFinish:是否录制光标
     */
    void onRecordMouse(const bool status);
    /**
     * @brief 通过工具栏设置是否打开麦克风音频录音
     * @param status
     */
    void setMicrophone(const bool status);
    /**
     * @brief 通过工具设置是否打开系统音频录音
     * @param status
     */
    void setSystemAudio(const bool status);

private slots:
    /**
     * @brief onRecordFinish:录屏完成
     */
    void onRecordFinish();

    /**
     * @brief onStartTranscode:开始转码
     */
    void onStartTranscode();

    /**
     * @brief onTranscodeFinish:转码完成
     */
    void onTranscodeFinish();

private:
    /**
     * @brief x11录屏进程
     */
    QProcess *m_recorderProcess = nullptr;

    /**
     * @brief 录屏的类型：gif mkv mp4
     */
    int recordType = 0;

    /**
     * @brief 录制的声音类型： 混音 单麦克风音频 单系统音频
     */
    int recordAudioInputType = 0;

    /**
     * @brief 是否录制鼠标
     */
    bool m_isRecordMouse = true;

    /**
     * @brief 录屏的范围
     */
    QRect m_recordRect;

    QString savePath;
    QString saveBaseName;
    QString saveTempDir;
    QString saveDir;
    QString saveAreaName;
    QString displayNumber;

    ConfigSettings *settings = nullptr;

    int byzanzProcessId = 0;

    /**
     * @brief 录屏的帧率
     */
    int m_framerate;

    /**
     * @brief 是否选择麦克风音频
     */
    bool m_selectedMic = false;
    /**
     * @brief 是否选择系统音频
     */
    bool m_selectedSystemAudio = true;

    /**
     * @brief mp4转码成gif的进程
     */
    QProcess *m_pTranscodeProcess = nullptr;
    /**
     * @brief 定时发送录屏正在运行的信号
     */
    QTimer *m_timer ;
};

#endif //RECORDPROCESS_H
