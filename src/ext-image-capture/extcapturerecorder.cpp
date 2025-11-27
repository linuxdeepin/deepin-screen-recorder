// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extcapturerecorder.h"
#include "extcaptureintegration.h"
#include "extcaptureframebuffer.h"
#include "../utils/log.h"

#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QProcess>
#include <QFileInfo>

ExtCaptureRecorder::ExtCaptureRecorder(QObject *parent)
    : QObject(parent)
    , m_extCapture(new ExtCaptureIntegration(this))
    , m_frameBuffer(new ExtCaptureFrameBuffer(this))
    , m_captureTimer(new QTimer(this))
    , m_state(Stopped)
    , m_frameRate(30)
    , m_frameCount(0)
    , m_startTime(0)
    , m_includeCursor(false)
    , m_frameWidth(0)
    , m_frameHeight(0)
    , m_frameStride(0)
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
    connect(m_extCapture, &ExtCaptureIntegration::error,
            this, &ExtCaptureRecorder::onExtCaptureError);

    // 设置捕获定时器
    m_captureTimer->setSingleShot(false);
    connect(m_captureTimer, &QTimer::timeout,
            this, &ExtCaptureRecorder::onCaptureTimer);

    qDebug() << "ExtCaptureRecorder initialized";
}

ExtCaptureRecorder::~ExtCaptureRecorder()
{
    if (m_state == Recording) {
        stopRecording();
    }
}

bool ExtCaptureRecorder::isAvailable() const
{
    return m_extCapture && m_extCapture->isAvailable();
}

bool ExtCaptureRecorder::startRecording(QScreen *screen, bool includeCursor, 
                                       const QString &outputPath, int frameRate)
{
    if (m_state != Stopped) {
        qWarning() << "ExtCaptureRecorder: Already recording or in transition";
        return false;
    }

    if (!isAvailable()) {
        qWarning() << "ExtCaptureRecorder: ext-image-copy-capture not available";
        emit error("ext-image-copy-capture protocol not available");
        return false;
    }

    if (!screen) {
        qWarning() << "ExtCaptureRecorder: Invalid screen provided";
        emit error("Invalid screen");
        return false;
    }

    // 设置录制参数
    m_includeCursor = includeCursor;
    m_frameRate = qMax(1, qMin(frameRate, 60)); // 限制在1-60fps
    m_outputPath = outputPath.isEmpty() ? generateDefaultOutputPath() : outputPath;
    
    // 重置计数器
    m_frameCount = 0;
    
    // 清空帧缓冲区
    if (m_frameBuffer) {
        m_frameBuffer->clear();
    }

    setState(Starting);

    // 开始ext-capture录制
    if (!m_extCapture->startScreenRecording(screen, includeCursor)) {
        qWarning() << "ExtCaptureRecorder: Failed to start ext-capture";
        setState(Error);
        emit error("Failed to start screen recording");
        return false;
    }

    qDebug() << "ExtCaptureRecorder: Recording started for screen:" << screen->name()
             << "FPS:" << m_frameRate << "Output:" << m_outputPath;
    return true;
}

void ExtCaptureRecorder::stopRecording()
{
    if (m_state != Recording) {
        return;
    }

    setState(Stopping);
    
    // 停止定时器
    m_captureTimer->stop();
    
    // 停止ext-capture
    if (m_extCapture) {
        m_extCapture->stopRecording();
    }

    // 停止帧缓冲区
    if (m_frameBuffer) {
        m_frameBuffer->setGetFrame(false);
    }
    
    // 完成录制
    finalizeRecording();

    qDebug() << "ExtCaptureRecorder: Recording stopped, captured" << m_frameCount << "frames";
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
    if (m_startTime == 0) {
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
    qCWarning(dsrApp) << "ExtCaptureRecorder: ext-capture available";
    emit protocolAvailable();
}

void ExtCaptureRecorder::onExtCaptureUnavailable()
{
    qDebug() << "ExtCaptureRecorder: ext-capture unavailable";
    
    if (m_state == Recording) {
        stopRecording();
        emit error("ext-capture became unavailable");
    }
}

void ExtCaptureRecorder::onRecordingStarted()
{
    setState(Recording);
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 启用帧缓冲区
    if (m_frameBuffer) {
        m_frameBuffer->setGetFrame(true);
    }
    
    // 设置定时器间隔
    int interval = 1000 / m_frameRate; // 毫秒
    m_captureTimer->start(interval);
    
    qDebug() << "ExtCaptureRecorder: Recording started, timer interval:" << interval << "ms";
    emit recordingStarted();
}

void ExtCaptureRecorder::onRecordingStopped()
{
    setState(Stopped);
    emit recordingStopped();
}

void ExtCaptureRecorder::onFrameReady(const void *data, size_t size, int width, int height, int stride, uint64_t timestamp)
{
    qCWarning(dsrApp) << "ExtCaptureRecorder::onFrameReady: *** FRAME DATA RECEIVED *** size:" << size << "width:" << width << "height:" << height << "stride:" << stride;
    
    if (!data || !m_frameBuffer) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::onFrameReady: Rejected - data:" << (data ? "valid" : "null") << "frameBuffer:" << (m_frameBuffer ? "valid" : "null");
        return;
    }
    
    // 如果之前是错误状态，现在有帧数据了，重置为录制状态
    if (m_state == Error) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::onFrameReady: Recovering from error state, setting to Recording";
        setState(Recording);
    }
    
    if (m_state != Recording) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::onFrameReady: Rejected - state:" << m_state;
        return;
    }

    // 初始化帧缓冲区（第一帧时）
    if (m_frameCount == 0) {
        m_frameWidth = width;
        m_frameHeight = height;
        m_frameStride = stride;
        
        if (!m_frameBuffer->initialize(width, height, stride)) {
            qWarning() << "ExtCaptureRecorder: Failed to initialize frame buffer";
            emit error("Failed to initialize frame buffer");
            return;
        }
        
        qDebug() << "ExtCaptureRecorder: Frame format:" << width << "x" << height << "stride:" << stride;
    }

    // 添加帧到缓冲区
    int64_t frameTimestamp = (timestamp > 0) ? static_cast<int64_t>(timestamp) : QDateTime::currentMSecsSinceEpoch() * 1000000LL;
    m_frameBuffer->appendBuffer(static_cast<const unsigned char*>(data), size, width, height, stride, frameTimestamp);
    
    m_frameCount++;
    
    // 发送进度更新
    emit progressUpdated(m_frameCount, recordingDuration());
    
    qDebug() << "ExtCaptureRecorder: Frame" << m_frameCount << "captured, size:" << size;
}

void ExtCaptureRecorder::onExtCaptureError(const QString &message)
{
    qWarning() << "ExtCaptureRecorder: ext-capture error:" << message;
    setState(Error);
    emit error(message);
}

void ExtCaptureRecorder::onCaptureTimer()
{
    if (m_state != Recording || !m_extCapture) {
        return;
    }

    // 触发帧捕获
    if (!m_extCapture->captureFrame()) {
        qWarning() << "ExtCaptureRecorder: Failed to capture frame" << m_frameCount;
    }
}

void ExtCaptureRecorder::setState(RecordState newState)
{
    if (m_state != newState) {
        RecordState oldState = m_state;
        m_state = newState;
        qDebug() << "ExtCaptureRecorder: State changed from" << oldState << "to" << newState;
    }
}

void ExtCaptureRecorder::initializeCapture()
{
    // 这里可以添加录制初始化逻辑
    // 例如：创建输出文件、初始化编码器等
    qDebug() << "ExtCaptureRecorder: Initialize capture for output:" << m_outputPath;
}

void ExtCaptureRecorder::finalizeRecording()
{
    qCWarning(dsrApp) << "ExtCaptureRecorder: Finalize recording";
    qCWarning(dsrApp) << "  - Total frames:" << m_frameCount;
    qCWarning(dsrApp) << "  - Duration:" << recordingDuration() << "ms";
    qCWarning(dsrApp) << "  - Frame rate:" << (m_frameCount * 1000.0 / recordingDuration()) << "fps";
    qCWarning(dsrApp) << "  - Output path:" << m_outputPath;
    
    if (m_frameBuffer && m_frameBuffer->hasFrames()) {
        qCWarning(dsrApp) << "  - Frame buffer has remaining frames:" << m_frameBuffer->frameCount();
        
        // 创建简单的视频文件（使用FFmpeg）
        if (createVideoFile()) {
            qCWarning(dsrApp) << "  - Video file created successfully:" << m_outputPath;
        } else {
            qCCritical(dsrApp) << "  - Failed to create video file";
        }
    } else {
        qCWarning(dsrApp) << "  - No frames available for encoding";
    }
    
    setState(Stopped);
}

void ExtCaptureRecorder::processFrameQueue()
{
    // 不再需要处理队列，帧数据直接存储在ExtCaptureFrameBuffer中
    // 外部编码器可以通过getFrameBuffer()获取帧数据
    if (m_frameBuffer) {
        qDebug() << "ExtCaptureRecorder: Frame buffer status: " << m_frameBuffer->frameCount() << " frames available";
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
    
    // 使用FFmpeg创建视频文件
    QString ffmpegCmd = QString("ffmpeg -y -f rawvideo -pix_fmt bgra -s %1x%2 -r %3 -i - -c:v libx264 -pix_fmt yuv420p \"%4\"")
                       .arg(m_frameWidth)
                       .arg(m_frameHeight)
                       .arg(m_frameRate)
                       .arg(m_outputPath);
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::createVideoFile: FFmpeg command:" << ffmpegCmd;
    
    QProcess ffmpegProcess;
    ffmpegProcess.start("sh", QStringList() << "-c" << ffmpegCmd);
    
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
    QString filename = QString("ext-capture-recording-%1.raw").arg(timestamp);
    
    return QDir(documentsPath).filePath(filename);
}

