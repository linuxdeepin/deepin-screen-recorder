// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extcaptureintegration.h"
#include "manager/extcapturemanager.h"
#include "session/extcapturesession.h"
#include "frame/extcaptureframe.h"
#include "../utils/log.h"

#include <QDebug>
#include <QTimer>

ExtCaptureIntegration::ExtCaptureIntegration(QObject *parent)
    : QObject(parent)
    , m_manager(new ExtCaptureManager(this))
    , m_session(nullptr)
    , m_recording(false)
{
    // 连接管理器信号
    connect(m_manager, &ExtCaptureManager::protocolAvailable,
            this, &ExtCaptureIntegration::onProtocolAvailable);
    connect(m_manager, &ExtCaptureManager::protocolUnavailable,
            this, &ExtCaptureIntegration::onProtocolUnavailable);
}

ExtCaptureIntegration::~ExtCaptureIntegration()
{
    stopRecording();
}

bool ExtCaptureIntegration::isAvailable() const
{
    return m_manager && m_manager->isProtocolAvailable();
}

bool ExtCaptureIntegration::startScreenRecording(QScreen *screen, bool includeCursor)
{
    qCWarning(dsrApp) << "ExtCaptureIntegration::startScreenRecording: *** STARTING *** includeCursor =" << includeCursor;
    
    if (m_recording) {
        qWarning() << "Already recording";
        return false;
    }

    if (!isAvailable()) {
        qWarning() << "ext-image-copy-capture protocol not available";
        emit error("Protocol not available");
        return false;
    }

    if (!screen) {
        qWarning() << "Invalid screen provided";
        emit error("Invalid screen");
        return false;
    }

    // 创建捕获会话
    qCWarning(dsrApp) << "ExtCaptureIntegration: Creating capture session with includeCursor =" << includeCursor;
    m_session = m_manager->createScreenCaptureSession(screen, includeCursor);
    if (!m_session) {
        qWarning() << "Failed to create capture session";
        emit error("Failed to create capture session");
        return false;
    }

    // 连接会话信号
    connect(m_session, &ExtCaptureSession::ready,
            this, &ExtCaptureIntegration::onSessionReady);
    connect(m_session, &ExtCaptureSession::stopped,
            this, &ExtCaptureIntegration::onSessionStopped);
    connect(m_session, &ExtCaptureSession::error,
            this, &ExtCaptureIntegration::onSessionError);
    connect(m_session, &ExtCaptureSession::frameReady,
            this, &ExtCaptureIntegration::onFrameReady);
    
    qCWarning(dsrApp) << "ExtCaptureIntegration: Session signals connected, session state:" << m_session->state();

    // 检查 session 是否已经准备就绪（信号可能在连接前就发出了）
    if (m_session->state() == ExtCaptureSession::Ready) {
        qCWarning(dsrApp) << "ExtCaptureIntegration: Session was already ready, triggering onSessionReady";
        // 用 QTimer::singleShot 确保在事件循环中异步调用
        QTimer::singleShot(0, this, &ExtCaptureIntegration::onSessionReady);
    }

    m_recording = true;
    qDebug() << "Screen recording started for screen:" << screen->name();
    return true;
}

void ExtCaptureIntegration::stopRecording()
{
    if (!m_recording) {
        return;
    }

    m_recording = false;

    if (m_session) {
        m_session->stop();
        m_session->deleteLater();
        m_session = nullptr;
    }

    emit recordingStopped();
    qDebug() << "Screen recording stopped";
}

bool ExtCaptureIntegration::captureFrame()
{
    if (!m_recording || !m_session) {
        qWarning() << "Not recording or no active session";
        return false;
    }

    if (m_session->state() != ExtCaptureSession::Ready) {
        qCWarning(dsrApp) << "ExtCaptureIntegration::captureFrame: Session not ready for frame capture, current state:" << m_session->state();
        return false;
    }

    ExtCaptureFrame *frame = m_session->createFrame();
    if (!frame) {
        qCWarning(dsrApp) << "ExtCaptureIntegration::captureFrame: Failed to create frame";
        emit error("Failed to create frame");
        return false;
    }

    qCWarning(dsrApp) << "ExtCaptureIntegration::captureFrame: Frame created successfully:" << frame;

    // 连接帧信号
    connect(frame, &ExtCaptureFrame::failed,
            this, &ExtCaptureIntegration::onFrameFailed);

    // 开始捕获
    qCWarning(dsrApp) << "ExtCaptureIntegration::captureFrame: Starting frame capture...";
    if (!frame->capture()) {
        qCWarning(dsrApp) << "ExtCaptureIntegration::captureFrame: Failed to start frame capture";
        frame->deleteLater();
        emit error("Failed to start frame capture");
        return false;
    }

    qCWarning(dsrApp) << "ExtCaptureIntegration::captureFrame: Frame capture started successfully";
    return true;
}

bool ExtCaptureIntegration::isRecording() const
{
    return m_recording;
}

void ExtCaptureIntegration::onProtocolAvailable()
{
    qDebug() << "ext-image-copy-capture protocol available";
    emit available();
}

void ExtCaptureIntegration::onProtocolUnavailable()
{
    qDebug() << "ext-image-copy-capture protocol unavailable";
    
    // 停止当前录制
    if (m_recording) {
        stopRecording();
    }
    
    emit unavailable();
}

void ExtCaptureIntegration::onSessionReady()
{
    qCWarning(dsrApp) << "ExtCaptureIntegration::onSessionReady: Capture session ready";
    emit recordingStarted();
}

void ExtCaptureIntegration::onSessionStopped()
{
    qDebug() << "Capture session stopped";
    if (m_recording) {
        stopRecording();
    }
}

void ExtCaptureIntegration::onSessionError(const QString &message)
{
    qWarning() << "Session error:" << message;
    emit error(message);
    
    if (m_recording) {
        stopRecording();
    }
}

void ExtCaptureIntegration::onFrameReady(ExtCaptureFrame *frame)
{
    if (!frame) {
        qWarning() << "Invalid frame received";
        return;
    }

    // 获取帧数据
    const FrameData &frameData = frame->frameData();
    
    if (frame->isDmaBuffer()) {
        // DMA Buffer模式：发射DMA Buffer专用信号
        qCWarning(dsrApp) << "ExtCaptureIntegration::onFrameReady: *** DMA BUFFER FRAME *** fd:" << frame->getDmaBufferFd();
        emit dmaFrameReady(frame->getDmaBufferFd(), frame->getGbmBufferObject(),
                          frameData.size, frameData.dimensions.width(), frameData.dimensions.height(),
                          frameData.stride, frameData.timestamp);
    } else {
        // SHM Buffer模式：传统方式映射内存
        void *data = frame->mapBuffer();
        
        if (!data) {
            qWarning() << "Failed to map frame buffer";
            frame->deleteLater();
            return;
        }

        // 发射帧就绪信号
        emit frameReady(data, frameData.size, 
                       frameData.dimensions.width(), frameData.dimensions.height(),
                       frameData.stride, frameData.timestamp);
        
        // 取消映射
        frame->unmapBuffer();
    }

    // 清理
    frame->deleteLater();

    qDebug() << "Frame processed:" << frameData.dimensions 
             << "format:" << frameData.format
             << "timestamp:" << frameData.timestamp;
}

void ExtCaptureIntegration::onFrameFailed(const QString &reason)
{
    qWarning() << "Frame capture failed:" << reason;
    emit error(QString("Frame capture failed: %1").arg(reason));
}

