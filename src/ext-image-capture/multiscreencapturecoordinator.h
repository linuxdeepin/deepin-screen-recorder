// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTISCREENCAPTURECOORDINATOR_H
#define MULTISCREENCAPTURECOORDINATOR_H

#include <QObject>
#include <QScreen>
#include <QRect>
#include <QSize>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QTimer>
#include <QHash>
#include <QPoint>

#include "capturepipelineworker.h"

class ExtCaptureManager;
class ExtCaptureSession;
class ExtCaptureFrame;
class CapturePipelineWorker;
class QThread;

/**
 * @brief 多屏录制协调器
 * 
 * 管理多个屏幕的DMA Buffer捕获会话，协调帧同步和拼接
 * 基于waylandintegration.cpp的成功模式实现
 */
class MultiScreenCaptureCoordinator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 屏幕布局信息
     */
    struct ScreenLayout {
        QScreen* screen;                // 屏幕对象
        QString name;                   // 输出名称（纯数据，供工作线程使用）
        QRect geometry;                 // 在虚拟桌面中的位置
        ExtCaptureSession* session;     // 捕获会话
        bool frameReady;                // 帧准备标记
        bool includeCursor;             // 是否包含光标
        
        ScreenLayout() 
            : screen(nullptr), session(nullptr), frameReady(false), includeCursor(false) {}
    };

public:
    explicit MultiScreenCaptureCoordinator(QObject *parent = nullptr);
    ~MultiScreenCaptureCoordinator();

    /**
     * @brief 设置捕获管理器
     */
    void setCaptureManager(ExtCaptureManager* manager);

    /**
     * @brief 开始多屏录制
     * @param screens 要录制的屏幕列表
     * @param includeCursor 是否包含光标
     * @param cropRegion 全局坐标下的录制选区
     * @return 是否成功开始
     */
    bool startMultiScreenCapture(const QList<QScreen*>& screens,
                                 bool includeCursor = false,
                                 const QRect& cropRegion = QRect());

    /**
     * @brief 停止多屏录制
     */
    void stopMultiScreenCapture();

    /**
     * @brief 捕获一帧
     * @return 是否成功开始捕获
     */
    bool captureMultiScreenFrame();

    /**
     * @brief 获取虚拟桌面尺寸
     */
    QSize getVirtualDesktopSize() const { return m_virtualDesktopSize; }

    /**
     * @brief 获取屏幕布局信息
     */
    const QList<ScreenLayout>& getScreenLayouts() const { return m_screenLayouts; }

    /**
     * @brief 是否正在录制
     */
    bool isCapturing() const { return m_capturing; }

    /**
     * @brief 获取支持的屏幕列表
     */
    QList<QScreen*> getAvailableScreens() const;

signals:
    /**
     * @brief 多屏捕获开始
     */
    void captureStarted();

    /**
     * @brief 多屏捕获停止
     */
    void captureStopped();

    /**
     * @brief 发生错误
     */
    void error(const QString& message);

    /**
     * @brief 合成帧准备就绪
     * @param compositeFrameData 合成后的帧数据
     * @param width 帧宽度
     * @param height 帧高度
     * @param stride 行字节数
     * @param timestamp 时间戳
     */
    void compositeFrameReady(const QByteArray& compositeFrameData, 
                           int width, int height, int stride, uint64_t timestamp);

private slots:
    /**
     * @brief 处理单个屏幕的帧准备就绪
     */
    void onScreenFrameReady(QScreen* screen, int dmaBufferFd, void* gbmBo, 
                           int width, int height, int stride, size_t size, uint64_t timestamp);

    /**
     * @brief 处理单个屏幕的 SHM 帧
     */
    void onScreenShmFrameReady(QScreen* screen, ExtCaptureFrame* frame);

    /**
     * @brief 处理会话错误
     */
    void onSessionError(const QString& message);

    /**
     * @brief 处理会话停止
     */
    void onSessionStopped();

    /**
     * @brief 所有输出会话就绪
     */
    void onSessionReady();

    /**
     * @brief 一帧多屏合成完成
     */
    void onCompositeFrameReady(const QByteArray& compositeFrameData,
                               int width, int height, int stride, uint64_t timestamp);

    /**
     * @brief 采集工作线程完成一轮多屏合成
     */
    void onPipelineMultiFrameReady(const QByteArray& compositeFrameData,
                                   int width, int height, int stride,
                                   uint64_t timestamp, quint64 roundId,
                                   qint64 elapsedMs);

    /**
     * @brief 采集工作线程处理失败
     */
    void onPipelineMultiFrameFailed(const QString& message, quint64 roundId);

    /**
     * @brief 捕获帧超时，允许下一轮重新请求
     */
    void onFrameSyncTimeout();

private:
    /**
     * @brief 将 CPU 可访问帧提交给多屏合成器
     */
    void submitScreenFrame(QScreen* screen, const QByteArray& frameData,
                           int shmBufferFd, int dmaBufferFd, void* gbmBo,
                           int width, int height, int stride, size_t size,
                           uint64_t timestamp);

    /**
     * @brief 计算虚拟桌面布局
     */
    void calculateVirtualDesktopLayout(const QList<QScreen*>& screens);

    /**
     * @brief 清理会话
     */
    void cleanupSessions();

    /**
     * @brief 获取屏幕的Wayland输出
     */
    void* getWaylandOutput(QScreen* screen);

    /**
     * @brief 重置帧准备标记
     */
    void resetFrameReadyFlags();

    /**
     * @brief 从DMA Buffer提取像素数据
     * @param dmaBufferFd DMA Buffer文件描述符
     * @param gbmBo GBM Buffer Object指针
     * @param width 帧宽度
     * @param height 帧高度
     * @param stride 行字节数
     * @param size 数据总大小
     * @return 提取的像素数据
     */
    QByteArray extractDmaBufferData(int dmaBufferFd, void* gbmBo, int width, int height, int stride, size_t size);

    /**
     * @brief 创建/停止采集工作线程
     */
    void ensurePipeline();
    void stopPipeline();

    /**
     * @brief 本轮所有输出帧到齐后一次性交给工作线程
     */
    void dispatchRound();

private:
    ExtCaptureManager* m_manager;                           // 捕获管理器
    QList<ScreenLayout> m_screenLayouts;                    // 屏幕布局信息
    QSize m_virtualDesktopSize;                             // 虚拟桌面总尺寸
    QPoint m_virtualDesktopOrigin;                          // 虚拟桌面左上角全局坐标
    QRect m_pendingCropRegion;                              // 全局坐标录制选区
    bool m_capturing;                                       // 是否正在捕获
    bool m_captureInFlight = false;                         // 当前轮次是否等待帧
    bool m_captureStartedEmitted = false;                   // 是否已通知上层开始
    int m_readySessionCount = 0;                            // 已就绪输出数
    QTimer* m_syncTimer;                                    // 帧采集超时定时器
    QHash<QString, FrameInput> m_roundFrames;               // 当前轮次已到达的输出帧
    quint64 m_roundId = 0;                                  // 当前采集轮次
    CapturePipelineWorker* m_pipeline = nullptr;            // 采集/合成工作对象
    QThread* m_pipelineThread = nullptr;                    // 采集/合成工作线程
};

#endif // MULTISCREENCAPTURECOORDINATOR_H
