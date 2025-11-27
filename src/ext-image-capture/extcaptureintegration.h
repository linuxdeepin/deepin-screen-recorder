// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTCAPTUREINTEGRATION_H
#define EXTCAPTUREINTEGRATION_H

#include <QObject>
#include <QScreen>

class ExtCaptureManager;
class ExtCaptureSession;
class ExtCaptureFrame;

/**
 * @brief ext-image-copy-capture集成接口
 * 
 * 提供统一的录屏捕获接口，简化上层调用
 */
class ExtCaptureIntegration : public QObject
{
    Q_OBJECT

public:
    explicit ExtCaptureIntegration(QObject *parent = nullptr);
    ~ExtCaptureIntegration();

    /**
     * @brief 检查协议是否可用
     */
    bool isAvailable() const;

    /**
     * @brief 开始屏幕录制
     * @param screen 要录制的屏幕
     * @param includeCursor 是否包含光标
     * @return 是否成功开始
     */
    bool startScreenRecording(QScreen *screen, bool includeCursor = false);

    /**
     * @brief 停止录制
     */
    void stopRecording();

    /**
     * @brief 捕获单帧
     * @return 是否成功开始捕获
     */
    bool captureFrame();

    /**
     * @brief 获取当前会话状态
     */
    bool isRecording() const;

signals:
    /**
     * @brief 协议变为可用
     */
    void available();

    /**
     * @brief 协议变为不可用
     */
    void unavailable();

    /**
     * @brief 录制开始
     */
    void recordingStarted();

    /**
     * @brief 录制停止
     */
    void recordingStopped();

    /**
     * @brief 帧数据就绪
     * @param data 帧数据指针
     * @param size 数据大小
     * @param width 帧宽度
     * @param height 帧高度
     * @param stride 行字节数
     * @param timestamp 时间戳
     */
    void frameReady(const void *data, size_t size, int width, int height, int stride, uint64_t timestamp);

    /**
     * @brief 发生错误
     */
    void error(const QString &message);

private slots:
    void onProtocolAvailable();
    void onProtocolUnavailable();
    void onSessionReady();
    void onSessionStopped();
    void onSessionError(const QString &message);
    void onFrameReady(ExtCaptureFrame *frame);
    void onFrameFailed(const QString &reason);

private:
    ExtCaptureManager *m_manager;
    ExtCaptureSession *m_session;
    bool m_recording;
};

#endif // EXTCAPTUREINTEGRATION_H
