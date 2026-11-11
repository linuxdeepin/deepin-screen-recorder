// Copyright (C) 2017-2026 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extcapturerecorder.h"
#include "extcaptureintegration.h"
#include "extcaptureframebuffer.h"
#include "framepumpworker.h"
#include "../utils/log.h"

#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QProcess>
#include <QFileInfo>
#include <cmath>
#include <drm/drm_fourcc.h>

ExtCaptureRecorder::ExtCaptureRecorder(QObject *parent)
    : QObject(parent)
    , m_extCapture(new ExtCaptureIntegration(this))
    , m_frameBuffer(new ExtCaptureFrameBuffer(this))
    , m_captureTimer(new QTimer(this))
    , m_captureWatchdog(new QTimer(this))
    , m_state(Stopped)
    , m_frameRate(30)
    , m_frameCount(0)
    , m_startTime(0)
    , m_includeCursor(false)
    , m_frameWidth(0)
    , m_frameHeight(0)
    , m_frameStride(0)
    , m_ffmpegProcess(nullptr)
    , m_streamingMode(true)  
    , m_ffmpegStarted(false)
    , m_framePump(nullptr)
    , m_framePumpThread(nullptr)
    , m_latestFrame(new LatestFrameStore)
    , m_lastCaptureRequestNs(-1)
    , m_captureIntervalNs(1000000000LL / 30)
    , m_captureSequence(0)
    , m_pumpOutputFrames(0)
    , m_pumpRepeatedFrames(0)
    , m_pumpBehindMs(0)
    , m_firstFrameWidth(0)
    , m_firstFrameHeight(0)
    , m_firstFrameStride(0)
{
    // 连接ext-capture信号
    connect(m_extCapture, &ExtCaptureIntegration::available,
            this, &ExtCaptureRecorder::onExtCaptureAvailable);
    connect(m_extCapture, &ExtCaptureIntegration::unavailable,
            this, &ExtCaptureRecorder::onExtCaptureUnavailable);
    connect(m_extCapture, &ExtCaptureIntegration::recordingStarted,
            this, &ExtCaptureRecorder::onRecordingStarted);
    connect(m_extCapture, &ExtCaptureIntegration::recordingStopped,
            this, &ExtCaptureRecorder::onRecordingStopped);
    connect(m_extCapture, &ExtCaptureIntegration::frameReady,
            this, &ExtCaptureRecorder::onFrameReady);
    connect(m_extCapture, &ExtCaptureIntegration::dmaFrameReady,
            this, &ExtCaptureRecorder::onDmaFrameReady);
    connect(m_extCapture, &ExtCaptureIntegration::multiScreenFrameReady,
            this, &ExtCaptureRecorder::onMultiScreenFrameReady);
    connect(m_extCapture, &ExtCaptureIntegration::error,
            this, &ExtCaptureRecorder::onExtCaptureError);

    // 设置捕获定时器
    m_captureTimer->setSingleShot(true);
    connect(m_captureTimer, &QTimer::timeout,
            this, &ExtCaptureRecorder::onCaptureTimer);
    m_captureWatchdog->setSingleShot(true);
    m_captureWatchdog->setInterval(3000);
    connect(m_captureWatchdog, &QTimer::timeout,
            this, &ExtCaptureRecorder::onCaptureWatchdog);

    qCDebug(dsrApp) << "ExtCaptureRecorder initialized";
}

ExtCaptureRecorder::~ExtCaptureRecorder()
{
    if (m_state == Recording) {
        stopRecording();
    }

    stopFramePump();
    delete m_latestFrame;
    m_latestFrame = nullptr;

    if (m_ffmpegProcess) {
        if (m_ffmpegProcess->state() != QProcess::NotRunning) {
            m_ffmpegProcess->closeWriteChannel();
            m_ffmpegProcess->waitForFinished(3000);
            if (m_ffmpegProcess->state() != QProcess::NotRunning) {
                m_ffmpegProcess->kill();
            }
        }
        m_ffmpegProcess->deleteLater();
        m_ffmpegProcess = nullptr;
    }
}

void ExtCaptureRecorder::setAudioOptions(int audioType,
                                         const QString &systemAudioDevice)
{
    m_audioOptions.type = qBound(static_cast<int>(AudioCaptureOptions::NoAudio),
                                 audioType,
                                 static_cast<int>(AudioCaptureOptions::MicrophoneAndSystemAudio));
    m_audioOptions.systemDevice = systemAudioDevice.trimmed();
    qCWarning(dsrApp) << "ExtCaptureRecorder::setAudioOptions:"
                      << "type:" << m_audioOptions.type
                      << "systemDevice:" << m_audioOptions.systemDevice;
}

bool ExtCaptureRecorder::isAvailable() const
{
    return m_extCapture && m_extCapture->isAvailable();
}

bool ExtCaptureRecorder::startRecording(QScreen *screen, bool includeCursor,
                                       const QString &outputPath, int frameRate,
                                       const QRect &captureRegion)
{
    qCWarning(dsrApp) << "ExtCaptureRecorder::startRecording: *** START RECORDING REQUEST *** current state:" << m_state;
    
    if (m_state != Stopped) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::startRecording: Already recording or in transition, state:" << m_state;
        return false;
    }

    if (!isAvailable()) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::startRecording: ext-image-copy-capture not available";
        emit error("ext-image-copy-capture protocol not available");
        return false;
    }

    if (!screen) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::startRecording: Invalid screen provided";
        emit error("Invalid screen");
        return false;
    }

    // 设置录制参数
    m_multiScreenRecording = false;
    m_includeCursor = includeCursor;
    m_frameRate = qMax(1, qMin(frameRate, 60)); // 限制在1-60fps
    m_captureIntervalNs = 1000000000LL / m_frameRate;
    m_outputPath = outputPath.isEmpty() ? generateDefaultOutputPath() : outputPath;
    m_captureOutputGeometry = screen->geometry();
    m_captureRegionGlobal = captureRegion.normalized().intersected(m_captureOutputGeometry);
    m_cropFilter.clear();
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::startRecording: Parameters - includeCursor:" << m_includeCursor << "FPS:" << m_frameRate << "Output:" << m_outputPath;
    
    // 重置计数器
    m_frameCount = 0;
    m_captureSequence = 0;
    m_lastCaptureRequestNs = -1;
    m_pumpOutputFrames = 0;
    m_pumpRepeatedFrames = 0;
    m_pumpBehindMs = 0;
    m_ffmpegStarted = false;
    m_firstFrameWidth = 0;
    m_firstFrameHeight = 0;
    m_firstFrameStride = 0;
    m_firstFrameBuffer.clear();
    if (m_latestFrame)
        m_latestFrame->clear();
    
    // 清空帧缓冲区
    if (m_frameBuffer) {
        m_frameBuffer->clear();
    }

    setState(Starting);

    // 开始ext-capture录制
    if (!m_extCapture->startScreenRecording(screen, includeCursor)) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::startRecording: Failed to start ext-capture";
        setState(Error);
        emit error("Failed to start screen recording");
        return false;
    }

    qCWarning(dsrApp) << "ExtCaptureRecorder::startRecording: *** RECORDING START SUCCESS ***";
    return true;
}

bool ExtCaptureRecorder::startMultiScreenRecording(const QList<QScreen *> &screens,
                                                   bool includeCursor,
                                                   const QString &outputPath,
                                                   int frameRate,
                                                   const QRect &captureRegion)
{
    qCWarning(dsrApp) << "ExtCaptureRecorder::startMultiScreenRecording: screens:"
                      << screens.size() << "region:" << captureRegion;

    if (m_state != Stopped) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::startMultiScreenRecording: already active";
        return false;
    }
    if (!isAvailable()) {
        emit error(QStringLiteral("ext-image-copy-capture protocol not available"));
        return false;
    }
    if (screens.isEmpty() || captureRegion.isEmpty()) {
        emit error(QStringLiteral("Invalid cross-output recording parameters"));
        return false;
    }

    m_multiScreenRecording = true;
    m_includeCursor = includeCursor;
    m_frameRate = qMax(1, qMin(frameRate, 60));
    m_captureIntervalNs = 1000000000LL / m_frameRate;
    m_outputPath = outputPath.isEmpty() ? generateDefaultOutputPath() : outputPath;
    m_captureRegionGlobal = captureRegion.normalized();
    m_captureOutputGeometry = QRect();
    m_cropFilter.clear();
    m_frameCount = 0;
    m_captureSequence = 0;
    m_lastCaptureRequestNs = -1;
    m_pumpOutputFrames = 0;
    m_pumpRepeatedFrames = 0;
    m_pumpBehindMs = 0;
    m_ffmpegStarted = false;
    m_firstFrameWidth = 0;
    m_firstFrameHeight = 0;
    m_firstFrameStride = 0;
    m_firstFrameBuffer.clear();

    if (m_latestFrame)
        m_latestFrame->clear();

    if (m_frameBuffer)
        m_frameBuffer->clear();

    setState(Starting);
    if (!m_extCapture->startMultiScreenRecording(screens, includeCursor, m_captureRegionGlobal)) {
        setState(Error);
        emit error(QStringLiteral("Failed to start multi-output recording"));
        return false;
    }

    return true;
}

void ExtCaptureRecorder::stopRecording()
{
    if (m_state != Recording && m_state != Starting && m_state != Stopping && m_state != Error) {
        return;
    }

    setState(Stopping);
    
    // 立即断开所有帧事件信号连接，避免停止后仍有残留事件触发处理
    if (m_extCapture) {
        disconnect(m_extCapture, &ExtCaptureIntegration::frameReady, this, &ExtCaptureRecorder::onFrameReady);
        disconnect(m_extCapture, &ExtCaptureIntegration::dmaFrameReady, this, &ExtCaptureRecorder::onDmaFrameReady);
        disconnect(m_extCapture, &ExtCaptureIntegration::multiScreenFrameReady,
                   this, &ExtCaptureRecorder::onMultiScreenFrameReady);
    }
    
    // 停止定时器
    m_captureTimer->stop();
    m_captureWatchdog->stop();
    // 确保移除可能的挂起触发
    m_captureTimer->disconnect(this);
    connect(m_captureTimer, &QTimer::timeout, this, &ExtCaptureRecorder::onCaptureTimer);
    
    // 停止ext-capture
    if (m_extCapture) {
        m_extCapture->stopRecording();
    }

    // 停止帧缓冲区
    if (m_frameBuffer) {
        m_frameBuffer->setGetFrame(false);
    }

    // 底层正常会发出 recordingStopped 并在 onRecordingStopped 中收尾。
    // 若会话尚未真正启动（例如启动失败后调用停止），这里补一次收尾。
    if (m_state == Stopping) {
        finalizeRecording();
        emit recordingStopped();
    }
}

ExtCaptureRecorder::RecordState ExtCaptureRecorder::state() const
{
    return m_state;
}

int ExtCaptureRecorder::frameCount() const
{
    return m_frameCount;
}

qint64 ExtCaptureRecorder::recordingDuration() const
{
    if (m_state != Recording || m_startTime == 0) {
        return 0;
    }
    
    qint64 currentTime = (m_state == Recording) ? 
        QDateTime::currentMSecsSinceEpoch() : m_startTime;
    return currentTime - m_startTime;
}

void ExtCaptureRecorder::setOutputPath(const QString &path)
{
    if (m_state == Stopped) {
        m_outputPath = path;
    }
}

QString ExtCaptureRecorder::outputPath() const
{
    return m_outputPath;
}

ExtCaptureFrameBuffer* ExtCaptureRecorder::getFrameBuffer() const
{
    return m_frameBuffer;
}

void ExtCaptureRecorder::onExtCaptureAvailable()
{
    qCWarning(dsrApp) << "ExtCaptureRecorder::onExtCaptureAvailable: *** EXT-CAPTURE AVAILABLE ***";
    emit protocolAvailable();
}

void ExtCaptureRecorder::onExtCaptureUnavailable()
{
    qCDebug(dsrApp) << "ExtCaptureRecorder: ext-capture unavailable";
    
    if (m_state == Recording) {
        stopRecording();
        emit error("ext-capture became unavailable");
    }
}

void ExtCaptureRecorder::onRecordingStarted()
{
    qCWarning(dsrApp) << "ExtCaptureRecorder::onRecordingStarted: *** RECORDING ACTUALLY STARTED ***";
    setState(Recording);
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_captureClock.start();
    m_lastCaptureRequestNs = -1;
    m_pendingFrame = false;
    m_captureTimer->stop();
    m_captureWatchdog->stop();
    m_statTicks = 0;
    m_statRequests = 0;
    m_statBusy = 0;
    m_statFrames = 0;
    m_statLatencyTotalMs = 0;
    m_statLatencyMaxMs = 0;
    m_statProcessTotalMs = 0;
    m_statProcessMaxMs = 0;
    m_captureStatsTimer.start();
    
    if (m_streamingMode) {
        // 流式编码模式：等待第一帧到达时启动FFmpeg进程
        qCWarning(dsrApp) << "ExtCaptureRecorder::onRecordingStarted: Streaming mode - waiting for first frame to start FFmpeg";
    } else {
        // 缓冲模式：启用帧缓冲区
        if (m_frameBuffer) {
            m_frameBuffer->setGetFrame(true);
            qCWarning(dsrApp) << "ExtCaptureRecorder::onRecordingStarted: Frame buffer enabled";
        }
    }
    
    // 采集由“上一帧就绪”驱动；首帧直接发起，避免启动阶段 GUI 初始化把
    // 0ms 定时器推迟到数百毫秒之后。
    onCaptureTimer();
    qCWarning(dsrApp) << "ExtCaptureRecorder::onRecordingStarted: ready-driven capture started, target fps:" << m_frameRate;
    emit recordingStarted();
}

void ExtCaptureRecorder::onRecordingStopped()
{
    // 底层已停止，进行收尾
    setState(Stopped);
    
    // 先完成编码资源的收尾
    finalizeRecording();
    
    // 再对外发出停止信号，保证上层收到时已经真正完成
    emit recordingStopped();
}

void ExtCaptureRecorder::onFrameReady(const void *data, size_t size, int width, int height, int stride, uint64_t timestamp)
{
    if (!data || !m_frameBuffer) {
        return;
    }
    
    // 停止或非录制状态下一律丢弃
    if (m_state != Recording) {
        return;
    }
    recordFrameArrival();
    m_captureWatchdog->stop();

    const qint64 expectedSize = static_cast<qint64>(stride) * height;
    if (expectedSize <= 0 || static_cast<qint64>(size) < expectedSize) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::onFrameReady: short frame"
                          << size << "expected:" << expectedSize;
        scheduleNextCapture();
        return;
    }

    // 初始化帧缓冲区（第一帧时）
    if (m_frameCount == 0) {
        m_frameWidth = width;
        m_frameHeight = height;
        m_frameStride = stride;
        updateCropFilter(width, height);

        if (!m_streamingMode && !m_frameBuffer->initialize(width, height, stride)) {
            qCWarning(dsrApp) << "ExtCaptureRecorder: Failed to initialize frame buffer";
            emit error("Failed to initialize frame buffer");
            scheduleNextCapture();
            return;
        }
        
        qCDebug(dsrApp) << "ExtCaptureRecorder: Frame format:" << width << "x" << height << "stride:" << stride;
    }

    QElapsedTimer processTimer;
    processTimer.start();
    if (m_streamingMode) {
        const QByteArray frameData(static_cast<const char *>(data), expectedSize);
        storeFrameSnapshot(frameData, width, height, stride, timestamp);
    } else if (!m_streamingMode) {
        // 缓冲模式：添加帧到缓冲区
        int64_t frameTimestamp = (timestamp > 0) ? static_cast<int64_t>(timestamp) : QDateTime::currentMSecsSinceEpoch() * 1000000LL;
        m_frameBuffer->appendBuffer(static_cast<const unsigned char*>(data), size, width, height, stride, frameTimestamp);
    }
    const qint64 processMs = processTimer.elapsed();
    m_statProcessTotalMs += processMs;
    m_statProcessMaxMs = qMax(m_statProcessMaxMs, processMs);
    ++m_statFrames;
    
    m_frameCount++;
    
    // 发送进度更新
    emit progressUpdated(m_frameCount, recordingDuration());
    scheduleNextCapture();
}

void ExtCaptureRecorder::onDmaFrameReady(int dmaBufferFd, void *gbmBo, size_t size, int width, int height, int stride, uint64_t timestamp)
{
    Q_UNUSED(size)
    if (dmaBufferFd < 0 || !gbmBo) {
        return;
    }
    
    // 停止或非录制状态下一律丢弃
    if (m_state != Recording) {
        return;
    }
    recordFrameArrival();
    m_captureWatchdog->stop();

    // 初始化帧缓冲区（第一帧时）
    if (m_frameCount == 0) {
        m_frameWidth = width;
        m_frameHeight = height;
        m_frameStride = stride;
        updateCropFilter(width, height);
        
        qCDebug(dsrApp) << "ExtCaptureRecorder: DMA Buffer frame format:" << width << "x" << height << "stride:" << stride;
    }

    if (m_streamingMode) {
        QElapsedTimer processTimer;
        processTimer.start();
        const QByteArray frameData = copyDmaFrame(dmaBufferFd, gbmBo, width, height, stride);
        if (frameData.isEmpty()) {
            // 当前仍在 Session 的帧就绪回调中，异步报错，避免上层同步停止
            // 录制并销毁 Session 造成重入。
            QMetaObject::invokeMethod(this, [this]() {
                onExtCaptureError(QStringLiteral("DMA-BUF 帧映射失败：GBM CPU 映射不可用"));
            }, Qt::QueuedConnection);
            return;
        }
        storeFrameSnapshot(frameData, width, height, stride, timestamp);
        const qint64 processMs = processTimer.elapsed();
        m_statProcessTotalMs += processMs;
        m_statProcessMaxMs = qMax(m_statProcessMaxMs, processMs);
        ++m_statFrames;
    }
    
    m_frameCount++;
    
    // 发送进度更新
    emit progressUpdated(m_frameCount, recordingDuration());
    scheduleNextCapture();
}

void ExtCaptureRecorder::onExtCaptureError(const QString &message)
{
    setState(Error);
    emit error(message);
}

void ExtCaptureRecorder::onMultiScreenFrameReady(const QByteArray &data,
                                                 int width,
                                                 int height,
                                                 int stride,
                                                 uint64_t timestamp)
{
    if (m_state != Recording || data.isEmpty() || width <= 0 || height <= 0 || stride <= 0)
        return;

    recordFrameArrival();
    m_captureWatchdog->stop();

    const qint64 expectedSize = static_cast<qint64>(stride) * height;
    if (data.size() < expectedSize) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::onMultiScreenFrameReady: short frame"
                          << data.size() << "expected:" << expectedSize;
        scheduleNextCapture();
        return;
    }

    m_frameWidth = width;
    m_frameHeight = height;
    m_frameStride = stride;

    QElapsedTimer processTimer;
    processTimer.start();
    storeFrameSnapshot(data, width, height, stride, timestamp);
    const qint64 processMs = processTimer.elapsed();
    m_statProcessTotalMs += processMs;
    m_statProcessMaxMs = qMax(m_statProcessMaxMs, processMs);
    ++m_statFrames;

    ++m_frameCount;
    emit progressUpdated(m_frameCount, recordingDuration());
    scheduleNextCapture();
}

void ExtCaptureRecorder::onCaptureTimer()
{
    if (m_state != Recording || !m_extCapture) {
        return;
    }
    if (m_pendingFrame) {
        return;
    }

    ++m_statTicks;
    QElapsedTimer requestCallTimer;
    requestCallTimer.start();
    const bool requested = m_multiScreenRecording
            ? m_extCapture->captureMultiScreenFrame()
            : m_extCapture->captureFrame();
    const qint64 requestCallNs = requestCallTimer.nsecsElapsed();
    m_statRequestCallTotalNs += requestCallNs;
    m_statRequestCallMaxNs = qMax(m_statRequestCallMaxNs, requestCallNs);
    if (requested) {
        ++m_statRequests;
        m_pendingFrame = true;
        m_pendingFrameTimer.start();
        m_lastCaptureRequestNs = m_captureClock.nsecsElapsed();
        m_captureWatchdog->start();
    } else {
        ++m_statBusy;
        // 会话可能仍在切换状态，稍后重试；帧就绪后会直接触发下一帧请求。
        m_captureTimer->start(qBound(2, 1000 / qMax(1, m_frameRate), 10));
    }

    if (qEnvironmentVariableIsSet("DSR_CAPTURE_STATS")
            && m_captureStatsTimer.elapsed() >= 1000) {
        logCaptureStats();
    }
}

void ExtCaptureRecorder::onCaptureWatchdog()
{
    if (m_state != Recording || !m_pendingFrame)
        return;

    qCWarning(dsrApp) << "ExtCaptureRecorder: frame request timed out, retrying";
    m_pendingFrame = false;
    scheduleNextCapture();
}

void ExtCaptureRecorder::scheduleNextCapture()
{
    if (m_state != Recording || m_pendingFrame)
        return;

    qint64 delayMs = 0;
    if (m_lastCaptureRequestNs >= 0) {
        const qint64 elapsedNs = m_captureClock.nsecsElapsed() - m_lastCaptureRequestNs;
        if (elapsedNs < m_captureIntervalNs) {
            delayMs = (m_captureIntervalNs - elapsedNs + 999999LL) / 1000000LL;
        }
    }
    // 至少让事件循环跑一轮，确保上一帧的 destroy/flush 已交给 compositor。
    m_captureTimer->start(static_cast<int>(qMax<qint64>(1, delayMs)));
}

void ExtCaptureRecorder::logCaptureStats()
{
    const qint64 elapsedMs = m_captureStatsTimer.restart();
    const double fps = elapsedMs > 0
            ? m_statFrames * 1000.0 / elapsedMs
            : 0.0;
    const double avgLatency = m_statFrames > 0
            ? static_cast<double>(m_statLatencyTotalMs) / m_statFrames
            : 0.0;
    const double avgProcess = m_statFrames > 0
            ? static_cast<double>(m_statProcessTotalMs) / m_statFrames
            : 0.0;
    const double avgRequestCallMs = m_statTicks > 0
            ? static_cast<double>(m_statRequestCallTotalNs) / m_statTicks / 1000000.0
            : 0.0;
    const double maxRequestCallMs = static_cast<double>(m_statRequestCallMaxNs) / 1000000.0;
    qCWarning(dsrApp) << "capture stats:"
                      << "windowMs:" << elapsedMs
                      << "ticks:" << m_statTicks
                      << "requests:" << m_statRequests
                      << "busy:" << m_statBusy
                      << "frames:" << m_statFrames
                      << "fps:" << fps
                      << "avgRequestToReadyMs:" << avgLatency
                      << "maxRequestToReadyMs:" << m_statLatencyMaxMs
                      << "avgRequestCallMs:" << avgRequestCallMs
                      << "maxRequestCallMs:" << maxRequestCallMs
                      << "avgProcessMs:" << avgProcess
                      << "maxProcessMs:" << m_statProcessMaxMs
                      << "outputFrames:" << m_pumpOutputFrames
                      << "repeatedFrames:" << m_pumpRepeatedFrames
                      << "pumpBehindMs:" << m_pumpBehindMs;

    m_statTicks = 0;
    m_statRequests = 0;
    m_statBusy = 0;
    m_statFrames = 0;
    m_statLatencyTotalMs = 0;
    m_statLatencyMaxMs = 0;
    m_statProcessTotalMs = 0;
    m_statProcessMaxMs = 0;
    m_statRequestCallTotalNs = 0;
    m_statRequestCallMaxNs = 0;
}

void ExtCaptureRecorder::onFramePumpStats(int outputFrames, int repeatedFrames, qint64 behindMs)
{
    m_pumpOutputFrames = outputFrames;
    m_pumpRepeatedFrames = repeatedFrames;
    m_pumpBehindMs = behindMs;
}

void ExtCaptureRecorder::onFramePumpError(const QString &message)
{
    qCCritical(dsrApp) << "ExtCaptureRecorder: frame pump failed:" << message;
    emit error(message);
    if (m_state == Recording || m_state == Starting)
        stopRecording();
}

void ExtCaptureRecorder::recordFrameArrival()
{
    if (!m_pendingFrame) {
        return;
    }
    const qint64 latencyMs = m_pendingFrameTimer.elapsed();
    m_statLatencyTotalMs += latencyMs;
    m_statLatencyMaxMs = qMax(m_statLatencyMaxMs, latencyMs);
    m_pendingFrame = false;
}

bool ExtCaptureRecorder::ensureFramePump(int width, int height, int stride)
{
    if (m_framePump)
        return true;

    QDir outputDir = QFileInfo(m_outputPath).dir();
    if (!outputDir.exists() && !outputDir.mkpath(".")) {
        qCCritical(dsrApp) << "ExtCaptureRecorder: failed to create output directory" << outputDir.path();
        return false;
    }

    m_framePumpThread = new QThread(this);
    m_framePump = new FramePumpWorker(m_frameRate, m_outputPath, m_cropFilter,
                                      m_latestFrame, m_audioOptions);
    m_framePump->moveToThread(m_framePumpThread);
    connect(m_framePump, &FramePumpWorker::statsUpdated,
            this, &ExtCaptureRecorder::onFramePumpStats);
    connect(m_framePump, &FramePumpWorker::errorOccurred,
            this, &ExtCaptureRecorder::onFramePumpError);
    m_framePumpThread->start();

    QMetaObject::invokeMethod(m_framePump, "start", Qt::QueuedConnection,
                              Q_ARG(int, width), Q_ARG(int, height), Q_ARG(int, stride));
    return true;
}

void ExtCaptureRecorder::stopFramePump()
{
    if (!m_framePump && !m_framePumpThread)
        return;

    if (m_framePumpThread && m_framePumpThread->isRunning()) {
        if (m_framePump) {
            QMetaObject::invokeMethod(m_framePump, "stop", Qt::BlockingQueuedConnection);
            int outputFrames = 0;
            int repeatedFrames = 0;
            QMetaObject::invokeMethod(m_framePump, [worker = m_framePump, &outputFrames, &repeatedFrames]() {
                outputFrames = worker->outputFrames();
                repeatedFrames = worker->repeatedFrames();
                delete worker;
            },
                                      Qt::BlockingQueuedConnection);
            m_pumpOutputFrames = outputFrames;
            m_pumpRepeatedFrames = repeatedFrames;
            m_framePump = nullptr;
        }
        m_framePumpThread->quit();
        if (!m_framePumpThread->wait(8000))
            qCWarning(dsrApp) << "ExtCaptureRecorder: frame pump thread did not stop in time";
    } else if (m_framePump) {
        delete m_framePump;
        m_framePump = nullptr;
    }

    delete m_framePumpThread;
    m_framePumpThread = nullptr;
}

bool ExtCaptureRecorder::storeFrameSnapshot(const QByteArray &data, int width, int height,
                                            int stride, uint64_t timestamp)
{
    if (!m_latestFrame || data.isEmpty() || width <= 0 || height <= 0 || stride <= 0)
        return false;

    const qint64 expectedSize = static_cast<qint64>(stride) * height;
    if (data.size() < expectedSize) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::storeFrameSnapshot: short frame"
                          << data.size() << "expected:" << expectedSize;
        return false;
    }

    CaptureFrameSnapshotPtr snapshot = std::make_shared<CaptureFrameSnapshot>();
    if (data.size() == expectedSize)
        snapshot->data = data;
    else
        snapshot->data = QByteArray(data.constData(), expectedSize);
    snapshot->width = width;
    snapshot->height = height;
    snapshot->stride = stride;
    snapshot->ptsNs = static_cast<qint64>(timestamp);
    snapshot->sequence = ++m_captureSequence;
    m_latestFrame->set(snapshot);

    return ensureFramePump(width, height, stride);
}

QByteArray ExtCaptureRecorder::copyDmaFrame(int dmaBufferFd, void *gbmBo, int width,
                                            int height, int stride)
{
#ifdef ENABLE_UNIT_TEST
    Q_UNUSED(dmaBufferFd)
    Q_UNUSED(gbmBo)
    Q_UNUSED(width)
    Q_UNUSED(height)
    Q_UNUSED(stride)
    return QByteArray();
#else
    if (dmaBufferFd < 0 || !gbmBo || width <= 0 || height <= 0)
        return QByteArray();

    gbm_bo *bo = static_cast<gbm_bo *>(gbmBo);
    const int boWidth = static_cast<int>(gbm_bo_get_width(bo));
    const int boHeight = static_cast<int>(gbm_bo_get_height(bo));
    const int boStride = static_cast<int>(gbm_bo_get_stride(bo));
    if (boWidth <= 0 || boHeight <= 0 || boStride <= 0)
        return QByteArray();

    const uint64_t modifier = gbm_bo_get_modifier(bo);
    if (modifier != DRM_FORMAT_MOD_LINEAR) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::copyDmaFrame: non-linear DMA-BUF"
                          << "modifier:" << Qt::hex << modifier;
        return QByteArray();
    }

    const int outHeight = qMin(height, boHeight);
    const int outStride = stride > 0 ? stride : boStride;
    const qint64 outSize = static_cast<qint64>(outStride) * outHeight;
    if (outSize <= 0)
        return QByteArray();

    QByteArray result;
    result.resize(static_cast<int>(outSize));
    char *dst = result.data();

    // GBM mapping performs the required DMA-BUF transfer/cache synchronization
    // before CPU access. A raw mmap() may observe incomplete GPU writes and
    // produce sheared, stale scanlines even for a LINEAR buffer.
    uint32_t mapStride = 0;
    void *mapHandle = nullptr;
    void *gbmData = gbm_bo_map(bo, 0, 0, boWidth, boHeight,
                               GBM_BO_TRANSFER_READ, &mapStride, &mapHandle);
    if (gbmData) {
        const char *src = static_cast<const char *>(gbmData);
        for (int y = 0; y < outHeight; ++y)
            memcpy(dst + static_cast<qint64>(y) * outStride,
                   src + static_cast<qint64>(y) * mapStride,
                   qMin(outStride, static_cast<int>(mapStride)));
        gbm_bo_unmap(bo, mapHandle);
        return result;
    }

    // 裸 mmap() 不具备 DMA-BUF 的 transfer/cache 同步语义，也不理解
    // modifier。生产路径只接受 GBM 映射；失败时由上层报告错误。
    qCWarning(dsrApp) << "ExtCaptureRecorder::copyDmaFrame: GBM map failed";
    return QByteArray();
#endif
}

void ExtCaptureRecorder::setState(RecordState newState)
{
    if (m_state != newState) {
        m_state = newState;
    }
}

void ExtCaptureRecorder::initializeCapture()
{
    // 这里可以添加录制初始化逻辑
    // 例如：创建输出文件、初始化编码器等
    qCDebug(dsrApp) << "ExtCaptureRecorder: Initialize capture for output:" << m_outputPath;
}

void ExtCaptureRecorder::finalizeRecording()
{
    m_captureTimer->stop();
    m_captureWatchdog->stop();

    if (m_framePump) {
        stopFramePump();
        m_ffmpegStarted = false;
        if (!QFile::exists(m_outputPath) || QFileInfo(m_outputPath).size() == 0) {
            qCCritical(dsrApp) << "ExtCaptureRecorder::finalizeRecording: frame pump produced no video";
        } else {
            qCWarning(dsrApp) << "ExtCaptureRecorder::finalizeRecording: frame pump finalized without remux,"
                              << "output frames:" << m_pumpOutputFrames
                              << "repeated frames:" << m_pumpRepeatedFrames;
        }
    } else if (m_streamingMode && m_ffmpegStarted && m_ffmpegProcess) {
        // 流式编码模式：完成FFmpeg进程
        m_ffmpegProcess->closeWriteChannel();
        
        if (m_ffmpegProcess->waitForFinished(5000)) {
            int exitCode = m_ffmpegProcess->exitCode();
            if (exitCode == 0) {
                // 验证文件是否创建成功
                if (!QFile::exists(m_outputPath) || QFileInfo(m_outputPath).size() == 0) {
                    qCCritical(dsrApp) << "ExtCaptureRecorder::finalizeRecording: Video file was not created or is empty";
                }
            } else {
                qCCritical(dsrApp) << "ExtCaptureRecorder::finalizeRecording: FFmpeg process failed with exit code:" << exitCode;
                qCCritical(dsrApp) << "ExtCaptureRecorder::finalizeRecording: FFmpeg stderr:" << m_ffmpegProcess->readAllStandardError();
            }
        } else {
            qCCritical(dsrApp) << "ExtCaptureRecorder::finalizeRecording: FFmpeg process did not finish in 5 seconds";
            m_ffmpegProcess->kill();
        }
        
        m_ffmpegProcess->deleteLater();
        m_ffmpegProcess = nullptr;
        m_ffmpegStarted = false;

    } else if (m_frameBuffer && m_frameBuffer->hasFrames()) {
        // 缓冲模式：创建视频文件
        if (!createVideoFile()) {
            qCCritical(dsrApp) << "ExtCaptureRecorder::finalizeRecording: Failed to create video file";
        }
    }
    
    // 重置frame buffer以便下次录制
    if (m_frameBuffer) {
        m_frameBuffer->reset();
    }
    if (m_latestFrame)
        m_latestFrame->clear();
    
    // 重置录制状态和计数器，为下次录制做准备
    m_frameCount = 0;
    m_startTime = 0;
    m_frameWidth = 0;
    m_frameHeight = 0;
    m_frameStride = 0;
    m_lastCaptureRequestNs = -1;
    
    setState(Stopped);
}

void ExtCaptureRecorder::processFrameQueue()
{
    // 不再需要处理队列，帧数据直接存储在ExtCaptureFrameBuffer中
    // 外部编码器可以通过getFrameBuffer()获取帧数据
    if (m_frameBuffer) {
        qCDebug(dsrApp) << "ExtCaptureRecorder: Frame buffer status: " << m_frameBuffer->frameCount() << " frames available";
    }
}

bool ExtCaptureRecorder::createVideoFile()
{
    if (!m_frameBuffer || !m_frameBuffer->hasFrames()) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::createVideoFile: No frames available";
        return false;
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::createVideoFile: Creating video file with" << m_frameBuffer->frameCount() << "frames";
    
    // 创建输出目录
    QDir outputDir = QFileInfo(m_outputPath).dir();
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            qCCritical(dsrApp) << "ExtCaptureRecorder::createVideoFile: Failed to create output directory:" << outputDir.path();
            return false;
        }
    }
    
    // 使用FFmpeg创建视频文件 - WL_SHM_FORMAT_XBGR8888 正确映射为 rgba 格式
    QStringList arguments;
    arguments << "-y"
              << "-f" << "rawvideo"
              << "-pix_fmt" << "rgba"
              << "-s" << QString("%1x%2").arg(m_frameWidth).arg(m_frameHeight)
              << "-r" << QString::number(m_frameRate)
              << "-i" << "-";
    if (!m_cropFilter.isEmpty())
        arguments << "-vf" << m_cropFilter;
    arguments << "-c:v" << "libx264"
              << "-pix_fmt" << "yuv420p"
              << m_outputPath;

    qCWarning(dsrApp) << "ExtCaptureRecorder::createVideoFile: FFmpeg arguments:" << arguments;

    QProcess ffmpegProcess;
    ffmpegProcess.start("ffmpeg", arguments);
    
    if (!ffmpegProcess.waitForStarted()) {
        qCCritical(dsrApp) << "ExtCaptureRecorder::createVideoFile: Failed to start FFmpeg process";
        return false;
    }
    
    // 写入帧数据到FFmpeg
    int framesWritten = 0;
    while (m_frameBuffer->hasFrames()) {
        ExtFrameData frame;
        if (m_frameBuffer->getFrame(frame)) {
            // 写入原始帧数据到FFmpeg stdin
            qint64 bytesWritten = ffmpegProcess.write(reinterpret_cast<const char*>(frame.data), 
                                                    frame.height * frame.stride);
            if (bytesWritten != frame.height * frame.stride) {
                qCWarning(dsrApp) << "ExtCaptureRecorder::createVideoFile: Failed to write frame" << frame.index;
                break;
            }
            framesWritten++;
        } else {
            break;
        }
    }
    
    // 关闭FFmpeg输入
    ffmpegProcess.closeWriteChannel();
    
    // 等待FFmpeg完成
    if (!ffmpegProcess.waitForFinished(30000)) { // 30秒超时
        qCCritical(dsrApp) << "ExtCaptureRecorder::createVideoFile: FFmpeg process timeout";
        ffmpegProcess.kill();
        return false;
    }
    
    int exitCode = ffmpegProcess.exitCode();
    if (exitCode != 0) {
        qCCritical(dsrApp) << "ExtCaptureRecorder::createVideoFile: FFmpeg failed with exit code:" << exitCode;
        qCCritical(dsrApp) << "FFmpeg stderr:" << ffmpegProcess.readAllStandardError();
        return false;
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::createVideoFile: Successfully wrote" << framesWritten << "frames to" << m_outputPath;
    
    // 验证文件是否创建成功
    if (QFile::exists(m_outputPath) && QFileInfo(m_outputPath).size() > 0) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::createVideoFile: Video file created successfully, size:" << QFileInfo(m_outputPath).size() << "bytes";
        return true;
    } else {
        qCCritical(dsrApp) << "ExtCaptureRecorder::createVideoFile: Video file was not created or is empty";
        return false;
    }
}

QString ExtCaptureRecorder::generateDefaultOutputPath()
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documentsPath.isEmpty()) {
        documentsPath = QDir::homePath();
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    QString filename = QString("ext-capture-recording-%1.mp4").arg(timestamp);
    
    return QDir(documentsPath).filePath(filename);
}

bool ExtCaptureRecorder::startFFmpegProcess()
{
    if (m_ffmpegProcess) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::startFFmpegProcess: Process already exists";
        return false;
    }
    
    // 创建输出目录
    QDir outputDir = QFileInfo(m_outputPath).dir();
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            qCCritical(dsrApp) << "ExtCaptureRecorder::startFFmpegProcess: Failed to create output directory:" << outputDir.path();
            return false;
        }
    }
    
    QStringList arguments;
    arguments << "-y"
              << "-f" << "rawvideo"
              << "-pix_fmt" << "rgba"
              << "-s" << QString("%1x%2").arg(m_frameWidth).arg(m_frameHeight)
              << "-r" << QString::number(m_frameRate)
              << "-i" << "-";
    if (!m_cropFilter.isEmpty())
        arguments << "-vf" << m_cropFilter;
    arguments << "-c:v" << "libx264"
              << "-pix_fmt" << "yuv420p"
              << m_outputPath;

    qCWarning(dsrApp) << "ExtCaptureRecorder::startFFmpegProcess: Starting FFmpeg with arguments:" << arguments;
    
    m_ffmpegProcess = new QProcess(this);
    
    // 连接错误信号
    connect(m_ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                qCWarning(dsrApp) << "ExtCaptureRecorder: FFmpeg process finished with code:" << exitCode << "status:" << exitStatus;
            });
    
    connect(m_ffmpegProcess, &QProcess::errorOccurred,
            [this](QProcess::ProcessError error) {
                qCCritical(dsrApp) << "ExtCaptureRecorder: FFmpeg process error:" << error;
            });
    
    // 启动进程
    m_ffmpegProcess->start("ffmpeg", arguments);
    
    if (!m_ffmpegProcess->waitForStarted(3000)) {
        qCCritical(dsrApp) << "ExtCaptureRecorder::startFFmpegProcess: Failed to start FFmpeg process:" << m_ffmpegProcess->errorString();
        m_ffmpegProcess->deleteLater();
        m_ffmpegProcess = nullptr;
        return false;
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::startFFmpegProcess: FFmpeg process started successfully";
    return true;
}

bool ExtCaptureRecorder::startDmaFFmpegProcess()
{
#ifdef ENABLE_UNIT_TEST
    return false; // 单测桩：DMA-BUF/GBM 需真实 Wayland，跳过
#else
    if (m_ffmpegProcess) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::startDmaFFmpegProcess: FFmpeg process already exists";
        return false;
    }
    
    if (m_frameWidth <= 0 || m_frameHeight <= 0) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::startDmaFFmpegProcess: Invalid frame dimensions:" << m_frameWidth << "x" << m_frameHeight;
        return false;
    }
    
    m_ffmpegProcess = new QProcess(this);
    
    // 构建支持DMA Buffer的FFmpeg命令
    // 使用硬件加速编码器 (VAAPI/NVENC等)
    QStringList arguments;
    
    // 输入参数 - 使用硬件解码器接受DMA Buffer
    arguments << "-hide_banner"
              << "-loglevel" << "warning"
              << "-hwaccel" << "auto"                          // 自动选择硬件加速
              << "-f" << "rawvideo"                            // 原始视频格式
              << "-pix_fmt" << "rgba"                          // 像素格式：XBGR8888 -> RGBA (修复颜色通道顺序)
              << "-s" << QString("%1x%2").arg(m_frameWidth).arg(m_frameHeight)  // 分辨率
              << "-r" << QString::number(m_frameRate)          // 帧率
              << "-i" << "-";                                  // 从stdin读取（临时方案）

    if (!m_cropFilter.isEmpty())
        arguments << "-vf" << m_cropFilter;
    
    arguments << "-c:v" << "libx264"                          // 使用软件H.264编码器
              << "-preset" << "fast"                          // 编码预设
              << "-crf" << "23"                               // 质量参数
              << "-pix_fmt" << "yuv420p"                      // 输出像素格式
              << "-y"                                         // 覆盖输出文件
              << m_outputPath;                                // 输出文件
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::startDmaFFmpegProcess: Starting DMA Buffer FFmpeg with command:";
    qCWarning(dsrApp) << "ffmpeg" << arguments.join(" ");
    
    // 连接信号
    connect(m_ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                qCWarning(dsrApp) << "ExtCaptureRecorder: DMA Buffer FFmpeg process finished, exit code:" << exitCode << "status:" << exitStatus;
                if (exitCode != 0) {
                    qCWarning(dsrApp) << "ExtCaptureRecorder: DMA Buffer FFmpeg stderr:" << m_ffmpegProcess->readAllStandardError();
                }
            });

    connect(m_ffmpegProcess, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
                qCCritical(dsrApp) << "ExtCaptureRecorder: DMA Buffer FFmpeg process error:" << error;
                setState(Error);
                emit this->error("DMA Buffer FFmpeg process error");
            });
    
    m_ffmpegProcess->start("ffmpeg", arguments);
    
    if (!m_ffmpegProcess->waitForStarted(5000)) {
        qCCritical(dsrApp) << "ExtCaptureRecorder::startDmaFFmpegProcess: Failed to start DMA Buffer FFmpeg process:" << m_ffmpegProcess->errorString();
        delete m_ffmpegProcess;
        m_ffmpegProcess = nullptr;
        return false;
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::startDmaFFmpegProcess: DMA Buffer FFmpeg process started successfully";
    return true;
#endif
}

bool ExtCaptureRecorder::processDmaBufferFrame(int dmaBufferFd, void *gbmBo, int width, int height, int stride)
{
#ifdef ENABLE_UNIT_TEST
    Q_UNUSED(dmaBufferFd)
    Q_UNUSED(gbmBo)
    Q_UNUSED(width)
    Q_UNUSED(height)
    Q_UNUSED(stride)
    return false; // 单测桩：DMA-BUF/GBM 需真实 Wayland，跳过
#else
    if (!m_ffmpegProcess || m_ffmpegProcess->state() != QProcess::Running) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: FFmpeg process not running";
        return false;
    }

    // 与 onDmaFrameReady 共用同一条 GBM 映射路径。禁止在这里另写裸 mmap
    // 或生成测试图案，否则会掩盖真实驱动/格式问题。
    const QByteArray frameData = copyDmaFrame(dmaBufferFd, gbmBo, width, height, stride);
    if (frameData.isEmpty()) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: failed to map DMA-BUF";
        return false;
    }

    const qint64 bytesWritten = m_ffmpegProcess->write(frameData);
    if (bytesWritten != frameData.size()) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: short FFmpeg write"
                          << "expected:" << frameData.size() << "written:" << bytesWritten;
        return false;
    }

    return true;
#endif
}

void ExtCaptureRecorder::updateCropFilter(int frameWidth, int frameHeight)
{
    m_cropFilter.clear();
    if (frameWidth <= 0 || frameHeight <= 0 || m_captureRegionGlobal.isEmpty()
            || m_captureOutputGeometry.isEmpty()) {
        return;
    }

    const qreal scaleX = static_cast<qreal>(frameWidth) / m_captureOutputGeometry.width();
    const qreal scaleY = static_cast<qreal>(frameHeight) / m_captureOutputGeometry.height();
    const QRect local = m_captureRegionGlobal.translated(-m_captureOutputGeometry.topLeft());

    int x = qBound(0, qRound(local.x() * scaleX), frameWidth - 2);
    int y = qBound(0, qRound(local.y() * scaleY), frameHeight - 2);
    int width = qMin(qRound(local.width() * scaleX), frameWidth - x);
    int height = qMin(qRound(local.height() * scaleY), frameHeight - y);
    width = qMax(2, width - (width % 2));
    height = qMax(2, height - (height % 2));
    x -= x % 2;
    y -= y % 2;

    if (x == 0 && y == 0 && width == frameWidth && height == frameHeight)
        return;
    m_cropFilter = QStringLiteral("crop=%1:%2:%3:%4").arg(width).arg(height).arg(x).arg(y);
}
