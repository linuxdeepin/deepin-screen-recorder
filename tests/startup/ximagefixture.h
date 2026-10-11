// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QImage>
#include <QSysInfo>
#include <vector>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

struct XImageFixture
{
    std::vector<char> storage;
    XImage image{};
    XImageFixture(int width, int height, int bits = 32, bool native = true, int offset = 0)
        : storage((width * bits / 8 + 12) * height + offset)
    {
        image.width = width;
        image.height = height;
        image.format = ZPixmap;
        image.data = storage.data() + offset;
        const int nativeOrder = QSysInfo::ByteOrder == QSysInfo::LittleEndian ? LSBFirst : MSBFirst;
        image.byte_order = native ? nativeOrder : (nativeOrder == LSBFirst ? MSBFirst : LSBFirst);
        image.bitmap_unit = 32;
        image.bitmap_bit_order = image.byte_order;
        image.bitmap_pad = 32;
        image.depth = 24;
        image.bits_per_pixel = bits;
        image.bytes_per_line = ((width * bits / 8 + 3) & ~3) + 8;
        image.red_mask = 0xff0000;
        image.green_mask = 0xff00;
        image.blue_mask = 0xff;
        if (!XInitImage(&image))
            qFatal("Could not initialize XImage fixture");
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x)
                XPutPixel(&image, x, y, qRgb((x * 19 + y) % 256, (y * 53 + x) % 256, (x + y * 7) % 256));
        }
    }
};

inline QImage legacyXImageConversion(XImage *image)
{
    if (!image || !image->data || image->width <= 0 || image->height <= 0)
        return {};
    QImage result(image->width, image->height, QImage::Format_RGB32);
    for (int y = 0; y < image->height; ++y) {
        for (int x = 0; x < image->width; ++x) {
            unsigned long pixel = XGetPixel(image, x, y);
            result.setPixel(x, y, qRgb((pixel >> 16) & 0xff, (pixel >> 8) & 0xff, pixel & 0xff));
        }
    }
    return result;
}
