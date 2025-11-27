// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTCAPTUREINTEGRATION_H
#define EXTCAPTUREINTEGRATION_H

#include <QObject>
#include <QScreen>
#include <QList>
#include <QRect>

class ExtCaptureManager;
class ExtCaptureSession;
class ExtCaptureFrame;
class MultiScreenCaptureCoordinator;

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
     * @brief 开始多屏录制
     * @param screens 要录制的屏幕列表
     * @param includeCursor 是否包含光标
     * @return 是否成功开始
     */
    bool startMultiScreenRecording(const QList<QScreen*>& screens, bool includeCursor = false);

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
     * @brief 捕获多屏帧
     * @return 是否成功开始捕获
     */
    bool captureMultiScreenFrame();

    /**
     * @brief 获取当前会话状态
     */
    bool isRecording() const;

    /**
     * @brief 是否正在进行多屏录制
     */
    bool isMultiScreenRecording() const;

    /**
     * @brief 获取支持的屏幕列表
     */
    QList<QScreen*> getAvailableScreens() const;

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
     * @brief DMA Buffer帧就绪
     * @param dmaBufferFd DMA Buffer文件描述符
     * @param gbmBo GBM Buffer Object指针
     * @param size 数据大小
     * @param width 帧宽度
     * @param height 帧高度
     * @param stride 行字节数
     * @param timestamp 时间戳
     */
    void dmaFrameReady(int dmaBufferFd, void *gbmBo, size_t size, int width, int height, int stride, uint64_t timestamp);

    /**
     * @brief 多屏合成帧就绪
     * @param compositeFrameData 合成后的帧数据
     * @param width 帧宽度
     * @param height 帧高度
     * @param stride 行字节数
     * @param timestamp 时间戳
     */
    void multiScreenFrameReady(const QByteArray& compositeFrameData, int width, int height, int stride, uint64_t timestamp);

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

private slots:
    void onMultiScreenFrameReady(const QByteArray& compositeFrameData, int width, int height, int stride, uint64_t timestamp);
    void onMultiScreenCaptureStarted();
    void onMultiScreenCaptureStopped();
    void onMultiScreenCaptureError(const QString& message);

private:
    ExtCaptureManager *m_manager;
    ExtCaptureSession *m_session;
    MultiScreenCaptureCoordinator *m_multiScreenCoordinator;
    bool m_recording;
    bool m_multiScreenRecording;
};

#endif // EXTCAPTUREINTEGRATION_H
