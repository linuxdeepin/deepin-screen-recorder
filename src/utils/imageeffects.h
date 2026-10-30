// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGE_EFFECTS_H
#define IMAGE_EFFECTS_H

#include <QImage>

namespace Dsr::ImageEffects {

/**
 * Clip an image to a rounded rectangle.
 *
 * The radius is expressed in logical pixels and scaled by the image's
 * devicePixelRatio. The returned image keeps the source size and DPR so the
 * same helper can be used by the live UI path and by pixel-sized exports.
 */
QImage roundedCorners(const QImage &source, qreal radius);

/**
 * Composite an image with an alpha channel onto a white background.
 *
 * Images without Alpha are returned unchanged. DevicePixelRatio is preserved.
 */
QImage compositeOnWhite(const QImage &source);

} // namespace Dsr::ImageEffects

#endif // IMAGE_EFFECTS_H
