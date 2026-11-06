// SPDX-FileCopyrightText: 2026 svan71
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandtoolbarplacement.h"

#include <algorithm>
#include <limits>

namespace TreelandToolBarPlacement {

namespace {

bool toolbarFitsAt(const QPoint &topLeft,
                   const QSize &toolBarSize,
                   const QRect &screenGeom)
{
    if (toolBarSize.width() <= 0 || toolBarSize.height() <= 0 || screenGeom.isEmpty()) {
        return false;
    }
    const QRect bar(topLeft, toolBarSize);
    return screenGeom.contains(bar);
}

} // namespace

QRect pickScreenGeometry(const QRect &captureRegion,
                         const QList<QRect> &screens,
                         const QRect &fallback)
{
    if (screens.isEmpty()) {
        return fallback;
    }

    const QPoint center = captureRegion.center();
    for (const QRect &screen : screens) {
        if (!screen.isEmpty() && screen.contains(center)) {
            return screen;
        }
    }

    QRect best;
    int bestArea = -1;
    for (const QRect &screen : screens) {
        if (screen.isEmpty()) {
            continue;
        }
        const QRect inter = screen.intersected(captureRegion);
        const int area = inter.isEmpty() ? 0 : inter.width() * inter.height();
        if (area > bestArea) {
            bestArea = area;
            best = screen;
        }
    }

    if (bestArea > 0 && !best.isEmpty()) {
        return best;
    }

    // No intersection: pick the screen whose center is nearest the capture center.
    best = screens.first();
    qint64 bestDist = std::numeric_limits<qint64>::max();
    for (const QRect &screen : screens) {
        if (screen.isEmpty()) {
            continue;
        }
        const QPoint sc = screen.center();
        const qint64 dx = qint64(sc.x()) - center.x();
        const qint64 dy = qint64(sc.y()) - center.y();
        const qint64 dist = dx * dx + dy * dy;
        if (dist < bestDist) {
            bestDist = dist;
            best = screen;
        }
    }
    return best.isEmpty() ? fallback : best;
}

QPoint clampToScreen(const QPoint &topLeft,
                     const QSize &toolBarSize,
                     const QRect &screenGeom)
{
    if (screenGeom.isEmpty() || toolBarSize.width() <= 0 || toolBarSize.height() <= 0) {
        return topLeft;
    }

    const int maxX = screenGeom.x() + std::max(0, screenGeom.width() - toolBarSize.width());
    const int maxY = screenGeom.y() + std::max(0, screenGeom.height() - toolBarSize.height());
    const int x = std::clamp(topLeft.x(), screenGeom.x(), maxX);
    const int y = std::clamp(topLeft.y(), screenGeom.y(), maxY);
    return QPoint(x, y);
}

QPoint placeToolBar(const QRect &captureRegion,
                    const QSize &toolBarSize,
                    const QRect &screenGeom,
                    int ySpacing)
{
    if (screenGeom.isEmpty() || toolBarSize.width() <= 0 || toolBarSize.height() <= 0) {
        return clampToScreen(captureRegion.topLeft(), toolBarSize, screenGeom);
    }

    // Right-align to the capture region (same preference as updateToolBarPos on X11).
    const int preferredX = captureRegion.x() + captureRegion.width() - toolBarSize.width();
    const int x = clampToScreen(QPoint(preferredX, screenGeom.y()), toolBarSize, screenGeom).x();

    const int spacing = std::max(0, ySpacing);

    // Prefer outside below the selection.
    const int yBelow = captureRegion.y() + captureRegion.height() + spacing;
    if (toolbarFitsAt(QPoint(x, yBelow), toolBarSize, screenGeom)) {
        return QPoint(x, yBelow);
    }

    // Else outside above the selection.
    const int yAbove = captureRegion.y() - toolBarSize.height() - spacing;
    if (toolbarFitsAt(QPoint(x, yAbove), toolBarSize, screenGeom)) {
        return QPoint(x, yAbove);
    }

    // Neither outside slot fits: place inside the capture near its top, then clamp.
    const int yInside = captureRegion.y() + spacing;
    return clampToScreen(QPoint(x, yInside), toolBarSize, screenGeom);
}

} // namespace TreelandToolBarPlacement
