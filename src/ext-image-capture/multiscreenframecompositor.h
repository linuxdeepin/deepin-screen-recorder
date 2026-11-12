// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MULTISCREENFRAMECOMPOSITOR_H
#define MULTISCREENFRAMECOMPOSITOR_H

#include <QObject>
#include <QImage>
#include <QRect>
#include <QSize>
#include <QByteArray>
#include <QVector>
#include <QPair>
#include <QPainter>
#include <QMutex>
#include <QMutexLocker>
#include <QScreen>
#include <QString>

/**
 * @brief 与 QScreen 解耦的屏幕布局。
 *
 * 合成器在采集工作线程中运行，不能访问 GUI 线程的 QScreen 对象，
 * 因此布局信息以纯数据形式传入。
 */
struct ScreenLayoutInfo {
    QString name;
    QRect geometry;  // 虚拟桌面坐标系下的逻辑像素位置
};

// DMA Buffer frame information structure
struct DmaFrameInfo {
    // QScreen 仅用于兼容旧的 GUI 线程调用和单元测试；工作线程路径只使用
    // screenName，绝不能在这里解引用 QScreen。
    QScreen* screen = nullptr;
    QString screenName;
    QRect screenGeometry; // Screen position in virtual desktop
    quint64 roundId = 0;
    int dmaBufferFd = -1;
    void* gbmBo = nullptr;
    int width = 0;
    int height = 0;
    int stride = 0;
    size_t size = 0;
    uint64_t timestamp = 0;
    QByteArray frameData; // CPU-accessible pixel data
    bool ready = false;
};

/**
 * @brief Multi-screen frame compositor based on waylandintegration.cpp success pattern
 * 
 * This class implements real-time frame stitching for multi-screen recording.
 * It follows the proven approach from waylandintegration.cpp:1039-1047 where
 * QPainter is used to draw multiple QImage frames onto a single canvas.
 * 
 * Key design principles:
 * 1. Use QPainter for efficient GPU-accelerated composition
 * 2. Support arbitrary screen layouts and geometries  
 * 3. Handle DMA Buffer to QImage conversion
 * 4. Provide cropping support for region recording
 */
class MultiScreenFrameCompositor : public QObject
{
    Q_OBJECT

public:
    explicit MultiScreenFrameCompositor(QObject *parent = nullptr);
    ~MultiScreenFrameCompositor();

    // Configuration methods
    void setVirtualDesktopSize(const QSize& size);
    // 旧接口：仅在 GUI 线程/单元测试中使用。
    void setScreenLayouts(const QList<QScreen*>& screens);
    // 工作线程接口：布局信息全部为纯数据。
    void setScreenLayouts(const QVector<ScreenLayoutInfo>& layouts);
    void setCropRegion(const QRect& region); // For region recording
    void clearCropRegion(); // For full-screen recording

    // Physical-pixel canvas scale. The captured frames are physical pixels,
    // while QScreen geometries and crop regions are logical pixels.
    void setOutputScale(qreal scale);
    qreal getOutputScale() const;

    // Frame composition interface
    void beginRound(quint64 roundId);
    bool addScreenFrame(const DmaFrameInfo& frameInfo);
    bool isCompositionReady() const;
    QByteArray composeFrames();
    
    // Utility methods
    void reset();
    QSize getOutputSize() const;
    int getOutputStride() const;

signals:
    void compositionComplete(const QByteArray& compositeFrame, int width, int height, int stride, uint64_t timestamp);
    void compositionError(const QString& message);

private slots:
    void performComposition();

private:
    // Core composition methods
    QByteArray performFrameStitching();
    QImage convertDmaBufferToQImage(const DmaFrameInfo& frameInfo);
    QRect calculateDrawRect(const QRect& screenGeometry) const;
    QString screenLabel(const DmaFrameInfo& frameInfo) const;
    
    // Helper methods
    bool areAllFramesReady() const;
    uint64_t getLatestTimestamp() const;
    void resetFrameReadyFlags();
    QSize scaledOutputSize(const QSize &logicalSize) const;
    QRect scaleLogicalRect(const QRect &rect, qreal scale) const;

private:
    // Configuration
    QSize m_virtualDesktopSize;
    QVector<QRect> m_screenGeometries; // Screen positions in virtual desktop
    QRect m_contentBounds; // Union of all captured outputs in global coordinates
    QRect m_cropRegion; // Optional crop region
    bool m_hasCropRegion = false;
    
    // Frame data storage
    QVector<DmaFrameInfo> m_frameBuffers;
    mutable QMutex m_frameMutex;
    
    // Output configuration
    QSize m_outputSize; // Final output size in physical pixels
    int m_outputStride;
    qreal m_outputScale = 1.0;
    
    // Performance tracking
    int m_frameCount = 0;
    quint64 m_activeRoundId = 0;
};

#endif // MULTISCREENFRAMECOMPOSITOR_H
