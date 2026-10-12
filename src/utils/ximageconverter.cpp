// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ximageconverter.h"
#include <QSysInfo>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace {
// True when the XImage buffer is already laid out as native-endian 0x??RRGGBB words.
bool isNativeRgb32(const XImage *image)
{
    const int nativeOrder = QSysInfo::ByteOrder == QSysInfo::LittleEndian ? LSBFirst : MSBFirst;
    return image->format == ZPixmap && image->bits_per_pixel == 32
        && (image->depth == 24 || image->depth == 32)
        && image->red_mask == 0xff0000 && image->green_mask == 0xff00 && image->blue_mask == 0xff
        && image->byte_order == nativeOrder && image->xoffset == 0
        && image->bytes_per_line >= image->width * 4
        && image->bytes_per_line % alignof(quint32) == 0
        && reinterpret_cast<quintptr>(image->data) % alignof(quint32) == 0;
}

void releaseXImageData(void *data)
{
    XFree(data);
}
}

QImage XImageConverter::toImage(XImage *image)
{
    if (!image || !image->data || image->width <= 0 || image->height <= 0 || !image->f.get_pixel)
        return {};

    if (isNativeRgb32(image)) {
        // Force the unused high byte opaque in place, then hand the buffer to QImage.
        for (int y = 0; y < image->height; ++y) {
            quint32 *row = reinterpret_cast<quint32 *>(image->data + qsizetype(y) * image->bytes_per_line);
            for (int x = 0; x < image->width; ++x)
                row[x] |= 0xff000000u;
        }
        uchar *data = reinterpret_cast<uchar *>(image->data);
        QImage result(data, image->width, image->height, image->bytes_per_line,
                      QImage::Format_RGB32, releaseXImageData, data);
        if (!result.isNull())
            image->data = nullptr;
        return result;
    }

    // Preserve the existing conversion for uncommon XImage layouts.
    QImage result(image->width, image->height, QImage::Format_RGB32);
    if (result.isNull())
        return result;
    for (int y = 0; y < image->height; ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(result.scanLine(y));
        for (int x = 0; x < image->width; ++x) {
            const unsigned long pixel = XGetPixel(image, x, y);
            row[x] = qRgb((pixel >> 16) & 0xff, (pixel >> 8) & 0xff, pixel & 0xff);
        }
    }
    return result;
}
