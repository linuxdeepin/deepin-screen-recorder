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

class ExtCaptureManager;
class ExtCaptureSession;
class MultiScreenFrameCompositor;

// Forward declaration for DmaFrameInfo
struct DmaFrameInfo;

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
        QRect geometry;                 // 在虚拟桌面中的位置
        ExtCaptureSession* session;     // 捕获会话
        bool frameReady;                // 帧准备标记
        bool includeCursor;             // 是否包含光标
        
        ScreenLayout() 
            : screen(nullptr), session(nullptr), frameReady(false), includeCursor(false) {}
    };

    /**
     * @brief 帧数据结构
     */
    struct ScreenFrameData {
        int dmaBufferFd;        // DMA Buffer文件描述符
        void* gbmBo;            // GBM Buffer Object指针
        int width;              // 帧宽度
        int height;             // 帧高度
        int stride;             // 行字节数
        size_t size;            // 数据大小
        uint64_t timestamp;     // 时间戳
        bool ready;             // 数据是否准备好
        
        ScreenFrameData() 
            : dmaBufferFd(-1), gbmBo(nullptr), width(0), height(0), 
              stride(0), size(0), timestamp(0), ready(false) {}
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
     * @return 是否成功开始
     */
    bool startMultiScreenCapture(const QList<QScreen*>& screens, bool includeCursor = false);

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
     * @brief 处理会话错误
     */
    void onSessionError(const QString& message);

    /**
     * @brief 处理会话停止
     */
    void onSessionStopped();

    /**
     * @brief 检查帧同步和触发合成
     */
    void checkFrameSyncAndCompose();

private:
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
     * @brief 检查所有屏幕帧是否准备就绪
     * 基于waylandintegration.cpp的flag检查模式
     */
    bool allScreenFramesReady() const;

    /**
     * @brief 重置帧准备标记
     */
    void resetFrameReadyFlags();

    /**
     * @brief 合成多屏帧数据
     * 基于waylandintegration.cpp的成功拼接模式
     */
    QByteArray composeMultiScreenFrame();

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

private:
    ExtCaptureManager* m_manager;                           // 捕获管理器
    QList<ScreenLayout> m_screenLayouts;                    // 屏幕布局信息
    QMap<QScreen*, ScreenFrameData> m_screenFrames;         // 屏幕帧数据缓存
    QSize m_virtualDesktopSize;                             // 虚拟桌面总尺寸
    bool m_capturing;                                       // 是否正在捕获
    QMutex m_frameMutex;                                    // 帧数据保护锁
    MultiScreenFrameCompositor* m_compositor;               // 帧拼接器（后续实现）
    QTimer* m_syncTimer;                                    // 同步定时器
};

#endif // MULTISCREENCAPTURECOORDINATOR_H
