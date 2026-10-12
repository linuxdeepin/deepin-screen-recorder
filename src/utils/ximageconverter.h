// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef XIMAGECONVERTER_H
#define XIMAGECONVERTER_H
#include <QImage>
struct _XImage;

namespace XImageConverter {
// Returns an opaque RGB32 image. When the XImage already stores native RGB32
// pixels, its buffer is reused: alpha is forced in place and image->data is set
// to nullptr. Callers must still release the XImage with XDestroyImage.
QImage toImage(_XImage *image);
}
#endif
