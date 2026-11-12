// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMAFRAMEGPUCONVERTER_H
#define DMAFRAMEGPUCONVERTER_H

#include <QByteArray>
#include <QString>

#include <cstdint>

struct gbm_device;

/**
 * @brief 将任意 DRM modifier 的 DMA-BUF 转换为紧密 RGBA 行数据。
 *
 * 仅允许在同一线程创建和使用。无 GPU 上下文时会返回明确的错误，
 * 由调用方决定是否回退 SHM。
 */
class DmaFrameGpuConverter
{
public:
    explicit DmaFrameGpuConverter(gbm_device *device);
    ~DmaFrameGpuConverter();

    DmaFrameGpuConverter(const DmaFrameGpuConverter &) = delete;
    DmaFrameGpuConverter &operator=(const DmaFrameGpuConverter &) = delete;

    bool isAvailable(QString *error = nullptr);

    bool canImport(int fd, int width, int height, int stride, uint32_t offset,
                   uint32_t format, uint64_t modifier, QString *error = nullptr);

    bool convertToRgba(int fd, int width, int height, int stride, uint32_t offset,
                       uint32_t format, uint64_t modifier, QByteArray *output,
                       QString *error = nullptr);

private:
    struct Private;
    Private *d;
};

#endif // DMAFRAMEGPUCONVERTER_H
