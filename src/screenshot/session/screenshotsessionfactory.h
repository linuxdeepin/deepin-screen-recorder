// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHOT_SESSION_FACTORY_H
#define SCREENSHOT_SESSION_FACTORY_H

#include <QObject>
#include <QString>

namespace Dsr::Screenshot {

class ScreenshotSessionController;

class ScreenshotSessionFactory
{
public:
    static bool isCurrentPlatformSupported();
    static ScreenshotSessionController *createForCurrentPlatform(QObject *parent,
                                                                 QString *error = nullptr);
};

} // namespace Dsr::Screenshot

#endif // SCREENSHOT_SESSION_FACTORY_H
