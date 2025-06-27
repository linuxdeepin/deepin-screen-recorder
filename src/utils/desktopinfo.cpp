// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopinfo.h"

#include <QProcessEnvironment>

DesktopInfo::DesktopInfo()
{
    qCDebug(dsrApp) << "DesktopInfo constructor called.";
    auto e = QProcessEnvironment::systemEnvironment();
    XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    qCDebug(dsrApp) << "XDG_SESSION_TYPE: " << XDG_SESSION_TYPE << ", WAYLAND_DISPLAY: " << WAYLAND_DISPLAY;
}

bool DesktopInfo::waylandDectected()
{
    qCDebug(dsrApp) << "Checking if Wayland is detected.";
    bool detected = XDG_SESSION_TYPE == QLatin1String("wayland") ||
           WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
    qCDebug(dsrApp) << "Wayland detected: " << detected;
    return detected;
}
