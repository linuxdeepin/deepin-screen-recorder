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

// DMA Buffer frame information structure
struct DmaFrameInfo {
    QScreen* screen = nullptr;
    QRect screenGeometry; // Screen position in virtual desktop
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
    void setScreenLayouts(const QList<QScreen*>& screens);
    void setCropRegion(const QRect& region); // For region recording
    void clearCropRegion(); // For full-screen recording

    // Frame composition interface
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
    
    // Helper methods
    bool areAllFramesReady() const;
    uint64_t getLatestTimestamp() const;
    void resetFrameReadyFlags();

private:
    // Configuration
    QSize m_virtualDesktopSize;
    QVector<QRect> m_screenGeometries; // Screen positions in virtual desktop
    QRect m_cropRegion; // Optional crop region
    bool m_hasCropRegion = false;
    
    // Frame data storage
    QVector<DmaFrameInfo> m_frameBuffers;
    mutable QMutex m_frameMutex;
    
    // Output configuration
    QSize m_outputSize; // Final output size (virtual desktop or crop region)
    int m_outputStride;
    
    // Performance tracking
    int m_frameCount = 0;
};

#endif // MULTISCREENFRAMECOMPOSITOR_H
