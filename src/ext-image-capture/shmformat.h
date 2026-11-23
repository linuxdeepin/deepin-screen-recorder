// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSR_EXT_IMAGE_CAPTURE_SHMFORMAT_H
#define DSR_EXT_IMAGE_CAPTURE_SHMFORMAT_H

#include <QImage>
#include <QByteArray>
#include <QString>

#include <array>
#include <cctype>
#include <cstdint>
#include <limits>
#include <numeric>

#include <wayland-client.h>

// wl_shm 像素格式的单一事实来源。
//
// 这张表同时驱动三件事：捕获会话选择格式、按每像素字节数计算缓冲区 stride、
// 把帧数据解码为 QImage。只要某个格式不在表里，就绝不会被选中，也不会被交给
// QImage，因此不会出现"选中了却解不了"的帧格式无效失败。新增格式只需在
// shmFormatTable() 追加一行，三处自动保持一致。
namespace Dsr::ExtImageCapture {

// Qt 要求每行像素按 4 字节对齐；wlroots 还要求 stride 是每像素字节数的整数倍
// （因此 24 位格式需按 12 字节对齐）。这里取二者最小公倍数为对齐粒度。
constexpr int kQtScanlineAlignment = 4;

// fourcc 由 4 个 ASCII 字节组成。
constexpr int kFourccLength = 4;
constexpr int kBitsPerByte = 8;
constexpr uint32_t kByteMask = 0xffu;

// 编码器与合成器统一使用的紧密 RGBA8888 布局。
constexpr int kRgbaBytesPerPixel = 4;
constexpr int kRgbaAlphaOffset = 3;
constexpr uchar kOpaqueAlpha = 255;

// 各格式的字节宽度。
constexpr int kBytesPerPixel32 = 4;
constexpr int kBytesPerPixel24 = 3;

struct ShmFormatInfo {
    uint32_t waylandFormat;
    QImage::Format imageFormat;
    int bytesPerPixel;
};

// 按捕获偏好排序：32 位格式在前（与历史行为一致），24 位格式在后。
inline const auto &shmFormatTable()
{
    static constexpr std::array<ShmFormatInfo, 6> table = {{
        {WL_SHM_FORMAT_XRGB8888, QImage::Format_RGB32, kBytesPerPixel32},
        {WL_SHM_FORMAT_ARGB8888, QImage::Format_ARGB32_Premultiplied, kBytesPerPixel32},
        {WL_SHM_FORMAT_XBGR8888, QImage::Format_RGBX8888, kBytesPerPixel32}, // 'XB24'
        {WL_SHM_FORMAT_ABGR8888, QImage::Format_RGBA8888_Premultiplied, kBytesPerPixel32}, // 'AB24'
        // 24 位：小端机上 wl_shm BGR888(BG24) 的内存字节序为 R,G,B，对应
        // QImage::Format_RGB888；RGB888(RG24) 为 B,G,R，对应 Format_BGR888。
        {WL_SHM_FORMAT_BGR888,   QImage::Format_RGB888,   kBytesPerPixel24}, // 'BG24'
        {WL_SHM_FORMAT_RGB888,   QImage::Format_BGR888,   kBytesPerPixel24}, // 'RG24'
    }};
    return table;
}

inline const ShmFormatInfo *shmFormatInfo(uint32_t format)
{
    for (const ShmFormatInfo &info : shmFormatTable()) {
        if (info.waylandFormat == format)
            return &info;
    }
    return nullptr;
}

template <typename Formats>
inline const ShmFormatInfo *selectShmFormat(const Formats &advertised)
{
    for (const ShmFormatInfo &info : shmFormatTable()) {
        if (advertised.contains(info.waylandFormat))
            return &info;
    }
    return nullptr;
}

// 该格式能否被本客户端解码；不能则返回 QImage::Format_Invalid。
inline QImage::Format imageFormatForShm(uint32_t format)
{
    const ShmFormatInfo *info = shmFormatInfo(format);
    return info ? info->imageFormat : QImage::Format_Invalid;
}

inline int bytesPerPixelForShm(uint32_t format)
{
    const ShmFormatInfo *info = shmFormatInfo(format);
    return info ? info->bytesPerPixel : 0;
}

// Qt scanlines are 4-byte aligned; wlroots also requires a whole number
// of pixels per stride (12-byte alignment for 24-bit RGB).
inline int strideForShm(uint32_t format, qint64 width)
{
    const int bpp = bytesPerPixelForShm(format);
    if (bpp <= 0 || width <= 0)
        return 0;
    constexpr auto limit = std::numeric_limits<int32_t>::max();
    if (width > limit / bpp)
        return 0;
    const int alignment = std::lcm(kQtScanlineAlignment, bpp);
    const qint64 stride = ((width * bpp + alignment - 1) / alignment) * alignment;
    return stride <= limit ? static_cast<int>(stride) : 0;
}

// create_pool takes a signed 32-bit size on the wire.
inline int bufferSizeForShm(int stride, qint64 height)
{
    if (stride <= 0 || height <= 0 || height > std::numeric_limits<int32_t>::max() / stride)
        return 0;
    return static_cast<int>(stride * height);
}

// The returned view borrows data; the caller must copy or convert it before unmapping.
inline QImage shmImageView(const void *data, size_t size, int width, int height,
                           int stride, uint32_t format)
{
    const int bpp = bytesPerPixelForShm(format);
    const int required = bufferSizeForShm(stride, height);
    if (!data || !bpp || width <= 0 || !required || size < static_cast<size_t>(required)
            || static_cast<qint64>(width) * bpp > stride)
        return {};
    return QImage(static_cast<const uchar *>(data), width, height, stride, imageFormatForShm(format));
}

// 紧密 RGBA8888 的行步长，供编码/合成沿路的缓冲区使用。
inline int rgbaBytesPerLine(qint64 width)
{
    return strideForShm(WL_SHM_FORMAT_XBGR8888, width);
}

// The encoder expects tightly packed, owned RGBA bytes.
inline QByteArray shmToRgba(const void *data, size_t size, int width, int height,
                            int stride, uint32_t format)
{
    const int outStride = rgbaBytesPerLine(width);
    const int outSize = bufferSizeForShm(outStride, height);
    if (!outSize)
        return {};
    const QImage rgba = shmImageView(data, size, width, height, stride, format)
                                .convertToFormat(QImage::Format_RGBA8888);
    if (rgba.isNull() || rgba.bytesPerLine() != outStride)
        return {};
    QByteArray pixels(reinterpret_cast<const char *>(rgba.constBits()), outSize);
    // Wayland 的 XRGB/XBGR 缓冲区里未使用的第 4 字节不保证是 255，需显式置为不透明。
    if (format == WL_SHM_FORMAT_XRGB8888 || format == WL_SHM_FORMAT_XBGR8888) {
        auto *bytes = reinterpret_cast<uchar *>(pixels.data());
        for (qsizetype i = kRgbaAlphaOffset; i < pixels.size(); i += kRgbaBytesPerPixel)
            bytes[i] = kOpaqueAlpha;
    }
    return pixels;
}

// 便于日志输出：十六进制值，若对应 fourcc 可打印则附带字符形式。
inline QString shmFormatToString(uint32_t format)
{
    if (format == WL_SHM_FORMAT_ARGB8888)
        return QStringLiteral("0x00000000 (ARGB8888)");
    if (format == WL_SHM_FORMAT_XRGB8888)
        return QStringLiteral("0x00000001 (XRGB8888)");
    QString text = QStringLiteral("0x%1").arg(format, kFourccLength * 2, 16, QLatin1Char('0'));
    std::array<char, kFourccLength + 1> fourcc{};
    bool printable = true;
    for (int i = 0; i < kFourccLength; ++i) {
        const auto byte = static_cast<unsigned char>((format >> (i * kBitsPerByte)) & kByteMask);
        fourcc[i] = static_cast<char>(byte);
        if (!std::isprint(byte))
            printable = false;
    }
    if (printable)
        return text + QStringLiteral(" (") + QString::fromLatin1(fourcc.data(), kFourccLength) + QLatin1Char(')');
    return text;
}

} // namespace Dsr::ExtImageCapture

#endif // DSR_EXT_IMAGE_CAPTURE_SHMFORMAT_H
