// SPDX-FileCopyrightText: 2026 svan71
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELANDTOOLBARPLACEMENT_H
#define TREELANDTOOLBARPLACEMENT_H

#include <QList>
#include <QPoint>
#include <QRect>
#include <QSize>

/**
 * Pure geometry helpers for Treeland screenshot toolbar placement.
 *
 * Coordinates are logical pixels in the same space as the Treeland capture
 * region and the MainWindow overlay (typically virtual-desktop origin).
 * Kept free of compositor / widget state so unit tests can exercise edge cases.
 */
namespace TreelandToolBarPlacement {

/** Default gap between capture rect and outside toolbar placement. */
inline constexpr int kDefaultYSpacing = 5;

/**
 * Pick the screen that should own the toolbar for @p captureRegion.
 * Prefers the screen containing the region center; falls back to the screen
 * with the largest intersection area; finally @p fallback.
 */
QRect pickScreenGeometry(const QRect &captureRegion,
                         const QList<QRect> &screens,
                         const QRect &fallback);

/**
 * Clamp a top-left toolbar position so the full toolbar rectangle stays inside
 * @p screenGeom. If the toolbar is larger than the screen on an axis, pin that
 * axis to the screen origin (best-effort visibility).
 */
QPoint clampToScreen(const QPoint &topLeft,
                     const QSize &toolBarSize,
                     const QRect &screenGeom);

/**
 * Deterministic Treeland screenshot toolbar placement:
 *  - horizontal: right-align to the capture rect (legacy X11 behavior), then clamp;
 *  - vertical: prefer outside below with @p ySpacing when the full toolbar fits
 *    on the chosen screen; else outside above when it fits; else inside the
 *    capture (top + spacing) and clamp.
 * Always returns a position where the full toolbar rect is inside @p screenGeom
 * whenever toolBarSize fits on that screen.
 */
QPoint placeToolBar(const QRect &captureRegion,
                    const QSize &toolBarSize,
                    const QRect &screenGeom,
                    int ySpacing = kDefaultYSpacing);

} // namespace TreelandToolBarPlacement

#endif // TREELANDTOOLBARPLACEMENT_H
