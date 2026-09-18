// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PUTILS_H
#define PUTILS_H

#include <QObject>

class PUtils : public QObject
{
public:
    static bool isWaylandMode;
    /** TreeLand 合成器（Wayland 会话下 DDE_CURRENT_COMPOSITOR=TreeLand） */
    static bool isTreelandMode;
};

#endif  // PUTILS_H
