// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAPTURELAYOUT_H
#define CAPTURELAYOUT_H

#include <QRect>
#include <QSize>
#include <QtMath>

namespace CaptureLayout {

// Map edges, rather than rounding widths independently, to keep shared edges identical.
inline QRect mapRect(const QRect &logicalRect, const QPoint &origin, qreal scale)
{
    const int left = qRound((logicalRect.x() - origin.x()) * scale);
    const int top = qRound((logicalRect.y() - origin.y()) * scale);
    const int right = qRound((logicalRect.x() + logicalRect.width() - origin.x()) * scale);
    const int bottom = qRound((logicalRect.y() + logicalRect.height() - origin.y()) * scale);
    return QRect(left, top, right - left, bottom - top);
}

}

#endif
