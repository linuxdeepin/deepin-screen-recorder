// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUT_REGION_GEOMETRY_H
#define INPUT_REGION_GEOMETRY_H

#include "coordinatemapper.h"

#include <QList>
#include <QRect>
#include <QRegion>

namespace Dsr::Screenshot {

// Computes the input region for a full-output mask surface.  The selection is
// removed so pointer input reaches the window below, while interactive UI
// rectangles are added back so the user can still operate the overlay.
inline QRegion scrollInputRegion(const QRect &surfaceRect,
                                 const OutputDescriptor &output,
                                 const QRect &globalSelection,
                                 const QList<QRect> &globalInteractiveRects)
{
    if (surfaceRect.isEmpty())
        return {};

    QRegion region(surfaceRect);
    const QRect localSelection =
            CoordinateMapper::globalToOutputLocal(globalSelection, output)
                    .intersected(surfaceRect);
    if (!localSelection.isEmpty())
        region -= QRegion(localSelection);

    for (const QRect &globalRect : globalInteractiveRects) {
        const QRect local =
                CoordinateMapper::globalToOutputLocal(globalRect, output)
                        .intersected(surfaceRect);
        if (!local.isEmpty())
            region += QRegion(local);
    }
    return region;
}

} // namespace Dsr::Screenshot

#endif // INPUT_REGION_GEOMETRY_H
