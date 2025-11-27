// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTCAPTURESESSION_H
#define EXTCAPTURESESSION_H

#include <QObject>
#include <QSize>
#include <QList>

QT_BEGIN_NAMESPACE
class QWaylandOutput;
QT_END_NAMESPACE

class ExtCaptureFrame;
struct wl_shm;

/**
 * @brief 图像捕获会话配置
 */
struct CaptureConfig {
    QSize bufferSize;           // 缓冲区大小
    uint32_t format = 0;        // 像素格式
    bool useDmaBuffer = false;  // 是否使用DMA缓冲区
    bool paintCursors = false;  // 是否绘制光标
};

/**
 * @brief ext-image-copy-capture捕获会话
 * 
 * 管理单次录屏会话的生命周期，处理缓冲区约束和帧捕获
 */
class ExtCaptureSession : public QObject
{
    Q_OBJECT

public:
    enum SessionState {
        Uninitialized,  // 未初始化
        Initializing,   // 初始化中
        Ready,          // 就绪
        Capturing,      // 捕获中
        Stopped,        // 已停止
        Error           // 错误状态
    };

    explicit ExtCaptureSession(QObject *parent = nullptr);
    ~ExtCaptureSession();

    /**
     * @brief 初始化会话
     * @param manager 协议管理器
     * @param imageSource 图像捕获源
     * @param paintCursors 是否绘制光标
     * @return 是否成功初始化
     */
    bool initialize(void *manager, void *imageSource, bool paintCursors);

    /**
     * @brief 获取会话状态
     */
    SessionState state() const;

    /**
     * @brief 获取缓冲区配置
     */
    const CaptureConfig& config() const;

    /**
     * @brief 创建捕获帧
     * @return 捕获帧对象，失败返回nullptr
     */
    ExtCaptureFrame* createFrame();

    /**
     * @brief 停止捕获会话
     */
    void stop();

    /**
     * @brief 获取 Wayland SHM 对象
     * @return wl_shm 对象，用于创建共享内存缓冲区
     */
    wl_shm* getWaylandShm() const;

    /**
     * @brief 获取支持的像素格式列表
     */
    QList<uint32_t> supportedFormats() const;

signals:
    /**
     * @brief 会话就绪，可以开始捕获
     */
    void ready();

    /**
     * @brief 会话停止
     */
    void stopped();

    /**
     * @brief 捕获错误
     */
    void error(const QString &message);

    /**
     * @brief 帧就绪
     */
    void frameReady(ExtCaptureFrame *frame);

protected:
    // 协议事件处理
    void handleBufferSize(uint32_t width, uint32_t height);
    void handleShmFormat(uint32_t format);
    void handleDmabufDevice(const QByteArray &device);
    void handleDmabufFormat(uint32_t format, const QList<uint64_t> &modifiers);
    void handleDone();
    void handleStopped();

private:
    void setState(SessionState newState);
    void selectOptimalFormat();

    class Private;
    Private *d;
};

#endif // EXTCAPTURESESSION_H
