// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SELECTION_CONSTRAINTS_H
#define SELECTION_CONSTRAINTS_H

#include <QRect>
#include <QRectF>
#include <QSize>

namespace Dsr::Screenshot {

struct SelectionConstraints {
    QRect desktopBounds;
    QRectF annotationBounds;
    QSize minimumSize = QSize(8, 8);
    bool keepInsideDesktop = true;
    bool expandSelectionForAnnotations = true;
};

QRect resolveSelectionGeometry(const QRect &candidate,
                               const SelectionConstraints &constraints);

} // namespace Dsr::Screenshot

#endif // SELECTION_CONSTRAINTS_H
