// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHOT_PLATFORMDETECTION_H
#define SCREENSHOT_PLATFORMDETECTION_H

#include <QByteArray>
#include <QProcessEnvironment>
#include <QString>

namespace Dsr::Screenshot {

inline bool isTreeLandEnvironment()
{
    const QString compositor = QProcessEnvironment::systemEnvironment()
            .value(QStringLiteral("DDE_CURRENT_COMPOSITOR"))
            .trimmed();
    if (compositor.compare(QStringLiteral("TreeLand"), Qt::CaseInsensitive) == 0)
        return true;

    // Explicit demo opt-in. Never infer TreeLand from "the session is
    // Wayland": KWin Wayland also matches that and must keep the legacy path.
    return qEnvironmentVariableIntValue("DSR_TREELAND_DEMO") == 1;
}

} // namespace Dsr::Screenshot

#endif // SCREENSHOT_PLATFORMDETECTION_H
