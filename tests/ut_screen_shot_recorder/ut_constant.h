// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include "../../src/constant.h"

using namespace testing;

// constant.cpp: pure static constant definitions — 100% testable.
class ConstantTest : public Test
{
};

TEST_F(ConstantTest, PaddingValue)
{
    EXPECT_EQ(Constant::RECTANGLE_PADDING, 24);
}

TEST_F(ConstantTest, RadiusValue)
{
    EXPECT_EQ(Constant::RECTANGLE_RADIUS, 8);
}

TEST_F(ConstantTest, FontSizeValue)
{
    EXPECT_EQ(Constant::RECTANGLE_FONT_SIZE, 11);
}

TEST_F(ConstantTest, AllConstantsArePositive)
{
    EXPECT_GT(Constant::RECTANGLE_PADDING, 0);
    EXPECT_GT(Constant::RECTANGLE_RADIUS, 0);
    EXPECT_GT(Constant::RECTANGLE_FONT_SIZE, 0);
}
