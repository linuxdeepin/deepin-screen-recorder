// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTCAPTUREBRIDGE_H
#define EXTCAPTUREBRIDGE_H

#include <QObject>
#include <QTimer>
#include <QMutex>

class ExtCaptureRecorder;
class ExtCaptureFrameBuffer;
class RecordAdmin;


class ExtCaptureBridge : public QObject
{
    Q_OBJECT

public:
    explicit ExtCaptureBridge(QObject *parent = nullptr);
    ~ExtCaptureBridge();

    /**
     * @brief 设置ext-capture录屏器
     * @param recorder ExtCaptureRecorder实例
     */
    void setExtCaptureRecorder(ExtCaptureRecorder *recorder);

    /**
     * @brief 设置RecordAdmin编码器
     * @param recordAdmin RecordAdmin实例
     */
    void setRecordAdmin(RecordAdmin *recordAdmin);

    /**
     * @brief 开始桥接数据传输
     * @param screenWidth 屏幕宽度
     * @param screenHeight 屏幕高度
     * @return 成功返回true
     */
    bool startBridge(int screenWidth, int screenHeight);

    /**
     * @brief 停止桥接数据传输
     */
    void stopBridge();

    /**
     * @brief 检查是否正在桥接
     */
    bool isBridging() const;

    /**
     * @brief 获取已处理的帧数量
     */
    int getProcessedFrameCount() const;

    /**
     * @brief 获取桥接状态信息
     */
    QString getStatusInfo() const;

signals:
    /**
     * @brief 桥接开始
     */
    void bridgeStarted();

    /**
     * @brief 桥接停止
     */
    void bridgeStopped();

    /**
     * @brief 桥接错误
     */
    void bridgeError(const QString &message);

    /**
     * @brief 帧处理进度
     */
    void frameProcessed(int frameCount, qint64 timestamp);

private slots:
    /**
     * @brief 处理帧数据的定时器槽
     */
    void processFrames();

    /**
     * @brief ExtCaptureRecorder录制开始
     */
    void onRecordingStarted();

    /**
     * @brief ExtCaptureRecorder录制停止
     */
    void onRecordingStopped();

    /**
     * @brief ExtCaptureRecorder错误
     */
    void onRecordingError(const QString &message);

private:
    /**
     * @brief 初始化RecordAdmin
     */
    bool initializeRecordAdmin();

    /**
     * @brief 处理单个帧
     */
    bool processSingleFrame();

    /**
     * @brief 转换帧格式（如果需要）
     */
    bool convertFrameFormat(const unsigned char *srcData, int width, int height, int stride,
                           unsigned char *dstData, size_t dstSize);

    /**
     * @brief 计算帧时间戳
     */
    int64_t calculateFrameTimestamp();

    ExtCaptureRecorder *m_extCaptureRecorder;   // ext-capture录屏器
    ExtCaptureFrameBuffer *m_frameBuffer;       // 帧缓冲区
    RecordAdmin *m_recordAdmin;                 // 录制管理器
    QTimer *m_processTimer;                     // 帧处理定时器
    
    bool m_bridging;                            // 是否正在桥接
    bool m_recordAdminInitialized;              // RecordAdmin是否已初始化
    int m_processedFrameCount;                  // 已处理帧数
    int m_screenWidth;                          // 屏幕宽度
    int m_screenHeight;                         // 屏幕高度
    qint64 m_bridgeStartTime;                   // 桥接开始时间
    
    QMutex m_processMutex;                      // 处理锁
    
    // 统计信息
    int m_totalFramesReceived;                  // 总接收帧数
    int m_totalFramesProcessed;                 // 总处理帧数
    int m_droppedFrames;                        // 丢弃帧数
};

#endif // EXTCAPTUREBRIDGE_H
