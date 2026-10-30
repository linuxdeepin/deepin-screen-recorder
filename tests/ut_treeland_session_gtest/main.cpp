// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <gtest/gtest.h>

#include "ut_selectionoverlaygeometry_gtest.h"
#include "ut_screenshotsessioncontroller_gtest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
