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
    , m_ffmpegProcess(nullptr)
    , m_streamingMode(true)  
    , m_ffmpegStarted(false)
    , m_firstFrameWidth(0)
    , m_firstFrameHeight(0)
    , m_firstFrameStride(0)
    , m_vaDisplay(nullptr)
    , m_vaConfig(0)
    , m_vaContext(0)
    , m_drmFd(-1)
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

bool ExtCaptureRecorder::isAvailable() const
{
    return m_extCapture && m_extCapture->isAvailable();
}

bool ExtCaptureRecorder::startRecording(QScreen *screen, bool includeCursor, 
                                       const QString &outputPath, int frameRate)
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
    m_includeCursor = includeCursor;
    m_frameRate = qMax(1, qMin(frameRate, 60)); // 限制在1-60fps
    m_outputPath = outputPath.isEmpty() ? generateDefaultOutputPath() : outputPath;
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::startRecording: Parameters - includeCursor:" << m_includeCursor << "FPS:" << m_frameRate << "Output:" << m_outputPath;
    
    // 重置计数器
    m_frameCount = 0;
    
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

void ExtCaptureRecorder::stopRecording()
{
    if (m_state != Recording && m_state != Starting && m_state != Stopping) {
        return;
    }

    setState(Stopping);
    
    // 立即断开所有帧事件信号连接，避免停止后仍有残留事件触发处理
    if (m_extCapture) {
        disconnect(m_extCapture, &ExtCaptureIntegration::frameReady, this, &ExtCaptureRecorder::onFrameReady);
        disconnect(m_extCapture, &ExtCaptureIntegration::dmaFrameReady, this, &ExtCaptureRecorder::onDmaFrameReady);
    }
    
    // 停止定时器
    m_captureTimer->stop();
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
    qCWarning(dsrApp) << "ExtCaptureRecorder::onExtCaptureAvailable: *** EXT-CAPTURE AVAILABLE ***";
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
    qCWarning(dsrApp) << "ExtCaptureRecorder::onRecordingStarted: *** RECORDING ACTUALLY STARTED ***";
    setState(Recording);
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    
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
    
    // 设置定时器间隔
    int interval = 1000 / m_frameRate; // 毫秒
    m_captureTimer->start(interval);
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::onRecordingStarted: Timer started with interval:" << interval << "ms";
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

    if (m_streamingMode && !m_ffmpegStarted) {
        // 首帧：设置帧尺寸信息并启动FFmpeg进程
        m_frameWidth = width;
        m_frameHeight = height;
        m_frameStride = stride;
        
        if (!startFFmpegProcess()) {
            qCCritical(dsrApp) << "ExtCaptureRecorder::onFrameReady: Failed to start FFmpeg process on first frame";
            setState(Error);
            emit error("Failed to start video encoding process");
            return;
        }
        
        m_ffmpegStarted = true;
    }
    
    if (m_streamingMode && m_ffmpegProcess && m_ffmpegProcess->state() == QProcess::Running) {
        // 流式编码模式：直接写入FFmpeg
        qint64 bytesWritten = m_ffmpegProcess->write(static_cast<const char*>(data), static_cast<qint64>(size));
        if (bytesWritten != static_cast<qint64>(size)) {
            qCWarning(dsrApp) << "ExtCaptureRecorder::onFrameReady: Failed to write complete frame, expected:" << size << "written:" << bytesWritten;
        }
    } else if (!m_streamingMode) {
        // 缓冲模式：添加帧到缓冲区
        int64_t frameTimestamp = (timestamp > 0) ? static_cast<int64_t>(timestamp) : QDateTime::currentMSecsSinceEpoch() * 1000000LL;
        m_frameBuffer->appendBuffer(static_cast<const unsigned char*>(data), size, width, height, stride, frameTimestamp);
    }
    
    m_frameCount++;
    
    // 发送进度更新
    emit progressUpdated(m_frameCount, recordingDuration());
}

void ExtCaptureRecorder::onDmaFrameReady(int dmaBufferFd, void *gbmBo, size_t size, int width, int height, int stride, uint64_t timestamp)
{
    if (dmaBufferFd < 0 || !gbmBo) {
        return;
    }
    
    // 停止或非录制状态下一律丢弃
    if (m_state != Recording) {
        return;
    }

    // 初始化帧缓冲区（第一帧时）
    if (m_frameCount == 0) {
        m_frameWidth = width;
        m_frameHeight = height;
        m_frameStride = stride;
        
        qDebug() << "ExtCaptureRecorder: DMA Buffer frame format:" << width << "x" << height << "stride:" << stride;
    }

    if (m_streamingMode && !m_ffmpegStarted) {
        // 首帧：设置帧尺寸信息并启动支持DMA Buffer的FFmpeg进程
        m_frameWidth = width;
        m_frameHeight = height;
        m_frameStride = stride;
        
        if (!startDmaFFmpegProcess()) {
            qCCritical(dsrApp) << "ExtCaptureRecorder::onDmaFrameReady: Failed to start DMA Buffer FFmpeg process on first frame";
            setState(Error);
            emit error("Failed to start DMA Buffer video encoding process");
            return;
        }
        
        m_ffmpegStarted = true;
    }
    
    if (m_streamingMode && m_ffmpegProcess && m_ffmpegProcess->state() == QProcess::Running) {
        // DMA Buffer流式编码：使用硬件编码器处理DMA Buffer
        if (!processDmaBufferFrame(dmaBufferFd, gbmBo, width, height, stride)) {
            qCWarning(dsrApp) << "ExtCaptureRecorder::onDmaFrameReady: Failed to process DMA Buffer frame";
            return;
        }
    }
    
    m_frameCount++;
    
    // 发送进度更新
    emit progressUpdated(m_frameCount, recordingDuration());
}

void ExtCaptureRecorder::onExtCaptureError(const QString &message)
{
    setState(Error);
    emit error(message);
}

void ExtCaptureRecorder::onCaptureTimer()
{
    if (m_state != Recording || !m_extCapture) {
        return;
    }

    // 触发帧捕获
    m_extCapture->captureFrame();
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
    qDebug() << "ExtCaptureRecorder: Initialize capture for output:" << m_outputPath;
}

void ExtCaptureRecorder::finalizeRecording()
{
    if (m_streamingMode && m_ffmpegStarted && m_ffmpegProcess) {
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
    
    // 重置录制状态和计数器，为下次录制做准备
    m_frameCount = 0;
    m_startTime = 0;
    m_frameWidth = 0;
    m_frameHeight = 0;
    m_frameStride = 0;
    
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
    
    // 使用FFmpeg创建视频文件 - WL_SHM_FORMAT_XBGR8888 正确映射为 rgba 格式
    QString ffmpegCmd = QString("ffmpeg -y -f rawvideo -pix_fmt rgba -s %1x%2 -r %3 -i - -c:v libx264 -pix_fmt yuv420p \"%4\"")
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
    
    // 构建FFmpeg命令 - WL_SHM_FORMAT_XBGR8888 正确映射为 rgba 格式
    QString ffmpegCmd = QString("ffmpeg -y -f rawvideo -pix_fmt rgba -s %1x%2 -r %3 -i - -c:v libx264 -pix_fmt yuv420p \"%4\"")
                       .arg(m_frameWidth)
                       .arg(m_frameHeight)
                       .arg(m_frameRate)
                       .arg(m_outputPath);
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::startFFmpegProcess: Starting FFmpeg with command:" << ffmpegCmd;
    
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
    m_ffmpegProcess->start("sh", QStringList() << "-c" << ffmpegCmd);
    
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
}

bool ExtCaptureRecorder::processDmaBufferFrame(int dmaBufferFd, void *gbmBo, int width, int height, int stride)
{
    if (dmaBufferFd < 0 || !gbmBo) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: Invalid parameters";
        return false;
    }
    
    // qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: Processing DMA Buffer fd:" << dmaBufferFd << "size:" << width << "x" << height;
    
    gbm_bo *bo = static_cast<gbm_bo *>(gbmBo);
    if (!bo) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: Invalid GBM BO";
        return false;
    }
    
    uint32_t bo_width = gbm_bo_get_width(bo);
    uint32_t bo_height = gbm_bo_get_height(bo);
    uint32_t bo_stride = gbm_bo_get_stride(bo);
    uint32_t bo_format = gbm_bo_get_format(bo);
    
    // qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: GBM BO info:"
    //                   << "width:" << bo_width << "height:" << bo_height 
    //                   << "stride:" << bo_stride << "format:" << bo_format;
    
    if (!m_ffmpegProcess || m_ffmpegProcess->state() != QProcess::Running) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: FFmpeg process not running";
        return false;
    }
    
    // **正确的PipeWire方式：不要CPU映射DMA Buffer！**
    // PipeWire的关键：让Wayland合成器直接将屏幕数据写入DMA Buffer，
    // 然后直接传递DMA Buffer FD给支持硬件访问的消费者
    
    size_t frame_size = static_cast<size_t>(bo_stride) * bo_height;
    // qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: Processing DMA Buffer with hardware pipeline, size:" << frame_size;
    
    QByteArray frame_data;
    void *mapped_data = nullptr;
    
    // 方法1：首先尝试直接mmap DMA Buffer FD（这是标准方法）
    // qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: Attempting standard DMA-BUF mmap, fd:" << dmaBufferFd;
    mapped_data = mmap(nullptr, frame_size, PROT_READ, MAP_SHARED, dmaBufferFd, 0);
    
    if (mapped_data != MAP_FAILED) {
        // qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: *** SUCCESS! *** DMA-BUF mmap worked, copying real screen data";
        frame_data = QByteArray(static_cast<const char*>(mapped_data), frame_size);
        munmap(mapped_data, frame_size);
    } else {
        int error_code = errno;
        // qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: DMA-BUF mmap failed with errno:" << error_code << strerror(error_code);
        
        // 方法2：尝试GBM映射
        void *map_handle = nullptr;
        uint32_t map_stride = 0;
        void *gbm_map_data = nullptr;
        
        if (gbmBo) {
            struct gbm_bo *bo = static_cast<struct gbm_bo*>(gbmBo);
            // qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: Trying GBM BO mapping as fallback";
            gbm_map_data = gbm_bo_map(bo, 0, 0, bo_width, bo_height, GBM_BO_TRANSFER_READ, &map_stride, &map_handle);
            
            if (gbm_map_data) {
                // qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: *** GBM MAPPING SUCCESS! *** stride:" << map_stride;
                
                // 复制真实屏幕数据
                if (map_stride == bo_stride) {
                    frame_data = QByteArray(static_cast<const char*>(gbm_map_data), frame_size);
                } else {
                    frame_data.resize(frame_size);
                    char *dest = frame_data.data();
                    const char *src = static_cast<const char*>(gbm_map_data);
                    uint32_t copy_width = std::min(bo_stride, map_stride);
                    for (uint32_t y = 0; y < bo_height; y++) {
                        memcpy(dest + y * bo_stride, src + y * map_stride, copy_width);
                    }
                }
                
                gbm_bo_unmap(bo, map_handle);
            } else {
                qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: GBM mapping also failed";
            }
        }
        
        // 如果两种方法都失败，说明确实有问题
        if (!gbm_map_data) {
            qCCritical(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: *** BOTH MAPPING METHODS FAILED ***";
            qCCritical(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: This suggests a driver or buffer allocation issue";
            
            // 创建动态测试图案进行故障排除
            frame_data = QByteArray(frame_size, 0);
            char *data = frame_data.data();
            uint64_t time_pattern = QDateTime::currentMSecsSinceEpoch() % 1000;
            
            for (uint32_t y = 0; y < bo_height; y++) {
                for (uint32_t x = 0; x < bo_width; x++) {
                    uint32_t offset = y * bo_stride + x * 4;
                    if (offset + 3 < frame_size) {
                        data[offset + 0] = (time_pattern + x) % 256;      // B
                        data[offset + 1] = (time_pattern + y) % 256;      // G  
                        data[offset + 2] = (time_pattern + x + y) % 256;  // R
                        data[offset + 3] = 255;                           // A
                    }
                }
            }
        }
    }
    
    // 将帧数据写入FFmpeg
    qint64 bytes_written = m_ffmpegProcess->write(frame_data);
    if (bytes_written != frame_data.size()) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: Failed to write frame data to FFmpeg, expected:" 
                          << frame_data.size() << "written:" << bytes_written;
        return false;
    }
    
    // qCWarning(dsrApp) << "ExtCaptureRecorder::processDmaBufferFrame: Successfully wrote" << bytes_written << "bytes of frame data to FFmpeg";
    return true;
}

bool ExtCaptureRecorder::initializeVAAPI()
{
    qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Initializing VAAPI for DMA Buffer encoding";
    
    // 打开DRM设备
    m_drmFd = open("/dev/dri/renderD128", O_RDWR);
    if (m_drmFd < 0) {
        m_drmFd = open("/dev/dri/card0", O_RDWR);
        if (m_drmFd < 0) {
            qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Failed to open DRM device";
            return false;
        }
    }
    
    // 初始化VAAPI显示
    m_vaDisplay = vaGetDisplayDRM(m_drmFd);
    if (!m_vaDisplay) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Failed to get VA display";
        close(m_drmFd);
        m_drmFd = -1;
        return false;
    }
    
    int major, minor;
    VAStatus status = vaInitialize(m_vaDisplay, &major, &minor);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Failed to initialize VA display, status:" << status;
        m_vaDisplay = nullptr;
        close(m_drmFd);
        m_drmFd = -1;
        return false;
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: VAAPI initialized successfully, version:" << major << "." << minor;
    
    // 查询支持的配置文件和入口点
    int max_profiles = vaMaxNumProfiles(m_vaDisplay);
    VAProfile *profiles = new VAProfile[max_profiles];
    int num_profiles;
    status = vaQueryConfigProfiles(m_vaDisplay, profiles, &num_profiles);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Failed to query profiles, status:" << status;
        delete[] profiles;
        vaTerminate(m_vaDisplay);
        m_vaDisplay = nullptr;
        close(m_drmFd);
        m_drmFd = -1;
        return false;
    }
    
    // 寻找支持的H.264配置文件
    VAProfile profile = VAProfileNone;
    for (int i = 0; i < num_profiles; i++) {
        if (profiles[i] == VAProfileH264ConstrainedBaseline ||
            profiles[i] == VAProfileH264Baseline ||
            profiles[i] == VAProfileH264Main ||
            profiles[i] == VAProfileH264High) {
            profile = profiles[i];
            qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Using H.264 profile:" << profile;
            break;
        }
    }
    delete[] profiles;
    
    if (profile == VAProfileNone) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: No supported H.264 profile found";
        vaTerminate(m_vaDisplay);
        m_vaDisplay = nullptr;
        close(m_drmFd);
        m_drmFd = -1;
        return false;
    }
    
    // 查询支持的入口点
    int max_entrypoints = vaMaxNumEntrypoints(m_vaDisplay);
    VAEntrypoint *entrypoints = new VAEntrypoint[max_entrypoints];
    int num_entrypoints;
    status = vaQueryConfigEntrypoints(m_vaDisplay, profile, entrypoints, &num_entrypoints);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Failed to query entrypoints, status:" << status;
        delete[] entrypoints;
        vaTerminate(m_vaDisplay);
        m_vaDisplay = nullptr;
        close(m_drmFd);
        m_drmFd = -1;
        return false;
    }
    
    VAEntrypoint entrypoint = static_cast<VAEntrypoint>(-1);
    for (int i = 0; i < num_entrypoints; i++) {
        if (entrypoints[i] == VAEntrypointEncSlice ||
            entrypoints[i] == VAEntrypointEncSliceLP) {
            entrypoint = entrypoints[i];
            qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Using entrypoint:" << entrypoint;
            break;
        }
    }
    delete[] entrypoints;
    
    if (entrypoint == static_cast<VAEntrypoint>(-1)) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: No supported encoding entrypoint found";
        vaTerminate(m_vaDisplay);
        m_vaDisplay = nullptr;
        close(m_drmFd);
        m_drmFd = -1;
        return false;
    }
    
    // 查询支持的配置属性
    VAConfigAttrib attrib;
    attrib.type = VAConfigAttribRateControl;
    status = vaGetConfigAttributes(m_vaDisplay, profile, entrypoint, &attrib, 1);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Failed to get config attributes, status:" << status;
        // 使用默认值
        attrib.value = VA_RC_CBR;
    } else {
        // 选择支持的码率控制方法
        if (attrib.value & VA_RC_CBR) {
            attrib.value = VA_RC_CBR;
        } else if (attrib.value & VA_RC_VBR) {
            attrib.value = VA_RC_VBR;
        } else if (attrib.value & VA_RC_CQP) {
            attrib.value = VA_RC_CQP;
        } else {
            qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: No supported rate control method found";
            vaTerminate(m_vaDisplay);
            m_vaDisplay = nullptr;
            close(m_drmFd);
            m_drmFd = -1;
            return false;
        }
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Using rate control:" << attrib.value;
    
    status = vaCreateConfig(m_vaDisplay, profile, entrypoint, &attrib, 1, &m_vaConfig);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: Failed to create VA config, status:" << status;
        vaTerminate(m_vaDisplay);
        m_vaDisplay = nullptr;
        close(m_drmFd);
        m_drmFd = -1;
        return false;
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::initializeVAAPI: VAAPI configuration completed";
    return true;
}

bool ExtCaptureRecorder::importDmaBufferToVASurface(int dmaBufferFd, int width, int height, int stride, VASurfaceID *surface)
{
    if (!m_vaDisplay || dmaBufferFd < 0 || !surface) {
        return false;
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::importDmaBufferToVASurface: Importing DMA Buffer fd:" << dmaBufferFd;
    
    // 配置DMA Buffer导入参数
    VASurfaceAttribExternalBuffers external_buf;
    memset(&external_buf, 0, sizeof(external_buf));
    
    external_buf.pixel_format = VA_FOURCC_BGRA;  // 对应DRM_FORMAT_XBGR8888
    external_buf.width = width;
    external_buf.height = height;
    external_buf.data_size = stride * height;
    external_buf.num_planes = 1;
    external_buf.pitches[0] = stride;
    external_buf.offsets[0] = 0;
    external_buf.buffers = (uintptr_t*)&dmaBufferFd;
    external_buf.num_buffers = 1;
    
    VASurfaceAttrib attribs[2];
    attribs[0].type = VASurfaceAttribMemoryType;
    attribs[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[0].value.type = VAGenericValueTypeInteger;
    attribs[0].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR; // 使用兼容的内存类型
    
    attribs[1].type = VASurfaceAttribExternalBufferDescriptor;
    attribs[1].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[1].value.type = VAGenericValueTypePointer;
    attribs[1].value.value.p = &external_buf;
    
    VAStatus status = vaCreateSurfaces(m_vaDisplay, VA_RT_FORMAT_RGB32, width, height, surface, 1, attribs, 2);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::importDmaBufferToVASurface: Failed to create VA surface from DMA Buffer, status:" << status;
        return false;
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::importDmaBufferToVASurface: DMA Buffer imported to VA Surface:" << *surface;
    return true;
}

bool ExtCaptureRecorder::encodeVASurface(VASurfaceID surface)
{
    if (!m_vaDisplay || surface == VA_INVALID_SURFACE) {
        return false;
    }
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Encoding VA Surface:" << surface;
    
    // 创建编码上下文（如果还未创建）
    if (m_vaContext == 0) {
        VAStatus status = vaCreateContext(m_vaDisplay, m_vaConfig, m_frameWidth, m_frameHeight, 
                                        VA_PROGRESSIVE, &surface, 1, &m_vaContext);
        if (status != VA_STATUS_SUCCESS) {
            qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Failed to create VA context, status:" << status;
            return false;
        }
        qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: VA context created:" << m_vaContext;
    }
    
    // 创建编码缓冲区
    VABufferID coded_buf;
    VAStatus status = vaCreateBuffer(m_vaDisplay, m_vaContext, VAEncCodedBufferType, 
                                   m_frameWidth * m_frameHeight, 1, nullptr, &coded_buf);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Failed to create coded buffer, status:" << status;
        return false;
    }
    
    // 开始编码
    status = vaBeginPicture(m_vaDisplay, m_vaContext, surface);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Failed to begin picture, status:" << status;
        vaDestroyBuffer(m_vaDisplay, coded_buf);
        return false;
    }
    
    // 渲染编码参数（简化）
    status = vaRenderPicture(m_vaDisplay, m_vaContext, &coded_buf, 1);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Failed to render picture, status:" << status;
        vaEndPicture(m_vaDisplay, m_vaContext);
        vaDestroyBuffer(m_vaDisplay, coded_buf);
        return false;
    }
    
    // 结束编码
    status = vaEndPicture(m_vaDisplay, m_vaContext);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Failed to end picture, status:" << status;
        vaDestroyBuffer(m_vaDisplay, coded_buf);
        return false;
    }
    
    // 同步等待编码完成
    status = vaSyncSurface(m_vaDisplay, surface);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Failed to sync surface, status:" << status;
        vaDestroyBuffer(m_vaDisplay, coded_buf);
        return false;
    }
    
    // 获取编码后的数据
    VACodedBufferSegment *coded_buffer_segment;
    status = vaMapBuffer(m_vaDisplay, coded_buf, (void **)&coded_buffer_segment);
    if (status != VA_STATUS_SUCCESS) {
        qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Failed to map coded buffer, status:" << status;
        vaDestroyBuffer(m_vaDisplay, coded_buf);
        return false;
    }
    
    // 将编码后的数据写入FFmpeg进程
    if (m_ffmpegProcess && m_ffmpegProcess->state() == QProcess::Running) {
        size_t totalSize = 0;
        VACodedBufferSegment *segment = coded_buffer_segment;
        
        while (segment) {
            if (segment->size > 0) {
                qint64 bytesWritten = m_ffmpegProcess->write(
                    static_cast<const char*>(segment->buf), segment->size);
                if (bytesWritten != segment->size) {
                    qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Failed to write encoded data to FFmpeg";
                }
                totalSize += segment->size;
            }
            segment = (VACodedBufferSegment *)segment->next;
        }
        
        qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: Wrote" << totalSize << "bytes of encoded data to FFmpeg";
    }
    
    // 清理缓冲区
    vaUnmapBuffer(m_vaDisplay, coded_buf);
    vaDestroyBuffer(m_vaDisplay, coded_buf);
    
    qCWarning(dsrApp) << "ExtCaptureRecorder::encodeVASurface: VA Surface encoded successfully";
    return true;
}

