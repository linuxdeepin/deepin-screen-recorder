// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PUTILS_H
#define UT_PUTILS_H
#include <gtest/gtest.h>
#include "putils.h"

TEST_F(PinScreenShotsTest, putilsDefaultValues)
{
    EXPECT_FALSE(PUtils::isWaylandMode);
    EXPECT_FALSE(PUtils::isTreelandMode);
}

TEST_F(PinScreenShotsTest, putilsToggleWaylandMode)
{
    PUtils::isWaylandMode = true;
    EXPECT_TRUE(PUtils::isWaylandMode);
    PUtils::isWaylandMode = false;
    EXPECT_FALSE(PUtils::isWaylandMode);
}

TEST_F(PinScreenShotsTest, putilsToggleTreelandMode)
{
    PUtils::isTreelandMode = true;
    EXPECT_TRUE(PUtils::isTreelandMode);
    PUtils::isTreelandMode = false;
    EXPECT_FALSE(PUtils::isTreelandMode);
}

#endif // UT_PUTILS_H
