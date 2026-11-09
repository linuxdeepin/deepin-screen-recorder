// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandscrollcompositor.h"

#include <QPainter>

namespace Dsr::Screenshot {

namespace {

QRect selectionRectInImage(const QImage &image,
                           const QRect &outputGeometry,
                           const QRect &selection)
{
    if (image.isNull() || outputGeometry.isEmpty() || selection.isEmpty())
        return {};

    const QRect local = selection.translated(-outputGeometry.topLeft());
    const qreal scaleX = static_cast<qreal>(image.width()) / outputGeometry.width();
    const qreal scaleY = static_cast<qreal>(image.height()) / outputGeometry.height();
    const int left = qRound(local.left() * scaleX);
    const int top = qRound(local.top() * scaleY);
    const int right = qRound(local.right() * scaleX);
    const int bottom = qRound(local.bottom() * scaleY);
    return QRect(QPoint(left, top), QPoint(right, bottom)).intersected(image.rect());
}

} // namespace

QImage composeScrollSelectionFrame(const QList<OutputDescriptor> &outputs,
                                   const QList<QImage> &images,
                                   const QRect &globalSelection)
{
    if (images.size() != outputs.size() || globalSelection.isEmpty())
        return {};

    // Use the DPR of the output covering most of the selection as the canvas
    // scale. Mixed-DPR layouts then keep one consistent pixel grid instead of
    // resampling every output to a different scale.
    qreal canvasScale = 1.0;
    qint64 bestOverlapArea = 0;
    for (const OutputDescriptor &output : outputs) {
        const QRect overlap = output.logicalGeometry.intersected(globalSelection);
        const qint64 area = static_cast<qint64>(overlap.width()) * overlap.height();
        if (area > bestOverlapArea) {
            bestOverlapArea = area;
            canvasScale = output.devicePixelRatio > 0 ? output.devicePixelRatio : 1.0;
        }
    }

    const QSize canvasSize(qMax(1, qRound(globalSelection.width() * canvasScale)),
                           qMax(1, qRound(globalSelection.height() * canvasScale)));
    QImage canvas(canvasSize, QImage::Format_RGB32);
    canvas.fill(Qt::black);

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    for (int i = 0; i < outputs.size(); ++i) {
        const OutputDescriptor &output = outputs[i];
        const QRect overlap = output.logicalGeometry.intersected(globalSelection);
        if (overlap.isEmpty())
            continue;

        const QRect source = selectionRectInImage(images[i],
                                                  output.logicalGeometry,
                                                  overlap);
        if (source.isEmpty()) {
            painter.end();
            return {};
        }
        const QRect target(
                QPoint(qRound((overlap.x() - globalSelection.x()) * canvasScale),
                       qRound((overlap.y() - globalSelection.y()) * canvasScale)),
                QSize(qMax(1, qRound(overlap.width() * canvasScale)),
                      qMax(1, qRound(overlap.height() * canvasScale))));
        painter.drawImage(target, images[i], source);
    }
    painter.end();
    return canvas;
}

} // namespace Dsr::Screenshot
