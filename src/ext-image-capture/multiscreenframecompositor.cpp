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
        m_outputSize = scaledOutputSize(size);
        m_outputStride = m_outputSize.width() * 4; // RGBA format
    }

    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Virtual desktop size set to" << size
                    << "output scale:" << m_outputScale
                    << "physical output size:" << m_outputSize;
}

void MultiScreenFrameCompositor::setOutputScale(qreal scale)
{
    QMutexLocker locker(&m_frameMutex);
    if (!qIsFinite(scale) || scale <= 0.0)
        scale = 1.0;
    m_outputScale = scale;

    const QSize logicalSize = m_hasCropRegion ? m_cropRegion.size()
                                              : m_virtualDesktopSize;
    m_outputSize = scaledOutputSize(logicalSize);
    m_outputStride = m_outputSize.isEmpty() ? 0 : m_outputSize.width() * 4;

    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Output scale set to" << m_outputScale
                    << "logical size:" << logicalSize
                    << "physical output size:" << m_outputSize;
}

qreal MultiScreenFrameCompositor::getOutputScale() const
{
    QMutexLocker locker(&m_frameMutex);
    return m_outputScale;
}

void MultiScreenFrameCompositor::setScreenLayouts(const QList<QScreen*>& screens)
{
    QMutexLocker locker(&m_frameMutex);
    
    m_screenGeometries.clear();
    m_frameBuffers.clear();
    m_contentBounds = QRect();
    
    for (QScreen* screen : screens) {
        if (screen) {
            // Get screen geometry in virtual desktop coordinates
            QRect geometry = screen->geometry();
            m_screenGeometries.append(geometry);
            m_contentBounds = m_contentBounds.isNull()
                    ? geometry
                    : m_contentBounds.united(geometry);
            
            // Initialize frame buffer for this screen
            DmaFrameInfo frameInfo;
            frameInfo.screen = screen;
            frameInfo.screenName = screen->name();
            frameInfo.screenGeometry = geometry;
            m_frameBuffers.append(frameInfo);
            
            qCDebug(dsrApp) << "MultiScreenFrameCompositor: Added screen" << screen->name() 
                            << "geometry:" << geometry;
        }
    }

    if (!m_hasCropRegion) {
        m_outputSize = scaledOutputSize(m_contentBounds.size());
        m_outputStride = m_outputSize.width() * 4;
    }

    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Configured" << m_frameBuffers.size()
                    << "screens; output scale:" << m_outputScale
                    << "physical output size:" << m_outputSize;
}

void MultiScreenFrameCompositor::setScreenLayouts(const QVector<ScreenLayoutInfo>& layouts)
{
    QMutexLocker locker(&m_frameMutex);

    m_screenGeometries.clear();
    m_frameBuffers.clear();
    m_contentBounds = QRect();

    for (const ScreenLayoutInfo &layout : layouts) {
        if (layout.geometry.isEmpty())
            continue;

        m_screenGeometries.append(layout.geometry);
        m_contentBounds = m_contentBounds.isNull()
                ? layout.geometry
                : m_contentBounds.united(layout.geometry);

        DmaFrameInfo frameInfo;
        frameInfo.screenName = layout.name;
        frameInfo.screenGeometry = layout.geometry;
        m_frameBuffers.append(frameInfo);

        qCDebug(dsrApp) << "MultiScreenFrameCompositor: Added output" << layout.name
                        << "geometry:" << layout.geometry;
    }

    if (!m_hasCropRegion) {
        m_outputSize = scaledOutputSize(m_contentBounds.size());
        m_outputStride = m_outputSize.width() * 4;
    }

    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Configured" << m_frameBuffers.size()
                    << "outputs from pure layout data; output scale:" << m_outputScale
                    << "physical output size:" << m_outputSize;
}

void MultiScreenFrameCompositor::beginRound(quint64 roundId)
{
    QMutexLocker locker(&m_frameMutex);
    m_activeRoundId = roundId;
    for (auto &buffer : m_frameBuffers)
        buffer.ready = false;

    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Begin round" << roundId;
}

void MultiScreenFrameCompositor::setCropRegion(const QRect& region)
{
    QMutexLocker locker(&m_frameMutex);
    m_cropRegion = region.normalized();
    m_hasCropRegion = true;

    // Keep the canvas at physical resolution. The crop region and screen
    // geometries are logical pixels, while captured frames are physical
    // pixels; libx264 with yuv420p also requires even output dimensions.
    m_outputSize = scaledOutputSize(m_cropRegion.size());
    m_outputStride = m_outputSize.width() * 4; // RGBA format

    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Crop region set to" << m_cropRegion
                    << "output scale:" << m_outputScale
                    << "physical output size:" << m_outputSize;
}

void MultiScreenFrameCompositor::clearCropRegion()
{
    QMutexLocker locker(&m_frameMutex);
    m_hasCropRegion = false;
    m_outputSize = scaledOutputSize(m_virtualDesktopSize);
    m_outputStride = m_outputSize.width() * 4; // RGBA format

    qCDebug(dsrApp) << "MultiScreenFrameCompositor: Crop region cleared"
                    << "output scale:" << m_outputScale
                    << "physical output size:" << m_outputSize;
}

bool MultiScreenFrameCompositor::addScreenFrame(const DmaFrameInfo& frameInfo)
{
    QMutexLocker locker(&m_frameMutex);
    
    // Find the frame buffer for this screen
    for (auto& buffer : m_frameBuffers) {
        const bool nameMatch = !frameInfo.screenName.isEmpty()
                && buffer.screenName == frameInfo.screenName;
        const bool legacyPointerMatch = frameInfo.screen != nullptr
                && buffer.screen == frameInfo.screen;
        if (!nameMatch && !legacyPointerMatch)
            continue;

        if (m_activeRoundId != 0 && frameInfo.roundId != 0
                && frameInfo.roundId != m_activeRoundId) {
            qCWarning(dsrApp) << "MultiScreenFrameCompositor: dropping stale frame for output"
                              << screenLabel(frameInfo) << "round:" << frameInfo.roundId
                              << "active round:" << m_activeRoundId;
            return false;
        }

        // 旧 GUI 接口会在 frameInfo.screen 里带 QScreen；工作线程接口只带名称。
        if (!buffer.screen)
            buffer.screen = frameInfo.screen;

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

        qCDebug(dsrApp) << "MultiScreenFrameCompositor: Frame added for output"
                        << screenLabel(frameInfo) << "timestamp:" << frameInfo.timestamp;

        // Check if all frames are ready for composition
        if (areAllFramesReady()) {
            qCDebug(dsrApp) << "MultiScreenFrameCompositor: All frames ready, triggering composition";
            // Use queued connection to avoid blocking the caller
            QMetaObject::invokeMethod(this, "performComposition", Qt::QueuedConnection);
        }

        return true;
    }

    qCWarning(dsrApp) << "MultiScreenFrameCompositor: No frame buffer found for output"
                      << screenLabel(frameInfo);
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
    m_activeRoundId = 0;
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
    
    const QRect outputOrigin = m_hasCropRegion ? m_cropRegion : m_contentBounds;

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
            qCWarning(dsrApp) << "MultiScreenFrameCompositor: Failed to convert frame for output"
                              << screenLabel(frameBuffer);
            continue;
        }
        
        const QRect capturedRegion = m_hasCropRegion ? m_cropRegion : m_contentBounds;
        const QRect intersection = frameBuffer.screenGeometry.intersected(capturedRegion);
        if (intersection.isEmpty())
            continue;

        const qreal scaleX = static_cast<qreal>(screenImage.width())
                / qMax(1, frameBuffer.screenGeometry.width());
        const qreal scaleY = static_cast<qreal>(screenImage.height())
                / qMax(1, frameBuffer.screenGeometry.height());
        const QRect local = intersection.translated(-frameBuffer.screenGeometry.topLeft());
        const int sourceLeft = qRound(local.left() * scaleX);
        const int sourceTop = qRound(local.top() * scaleY);
        const int sourceRight = qRound((local.left() + local.width()) * scaleX);
        const int sourceBottom = qRound((local.top() + local.height()) * scaleY);
        QRect sourceRect(sourceLeft,
                         sourceTop,
                         qMax(0, sourceRight - sourceLeft),
                         qMax(0, sourceBottom - sourceTop));
        sourceRect = sourceRect.intersected(screenImage.rect());
        if (sourceRect.isEmpty())
            continue;

        QRect targetRect = scaleLogicalRect(intersection.translated(-outputOrigin.topLeft()),
                                            m_outputScale);
        targetRect = targetRect.intersected(compositeImage.rect());
        if (targetRect.isEmpty())
            continue;
        painter.setRenderHint(QPainter::SmoothPixmapTransform,
                              targetRect.size() != sourceRect.size());
        painter.drawImage(targetRect, screenImage, sourceRect);

        qCDebug(dsrApp) << "MultiScreenFrameCompositor: Drew output"
                        << screenLabel(frameBuffer)
                        << "target:" << targetRect << "source:" << sourceRect;
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
        qCDebug(dsrApp) << "MultiScreenFrameCompositor: Using CPU-accessible frame data for output"
                        << screenLabel(frameInfo);
        
        // Capture frames use DRM_FORMAT_XBGR8888. On little-endian systems the
        // memory order is R,G,B,X, while the final X byte is unused rather
        // than alpha. RGBX8888 preserves that byte order and keeps the image
        // opaque; RGB32 would swap red and blue, and RGBA8888 would treat the
        // final zero byte as transparency.
        return QImage(reinterpret_cast<const uchar*>(frameInfo.frameData.constData()),
                     frameInfo.width, frameInfo.height, frameInfo.stride,
                     QImage::Format_RGBX8888).copy();
    }
    
    // If no CPU data available, we need to handle DMA Buffer conversion
    qCWarning(dsrApp) << "MultiScreenFrameCompositor: No CPU-accessible data for output"
                      << screenLabel(frameInfo) << ", attempting DMA Buffer conversion";
    
    // For DMA Buffer conversion, we would need to:
    // 1. Import the DMA Buffer FD
    // 2. Map it to CPU-accessible memory (if possible)
    // 3. Convert the pixel format
    // 
    // However, based on our previous research, direct CPU mapping of DMA Buffers
    // often fails on AMD GPUs. The frameData should already contain the converted
    // pixel data from the capture process.
    
    qCWarning(dsrApp) << "MultiScreenFrameCompositor: No usable image for output"
                      << screenLabel(frameInfo);
    return QImage();
}

QString MultiScreenFrameCompositor::screenLabel(const DmaFrameInfo& frameInfo) const
{
    if (!frameInfo.screenName.isEmpty())
        return frameInfo.screenName;
    if (frameInfo.screen)
        return frameInfo.screen->name();
    return QStringLiteral("<unknown>");
}

QSize MultiScreenFrameCompositor::scaledOutputSize(const QSize &logicalSize) const
{
    if (logicalSize.isEmpty())
        return QSize();

    int width = qMax(2, qRound(logicalSize.width() * m_outputScale));
    int height = qMax(2, qRound(logicalSize.height() * m_outputScale));
    width -= width % 2;
    height -= height % 2;
    return QSize(width, height);
}

QRect MultiScreenFrameCompositor::scaleLogicalRect(const QRect &rect, qreal scale) const
{
    if (rect.isEmpty() || !qIsFinite(scale) || scale <= 0.0)
        return QRect();

    const int left = qRound(rect.left() * scale);
    const int top = qRound(rect.top() * scale);
    const int right = qRound((rect.left() + rect.width()) * scale);
    const int bottom = qRound((rect.top() + rect.height()) * scale);
    return QRect(left, top, qMax(0, right - left), qMax(0, bottom - top));
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
