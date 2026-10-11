// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ximageconverter.h"
#include <QSysInfo>
#include <cstring>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

QImage XImageConverter::toImage(XImage *image)
{
    if (!image || !image->data || image->width <= 0 || image->height <= 0
        || image->bits_per_pixel <= 0 || image->bits_per_pixel > 32
        || image->xoffset < 0 || image->bytes_per_line <= 0
        || qint64(image->bytes_per_line) * 8 < qint64(image->width) * image->bits_per_pixel
        || !image->f.get_pixel) {
        return {};
    }

    QImage result(image->width, image->height, QImage::Format_RGB32);
    if (result.isNull())
        return result;

    const int nativeOrder = QSysInfo::ByteOrder == QSysInfo::LittleEndian ? LSBFirst : MSBFirst;
    const bool direct = image->format == ZPixmap && image->bits_per_pixel == 32
        && (image->depth == 24 || image->depth == 32)
        && image->red_mask == 0xff0000 && image->green_mask == 0xff00 && image->blue_mask == 0xff
        && image->byte_order == nativeOrder && image->xoffset == 0
        && image->bytes_per_line % alignof(quint32) == 0
        && reinterpret_cast<quintptr>(image->data) % alignof(quint32) == 0;

    for (int y = 0; y < image->height; ++y) {
        if (direct) {
            const char *source = image->data + qsizetype(y) * image->bytes_per_line;
            QRgb *destination = reinterpret_cast<QRgb *>(result.scanLine(y));
            for (int x = 0; x < image->width; ++x) {
                quint32 pixel;
                // memcpy avoids aliasing assumptions about Xlib-owned storage.
                std::memcpy(&pixel, source + qsizetype(x) * sizeof(pixel), sizeof(pixel));
                destination[x] = pixel | 0xff000000u;
            }
        } else {
            // Preserve the existing conversion for uncommon XImage layouts.
            for (int x = 0; x < image->width; ++x) {
                const unsigned long pixel = XGetPixel(image, x, y);
                result.setPixel(x, y, qRgb((pixel >> 16) & 0xff, (pixel >> 8) & 0xff, pixel & 0xff));
            }
        }
    }
    return result;
}
