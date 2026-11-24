// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_PIN_PLACEMENT_H
#define TREELAND_PIN_PLACEMENT_H

#include <memory>

class QObject;
class QPoint;
class QWindow;

namespace Dsr::PinScreenshots {

// Keep the returned object alive for the lifetime of the pin window.
std::unique_ptr<QObject> requestTreelandPinPlacement(QWindow *window, const QPoint &position);

} // namespace Dsr::PinScreenshots

#endif // TREELAND_PIN_PLACEMENT_H
