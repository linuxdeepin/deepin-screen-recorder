// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PUTILS_H
#define PUTILS_H

#include <QObject>

class PUtils : public QObject
{
public:
    static bool isWaylandMode;
};

#endif  // PUTILS_H
