// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiscreencapturecoordinator.h"
#include "multiscreenframecompositor.h"
#include "manager/extcapturemanager.h"
#include "session/extcapturesession.h"
#include "frame/extcaptureframe.h"
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

#include <QGuiApplication>
#include <QDebug>
#include <QMutexLocker>
#include <QTimer>
#include <qpa/qplatformnativeinterface.h>

namespace {

constexpr int FrameSyncTimeoutMs = 250;

} // namespace

MultiScreenCaptureCoordinator::MultiScreenCaptureCoordinator(QObject *parent)
    : QObject(parent)
    , m_manager(nullptr)
    , m_capturing(false)
    , m_compositor(nullptr)
    , m_syncTimer(new QTimer(this))
{
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Initializing";
    
    // 创建帧拼接器
    m_compositor = new MultiScreenFrameCompositor(this);
    
    // 连接拼接器信号
    connect(m_compositor, &MultiScreenFrameCompositor::compositionComplete,
            this, &MultiScreenCaptureCoordinator::onCompositeFrameReady);
    connect(m_compositor, &MultiScreenFrameCompositor::compositionError,
            this, &MultiScreenCaptureCoordinator::error);
    
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

    m_readySessionCount = 0;
    m_captureInFlight = false;
    m_captureStartedEmitted = false;

    // 计算虚拟桌面布局
    calculateVirtualDesktopLayout(screens);

    if (m_compositor) {
        if (cropRegion.isValid() && !cropRegion.isEmpty())
            m_compositor->setCropRegion(cropRegion.normalized());
        else
            m_compositor->clearCropRegion();
    }

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
        return;
    }

    m_capturing = false;
    m_captureInFlight = false;
    m_syncTimer->stop();

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

    if (m_captureInFlight) {
        return true;
    }

    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::captureMultiScreenFrame: Triggering capture for" 
                    << m_screenLayouts.size() << "screens";

    // 重置帧准备标记
    resetFrameReadyFlags();
    m_captureInFlight = true;

    // 触发所有屏幕的帧捕获
    int captureCount = 0;
    for (auto& layout : m_screenLayouts) {
        if (layout.session) {
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
    QMutexLocker locker(&m_frameMutex);
    
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::onScreenFrameReady: Screen" << screen->name()
                    << "frame ready, size:" << size << "dimensions:" << width << "x" << height;

    // 创建DmaFrameInfo并传递给compositor
    DmaFrameInfo frameInfo;
    frameInfo.screen = screen;
    frameInfo.screenGeometry = screen->geometry();
    frameInfo.dmaBufferFd = dmaBufferFd;
    frameInfo.gbmBo = gbmBo;
    frameInfo.width = width;
    frameInfo.height = height;
    frameInfo.stride = stride;
    frameInfo.size = size;
    frameInfo.timestamp = timestamp;
    frameInfo.ready = true;
    
    // 从DMA Buffer或GBM Buffer Object中提取CPU可访问的像素数据
    // 使用与ExtCaptureRecorder相同的完整实现
    frameInfo.frameData = extractDmaBufferData(dmaBufferFd, gbmBo, width, height, stride, size);

    if (frameInfo.frameData.isEmpty()) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Failed to map frame for screen"
                          << screen->name();
        return;
    }
    
    // 保持原有的帧数据结构用于兼容性
    ScreenFrameData& frameData = m_screenFrames[screen];
    frameData.dmaBufferFd = dmaBufferFd;
    frameData.gbmBo = gbmBo;
    frameData.width = width;
    frameData.height = height;
    frameData.stride = stride;
    frameData.size = size;
    frameData.timestamp = timestamp;
    frameData.ready = true;

    // 标记对应的屏幕布局为帧准备就绪
    for (auto& layout : m_screenLayouts) {
        if (layout.screen == screen) {
            layout.frameReady = true;
            break;
        }
    }

    if (!m_compositor || !m_compositor->addScreenFrame(frameInfo)) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Failed to add frame to compositor for screen"
                          << screen->name();
        return;
    }

    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Frame data updated for screen"
                    << screen->name() << "and passed to compositor";
}

void MultiScreenCaptureCoordinator::onSessionError(const QString& message)
{
    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::onSessionError:" << message;
    m_captureInFlight = false;
    m_syncTimer->stop();
    emit error(QString("Session error: %1").arg(message));
}

void MultiScreenCaptureCoordinator::onSessionStopped()
{
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::onSessionStopped";
    if (!m_capturing)
        return;

    m_captureInFlight = false;
    m_syncTimer->stop();
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
    if (!m_capturing || compositeFrameData.isEmpty())
        return;

    m_captureInFlight = false;
    m_syncTimer->stop();
    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Composite frame ready, size:"
                      << compositeFrameData.size() << "dimensions:" << width << "x" << height;
    emit compositeFrameReady(compositeFrameData, width, height, stride, timestamp);
}

void MultiScreenCaptureCoordinator::onFrameSyncTimeout()
{
    if (!m_captureInFlight)
        return;

    qCWarning(dsrApp) << "MultiScreenCaptureCoordinator: Timed out waiting for output frames; retrying on the next tick";
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

    for (QScreen* screen : screens) {
        QRect geometry = screen->geometry();
        qCDebug(dsrApp) << "Screen" << screen->name() << "geometry:" << geometry;
        
        minX = qMin(minX, geometry.x());
        minY = qMin(minY, geometry.y());
        maxX = qMax(maxX, geometry.x() + geometry.width());
        maxY = qMax(maxY, geometry.y() + geometry.height());
    }

    // 计算虚拟桌面尺寸
    m_virtualDesktopSize = QSize(maxX - minX, maxY - minY);
    qCDebug(dsrApp) << "Virtual desktop bounds: (" << minX << "," << minY << ") to (" << maxX << "," << maxY << ")";
    qCDebug(dsrApp) << "Virtual desktop size:" << m_virtualDesktopSize;

    // 创建屏幕布局信息
    for (QScreen* screen : screens) {
        ScreenLayout layout;
        layout.screen = screen;
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
        
        qCDebug(dsrApp) << "Screen layout:" << screen->name() 
                        << "virtual geometry:" << layout.geometry
                        << "original geometry:" << screenGeometry;
    }
    
    // 配置compositor
    if (m_compositor) {
        m_compositor->setVirtualDesktopSize(m_virtualDesktopSize);
        m_compositor->setScreenLayouts(screens);
        m_compositor->clearCropRegion(); // 默认全屏录制，后续可通过接口设置裁剪区域
        
        qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Compositor configured with" 
                        << screens.size() << "screens, virtual desktop size:" << m_virtualDesktopSize;
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
    m_screenFrames.clear();
    m_virtualDesktopSize = QSize();
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
    
    for (auto& frameData : m_screenFrames) {
        frameData.ready = false;
    }
    
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator: Frame ready flags reset";
}

QByteArray MultiScreenCaptureCoordinator::extractDmaBufferData(int dmaBufferFd, void* gbmBo, int width, int height, int stride, size_t size)
{
    if (dmaBufferFd < 0 || !gbmBo) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: Invalid parameters";
        return QByteArray();
    }
#ifdef ENABLE_UNIT_TEST
    // 测试桩：GBM/DMA-BUF mmap 提取路径不在单元测试覆盖
    Q_UNUSED(width)
    Q_UNUSED(height)
    Q_UNUSED(stride)
    Q_UNUSED(size)
    return QByteArray();
#else
    
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: Processing DMA Buffer fd:" << dmaBufferFd << "size:" << width << "x" << height;
    
    gbm_bo *bo = static_cast<gbm_bo *>(gbmBo);
    if (!bo) {
        qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: Invalid GBM BO";
        return QByteArray();
    }
    
    uint32_t bo_width = gbm_bo_get_width(bo);
    uint32_t bo_height = gbm_bo_get_height(bo);
    uint32_t bo_stride = gbm_bo_get_stride(bo);
    uint32_t bo_format = gbm_bo_get_format(bo);
    
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: GBM BO info:"
                    << "width:" << bo_width << "height:" << bo_height 
                    << "stride:" << bo_stride << "format:" << bo_format;
    
    size_t frame_size = static_cast<size_t>(bo_stride) * bo_height;
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: Extracting DMA Buffer data, size:" << frame_size;
    
    QByteArray frame_data;
    void *mapped_data = nullptr;
    
    // 方法1：首先尝试直接mmap DMA Buffer FD（这是标准方法）
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: Attempting standard DMA-BUF mmap, fd:" << dmaBufferFd;
    mapped_data = mmap(nullptr, frame_size, PROT_READ, MAP_SHARED, dmaBufferFd, 0);
    
    if (mapped_data != MAP_FAILED) {
        qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: *** SUCCESS! *** DMA-BUF mmap worked, copying real screen data";
        frame_data = QByteArray(static_cast<const char*>(mapped_data), frame_size);
        munmap(mapped_data, frame_size);
    } else {
        int error_code = errno;
        qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: DMA-BUF mmap failed with errno:" << error_code << strerror(error_code);
        
        // 方法2：尝试GBM映射
        void *map_handle = nullptr;
        uint32_t map_stride = 0;
        void *gbm_map_data = nullptr;
        
        if (gbmBo) {
            struct gbm_bo *bo = static_cast<struct gbm_bo*>(gbmBo);
            qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: Trying GBM BO mapping as fallback";
            gbm_map_data = gbm_bo_map(bo, 0, 0, bo_width, bo_height, GBM_BO_TRANSFER_READ, &map_stride, &map_handle);
            
            if (gbm_map_data) {
                qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: *** GBM MAPPING SUCCESS! *** stride:" << map_stride;
                
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
                qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: GBM mapping also failed";
            }
        }
        
        // 如果两种方法都失败，返回空数据让compositor使用占位符
        if (!gbm_map_data) {
            qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: *** BOTH MAPPING METHODS FAILED ***";
            qCWarning(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: This suggests a driver or buffer allocation issue";
            
            // 返回空数据，让compositor生成占位符图案
            return QByteArray();
        }
    }
    
    qCDebug(dsrApp) << "MultiScreenCaptureCoordinator::extractDmaBufferData: Successfully extracted" << frame_data.size() << "bytes of frame data";
    return frame_data;
#endif
}
