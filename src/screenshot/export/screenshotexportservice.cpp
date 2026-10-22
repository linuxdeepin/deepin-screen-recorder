// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshotexportservice.h"

#include "../core/coordinatemapper.h"
#include "../ports/iannotationeditor.h"

#include <QPainter>
#include <QtMath>

namespace Dsr::Screenshot {

namespace {

QRect intersectionForFrame(const QRect &selection, const OutputFrame &frame)
{
    return selection.intersected(frame.output.logicalGeometry);
}

} // namespace

Result ScreenshotExportService::render(const FrozenFrameSet &frames,
                                       const QRect &globalSelection,
                                       IAnnotationEditor *annotations) const
{
    Result result;
    if (frames.isEmpty() || globalSelection.isEmpty())
        return result;

    const QRect selection = globalSelection.normalized()
            .intersected(frames.virtualDesktopRect);
    if (selection.isEmpty())
        return result;

    qreal outputScale = 0.0;
    for (const OutputFrame &frame : frames.outputs) {
        if (frame.image.isNull())
            continue;
        if (!selection.intersects(frame.output.logicalGeometry))
            continue;
        outputScale = qMax(outputScale, frame.pixelScale);
    }
    if (outputScale <= 0.0)
        return result;

    const QSize pixelSize(qMax(1, qCeil(selection.width() * outputScale)),
                          qMax(1, qCeil(selection.height() * outputScale)));
    QImage image(pixelSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    for (const OutputFrame &frame : frames.outputs) {
        if (frame.image.isNull())
            continue;

        const QRect intersection = intersectionForFrame(selection, frame);
        if (intersection.isEmpty())
            continue;

        const QRect source = CoordinateMapper::globalToFramePixel(intersection,
                                                                   frame.output,
                                                                   frame.pixelScale)
                .intersected(frame.image.rect());
        if (source.isEmpty())
            continue;

        const QRectF destination((intersection.left() - selection.left()) * outputScale,
                                 (intersection.top() - selection.top()) * outputScale,
                                 intersection.width() * outputScale,
                                 intersection.height() * outputScale);
        painter.drawImage(destination, frame.image, source);
    }

    if (annotations) {
        painter.save();
        painter.setClipRect(image.rect());
        painter.setRenderHint(QPainter::Antialiasing, true);
        annotations->render(painter, selection, outputScale);
        painter.restore();
    }
    painter.end();

    result.image = image;
    result.globalSelection = selection;
    result.frameGeneration = frames.generation;
    for (const OutputFrame &frame : frames.outputs)
        result.outputs.append(frame.output);
    return result;
}

} // namespace Dsr::Screenshot
