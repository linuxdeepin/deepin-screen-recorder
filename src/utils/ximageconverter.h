// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef XIMAGECONVERTER_H
#define XIMAGECONVERTER_H
#include <QImage>
struct _XImage;

namespace XImageConverter {
// Returns an owned, opaque RGB32 image; it never retains the XImage buffer.
QImage toImage(_XImage *image);
}
#endif
