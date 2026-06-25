// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include "../../src/utils/shapesutils.h"

using namespace testing;

// shapesutils.cpp: Toolshape constructor and field initialization
class ShapesUtilsExtTest : public Test
{
};

TEST_F(ShapesUtilsExtTest, ToolshapeDefaultConstructor)
{
    Toolshape ts;
    // Default constructor sets 4 mainPoints to (0,0) and clears portion
    EXPECT_EQ(ts.mainPoints.size(), 4);
    for (const QPointF &p : ts.mainPoints) {
        EXPECT_EQ(p.x(), 0);
        EXPECT_EQ(p.y(), 0);
    }
    EXPECT_TRUE(ts.portion.isEmpty());
}

TEST_F(ShapesUtilsExtTest, ToolshapeFieldReadWrite)
{
    Toolshape ts;
    ts.type = "rectangle";
    ts.index = 5;
    ts.lineWidth = 3;
    ts.colorIndex = 2;
    ts.isBlur = true;
    ts.isOval = 1;
    ts.fontSize = 14;
    ts.radius = 20;

    EXPECT_EQ(ts.type, "rectangle");
    EXPECT_EQ(ts.index, 5);
    EXPECT_EQ(ts.lineWidth, 3);
    EXPECT_EQ(ts.colorIndex, 2);
    EXPECT_TRUE(ts.isBlur);
    EXPECT_EQ(ts.isOval, 1);
    EXPECT_EQ(ts.fontSize, 14);
    EXPECT_EQ(ts.radius, 20);
}

TEST_F(ShapesUtilsExtTest, ToolshapeMainPointsModified)
{
    Toolshape ts;
    ts.mainPoints[0] = QPointF(10, 20);
    ts.mainPoints[1] = QPointF(30, 40);
    ts.mainPoints[2] = QPointF(50, 60);
    ts.mainPoints[3] = QPointF(70, 80);

    EXPECT_EQ(ts.mainPoints[0], QPointF(10, 20));
    EXPECT_EQ(ts.mainPoints[3], QPointF(70, 80));
}

TEST_F(ShapesUtilsExtTest, ToolshapePointsList)
{
    Toolshape ts;
    ts.points = {QPointF(1, 1), QPointF(2, 2), QPointF(3, 3)};
    EXPECT_EQ(ts.points.size(), 3);
}

TEST_F(ShapesUtilsExtTest, ToolshapePortionCleared)
{
    Toolshape ts;
    // portion is QList<QList<qreal>>; cleared in constructor
    ts.portion.append({1.0, 2.0});
    EXPECT_FALSE(ts.portion.isEmpty());
    ts.portion.clear();
    EXPECT_TRUE(ts.portion.isEmpty());
}

TEST_F(ShapesUtilsExtTest, ToolshapeDefaultValues)
{
    Toolshape ts;
    EXPECT_EQ(ts.index, -1);
    EXPECT_EQ(ts.lineWidth, 1);
    EXPECT_EQ(ts.colorIndex, 0);
    EXPECT_FALSE(ts.isBlur);
    EXPECT_EQ(ts.isOval, 0);
    EXPECT_FALSE(ts.isShiftPressed);
    EXPECT_EQ(ts.fontSize, 1);
    EXPECT_EQ(ts.radius, 10);
}

TEST_F(ShapesUtilsExtTest, MultipleToolshapes)
{
    Toolshape ts1, ts2;
    ts1.type = "rect";
    ts2.type = "arrow";
    EXPECT_NE(ts1.type, ts2.type);
}

TEST_F(ShapesUtilsExtTest, FourPointsTypedef)
{
    FourPoints fp = {QPointF(0, 0), QPointF(100, 0), QPointF(100, 100), QPointF(0, 100)};
    EXPECT_EQ(fp.size(), 4);
}
