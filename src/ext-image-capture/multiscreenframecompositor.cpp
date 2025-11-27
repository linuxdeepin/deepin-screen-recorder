// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "multiscreenframecompositor.h"
#include "utils.h"
#include <QDebug>
#include <QDateTime>
#include <QGuiApplication>
#include <QThread>

// GBM and DRM headers for DMA Buffer processing
extern "C" {
#include <gbm.h>
#include <drm_fourcc.h>
}

#include "../utils/log.h"

MultiScreenFrameCompositor::MultiScreenFrameCompositor(QObject *parent)
    : QObject(parent)
    , m_outputStride(0)
{
    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Constructor";
}

MultiScreenFrameCompositor::~MultiScreenFrameCompositor()
{
    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Destructor";
    reset();
}

void MultiScreenFrameCompositor::setVirtualDesktopSize(const QSize& size)
{
    QMutexLocker locker(&m_frameMutex);
    m_virtualDesktopSize = size;
    
    if (!m_hasCropRegion) {
        m_outputSize = size;
        m_outputStride = size.width() * 4; // RGBA format
    }
    
    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Virtual desktop size set to" << size;
}

void MultiScreenFrameCompositor::setScreenLayouts(const QList<QScreen*>& screens)
{
    QMutexLocker locker(&m_frameMutex);
    
    m_screenGeometries.clear();
    m_frameBuffers.clear();
    
    for (QScreen* screen : screens) {
        if (screen) {
            // Get screen geometry in virtual desktop coordinates
            QRect geometry = screen->geometry();
            m_screenGeometries.append(geometry);
            
            // Initialize frame buffer for this screen
            DmaFrameInfo frameInfo;
            frameInfo.screen = screen;
            frameInfo.screenGeometry = geometry;
            m_frameBuffers.append(frameInfo);
            
            qCDebug(dsrApp) << "MultiScreenFrameCompositor: Added screen" << screen->name() 
                            << "geometry:" << geometry;
        }
    }
    
    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Configured" << m_frameBuffers.size() << "screens";
}

void MultiScreenFrameCompositor::setCropRegion(const QRect& region)
{
    QMutexLocker locker(&m_frameMutex);
    m_cropRegion = region;
    m_hasCropRegion = true;
    m_outputSize = region.size();
    m_outputStride = region.width() * 4; // RGBA format
    
    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Crop region set to" << region;
}

void MultiScreenFrameCompositor::clearCropRegion()
{
    QMutexLocker locker(&m_frameMutex);
    m_hasCropRegion = false;
    m_outputSize = m_virtualDesktopSize;
    m_outputStride = m_virtualDesktopSize.width() * 4; // RGBA format
    
    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Crop region cleared";
}

bool MultiScreenFrameCompositor::addScreenFrame(const DmaFrameInfo& frameInfo)
{
    QMutexLocker locker(&m_frameMutex);
    
    // Find the frame buffer for this screen
    for (auto& buffer : m_frameBuffers) {
        if (buffer.screen == frameInfo.screen) {
            // Update frame data
            buffer.dmaBufferFd = frameInfo.dmaBufferFd;
            buffer.gbmBo = frameInfo.gbmBo;
            buffer.width = frameInfo.width;
            buffer.height = frameInfo.height;
            buffer.stride = frameInfo.stride;
            buffer.size = frameInfo.size;
            buffer.timestamp = frameInfo.timestamp;
            buffer.frameData = frameInfo.frameData;
            buffer.ready = true;
            
            qCDebug(dsrApp) << "MultiScreenFrameCompositor: Frame added for screen" 
                            << frameInfo.screen->name() << "timestamp:" << frameInfo.timestamp;
            
            // Check if all frames are ready for composition
            if (areAllFramesReady()) {
                qCDebug(dsrApp) << "MultiScreenFrameCompositor: All frames ready, triggering composition";
                // Use queued connection to avoid blocking the caller
                QMetaObject::invokeMethod(this, "performComposition", Qt::QueuedConnection);
            }
            
            return true;
        }
    }
    
    qCWarning(dsrApp) << "MultiScreenFrameCompositor: No frame buffer found for screen" 
                      << frameInfo.screen->name();
    return false;
}

bool MultiScreenFrameCompositor::isCompositionReady() const
{
    QMutexLocker locker(&m_frameMutex);
    return areAllFramesReady();
}

QByteArray MultiScreenFrameCompositor::composeFrames()
{
    QMutexLocker locker(&m_frameMutex);
    
    if (!areAllFramesReady()) {
        qCWarning(dsrApp) << "MultiScreenFrameCompositor: Not all frames ready for composition";
        return QByteArray();
    }
    
    return performFrameStitching();
}

void MultiScreenFrameCompositor::reset()
{
    QMutexLocker locker(&m_frameMutex);
    
    for (auto& buffer : m_frameBuffers) {
        buffer.ready = false;
        buffer.frameData.clear();
    }
    
    m_frameCount = 0;
    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Reset complete";
}

QSize MultiScreenFrameCompositor::getOutputSize() const
{
    QMutexLocker locker(&m_frameMutex);
    return m_outputSize;
}

int MultiScreenFrameCompositor::getOutputStride() const
{
    QMutexLocker locker(&m_frameMutex);
    return m_outputStride;
}

void MultiScreenFrameCompositor::performComposition()
{
    QByteArray compositeFrame = composeFrames();
    
    if (!compositeFrame.isEmpty()) {
        uint64_t latestTimestamp = getLatestTimestamp();
        
        qCDebug(dsrApp) << "MultiScreenFrameCompositor: Composition complete, frame size:" 
                        << compositeFrame.size() << "timestamp:" << latestTimestamp;
        
        emit compositionComplete(compositeFrame, m_outputSize.width(), m_outputSize.height(), 
                               m_outputStride, latestTimestamp);
        
        // Reset for next composition
        resetFrameReadyFlags();
        m_frameCount++;
    } else {
        qCWarning(dsrApp) << "MultiScreenFrameCompositor: Composition failed";
        emit compositionError("Failed to compose multi-screen frame");
    }
}

QByteArray MultiScreenFrameCompositor::performFrameStitching()
{
    // Based on waylandintegration.cpp:1039-1047 successful pattern
    qCDebug(dsrApp) << "MultiScreenFrameCompositor::performFrameStitching: Starting composition";
    
    if (m_outputSize.isEmpty()) {
        qCWarning(dsrApp) << "MultiScreenFrameCompositor: Output size is empty";
        return QByteArray();
    }
    
    // Create the composite canvas - following waylandintegration.cpp:1039
    QImage compositeImage(m_outputSize, QImage::Format_RGBA8888);
    compositeImage.fill(Qt::GlobalColor::black); // waylandintegration.cpp:1043
    
    // Create QPainter for composition - waylandintegration.cpp:1044
    QPainter painter(&compositeImage);
    
    // Process each screen frame - waylandintegration.cpp:1045-1047
    for (const auto& frameBuffer : m_frameBuffers) {
        if (!frameBuffer.ready) {
            continue;
        }
        
        // Convert DMA Buffer to QImage
        QImage screenImage = convertDmaBufferToQImage(frameBuffer);
        if (screenImage.isNull()) {
            qCWarning(dsrApp) << "MultiScreenFrameCompositor: Failed to convert frame for screen"
                              << frameBuffer.screen->name();
            continue;
        }
        
        // Calculate where to draw this screen's image
        QRect drawRect = calculateDrawRect(frameBuffer.screenGeometry);
        
        if (drawRect.isValid()) {
            // Draw the screen image at its position - waylandintegration.cpp:1046
            painter.drawImage(drawRect.topLeft(), screenImage);
            
            qCDebug(dsrApp) << "MultiScreenFrameCompositor: Drew screen" << frameBuffer.screen->name()
                            << "at position" << drawRect.topLeft() << "size" << screenImage.size();
        }
    }
    
    painter.end();
    
    // Convert QImage to QByteArray for transmission
    QByteArray result;
    int imageSize = compositeImage.sizeInBytes();
    result.resize(imageSize);
    memcpy(result.data(), compositeImage.constBits(), imageSize);
    
    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Frame stitching complete, output size:" 
                    << result.size() << "dimensions:" << m_outputSize;
    
    return result;
}

QImage MultiScreenFrameCompositor::convertDmaBufferToQImage(const DmaFrameInfo& frameInfo)
{
    // If we already have CPU-accessible data, use it directly
    if (!frameInfo.frameData.isEmpty()) {
        qCDebug(dsrApp) << "MultiScreenFrameCompositor: Using CPU-accessible frame data for screen"
                        << frameInfo.screen->name();
        
        // Create QImage from the frame data
        // Assuming RGBA format as per DRM_FORMAT_XBGR8888 -> RGBA conversion
        return QImage(reinterpret_cast<const uchar*>(frameInfo.frameData.constData()),
                     frameInfo.width, frameInfo.height, frameInfo.stride, QImage::Format_RGBA8888).copy();
    }
    
    // If no CPU data available, we need to handle DMA Buffer conversion
    qCWarning(dsrApp) << "MultiScreenFrameCompositor: No CPU-accessible data for screen"
                      << frameInfo.screen->name() << ", attempting DMA Buffer conversion";
    
    // For DMA Buffer conversion, we would need to:
    // 1. Import the DMA Buffer FD
    // 2. Map it to CPU-accessible memory (if possible)
    // 3. Convert the pixel format
    // 
    // However, based on our previous research, direct CPU mapping of DMA Buffers
    // often fails on AMD GPUs. The frameData should already contain the converted
    // pixel data from the capture process.
    
    // Return a placeholder pattern to identify issues
    QImage placeholder(frameInfo.width, frameInfo.height, QImage::Format_RGBA8888);
    placeholder.fill(QColor(255, 0, 255, 128)); // Magenta with transparency
    
    qCWarning(dsrApp) << "MultiScreenFrameCompositor: Using placeholder image for screen"
                      << frameInfo.screen->name();
    
    return placeholder;
}

QRect MultiScreenFrameCompositor::calculateDrawRect(const QRect& screenGeometry) const
{
    if (m_hasCropRegion) {
        // For crop region recording, calculate intersection and adjust coordinates
        QRect intersection = screenGeometry.intersected(m_cropRegion);
        if (intersection.isEmpty()) {
            return QRect(); // This screen doesn't contribute to the crop region
        }
        
        // Translate coordinates to crop region space
        QRect drawRect = intersection.translated(-m_cropRegion.topLeft());
        
        qCDebug(dsrApp) << "MultiScreenFrameCompositor: Crop mode - screen geometry:" << screenGeometry
                        << "crop region:" << m_cropRegion << "draw rect:" << drawRect;
        
        return drawRect;
    } else {
        // For full desktop recording, screen position is its virtual desktop position
        return screenGeometry;
    }
}

bool MultiScreenFrameCompositor::areAllFramesReady() const
{
    for (const auto& buffer : m_frameBuffers) {
        if (!buffer.ready) {
            return false;
        }
    }
    return !m_frameBuffers.isEmpty();
}

uint64_t MultiScreenFrameCompositor::getLatestTimestamp() const
{
    uint64_t latest = 0;
    for (const auto& buffer : m_frameBuffers) {
        if (buffer.ready && buffer.timestamp > latest) {
            latest = buffer.timestamp;
        }
    }
    return latest;
}

void MultiScreenFrameCompositor::resetFrameReadyFlags()
{
    for (auto& buffer : m_frameBuffers) {
        buffer.ready = false;
    }
}

