// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_SCROLL_COMPOSITOR_H
#define TREELAND_SCROLL_COMPOSITOR_H

#include "../../core/screenshottypes.h"

#include <QImage>
#include <QList>
#include <QRect>

namespace Dsr::Screenshot {

/**
 * Composes every output intersecting @p globalSelection into one image whose
 * size matches the selection at the canvas scale.
 *
 * The canvas scale is the device pixel ratio of the output covering the
 * largest part of the selection, so mixed-DPR layouts keep a single pixel
 * grid. Outputs are placed by their logical geometry; negative virtual-desktop
 * origins are handled by translating each overlap.
 *
 * Returns a null image when the inputs are inconsistent or a crop is empty.
 */
QImage composeScrollSelectionFrame(const QList<OutputDescriptor> &outputs,
                                   const QList<QImage> &images,
                                   const QRect &globalSelection);

} // namespace Dsr::Screenshot

#endif // TREELAND_SCROLL_COMPOSITOR_H
