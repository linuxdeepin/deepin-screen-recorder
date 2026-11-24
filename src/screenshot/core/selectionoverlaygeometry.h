// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SELECTION_OVERLAY_GEOMETRY_H
#define SELECTION_OVERLAY_GEOMETRY_H

#include <QLineF>
#include <QList>
#include <QPoint>
#include <QRect>

namespace Dsr::Screenshot::SelectionOverlayGeometry {

struct Outline {
    QRect localRect;
    bool top = false;
    bool right = false;
    bool bottom = false;
    bool left = false;
};

// Returns the part of a selection outline owned by one output. Border segments
// that lie between two adjacent outputs are intentionally omitted, so a
// cross-output selection never gets a false internal edge.
Outline outlineForOutput(const QRect &globalSelection,
                         const QRect &outputGeometry);

// Returns the visible border segments in output-local coordinates. The stroke
// is positioned inside the selection on owned outer edges, so a 2 px border on
// a screen-bottom edge occupies the last two pixel rows instead of ending one
// pixel above the screen.
QList<QLineF> strokedOutlineForOutput(const QRect &globalSelection,
                                      const QRect &outputGeometry,
                                      qreal strokeWidth);

// Returns the 1 px dashed frame used by the X11 screenshot/scroll selection.
// The frame surrounds the capture region and sits just outside it, matching
// X11's drawRect(recordX - 1, recordY - 1, ...). Screen edges are clamped so a
// full-screen selection still shows a border on every side.
QList<QLineF> dashedFrameForOutput(const QRect &globalSelection,
                                   const QRect &outputGeometry);

// Resolves a compositor snap target against the output layout. Treeland
// reports one global rectangle for a window that crosses outputs. Keep the
// window's visible bounds by intersecting it with every output it covers and
// uniting those intersections. This preserves cross-output window snapping
// while excluding shadows or decorations outside the output layout.
QRect snapRegionForOutputs(const QRect &globalRegion,
                           const QList<QRect> &outputGeometries);

// Returns handle centers in output-local coordinates. Handles outside this
// output are omitted; shared output boundaries therefore do not duplicate
// handles on both screens.
QList<QPoint> handlesForOutput(const QRect &globalSelection,
                               const QRect &outputGeometry);

qint64 selectionIntersectionArea(const QRect &globalSelection,
                                 const QRect &outputGeometry);

// Chooses exactly one owner for a selection-wide UI element. Empty selections
// fall back to the first output; otherwise the output with the largest
// intersection wins, with the first output winning ties.
int largestSelectionIntersectionIndex(const QRect &globalSelection,
                                      const QList<QRect> &outputGeometries);

} // namespace Dsr::Screenshot::SelectionOverlayGeometry

#endif // SELECTION_OVERLAY_GEOMETRY_H
