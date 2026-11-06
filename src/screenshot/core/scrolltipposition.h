// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCROLL_TIP_POSITION_H
#define SCROLL_TIP_POSITION_H

#include <QPoint>
#include <QRect>
#include <QSize>

namespace Dsr::Screenshot {

// Mirrors MainWindow::getScrollShotTipPosition():
//  - selections smaller than 300x100 center the prompt on the screen;
//  - otherwise the prompt is centered horizontally on the selection;
//  - a toolbar inside the selection puts the prompt below it;
//  - a toolbar below the selection puts the prompt above it;
//  - a toolbar above the selection puts the prompt below it.
inline QPoint scrollTipPosition(const QSize &surfaceSize,
                                const QRect &globalSelection,
                                const QRect &outputGeometry,
                                const QRect &toolbarRect,
                                const QSize &tipSize,
                                qreal pixelScale = 1.0)
{
    if (surfaceSize.isEmpty() || tipSize.isEmpty())
        return {};

    const qreal scale = pixelScale > 0 ? pixelScale : 1.0;

    const QRect selection = globalSelection.normalized();
    const QRect localSelection = selection.translated(
            -outputGeometry.normalized().topLeft());

    int x = 0;
    int y = 0;
    // X11 compares the physical-pixel size against 300x100.
    if (localSelection.width() * scale < 300
            || localSelection.height() * scale < 100) {
        x = (surfaceSize.width() - tipSize.width()) / 2;
        y = (surfaceSize.height() - tipSize.height()) / 2;
    } else {
        x = localSelection.x() + (localSelection.width() - tipSize.width()) / 2;
        const bool toolbarInsideSelection = !toolbarRect.isEmpty()
                && localSelection.top() <= toolbarRect.top()
                && localSelection.bottom() >= toolbarRect.bottom();
        if (toolbarInsideSelection) {
            y = toolbarRect.bottom() + 1 + 15;
        } else if (toolbarRect.y() > localSelection.y()) {
            y = toolbarRect.y() - tipSize.height() - 15;
        } else {
            y = toolbarRect.bottom() + 1 + 15;
        }
    }

    x = qBound(0, x, qMax(0, surfaceSize.width() - tipSize.width()));
    y = qBound(0, y, qMax(0, surfaceSize.height() - tipSize.height()));
    return QPoint(x, y);
}

} // namespace Dsr::Screenshot

#endif // SCROLL_TIP_POSITION_H
