// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiscreencapturecoordinator.h"
#include "multiscreenframecompositor.h"
#include "manager/extcapturemanager.h"
#include "session/extcapturesession.h"
#include "frame/extcaptureframe.h"
#include "dmaframegpuconverter.h"
#include "../utils/log.h"

// DMA Buffer处理所需的头文件
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <algorithm>
#include <utility>
#include <QDateTime>
#include <gbm.h>
#include <drm/drm_fourcc.h>

#include <QGuiApplication>
#include <QDebug>
#include <QMutexLocker>
#include <QTimer>
#include <QThread>
#include <qpa/qplatformnativeinterface.h>

namespace {

constexpr int FrameSyncTimeoutMs = 250;

} // namespace

MultiScreenCaptureCoordinator::MultiScreenCaptureCoordinator(QObject *parent)
    : QObject(parent)
    , m_manager(nullptr)
    , m_capturing(false)
    , m_syncTimer(new QTimer(this))
{
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Initializing";

    // 输出帧可能异步到达。超时只解除当前轮次，不终止录制，让定时器
    // 在下一拍重新提交帧请求。
    m_syncTimer->setSingleShot(true);
    m_syncTimer->setInterval(FrameSyncTimeoutMs);
    connect(m_syncTimer, &QTimer::timeout,
            this, &MultiScreenCaptureCoordinator::onFrameSyncTimeout);
}

MultiScreenCaptureCoordinator::~MultiScreenCaptureCoordinator()
{
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Destructor";
    stopMultiScreenCapture();
}

void MultiScreenCaptureCoordinator::setCaptureManager(ExtCaptureManager* manager)
{
    m_manager = manager;
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Capture manager set";
}

bool MultiScreenCaptureCoordinator::startMultiScreenCapture(const QList<QScreen*>& screens,
                                                            bool includeCursor,
                                                            const QRect& cropRegion)
{
    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::startMultiScreenCapture: Starting with" 
                      << screens.size() << "screens, includeCursor:" << includeCursor;

    if (m_capturing) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Already capturing";
        return false;
    }

    if (!m_manager) {
        qCCritical(dsrApp) << "MultiScreenCaptureCoordinator: No capture manager set";
        emit error("No capture manager available");
        return false;
    }

    if (screens.isEmpty()) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: No screens provided";
        emit error("No screens to capture");
        return false;
    }

    // 清理现有会话
    cleanupSessions();
    ensurePipeline();

    m_readySessionCount = 0;
    m_captureInFlight = false;
    m_captureStartedEmitted = false;
    m_pendingCropRegion = cropRegion.normalized();

    // 计算虚拟桌面布局
    calculateVirtualDesktopLayout(screens);

    // 为每个屏幕创建捕获会话
    int successCount = 0;
    for (QScreen* screen : screens) {
        qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Creating session for screen" 
                        << screen->name() << screen->geometry();

        // 创建ExtCaptureSession
        ExtCaptureSession* session = m_manager->createScreenCaptureSession(screen, includeCursor);
        if (!session) {
            qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Failed to create session for screen" 
                              << screen->name();
            cleanupSessions();
            stopPipeline();
            emit error(QStringLiteral("无法为输出 %1 创建录屏会话").arg(screen->name()));
            return false;
        }

        // 查找对应的布局信息
        ScreenLayout* layout = nullptr;
        for (auto& screenLayout : m_screenLayouts) {
            if (screenLayout.screen == screen) {
                layout = &screenLayout;
                break;
            }
        }

        if (!layout) {
            qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: No layout found for screen" 
                              << screen->name();
            session->deleteLater();
            cleanupSessions();
            stopPipeline();
            emit error(QStringLiteral("输出 %1 缺少录屏布局").arg(screen->name()));
            return false;
        }

        // 设置会话信息
        layout->session = session;
        layout->includeCursor = includeCursor;
        layout->frameReady = false;

        connect(session, &ExtCaptureSession::ready,
                this, &MultiScreenCaptureCoordinator::onSessionReady);

        // 连接会话信号 - 使用lambda捕获screen指针
        connect(session, &ExtCaptureSession::dmaFrameReady,
                this, [this, screen](int dmaBufferFd, void* gbmBo, size_t size, 
                                   int width, int height, int stride, uint64_t timestamp) {
                    onScreenFrameReady(screen, dmaBufferFd, gbmBo, width, height, stride, size, timestamp);
                });

        // 每个输出独立选择 transport。某台输出回退 SHM 时，多屏合成
        // 不能只等 dmaFrameReady，否则这一轮永远凑不齐所有屏幕。
        connect(session, &ExtCaptureSession::frameReady,
                this, [this, screen](ExtCaptureFrame *frame) {
                    if (frame && !frame->isDmaBuffer())
                        onScreenShmFrameReady(screen, frame);
                });

        connect(session, &ExtCaptureSession::error,
                this, &MultiScreenCaptureCoordinator::onSessionError);

        connect(session, &ExtCaptureSession::stopped,
                this, &MultiScreenCaptureCoordinator::onSessionStopped);

        successCount++;
        qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Session created successfully for screen" 
                        << screen->name();
    }

    if (successCount != screens.size()) {
        qCCritical(dsrApp) << "MultiScreenCaptureCoordinator: Only" << successCount
                           << "of" << screens.size() << "sessions were created";
        cleanupSessions();
        stopPipeline();
        emit error(QStringLiteral("并非所有输出都成功创建录屏会话"));
        return false;
    }

    m_capturing = true;
    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Started capturing" << successCount << "screens";
    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Virtual desktop size:" << m_virtualDesktopSize;

    // 创建会话时约束事件可能已经完成。延迟一次就绪检查，覆盖信号早于
    // connect() 的竞态。
    QTimer::singleShot(0, this, &MultiScreenCaptureCoordinator::onSessionReady);
    return true;
}

void MultiScreenCaptureCoordinator::stopMultiScreenCapture()
{
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::stopMultiScreenCapture";

    if (!m_capturing) {
        stopPipeline();
        return;
    }

    m_capturing = false;
    m_captureInFlight = false;
    m_syncTimer->stop();
    m_roundFrames.clear();
    stopPipeline();

    cleanupSessions();

    emit captureStopped();
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Capture stopped";
}

bool MultiScreenCaptureCoordinator::captureMultiScreenFrame()
{
    if (!m_capturing) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Not capturing";
        return false;
    }

    if (!m_pipeline) {
        qCCritical(dsrApp) << "MultiScreenCaptureCoordinator: capture pipeline unavailable";
        emit error(QStringLiteral("采集工作线程不可用"));
        return false;
    }

    if (m_captureInFlight) {
        return true;
    }

    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::captureMultiScreenFrame: Triggering capture for" 
                    << m_screenLayouts.size() << "screens";

    // 重置帧准备标记
    resetFrameReadyFlags();
    ++m_roundId;
    m_captureInFlight = true;

    // 触发所有屏幕的帧捕获
    int captureCount = 0;
    for (auto& layout : m_screenLayouts) {
        if (layout.session) {
            if (layout.session->state() == ExtCaptureSession::Capturing)
                layout.session->cancelFrame();

            // 使用createFrame方法创建帧并触发捕获
            ExtCaptureFrame* frame = layout.session->createFrame();
            if (frame) {
                if (frame->capture()) {
                    captureCount++;
                    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Submitted frame for screen"
                                    << layout.screen->name();
                } else {
                    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Failed to submit frame for screen"
                                      << layout.screen->name();
                }
            } else {
                qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Failed to create frame for screen" 
                                  << layout.screen->name();
            }
        }
    }

    if (captureCount != m_screenLayouts.size()) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Submitted only"
                          << captureCount << "of" << m_screenLayouts.size() << "frames";
        if (captureCount > 0)
            m_syncTimer->start();
        else
            m_captureInFlight = false;
        return false;
    }

    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Triggered capture for" << captureCount << "screens";
    
    // 启动本轮帧采集超时保护。
    m_syncTimer->start();
    
    return true;
}

void MultiScreenCaptureCoordinator::ensurePipeline()
{
    if (m_pipeline && m_pipelineThread && m_pipelineThread->isRunning())
        return;

    stopPipeline();

    m_pipelineThread = new QThread(this);
    m_pipeline = new CapturePipelineWorker;
    m_pipeline->moveToThread(m_pipelineThread);

    connect(m_pipeline, &CapturePipelineWorker::multiFrameReady,
            this, &MultiScreenCaptureCoordinator::onPipelineMultiFrameReady);
    connect(m_pipeline, &CapturePipelineWorker::multiFrameFailed,
            this, &MultiScreenCaptureCoordinator::onPipelineMultiFrameFailed);

    m_pipelineThread->start();
    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: capture pipeline thread started";
}

void MultiScreenCaptureCoordinator::stopPipeline()
{
    if (!m_pipeline && !m_pipelineThread)
        return;

    if (m_pipelineThread && m_pipelineThread->isRunning()) {
        if (m_pipeline) {
            QMetaObject::invokeMethod(m_pipeline,
                                      [worker = m_pipeline]() { delete worker; },
                                      Qt::BlockingQueuedConnection);
            m_pipeline = nullptr;
        }
        m_pipelineThread->quit();
        if (!m_pipelineThread->wait(8000))
            qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: capture pipeline thread did not stop in time";
    } else if (m_pipeline) {
        delete m_pipeline;
        m_pipeline = nullptr;
    }

    delete m_pipelineThread;
    m_pipelineThread = nullptr;
    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: capture pipeline thread stopped";
}

void MultiScreenCaptureCoordinator::dispatchRound()
{
    if (!m_pipeline || m_roundFrames.size() != m_screenLayouts.size()) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: cannot dispatch incomplete round"
                          << m_roundId << m_roundFrames.size() << "/" << m_screenLayouts.size();
        return;
    }

    QVector<FrameInput> inputs;
    inputs.reserve(m_screenLayouts.size());
    for (const ScreenLayout &layout : std::as_const(m_screenLayouts)) {
        const auto it = m_roundFrames.constFind(layout.name);
        if (it == m_roundFrames.constEnd()) {
            qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: missing frame for output"
                              << layout.name;
            resetFrameReadyFlags();
            m_captureInFlight = false;
            m_syncTimer->stop();
            emit error(QStringLiteral("输出 %1 缺少本轮帧").arg(layout.name));
            return;
        }
        inputs.append(it.value());
    }

    m_roundFrames.clear();
    // 帧已全部收集，不再用采集超时保护合成耗时；工作线程完成或失败后
    // 才会解除 m_captureInFlight，避免下一轮覆盖仍在处理的帧。
    m_syncTimer->stop();

    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: dispatching round" << m_roundId
                      << "with" << inputs.size() << "outputs to capture pipeline";
    QMetaObject::invokeMethod(m_pipeline, "processMultiRound", Qt::QueuedConnection,
                              Q_ARG(quint64, m_roundId),
                              Q_ARG(QVector<FrameInput>, inputs));
}

QList<QScreen*> MultiScreenCaptureCoordinator::getAvailableScreens() const
{
    return QGuiApplication::screens();
}

void MultiScreenCaptureCoordinator::onScreenFrameReady(QScreen* screen, int dmaBufferFd, void* gbmBo, 
                                                     int width, int height, int stride, size_t size, uint64_t timestamp)
{
    if (!screen) {
        qCWarning(dsrApp) << "onScreenFrameReady: screen is null, ignoring frame.";
        return;
    }

    if (!m_capturing || !m_captureInFlight) {
        qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: ignoring DMA frame outside active round for"
                        << screen->name();
        return;
    }

    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::onScreenFrameReady: Screen" << screen->name()
                    << "frame ready, size:" << size << "dimensions:" << width << "x" << height;

    // GUI 线程只提取元数据并 dup fd；GBM 导入/映射/拷贝全部交给工作线程。
    submitScreenFrame(screen, QByteArray(), -1, dmaBufferFd, gbmBo, width, height,
                      stride, size, timestamp);
}

void MultiScreenCaptureCoordinator::onScreenShmFrameReady(QScreen *screen, ExtCaptureFrame *frame)
{
    if (!screen || !frame || frame->isDmaBuffer())
        return;

    const FrameData &source = frame->frameData();
    const int width = source.dimensions.width();
    const int height = source.dimensions.height();
    const int stride = static_cast<int>(source.stride);
    const size_t size = static_cast<size_t>(stride) * static_cast<size_t>(height);
    if (width <= 0 || height <= 0 || stride <= 0 || size == 0) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: invalid SHM frame for screen"
                          << screen->name();
        return;
    }

    const int shmFd = frame->getShmBufferFd();
    if (shmFd >= 0) {
        // 只 dup memfd，真正 mmap/拷贝在采集工作线程完成。
        submitScreenFrame(screen, QByteArray(), shmFd, -1, nullptr, width, height,
                          stride, size, source.timestamp);
        return;
    }

    // 兼容没有暴露 memfd 的旧帧实现：退回 GUI 线程拷贝。
    void *data = frame->mapBuffer();
    if (!data) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Failed to map SHM frame for screen"
                          << screen->name();
        return;
    }
    const QByteArray frameData(static_cast<const char *>(data), static_cast<int>(size));
    frame->unmapBuffer();
    submitScreenFrame(screen, frameData, -1, -1, nullptr, width, height,
                      stride, size, source.timestamp);
}

void MultiScreenCaptureCoordinator::submitScreenFrame(QScreen *screen, const QByteArray &frameData,
                                                      int shmBufferFd, int dmaBufferFd,
                                                      void *gbmBo, int width, int height,
                                                      int stride, size_t size,
                                                      uint64_t timestamp)
{
    if (!screen || !m_capturing || !m_captureInFlight)
        return;

    ScreenLayout *targetLayout = nullptr;
    for (auto& layout : m_screenLayouts) {
        if (layout.screen == screen) {
            targetLayout = &layout;
            break;
        }
    }
    if (!targetLayout) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: no layout for output"
                          << screen->name();
        return;
    }

    if (targetLayout->frameReady) {
        qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: duplicate frame ignored for output"
                        << targetLayout->name << "round:" << m_roundId;
        return;
    }

    FrameInput input;
    input.screenName = targetLayout->name;
    input.screenGeometry = targetLayout->geometry;
    input.width = width;
    input.height = height;
    input.stride = stride;
    input.timestamp = timestamp;
    input.sequence = m_roundId;

    if (dmaBufferFd >= 0 && gbmBo) {
        input.isDma = true;
        QString error;
        const QString devicePath = targetLayout->session
                ? targetLayout->session->config().dmaDevicePath
                : QString();
        const uint64_t modifier = targetLayout->session
                ? targetLayout->session->config().dmaDecodeModifier
                : DRM_FORMAT_MOD_LINEAR;
        if (!makeDmaFrameSource(dmaBufferFd, gbmBo, width, height, stride, devicePath,
                                modifier, timestamp, m_roundId, &input.dma, &error)) {
            qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: failed to describe DMA frame for"
                              << targetLayout->name << ":" << error;
            return;
        }
    } else if (shmBufferFd >= 0) {
        QString error;
        if (!makeShmFrameSource(shmBufferFd, size, width, height, stride, timestamp,
                                m_roundId, &input.shm, &error)) {
            qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: failed to describe SHM frame for"
                              << targetLayout->name << ":" << error;
            return;
        }
    } else {
        input.isDma = false;
        input.cpuData = frameData;
        if (input.cpuData.isEmpty()) {
            qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: empty CPU frame for output"
                              << targetLayout->name;
            return;
        }
    }

    targetLayout->frameReady = true;
    m_roundFrames.insert(targetLayout->name, input);

    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: collected output"
                    << targetLayout->name << "round:" << m_roundId
                    << "(" << m_roundFrames.size() << "/" << m_screenLayouts.size() << ")";

    if (m_roundFrames.size() == m_screenLayouts.size())
        dispatchRound();
}

void MultiScreenCaptureCoordinator::onSessionError(const QString& message)
{
    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::onSessionError:" << message;
    m_captureInFlight = false;
    m_syncTimer->stop();
    resetFrameReadyFlags();
    emit error(QString("Session error: %1").arg(message));
}

void MultiScreenCaptureCoordinator::onSessionStopped()
{
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::onSessionStopped";
    if (!m_capturing)
        return;

    m_captureInFlight = false;
    m_syncTimer->stop();
    resetFrameReadyFlags();
    emit error(QStringLiteral("录屏输出会话意外停止"));
}

void MultiScreenCaptureCoordinator::onSessionReady()
{
    if (!m_capturing || m_captureStartedEmitted)
        return;

    int readyCount = 0;
    for (const ScreenLayout &layout : std::as_const(m_screenLayouts)) {
        if (layout.session && layout.session->state() == ExtCaptureSession::Ready)
            ++readyCount;
    }

    m_readySessionCount = readyCount;
    if (m_readySessionCount < m_screenLayouts.size())
        return;

    m_captureStartedEmitted = true;
    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: All" << m_screenLayouts.size()
                      << "output sessions are ready";
    emit captureStarted();
}

void MultiScreenCaptureCoordinator::onCompositeFrameReady(const QByteArray& compositeFrameData,
                                                          int width,
                                                          int height,
                                                          int stride,
                                                          uint64_t timestamp)
{
    if (compositeFrameData.isEmpty())
        return;

    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Composite frame ready, size:"
                      << compositeFrameData.size() << "dimensions:" << width << "x" << height;
    emit compositeFrameReady(compositeFrameData, width, height, stride, timestamp);
}

void MultiScreenCaptureCoordinator::onPipelineMultiFrameReady(
        const QByteArray& compositeFrameData, int width, int height, int stride,
        uint64_t timestamp, quint64 roundId, qint64 elapsedMs)
{
    if (!m_capturing || !m_captureInFlight || roundId != m_roundId)
        return;

    m_captureInFlight = false;
    m_syncTimer->stop();

    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: pipeline round" << roundId
                      << "completed in" << elapsedMs << "ms";
    onCompositeFrameReady(compositeFrameData, width, height, stride, timestamp);
}

void MultiScreenCaptureCoordinator::onPipelineMultiFrameFailed(const QString& message,
                                                               quint64 roundId)
{
    if (!m_capturing || roundId != m_roundId)
        return;

    m_captureInFlight = false;
    m_syncTimer->stop();
    resetFrameReadyFlags();
    emit error(QStringLiteral("多屏采集失败：%1").arg(message));
}

void MultiScreenCaptureCoordinator::onFrameSyncTimeout()
{
    if (!m_captureInFlight)
        return;

    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Timed out waiting for output frames; retrying on the next tick";
    for (auto &layout : m_screenLayouts) {
        if (layout.session && layout.session->state() == ExtCaptureSession::Capturing)
            layout.session->cancelFrame();
    }
    resetFrameReadyFlags();
    m_captureInFlight = false;
}

void MultiScreenCaptureCoordinator::calculateVirtualDesktopLayout(const QList<QScreen*>& screens)
{
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::calculateVirtualDesktopLayout";
    
    m_screenLayouts.clear();
    
    if (screens.isEmpty()) {
        m_virtualDesktopSize = QSize();
        return;
    }

    // 计算虚拟桌面的边界
    int minX = INT_MAX, minY = INT_MAX;
    int maxX = INT_MIN, maxY = INT_MIN;
    qreal outputScale = 1.0;

    for (QScreen* screen : screens) {
        QRect geometry = screen->geometry();
        qCDebug(dsrApp) << "Screen" << screen->name() << "geometry:" << geometry;

        const qreal devicePixelRatio = screen->devicePixelRatio();
        if (qIsFinite(devicePixelRatio) && devicePixelRatio > outputScale)
            outputScale = devicePixelRatio;

        minX = qMin(minX, geometry.x());
        minY = qMin(minY, geometry.y());
        maxX = qMax(maxX, geometry.x() + geometry.width());
        maxY = qMax(maxY, geometry.y() + geometry.height());
    }

    // 计算虚拟桌面尺寸
    m_virtualDesktopSize = QSize(maxX - minX, maxY - minY);
    m_virtualDesktopOrigin = QPoint(minX, minY);
    qCDebug(dsrApp) << "Virtual desktop bounds: (" << minX << "," << minY << ") to (" << maxX << "," << maxY << ")";
    qCDebug(dsrApp) << "Virtual desktop size:" << m_virtualDesktopSize;

    // 创建屏幕布局信息
    for (QScreen* screen : screens) {
        ScreenLayout layout;
        layout.screen = screen;
        layout.name = screen->name().isEmpty()
                ? QStringLiteral("output-%1").arg(m_screenLayouts.size())
                : screen->name();
        // 将屏幕几何坐标转换为相对于虚拟桌面的坐标
        QRect screenGeometry = screen->geometry();
        layout.geometry = QRect(
            screenGeometry.x() - minX,
            screenGeometry.y() - minY,
            screenGeometry.width(),
            screenGeometry.height()
        );
        layout.session = nullptr;
        layout.frameReady = false;
        layout.includeCursor = false;

        m_screenLayouts.append(layout);
        
        qCDebug(dsrApp) << "Screen layout:" << layout.name
                        << "virtual geometry:" << layout.geometry
                        << "original geometry:" << screenGeometry;
    }

    // 采集帧是物理像素，而 QScreen geometry 和录制选区是逻辑像素；以所有
    // 输出中的最大 DPR 建立物理像素画布，避免 2K@125% 或 4K@200% 被降采样
    // 后变糊。配置全部以纯数据投递给工作线程。
    if (m_pipeline) {
        QVector<ScreenLayoutInfo> layoutInfos;
        layoutInfos.reserve(m_screenLayouts.size());
        for (const ScreenLayout &layout : std::as_const(m_screenLayouts))
            layoutInfos.append({layout.name, layout.geometry});

        const bool hasCrop = m_pendingCropRegion.isValid()
                && !m_pendingCropRegion.isEmpty();
        const QRect localCrop = hasCrop
                ? m_pendingCropRegion.translated(-m_virtualDesktopOrigin)
                : QRect();

        QMetaObject::invokeMethod(m_pipeline, "configureMulti", Qt::QueuedConnection,
                                  Q_ARG(QSize, m_virtualDesktopSize),
                                  Q_ARG(qreal, outputScale),
                                  Q_ARG(QRect, localCrop),
                                  Q_ARG(bool, hasCrop),
                                  Q_ARG(QVector<ScreenLayoutInfo>, layoutInfos));

        qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: dispatched compositor config,"
                        << "screens:" << screens.size()
                        << "virtual desktop size:" << m_virtualDesktopSize
                        << "output scale:" << outputScale
                        << "local crop:" << localCrop;
    }
}

void MultiScreenCaptureCoordinator::cleanupSessions()
{
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::cleanupSessions";
    
    // 清理现有会话
    for (auto& layout : m_screenLayouts) {
        if (layout.session) {
            disconnect(layout.session, nullptr, this, nullptr);
            layout.session->stop();
            layout.session->deleteLater();
            layout.session = nullptr;
        }
    }
    
    m_screenLayouts.clear();
    m_roundFrames.clear();
    m_virtualDesktopSize = QSize();
    m_virtualDesktopOrigin = QPoint();
    m_pendingCropRegion = QRect();
    m_captureInFlight = false;
    m_roundId = 0;
    m_readySessionCount = 0;
    m_captureStartedEmitted = false;
}

void* MultiScreenCaptureCoordinator::getWaylandOutput(QScreen* screen)
{
    if (!screen) {
        return nullptr;
    }
#ifdef ENABLE_UNIT_TEST
    // 测试桩：offscreen 平台无 Wayland output
    return nullptr;
#else

    QPlatformNativeInterface* nativeInterface = QGuiApplication::platformNativeInterface();
    if (!nativeInterface) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: No platform native interface";
        return nullptr;
    }

    void* waylandOutput = nativeInterface->nativeResourceForScreen("output", screen);
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Wayland output for screen" 
                    << screen->name() << ":" << waylandOutput;
    
    return waylandOutput;
#endif
}

void MultiScreenCaptureCoordinator::resetFrameReadyFlags()
{
    // 重置所有帧准备标记
    for (auto& layout : m_screenLayouts) {
        layout.frameReady = false;
    }
    m_roundFrames.clear();
    
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Frame ready flags reset";
}

QByteArray MultiScreenCaptureCoordinator::extractDmaBufferData(int dmaBufferFd, void* gbmBo, int width, int height, int stride, size_t size)
{
    Q_UNUSED(size)
    if (dmaBufferFd < 0 || !gbmBo || width <= 0 || height <= 0) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: Invalid parameters";
        return QByteArray();
    }
#ifdef ENABLE_UNIT_TEST
    Q_UNUSED(stride)
    return QByteArray();
#else
    gbm_bo *bo = static_cast<gbm_bo *>(gbmBo);
    uint64_t modifier = gbm_bo_get_modifier(bo);

    DmaFrameGpuConverter converter(gbm_bo_get_device(bo));
    QByteArray frameData;
    QString error;
    if (!converter.convertToRgba(dmaBufferFd, width, height,
                                 stride > 0 ? stride : static_cast<int>(gbm_bo_get_stride(bo)),
                                 gbm_bo_get_offset(bo, 0), gbm_bo_get_format(bo),
                                 modifier, &frameData, &error)) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData:"
                          << "GPU conversion failed:" << error;
        return QByteArray();
    }
    return frameData;
#endif
}
