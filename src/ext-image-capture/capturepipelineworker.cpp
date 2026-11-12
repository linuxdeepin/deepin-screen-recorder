// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "capturepipelineworker.h"
#include "dmaframegpuconverter.h"
#include "multiscreenframecompositor.h"
#include "../utils/log.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFile>

#include <drm/drm_fourcc.h>
#include <fcntl.h>
#include <gbm.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstring>

namespace {

bool fail(QString *error, const QString &message)
{
    if (error)
        *error = message;
    return false;
}

uint32_t planeStride(gbm_bo *bo, int fallback)
{
    if (!bo)
        return fallback > 0 ? static_cast<uint32_t>(fallback) : 0;
    const uint32_t stride = gbm_bo_get_stride_for_plane(bo, 0);
    return stride > 0 ? stride : (fallback > 0 ? static_cast<uint32_t>(fallback) : 0);
}

bool isRgbxMemoryFormat(uint32_t format)
{
    return format == DRM_FORMAT_XBGR8888 || format == DRM_FORMAT_ABGR8888;
}

bool isBgrxMemoryFormat(uint32_t format)
{
    return format == DRM_FORMAT_XRGB8888 || format == DRM_FORMAT_ARGB8888;
}

uint32_t swapRedBlue(uint32_t pixel)
{
    return (pixel & 0xff00ff00u)
            | ((pixel & 0x00ff0000u) >> 16)
            | ((pixel & 0x000000ffu) << 16);
}

} // namespace

DmaFrameSource::~DmaFrameSource()
{
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

ShmFrameSource::~ShmFrameSource()
{
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

bool makeDmaFrameSource(int fd, void *gbmBo, int width, int height, int stride,
                        const QString &devicePath, uint64_t modifier, uint64_t timestamp,
                        quint64 sequence, DmaFrameSourcePtr *source,
                        QString *error)
{
    if (source)
        source->reset();
    if (error)
        error->clear();

    if (fd < 0 || !gbmBo)
        return fail(error, QStringLiteral("DMA-BUF 帧缺少 fd 或 GBM BO"));
    if (width <= 0 || height <= 0)
        return fail(error, QStringLiteral("DMA-BUF 帧尺寸无效"));
    if (devicePath.trimmed().isEmpty())
        return fail(error, QStringLiteral("DMA-BUF 帧缺少 DRM 设备节点"));

    auto *bo = static_cast<gbm_bo *>(gbmBo);
    const uint64_t reportedModifier = gbm_bo_get_modifier(bo);
    // INVALID 表示合成器协商的是隐式布局；驱动每帧返回的实际 modifier
    // 才是解码依据。其余情况必须严格匹配协商结果。
    if (reportedModifier != modifier
            && modifier != DRM_FORMAT_MOD_INVALID
            && !(reportedModifier == DRM_FORMAT_MOD_INVALID
                 && modifier == DRM_FORMAT_MOD_LINEAR)) {
        return fail(error, QStringLiteral("DMA-BUF modifier 不匹配：期望 0x%1，实际 0x%2")
                               .arg(modifier, 0, 16)
                               .arg(reportedModifier, 0, 16));
    }

    const int duplicatedFd = fcntl(fd, F_DUPFD_CLOEXEC, 0);
    if (duplicatedFd < 0) {
        return fail(error, QStringLiteral("复制 DMA-BUF fd 失败：%1")
                               .arg(QString::fromLocal8Bit(strerror(errno))));
    }

    auto frame = std::make_shared<DmaFrameSource>();
    frame->fd = duplicatedFd;
    frame->devicePath = devicePath;
    frame->format = gbm_bo_get_format(bo);
    frame->modifier = modifier == DRM_FORMAT_MOD_INVALID
            ? reportedModifier
            : modifier;
    frame->offset = gbm_bo_get_offset(bo, 0);
    frame->width = width;
    frame->height = height;
    frame->stride = static_cast<int>(planeStride(bo, stride));
    frame->timestamp = timestamp;
    frame->sequence = sequence;

    if (frame->format == 0 || frame->stride <= 0) {
        return fail(error, QStringLiteral("DMA-BUF 帧元数据无效"));
    }

    if (source)
        *source = std::move(frame);
    return true;
}

bool makeShmFrameSource(int fd, size_t size, int width, int height, int stride,
                        uint64_t timestamp, quint64 sequence,
                        ShmFrameSourcePtr *source, QString *error)
{
    if (source)
        source->reset();
    if (error)
        error->clear();

    if (fd < 0)
        return fail(error, QStringLiteral("SHM 帧缺少 memfd"));
    if (width <= 0 || height <= 0 || stride <= 0)
        return fail(error, QStringLiteral("SHM 帧尺寸或 stride 无效"));

    const size_t required = static_cast<size_t>(stride) * static_cast<size_t>(height);
    if (size < required)
        return fail(error, QStringLiteral("SHM 帧数据不足：%1 < %2")
                               .arg(static_cast<qulonglong>(size))
                               .arg(static_cast<qulonglong>(required)));

    const int duplicatedFd = fcntl(fd, F_DUPFD_CLOEXEC, 0);
    if (duplicatedFd < 0) {
        return fail(error, QStringLiteral("复制 SHM memfd 失败：%1")
                               .arg(QString::fromLocal8Bit(strerror(errno))));
    }

    auto frame = std::make_shared<ShmFrameSource>();
    frame->fd = duplicatedFd;
    frame->size = size;
    frame->width = width;
    frame->height = height;
    frame->stride = stride;
    frame->timestamp = timestamp;
    frame->sequence = sequence;

    if (source)
        *source = std::move(frame);
    return true;
}

CapturePipelineWorker::CapturePipelineWorker(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<DmaFrameSourcePtr>("DmaFrameSourcePtr");
    qRegisterMetaType<ShmFrameSourcePtr>("ShmFrameSourcePtr");
    qRegisterMetaType<FrameInput>("FrameInput");
    qRegisterMetaType<QVector<FrameInput>>("QVector<FrameInput>");
    qRegisterMetaType<QVector<ScreenLayoutInfo>>("QVector<ScreenLayoutInfo>");
}

CapturePipelineWorker::~CapturePipelineWorker()
{
    releaseGbmDevices();
}

void CapturePipelineWorker::releaseGbmDevices()
{
    for (auto it = m_gbmDevices.begin(); it != m_gbmDevices.end(); ++it) {
        delete it->gpuConverter;
        it->gpuConverter = nullptr;
        if (it->device)
            gbm_device_destroy(it->device);
        if (it->fd >= 0)
            ::close(it->fd);
    }
    m_gbmDevices.clear();
}

struct gbm_device *CapturePipelineWorker::gbmDeviceForPath(const QString &devicePath,
                                                          QString *error)
{
    const auto cached = m_gbmDevices.constFind(devicePath);
    if (cached != m_gbmDevices.constEnd())
        return cached->device;

    const int drmFd = ::open(QFile::encodeName(devicePath).constData(),
                             O_RDWR | O_CLOEXEC);
    if (drmFd < 0) {
        fail(error, QStringLiteral("打开 DRM 节点 %1 失败：%2")
                        .arg(devicePath,
                             QString::fromLocal8Bit(strerror(errno))));
        return nullptr;
    }

    gbm_device *device = gbm_create_device(drmFd);
    if (!device) {
        ::close(drmFd);
        fail(error, QStringLiteral("创建 GBM 设备失败：%1").arg(devicePath));
        return nullptr;
    }

    GbmDeviceEntry entry;
    entry.fd = drmFd;
    entry.device = device;
    m_gbmDevices.insert(devicePath, entry);
    return device;
}

DmaFrameGpuConverter *CapturePipelineWorker::gpuConverterForPath(const QString &devicePath,
                                                                QString *error)
{
    auto it = m_gbmDevices.find(devicePath);
    if (it == m_gbmDevices.end()) {
        if (!gbmDeviceForPath(devicePath, error))
            return nullptr;
        it = m_gbmDevices.find(devicePath);
        if (it == m_gbmDevices.end())
            return nullptr;
    }

    if (!it->gpuConverter)
        it->gpuConverter = new DmaFrameGpuConverter(it->device);
    if (!it->gpuConverter->isAvailable(error)) {
        delete it->gpuConverter;
        it->gpuConverter = nullptr;
        return nullptr;
    }
    return it->gpuConverter;
}

QByteArray CapturePipelineWorker::convertDma(const DmaFrameSourcePtr &source,
                                             QString *error, qint64 *elapsedMs)
{
    if (error)
        error->clear();
    if (elapsedMs)
        *elapsedMs = 0;

    if (!source) {
        fail(error, QStringLiteral("DMA-BUF 帧描述为空"));
        return {};
    }
    if (source->fd < 0 || source->devicePath.isEmpty()) {
        fail(error, QStringLiteral("DMA-BUF 帧描述不完整"));
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    const int outStride = source->width * 4;
    const int outHeight = source->height;
    if (outStride <= 0 || outHeight <= 0
            || static_cast<qint64>(outStride) * outHeight > INT_MAX) {
        fail(error, QStringLiteral("DMA-BUF 输出尺寸无效"));
        return {};
    }

    QString cpuError;
    if (source->modifier == DRM_FORMAT_MOD_LINEAR) {
        gbm_device *device = gbmDeviceForPath(source->devicePath, &cpuError);
        if (device) {
            gbm_import_fd_modifier_data import{};
            import.width = static_cast<uint32_t>(source->width);
            import.height = static_cast<uint32_t>(source->height);
            import.format = source->format;
            import.num_fds = 1;
            import.fds[0] = source->fd;
            import.strides[0] = source->stride;
            import.offsets[0] = static_cast<int>(source->offset);
            import.modifier = source->modifier;

            gbm_bo *bo = gbm_bo_import(device, GBM_BO_IMPORT_FD_MODIFIER, &import,
                                       GBM_BO_USE_LINEAR);
            if (!bo) {
                cpuError = QStringLiteral("GBM 导入 LINEAR DMA-BUF 失败");
            } else {
                uint32_t mapStride = 0;
                void *mapHandle = nullptr;
                void *mapped = gbm_bo_map(bo, 0, 0, static_cast<uint32_t>(source->width),
                                          static_cast<uint32_t>(source->height),
                                          GBM_BO_TRANSFER_READ, &mapStride, &mapHandle);
                const bool formatSupported = isRgbxMemoryFormat(source->format)
                        || isBgrxMemoryFormat(source->format);
                if (mapped && formatSupported
                        && mapStride >= static_cast<uint32_t>(outStride)) {
                    QByteArray result;
                    result.resize(static_cast<int>(static_cast<qint64>(outStride) * outHeight));
                    const char *src = static_cast<const char *>(mapped)
                            + static_cast<qint64>(source->offset);
                    char *dst = result.data();
                    for (int y = 0; y < outHeight; ++y) {
                        const char *srcRow = src + static_cast<qint64>(y) * mapStride;
                        char *dstRow = dst + static_cast<qint64>(y) * outStride;
                        if (isBgrxMemoryFormat(source->format)) {
                            const uint32_t *srcPixels =
                                    reinterpret_cast<const uint32_t *>(srcRow);
                            uint32_t *dstPixels = reinterpret_cast<uint32_t *>(dstRow);
                            for (int x = 0; x < source->width; ++x)
                                dstPixels[x] = swapRedBlue(srcPixels[x]);
                        } else {
                            std::memcpy(dstRow, srcRow, outStride);
                        }
                    }
                    gbm_bo_unmap(bo, mapHandle);
                    gbm_bo_destroy(bo);
                    if (elapsedMs)
                        *elapsedMs = timer.elapsed();
                    return result;
                }
                if (mapped)
                    gbm_bo_unmap(bo, mapHandle);
                gbm_bo_destroy(bo);
                cpuError = formatSupported
                        ? QStringLiteral("LINEAR DMA-BUF 的 stride 不足：%1 < %2")
                                  .arg(mapStride).arg(outStride)
                        : QStringLiteral("LINEAR DMA-BUF 格式 0x%1 无 CPU 颜色转换规则")
                                  .arg(source->format, 0, 16);
            }
        }
    }

    // 非 LINEAR 或 CPU 快速路径不可用时，统一交给 EGL 解 tile/modifier，
    // 再由 glReadPixels 输出紧密 RGBA。数据仍通过 DMA-BUF 传入。
    QString gpuError;
    DmaFrameGpuConverter *converter = gpuConverterForPath(source->devicePath, &gpuError);
    if (converter) {
        QByteArray result;
        if (converter->convertToRgba(source->fd, source->width, source->height,
                                     source->stride, source->offset, source->format,
                                     source->modifier, &result, &gpuError)
                && !result.isEmpty()) {
            if (elapsedMs)
                *elapsedMs = timer.elapsed();
            return result;
        }
    }

    if (!cpuError.isEmpty() && !gpuError.isEmpty()) {
        fail(error, QStringLiteral("DMA-BUF 消费失败；CPU：%1；GPU：%2")
                        .arg(cpuError, gpuError));
    } else if (!gpuError.isEmpty()) {
        fail(error, gpuError);
    } else if (!cpuError.isEmpty()) {
        fail(error, cpuError);
    } else {
        fail(error, QStringLiteral("DMA-BUF 消费失败"));
    }
    if (elapsedMs)
        *elapsedMs = timer.elapsed();
    return {};
}

QByteArray CapturePipelineWorker::convertShm(const ShmFrameSourcePtr &source, QString *error)
{
    if (error)
        error->clear();

    if (!source) {
        fail(error, QStringLiteral("SHM 帧描述为空"));
        return {};
    }
    if (source->fd < 0 || source->width <= 0 || source->height <= 0
            || source->stride <= 0) {
        fail(error, QStringLiteral("SHM 帧描述不完整"));
        return {};
    }

    const size_t required = static_cast<size_t>(source->stride)
            * static_cast<size_t>(source->height);
    if (source->size < required) {
        fail(error, QStringLiteral("SHM 帧数据不足"));
        return {};
    }

    void *mapped = mmap(nullptr, source->size, PROT_READ, MAP_SHARED, source->fd, 0);
    if (mapped == MAP_FAILED) {
        fail(error, QStringLiteral("映射 SHM memfd 失败：%1")
                        .arg(QString::fromLocal8Bit(strerror(errno))));
        return {};
    }

    const int outStride = source->width * 4;
    QByteArray result;
    result.resize(static_cast<int>(static_cast<qint64>(outStride) * source->height));
    char *dst = result.data();
    const char *src = static_cast<const char *>(mapped);
    const size_t copyWidth = std::min(static_cast<size_t>(outStride),
                                      static_cast<size_t>(source->stride));

    for (int y = 0; y < source->height; ++y) {
        std::memcpy(dst + static_cast<qint64>(y) * outStride,
                    src + static_cast<qint64>(y) * source->stride,
                    copyWidth);
    }

    munmap(mapped, source->size);
    return result;
}

void CapturePipelineWorker::convertSingleDma(const DmaFrameSourcePtr &source, quint64 jobId)
{
    QString error;
    qint64 elapsedMs = 0;
    const QByteArray data = convertDma(source, &error, &elapsedMs);
    if (data.isEmpty()) {
        emit singleFrameFailed(error.isEmpty() ? QStringLiteral("DMA-BUF 转换失败") : error,
                               jobId);
        return;
    }

    emit singleFrameReady(data, source->width, source->height, source->width * 4,
                          source->timestamp, jobId, elapsedMs);
}

void CapturePipelineWorker::convertSingleShm(const ShmFrameSourcePtr &source, quint64 jobId)
{
    QElapsedTimer timer;
    timer.start();
    QString error;
    const QByteArray data = convertShm(source, &error);
    if (data.isEmpty()) {
        emit singleFrameFailed(error.isEmpty() ? QStringLiteral("SHM 帧转换失败") : error,
                               jobId);
        return;
    }

    emit singleFrameReady(data, source->width, source->height, source->width * 4,
                          source->timestamp, jobId, timer.elapsed());
}

void CapturePipelineWorker::convertSingleCpu(const QByteArray &data, int width, int height,
                                             int stride, uint64_t timestamp, quint64 jobId)
{
    if (data.isEmpty() || width <= 0 || height <= 0 || stride <= 0) {
        emit singleFrameFailed(QStringLiteral("CPU 帧数据无效"), jobId);
        return;
    }
    emit singleFrameReady(data, width, height, stride, timestamp, jobId, 0);
}

void CapturePipelineWorker::ensureCompositor()
{
    if (m_compositor)
        return;

    m_compositor = new MultiScreenFrameCompositor(this);
    connect(m_compositor, &MultiScreenFrameCompositor::compositionComplete,
            this, [this](const QByteArray &data, int width, int height, int stride,
                         uint64_t timestamp) {
                const quint64 roundId = m_activeRoundId;
                const qint64 elapsedMs = m_roundTimer.isValid()
                        ? m_roundTimer.elapsed()
                        : 0;
                m_activeRoundId = 0;
                emit multiFrameReady(data, width, height, stride, timestamp, roundId,
                                     elapsedMs);
            }, Qt::DirectConnection);
    connect(m_compositor, &MultiScreenFrameCompositor::compositionError,
            this, [this](const QString &message) {
                const quint64 roundId = m_activeRoundId;
                m_activeRoundId = 0;
                emit multiFrameFailed(message, roundId);
            }, Qt::DirectConnection);
}

void CapturePipelineWorker::configureMulti(const QSize &virtualDesktopSize, qreal outputScale,
                                           const QRect &cropRegion, bool hasCrop,
                                           const QVector<ScreenLayoutInfo> &layouts)
{
    ensureCompositor();
    m_compositor->setOutputScale(outputScale);
    if (hasCrop && cropRegion.isValid() && !cropRegion.isEmpty())
        m_compositor->setCropRegion(cropRegion);
    else
        m_compositor->clearCropRegion();
    m_compositor->setVirtualDesktopSize(virtualDesktopSize);
    m_compositor->setScreenLayouts(layouts);
    m_compositor->reset();
    m_activeRoundId = 0;

    qCWarning(dsrApp) << "CapturePipelineWorker: multi configured, outputs:"
                      << layouts.size() << "logical desktop:" << virtualDesktopSize
                      << "scale:" << outputScale << "crop:" << cropRegion
                      << "physical output:" << m_compositor->getOutputSize();
}

void CapturePipelineWorker::processMultiRound(quint64 roundId,
                                              const QVector<FrameInput> &frames)
{
    ensureCompositor();
    if (!m_compositor || frames.isEmpty()) {
        emit multiFrameFailed(QStringLiteral("多屏合成器未初始化或帧为空"), roundId);
        return;
    }

    m_activeRoundId = roundId;
    m_roundTimer.start();
    m_compositor->beginRound(roundId);

    for (const FrameInput &input : frames) {
        DmaFrameInfo info;
        info.screenName = input.screenName;
        info.screenGeometry = input.screenGeometry;
        info.roundId = roundId;
        info.width = input.width;
        info.height = input.height;
        // convertDma() 输出已重排为 width*4 紧密行。
        info.stride = (input.isDma || input.shm) ? input.width * 4 : input.stride;
        info.timestamp = input.timestamp;
        info.ready = true;

        if (input.isDma) {
            QString error;
            info.frameData = convertDma(input.dma, &error, nullptr);
            if (info.frameData.isEmpty()) {
                m_compositor->reset();
                m_activeRoundId = 0;
                emit multiFrameFailed(error.isEmpty()
                                              ? QStringLiteral("DMA-BUF 转换失败")
                                              : error,
                                      roundId);
                return;
            }
        } else if (input.shm) {
            QString error;
            info.frameData = convertShm(input.shm, &error);
            if (info.frameData.isEmpty()) {
                m_compositor->reset();
                m_activeRoundId = 0;
                emit multiFrameFailed(error.isEmpty()
                                              ? QStringLiteral("SHM 帧转换失败")
                                              : error,
                                      roundId);
                return;
            }
        } else {
            info.frameData = input.cpuData;
        }

        if (!m_compositor->addScreenFrame(info)) {
            m_compositor->reset();
            m_activeRoundId = 0;
            emit multiFrameFailed(QStringLiteral("多屏帧提交失败：%1").arg(input.screenName),
                                  roundId);
            return;
        }
    }

    // addScreenFrame() 在最后一块屏幕到达后投递 performComposition，
    // 由本线程事件循环完成 QPainter 合成，避免占用 GUI 线程。
}

void CapturePipelineWorker::resetMulti()
{
    if (m_compositor)
        m_compositor->reset();
    m_activeRoundId = 0;
}
