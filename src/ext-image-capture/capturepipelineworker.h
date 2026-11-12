// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAPTUREPIPELINEWORKER_H
#define CAPTUREPIPELINEWORKER_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QHash>
#include <QMetaType>
#include <QObject>
#include <QRect>
#include <QSize>
#include <QString>
#include <QVector>

#include <cstddef>
#include <memory>

#include "multiscreenframecompositor.h"

class MultiScreenFrameCompositor;
class DmaFrameGpuConverter;

/**
 * @brief 一个可从 GUI 线程安全移交的 DMA-BUF 帧描述。
 *
 * fd 是 dup 后的副本，由结构体析构时关闭；gbmBo 只用于在 GUI 线程提取
 * 格式、stride、modifier 等元数据，不会传到工作线程。
 */
struct DmaFrameSource
{
    int fd = -1;
    QString devicePath;
    uint32_t format = 0;
    uint64_t modifier = 0;
    uint32_t offset = 0;
    int width = 0;
    int height = 0;
    int stride = 0;
    uint64_t timestamp = 0;
    quint64 sequence = 0;

    ~DmaFrameSource();
};

using DmaFrameSourcePtr = std::shared_ptr<DmaFrameSource>;

/**
 * @brief 一个可从 GUI 线程安全移交的 wl_shm memfd 帧描述。
 *
 * fd 是 dup 后的副本；工作线程自行 mmap、读取并拷贝，避免 GUI 线程
 * 在帧回调里构造整帧 QByteArray。
 */
struct ShmFrameSource
{
    int fd = -1;
    size_t size = 0;
    int width = 0;
    int height = 0;
    int stride = 0;
    uint64_t timestamp = 0;
    quint64 sequence = 0;

    ~ShmFrameSource();
};

using ShmFrameSourcePtr = std::shared_ptr<ShmFrameSource>;

/**
 * @brief 多屏一轮采集中的单个输出帧。
 *
 * 该结构体只包含纯数据、memfd 或 dup 后的 DMA-BUF fd，工作线程不会
 * 接触 QScreen。
 */
struct FrameInput
{
    QString screenName;
    QRect screenGeometry;
    bool isDma = false;
    DmaFrameSourcePtr dma;
    ShmFrameSourcePtr shm;
    QByteArray cpuData;
    int width = 0;
    int height = 0;
    int stride = 0;
    uint64_t timestamp = 0;
    quint64 sequence = 0;
};

bool makeDmaFrameSource(int fd, void *gbmBo, int width, int height, int stride,
                        const QString &devicePath, uint64_t modifier, uint64_t timestamp,
                        quint64 sequence, DmaFrameSourcePtr *source,
                        QString *error);

bool makeShmFrameSource(int fd, size_t size, int width, int height, int stride,
                        uint64_t timestamp, quint64 sequence,
                        ShmFrameSourcePtr *source, QString *error);

/**
 * @brief 采集侧工作线程。
 *
 * 单屏录制在 GUI 线程只提取元数据并 dup fd；工作线程负责 DMA-BUF
 * 导入/映射或 wl_shm memfd 映射/拷贝。多屏录制在同一线程内依次转换
 * 本轮所有输出，再交给 MultiScreenFrameCompositor 合成。编码仍由
 * FramePumpWorker 的独立线程负责。
 */
class CapturePipelineWorker : public QObject
{
    Q_OBJECT

public:
    explicit CapturePipelineWorker(QObject *parent = nullptr);
    ~CapturePipelineWorker() override;

public slots:
    void convertSingleDma(const DmaFrameSourcePtr &source, quint64 jobId);
    void convertSingleShm(const ShmFrameSourcePtr &source, quint64 jobId);
    void convertSingleCpu(const QByteArray &data, int width, int height, int stride,
                          uint64_t timestamp, quint64 jobId);

    void configureMulti(const QSize &virtualDesktopSize, qreal outputScale,
                        const QRect &cropRegion, bool hasCrop,
                        const QVector<ScreenLayoutInfo> &layouts);
    void processMultiRound(quint64 roundId, const QVector<FrameInput> &frames);
    void resetMulti();

signals:
    void singleFrameReady(const QByteArray &data, int width, int height, int stride,
                          uint64_t timestamp, quint64 jobId, qint64 convertMs);
    void singleFrameFailed(const QString &message, quint64 jobId);

    void multiFrameReady(const QByteArray &data, int width, int height, int stride,
                         uint64_t timestamp, quint64 roundId, qint64 elapsedMs);
    void multiFrameFailed(const QString &message, quint64 roundId);

private:
    struct GbmDeviceEntry
    {
        int fd = -1;
        struct gbm_device *device = nullptr;
        DmaFrameGpuConverter *gpuConverter = nullptr;
    };

    void ensureCompositor();
    struct gbm_device *gbmDeviceForPath(const QString &devicePath, QString *error);
    DmaFrameGpuConverter *gpuConverterForPath(const QString &devicePath, QString *error);
    QByteArray convertDma(const DmaFrameSourcePtr &source, QString *error,
                          qint64 *elapsedMs);
    QByteArray convertShm(const ShmFrameSourcePtr &source, QString *error);
    void releaseGbmDevices();

    MultiScreenFrameCompositor *m_compositor = nullptr;
    quint64 m_activeRoundId = 0;
    QElapsedTimer m_roundTimer;
    QHash<QString, GbmDeviceEntry> m_gbmDevices;
};

Q_DECLARE_METATYPE(DmaFrameSourcePtr)
Q_DECLARE_METATYPE(ShmFrameSourcePtr)
Q_DECLARE_METATYPE(FrameInput)
Q_DECLARE_METATYPE(QVector<FrameInput>)
Q_DECLARE_METATYPE(QVector<ScreenLayoutInfo>)

#endif // CAPTUREPIPELINEWORKER_H
