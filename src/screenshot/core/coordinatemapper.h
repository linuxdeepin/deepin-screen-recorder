// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COORDINATE_MAPPER_H
#define COORDINATE_MAPPER_H

#include "screenshottypes.h"

#include <QPoint>
#include <QRect>

namespace Dsr::Screenshot::CoordinateMapper {

inline QPoint globalToOutputLocal(const QPoint &globalPos, const OutputDescriptor &output)
{
    return globalPos - output.logicalGeometry.topLeft();
}

inline QPoint outputLocalToGlobal(const QPoint &localPos, const OutputDescriptor &output)
{
    return localPos + output.logicalGeometry.topLeft();
}

inline QRect globalToOutputLocal(const QRect &globalRect, const OutputDescriptor &output)
{
    return globalRect.translated(-output.logicalGeometry.topLeft());
}

inline QRect outputLocalToGlobal(const QRect &localRect, const OutputDescriptor &output)
{
    return localRect.translated(output.logicalGeometry.topLeft());
}

inline QRect globalToFramePixel(const QRect &globalRect,
                                const OutputDescriptor &output,
                                qreal pixelScale)
{
    const QRect local = globalToOutputLocal(globalRect, output);
    const int left = qFloor(local.left() * pixelScale);
    const int top = qFloor(local.top() * pixelScale);
    const int right = qCeil((local.right() + 1) * pixelScale);
    const int bottom = qCeil((local.bottom() + 1) * pixelScale);
    return QRect(left, top, right - left, bottom - top);
}

} // namespace Dsr::Screenshot::CoordinateMapper

#endif // COORDINATE_MAPPER_H
