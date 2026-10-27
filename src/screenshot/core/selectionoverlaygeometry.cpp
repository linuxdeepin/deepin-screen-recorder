// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectionoverlaygeometry.h"

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
