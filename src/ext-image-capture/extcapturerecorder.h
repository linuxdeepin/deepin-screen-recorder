// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTCAPTURERECORDER_H
#define EXTCAPTURERECORDER_H

#include <QObject>
#include <QScreen>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QThread>
#include <QProcess>

// DMA Buffer相关头文件
#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <cstring>
#include <algorithm>
#include <QDateTime>

class ExtCaptureIntegration;
class ExtCaptureFrameBuffer;

// 使用ExtCaptureFrameBuffer中的ExtFrameData结构

/**
 * @brief ext-image-copy-capture录屏器
 * 
 * 使用ExtCaptureFrameBuffer管理帧数据，
 * 可以与现有的视频编码基础设施集成
 */
class ExtCaptureRecorder : public QObject
{
    Q_OBJECT

public:
    enum RecordState {
        Stopped,        // 停止状态
        Starting,       // 启动中
        Recording,      // 录制中
        Stopping,       // 停止中
        Error          // 错误状态
    };

    explicit ExtCaptureRecorder(QObject *parent = nullptr);
    ~ExtCaptureRecorder();

    /**
     * @brief 检查ext-capture是否可用
     */
    bool isAvailable() const;

    /**
     * @brief 开始录制
     * @param screen 要录制的屏幕
     * @param includeCursor 是否包含光标
     * @param outputPath 输出文件路径
     * @param frameRate 帧率(fps)
     * @return 是否成功开始
     */
    bool startRecording(QScreen *screen, bool includeCursor = false, 
                       const QString &outputPath = QString(), int frameRate = 30);

    /**
     * @brief 停止录制
     */
    void stopRecording();

    /**
     * @brief 获取录制状态
     */
    RecordState state() const;

    /**
     * @brief 获取录制的帧数
     */
    int frameCount() const;

    /**
     * @brief 获取录制时长(毫秒)
     */
    qint64 recordingDuration() const;

    /**
     * @brief 获取帧缓冲区 (用于集成现有编码系统)
     */
    ExtCaptureFrameBuffer* getFrameBuffer() const;

private:
    /**
     * @brief 创建视频文件
     */
    bool createVideoFile();

    /**
     * @brief 设置输出文件路径
     */
    void setOutputPath(const QString &path);

    /**
     * @brief 获取输出文件路径
     */
    QString outputPath() const;

signals:
    /**
     * @brief 协议变为可用
     */
    void protocolAvailable();

    /**
     * @brief 录制开始
     */
    void recordingStarted();

    /**
     * @brief 录制停止
     */
    void recordingStopped();

    /**
     * @brief 发生错误
     */
    void error(const QString &message);

    /**
     * @brief 录制进度更新
     */
    void progressUpdated(int frameCount, qint64 duration);

private slots:
    void onExtCaptureAvailable();
    void onExtCaptureUnavailable();
    void onRecordingStarted();
    void onRecordingStopped();
    void onFrameReady(const void *data, size_t size, int width, int height, int stride, uint64_t timestamp);
    void onDmaFrameReady(int dmaBufferFd, void *gbmBo, size_t size, int width, int height, int stride, uint64_t timestamp);
    void onExtCaptureError(const QString &message);
    void onCaptureTimer();

private:
    void setState(RecordState newState);
    void initializeCapture();
    void finalizeRecording();
    void processFrameQueue();
    QString generateDefaultOutputPath();
    bool startFFmpegProcess();
    bool startDmaFFmpegProcess();
    bool processDmaBufferFrame(int dmaBufferFd, void *gbmBo, int width, int height, int stride);

    ExtCaptureIntegration *m_extCapture;
    ExtCaptureFrameBuffer *m_frameBuffer;  // 保留用于兼容性
    QTimer *m_captureTimer;
    
    RecordState m_state;
    QString m_outputPath;
    int m_frameRate;
    int m_frameCount;
    qint64 m_startTime;
    bool m_includeCursor;
    
    // 录制参数
    int m_frameWidth;
    int m_frameHeight;
    int m_frameStride;
    
    // 流式编码相关
    QProcess *m_ffmpegProcess;
    bool m_streamingMode;
    bool m_ffmpegStarted;
    QByteArray m_firstFrameBuffer;
    
    // 首帧参数（用于DMA Buffer处理）
    int m_firstFrameWidth;
    int m_firstFrameHeight;
    int m_firstFrameStride;
};

#endif // EXTCAPTURERECORDER_H
