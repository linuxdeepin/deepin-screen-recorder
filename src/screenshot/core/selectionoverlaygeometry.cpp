// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectionoverlaygeometry.h"

#include <QtGlobal>

namespace Dsr::Screenshot::SelectionOverlayGeometry {

namespace {

QRect normalizedNonEmpty(const QRect &rect)
{
    return rect.normalized();
}

} // namespace

Outline outlineForOutput(const QRect &globalSelection,
                         const QRect &outputGeometry)
{
    Outline outline;
    const QRect selection = normalizedNonEmpty(globalSelection);
    const QRect output = normalizedNonEmpty(outputGeometry);
    if (selection.isEmpty() || output.isEmpty())
        return outline;

    outline.localRect = selection.translated(-output.topLeft())
            .intersected(QRect(QPoint(0, 0), output.size()));
    if (outline.localRect.isEmpty())
        return outline;

    // Keep each output responsible only for the parts of the logical outline
    // that actually lie on it. In particular, a vertical or horizontal output
    // boundary inside the selection must not be painted as an internal edge.
    outline.top = selection.top() >= output.top()
            && selection.top() <= output.bottom();
    outline.bottom = selection.bottom() >= output.top()
            && selection.bottom() <= output.bottom();
    outline.left = selection.left() >= output.left()
            && selection.left() <= output.right();
    outline.right = selection.right() >= output.left()
            && selection.right() <= output.right();
    return outline;
}

QList<QLineF> strokedOutlineForOutput(const QRect &globalSelection,
                                      const QRect &outputGeometry,
                                      qreal strokeWidth)
{
    QList<QLineF> result;
    const Outline outline = outlineForOutput(globalSelection, outputGeometry);
    if (outline.localRect.isEmpty())
        return result;

    const qreal inset = qMax<qreal>(strokeWidth, 1.0) / 2.0;
    const qreal left = outline.localRect.left();
    const qreal top = outline.localRect.top();
    const qreal rightExclusive = outline.localRect.right() + 1.0;
    const qreal bottomExclusive = outline.localRect.bottom() + 1.0;
    const qreal horizontalStart = outline.left ? left + inset : left;
    const qreal horizontalEnd = outline.right ? rightExclusive - inset : rightExclusive;
    const qreal verticalStart = outline.top ? top + inset : top;
    const qreal verticalEnd = outline.bottom ? bottomExclusive - inset : bottomExclusive;

    if (outline.top) {
        result.append(QLineF(horizontalStart, top + inset,
                             horizontalEnd, top + inset));
    }
    if (outline.right) {
        result.append(QLineF(rightExclusive - inset, verticalStart,
                             rightExclusive - inset, verticalEnd));
    }
    if (outline.bottom) {
        result.append(QLineF(horizontalStart, bottomExclusive - inset,
                             horizontalEnd, bottomExclusive - inset));
    }
    if (outline.left) {
        result.append(QLineF(left + inset, verticalStart,
                             left + inset, verticalEnd));
    }
    return result;
}

QRect snapRegionForOutputs(const QRect &globalRegion,
                           const QList<QRect> &outputGeometries)
{
    const QRect region = normalizedNonEmpty(globalRegion);
    if (region.isEmpty() || outputGeometries.isEmpty())
        return region;

    int intersectingOutputs = 0;
    QRect singleOutput;
    for (const QRect &geometry : outputGeometries) {
        const QRect output = normalizedNonEmpty(geometry);
        if (!region.intersects(output))
            continue;
        ++intersectingOutputs;
        singleOutput = output;
    }
    if (intersectingOutputs == 0)
        return region;
    if (intersectingOutputs == 1)
        return region.intersected(singleOutput);

    const int owner = largestSelectionIntersectionIndex(region, outputGeometries);
    return owner >= 0 ? normalizedNonEmpty(outputGeometries.at(owner)) : region;
}

QList<QPoint> handlesForOutput(const QRect &globalSelection,
                               const QRect &outputGeometry)
{
    QList<QPoint> result;
    const QRect selection = normalizedNonEmpty(globalSelection);
    const QRect output = normalizedNonEmpty(outputGeometry);
    if (selection.isEmpty() || output.isEmpty())
        return result;

    const QPoint center = selection.center();
    const QList<QPoint> globalHandles = {
        selection.topLeft(),
        selection.topRight(),
        selection.bottomLeft(),
        selection.bottomRight(),
        QPoint(center.x(), selection.top()),
        QPoint(selection.right(), center.y()),
        QPoint(center.x(), selection.bottom()),
        QPoint(selection.left(), center.y())
    };
    for (const QPoint &globalHandle : globalHandles) {
        if (output.contains(globalHandle))
            result.append(globalHandle - output.topLeft());
    }
    return result;
}

qint64 selectionIntersectionArea(const QRect &globalSelection,
                                 const QRect &outputGeometry)
{
    const QRect intersection = normalizedNonEmpty(globalSelection)
            .intersected(normalizedNonEmpty(outputGeometry));
    if (intersection.isEmpty())
        return 0;
    return static_cast<qint64>(intersection.width()) * intersection.height();
}

int largestSelectionIntersectionIndex(const QRect &globalSelection,
                                      const QList<QRect> &outputGeometries)
{
    int bestIndex = -1;
    qint64 bestArea = -1;
    for (int index = 0; index < outputGeometries.size(); ++index) {
        const qint64 area = selectionIntersectionArea(globalSelection,
                                                       outputGeometries.at(index));
        if (area > bestArea) {
            bestIndex = index;
            bestArea = area;
        }
    }
    return bestIndex;
}

} // namespace Dsr::Screenshot::SelectionOverlayGeometry
