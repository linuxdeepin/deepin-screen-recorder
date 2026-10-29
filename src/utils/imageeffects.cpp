// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageeffects.h"

#include <QPainter>
#include <QPainterPath>

namespace Dsr::ImageEffects {

namespace {

qreal devicePixelRatioFor(const QImage &image)
{
    return image.devicePixelRatio() > 0.0 ? image.devicePixelRatio() : 1.0;
}

} // namespace

QImage roundedCorners(const QImage &source, qreal radius)
{
    if (source.isNull() || radius <= 0.0)
        return source;

    QImage image = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const qreal dpr = devicePixelRatioFor(source);

    // Keep the painting coordinate system in device pixels while clipping.
    // Otherwise QPainter applies the image DPR a second time to the path and
    // only the top-left corner is clipped correctly on high-DPI outputs.
    image.setDevicePixelRatio(1.0);
    const qreal scaledRadius = radius * dpr;
    QImage out(image.size(), QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::transparent);
    out.setDevicePixelRatio(1.0);

    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, image.width(), image.height()),
                        scaledRadius,
                        scaledRadius);
    painter.setClipPath(path);
    painter.drawImage(0, 0, image);
    painter.end();

    out.setDevicePixelRatio(dpr);
    return out;
}

QImage compositeOnWhite(const QImage &source)
{
    if (source.isNull() || !source.hasAlphaChannel())
        return source;

    QImage image = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const qreal dpr = devicePixelRatioFor(source);
    QImage out(image.size(), QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::white);
    out.setDevicePixelRatio(dpr);

    QPainter painter(&out);
    painter.drawImage(0, 0, image);
    painter.end();
    return out;
}

} // namespace Dsr::ImageEffects
