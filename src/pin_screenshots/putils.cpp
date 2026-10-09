// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "putils.h"

#include <DGuiApplicationHelper>

bool PUtils::isWaylandMode = false;
bool PUtils::isTreelandMode = false;

bool PUtils::isDarkTheme()
{
    return Dtk::Gui::DGuiApplicationHelper::DarkType == Dtk::Gui::DGuiApplicationHelper::instance()->themeType();
}

QColor PUtils::themeColor(const QColor &light, const QColor &dark)
{
    return isDarkTheme() ? dark : light;
}
