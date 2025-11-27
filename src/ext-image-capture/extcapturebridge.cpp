// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extcapturebridge.h"
#include "extcapturerecorder.h"
#include "extcaptureframebuffer.h"

#ifdef KF5_WAYLAND_FLAGE_ON
#include "../waylandrecord/recordadmin.h"
#include "../waylandrecord/waylandintegration_p.h"
#include "../waylandrecord/avlibinterface.h"
#endif

#include "../utils/log.h"

#include <QDebug>
#include <QDateTime>
#include <QGuiApplication>

ExtCaptureBridge::ExtCaptureBridge(QObject *parent)
    : QObject(parent)
    , m_extCaptureRecorder(nullptr)
    , m_frameBuffer(nullptr)
    , m_recordAdmin(nullptr)
    , m_processTimer(new QTimer(this))
    , m_bridging(false)
    , m_recordAdminInitialized(false)
    , m_processedFrameCount(0)
    , m_screenWidth(0)
    , m_screenHeight(0)
    , m_bridgeStartTime(0)
    , m_totalFramesReceived(0)
    , m_totalFramesProcessed(0)
    , m_droppedFrames(0)
{
    // 设置定时器处理帧数据
    m_processTimer->setSingleShot(false);
    m_processTimer->setInterval(16);
    connect(m_processTimer, &QTimer::timeout, this, &ExtCaptureBridge::processFrames);
    
    qDebug() << "ExtCaptureBridge created";
}

ExtCaptureBridge::~ExtCaptureBridge()
{
    stopBridge();
    qDebug() << "ExtCaptureBridge destroyed";
}

void ExtCaptureBridge::setExtCaptureRecorder(ExtCaptureRecorder *recorder)
{
    if (m_bridging) {
        qWarning() << "ExtCaptureBridge: Cannot set recorder while bridging";
        return;
    }

    m_extCaptureRecorder = recorder;
    
    if (recorder) {
        m_frameBuffer = recorder->getFrameBuffer();
        
        // 连接录制状态信号
        connect(recorder, &ExtCaptureRecorder::recordingStarted, 
                this, &ExtCaptureBridge::onRecordingStarted);
        connect(recorder, &ExtCaptureRecorder::recordingStopped, 
                this, &ExtCaptureBridge::onRecordingStopped);
        connect(recorder, &ExtCaptureRecorder::error, 
                this, &ExtCaptureBridge::onRecordingError);
                
        qDebug() << "ExtCaptureBridge: ExtCaptureRecorder set";
    } else {
        m_frameBuffer = nullptr;
        qDebug() << "ExtCaptureBridge: ExtCaptureRecorder cleared";
    }
}

void ExtCaptureBridge::setRecordAdmin(RecordAdmin *recordAdmin)
{
    if (m_bridging) {
        qWarning() << "ExtCaptureBridge: Cannot set RecordAdmin while bridging";
        return;
    }

    m_recordAdmin = recordAdmin;
    m_recordAdminInitialized = false;
    
    qDebug() << "ExtCaptureBridge: RecordAdmin" << (recordAdmin ? "set" : "cleared");
}

bool ExtCaptureBridge::startBridge(int screenWidth, int screenHeight)
{
    if (m_bridging) {
        qWarning() << "ExtCaptureBridge: Already bridging";
        return false;
    }

    if (!m_extCaptureRecorder || !m_frameBuffer || !m_recordAdmin) {
        qCritical() << "ExtCaptureBridge: Missing required components";
        emit bridgeError("Missing required components");
        return false;
    }

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_bridgeStartTime = QDateTime::currentMSecsSinceEpoch();
    
    // 重置统计信息
    m_processedFrameCount = 0;
    m_totalFramesReceived = 0;
    m_totalFramesProcessed = 0;
    m_droppedFrames = 0;

    m_bridging = true;
    
    qCInfo(dsrApp) << "ExtCaptureBridge: Bridge started for screen size:" 
                   << m_screenWidth << "x" << m_screenHeight;
    
    emit bridgeStarted();
    return true;
}

void ExtCaptureBridge::stopBridge()
{
    if (!m_bridging) {
        return;
    }

    m_processTimer->stop();
    m_bridging = false;
    
    qCInfo(dsrApp) << "ExtCaptureBridge: Bridge stopped. Statistics:"
                   << "Received:" << m_totalFramesReceived
                   << "Processed:" << m_totalFramesProcessed
                   << "Dropped:" << m_droppedFrames;
    
    emit bridgeStopped();
}

bool ExtCaptureBridge::isBridging() const
{
    return m_bridging;
}

int ExtCaptureBridge::getProcessedFrameCount() const
{
    return m_processedFrameCount;
}

QString ExtCaptureBridge::getStatusInfo() const
{
    return QString("Bridging: %1, Processed: %2, Received: %3, Dropped: %4")
           .arg(m_bridging ? "Yes" : "No")
           .arg(m_totalFramesProcessed)
           .arg(m_totalFramesReceived)
           .arg(m_droppedFrames);
}

void ExtCaptureBridge::processFrames()
{
    if (!m_bridging || !m_frameBuffer) {
        return;
    }

    QMutexLocker locker(&m_processMutex);
    
    // 处理所有可用帧
    int processedThisRound = 0;
    const int maxFramesPerRound = 5; // 限制每轮处理的帧数
    
    while (m_frameBuffer->hasFrames() && processedThisRound < maxFramesPerRound) {
        if (processSingleFrame()) {
            processedThisRound++;
            m_totalFramesProcessed++;
        } else {
            m_droppedFrames++;
            break; // 处理失败，跳出循环
        }
    }
    
    if (processedThisRound > 0) {
        emit frameProcessed(m_totalFramesProcessed, QDateTime::currentMSecsSinceEpoch());
    }
}

void ExtCaptureBridge::onRecordingStarted()
{
    qCDebug(dsrApp) << "ExtCaptureBridge: Recording started, initializing RecordAdmin";
    
    if (!initializeRecordAdmin()) {
        qCCritical(dsrApp) << "ExtCaptureBridge: Failed to initialize RecordAdmin";
        emit bridgeError("Failed to initialize video encoding");
        return;
    }
    
    // 开始处理帧数据
    m_processTimer->start();
    qCDebug(dsrApp) << "ExtCaptureBridge: Frame processing started";
}

void ExtCaptureBridge::onRecordingStopped()
{
    qCDebug(dsrApp) << "ExtCaptureBridge: Recording stopped";
    stopBridge();
}

void ExtCaptureBridge::onRecordingError(const QString &message)
{
    qCCritical(dsrApp) << "ExtCaptureBridge: Recording error:" << message;
    emit bridgeError(QString("Recording error: %1").arg(message));
    stopBridge();
}

bool ExtCaptureBridge::initializeRecordAdmin()
{
#ifdef KF5_WAYLAND_FLAGE_ON
    if (m_recordAdminInitialized || !m_recordAdmin) {
        return m_recordAdminInitialized;
    }

    try {
        // 初始化RecordAdmin (类似waylandintegration_p.cpp中的逻辑)
        m_recordAdmin->init(m_screenWidth, m_screenHeight);
        m_recordAdminInitialized = true;
        
        qCDebug(dsrApp) << "ExtCaptureBridge: RecordAdmin initialized successfully";
        return true;
        
    } catch (const std::exception &e) {
        qCCritical(dsrApp) << "ExtCaptureBridge: RecordAdmin init failed:" << e.what();
        return false;
    }
#else
    qCWarning(dsrApp) << "ExtCaptureBridge: KF5_WAYLAND_FLAGE_ON not defined, RecordAdmin not available";
    return false;
#endif
}

bool ExtCaptureBridge::processSingleFrame()
{
    if (!m_frameBuffer || !m_recordAdmin || !m_recordAdminInitialized) {
        return false;
    }

    ExtFrameData frame;
    if (!m_frameBuffer->getFrame(frame)) {
        return false; // 没有可用帧
    }
    
    m_totalFramesReceived++;

#ifdef KF5_WAYLAND_FLAGE_ON
    try {
        // 创建waylandFrame结构
        WaylandIntegration::WaylandIntegrationPrivate::waylandFrame waylandFrame;
        waylandFrame.timestamp = frame.timestamp;
        waylandFrame.index = frame.index;
        waylandFrame.width = frame.width;
        waylandFrame.height = frame.height;
        waylandFrame.stride = frame.stride;
        
        // 复制帧数据
        size_t frameSize = static_cast<size_t>(frame.height * frame.stride);
        waylandFrame.m_frame = new unsigned char[frameSize];
        std::memcpy(waylandFrame.m_frame, frame.data, frameSize);
        
        // 调用现有的编码逻辑
        if (m_recordAdmin && m_recordAdmin->m_pOutputStream) {
            int result = m_recordAdmin->m_pOutputStream->writeVideoFrame(waylandFrame);
            
            // 清理内存
            delete[] waylandFrame.m_frame;
            
            if (result >= 0) {
                m_processedFrameCount++;
                return true;
            } else {
                qCWarning(dsrApp) << "ExtCaptureBridge: writeVideoFrame failed with result:" << result;
                return false;
            }
        }
        
        // 清理内存
        delete[] waylandFrame.m_frame;
        return false;
        
    } catch (const std::exception &e) {
        qCCritical(dsrApp) << "ExtCaptureBridge: Frame processing failed:" << e.what();
        return false;
    }
#else
    qCWarning(dsrApp) << "ExtCaptureBridge: Frame processing not available without KF5_WAYLAND_FLAGE_ON";
    return false;
#endif
}

bool ExtCaptureBridge::convertFrameFormat(const unsigned char *srcData, int width, int height, int stride,
                                        unsigned char *dstData, size_t dstSize)
{
    size_t srcSize = static_cast<size_t>(height * stride);
    
    if (srcSize > dstSize) {
        qCWarning(dsrApp) << "ExtCaptureBridge: Source frame too large for destination buffer";
        return false;
    }
    
    std::memcpy(dstData, srcData, srcSize);
    return true;
}

int64_t ExtCaptureBridge::calculateFrameTimestamp()
{
#ifdef KF5_WAYLAND_FLAGE_ON
    return avlibInterface::m_av_gettime();
#else
    return QDateTime::currentMSecsSinceEpoch() * 1000LL; 
#endif
}

