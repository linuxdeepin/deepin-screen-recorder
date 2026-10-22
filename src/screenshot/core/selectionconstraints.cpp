// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectionconstraints.h"

#include <QtMath>

namespace Dsr::Screenshot {

namespace {

QRect enforceMinimumSize(QRect rect, const QSize &minimumSize, const QRect &bounds)
{
    const int minWidth = qMax(1, minimumSize.width());
    const int minHeight = qMax(1, minimumSize.height());

    if (rect.width() < minWidth) {
        rect.setWidth(minWidth);
        if (bounds.isValid() && rect.right() > bounds.right())
            rect.moveRight(bounds.right());
    }
    if (rect.height() < minHeight) {
        rect.setHeight(minHeight);
        if (bounds.isValid() && rect.bottom() > bounds.bottom())
            rect.moveBottom(bounds.bottom());
    }
    return rect;
}

} // namespace

QRect resolveSelectionGeometry(const QRect &candidate, const SelectionConstraints &constraints)
{
    if (candidate.isEmpty())
        return QRect();

    QRect resolved = candidate.normalized();
    const QRect bounds = constraints.desktopBounds;

    if (constraints.keepInsideDesktop && bounds.isValid())
        resolved = resolved.intersected(bounds);
    if (resolved.isEmpty())
        return QRect();

    if (constraints.expandSelectionForAnnotations && !constraints.annotationBounds.isEmpty()) {
        QRect annotation = constraints.annotationBounds.toAlignedRect().normalized();
        if (bounds.isValid())
            annotation = annotation.intersected(bounds);
        if (!annotation.isEmpty())
            resolved = resolved.united(annotation);
    }

    if (bounds.isValid()) {
        resolved = enforceMinimumSize(resolved, constraints.minimumSize, bounds);
        resolved = resolved.intersected(bounds);
    } else {
        resolved = enforceMinimumSize(resolved, constraints.minimumSize, QRect());
    }

    return resolved;
}

} // namespace Dsr::Screenshot
