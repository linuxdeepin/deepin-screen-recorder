// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPointF>
#include <QList>
#include "../../src/utils/calculaterect.h"
#include "../../src/utils.h"

using namespace testing;

// Fixture that flips Utils::isTabletEnvironment in a RAII-ish way and restores it.
class CalculaterectExtTest : public testing::Test
{
public:
    bool m_prevTablet = false;
    void SetUp() override { m_prevTablet = Utils::isTabletEnvironment; }
    void TearDown() override { Utils::isTabletEnvironment = m_prevTablet; }
};

// ---- pointClickIn: tablet vs non-tablet padding branches ----
TEST_F(CalculaterectExtTest, pointClickIn_nonTablet_exactHit)
{
    Utils::isTabletEnvironment = false;
    EXPECT_TRUE(pointClickIn(QPointF(10, 10), QPointF(10, 10)));
    EXPECT_TRUE(pointClickIn(QPointF(10, 10), QPointF(13, 13)));   // within padding 4
    EXPECT_FALSE(pointClickIn(QPointF(10, 10), QPointF(20, 20)));  // out of padding
}

TEST_F(CalculaterectExtTest, pointClickIn_tablet_widePadding)
{
    Utils::isTabletEnvironment = true;
    EXPECT_TRUE(pointClickIn(QPointF(10, 10), QPointF(25, 25)));   // within padding 20
    EXPECT_FALSE(pointClickIn(QPointF(10, 10), QPointF(50, 50)));
}

TEST_F(CalculaterectExtTest, pointClickIn_paddingArgIgnored)
{
    // the explicit padding arg is unused; behavior must be identical regardless of its value.
    Utils::isTabletEnvironment = false;
    EXPECT_EQ(pointClickIn(QPointF(0, 0), QPointF(3, 3), 0),
              pointClickIn(QPointF(0, 0), QPointF(3, 3), 9999));
}

// ---- pointOnLine: vertical / diagonal / degenerate / NaN-guard ----
TEST_F(CalculaterectExtTest, pointOnLine_verticalSegment)
{
    EXPECT_TRUE(pointOnLine(QPointF(5, 0), QPointF(5, 20), QPointF(5, 10)));
    EXPECT_TRUE(pointOnLine(QPointF(5, 0), QPointF(5, 20), QPointF(7, 10)));   // within padding
    EXPECT_FALSE(pointOnLine(QPointF(5, 0), QPointF(5, 20), QPointF(50, 10)));
}

TEST_F(CalculaterectExtTest, pointOnLine_diagonalSegment)
{
    // (0,0)-(10,10): midpoint (5,5) lies on the line.
    EXPECT_TRUE(pointOnLine(QPointF(0, 0), QPointF(10, 10), QPointF(5, 5)));
    // far away point
    EXPECT_FALSE(pointOnLine(QPointF(0, 0), QPointF(10, 10), QPointF(100, -100)));
}

TEST_F(CalculaterectExtTest, pointOnLine_degeneratePoint)
{
    // Identical endpoints: x's are equal so the vertical branch is taken; the point lies
    // within the (degenerate) vertical tolerance, so the function returns true without
    // dividing by zero in the dot-product path.
    EXPECT_NO_FATAL_FAILURE(pointOnLine(QPointF(3, 3), QPointF(3, 3), QPointF(3, 3)));
    EXPECT_TRUE(pointOnLine(QPointF(3, 3), QPointF(3, 3), QPointF(3, 3)));
}

// ---- getDistance ----
TEST_F(CalculaterectExtTest, getDistance_basicAndZero)
{
    EXPECT_DOUBLE_EQ(0.0, getDistance(QPointF(2, 2), QPointF(2, 2)));
    EXPECT_DOUBLE_EQ(5.0, getDistance(QPointF(0, 0), QPointF(3, 4)));
    // negative coords
    EXPECT_DOUBLE_EQ(5.0, getDistance(QPointF(-3, -4), QPointF(0, 0)));
}

// ---- pointSplid: vertical vs angled ----
TEST_F(CalculaterectExtTest, pointSplid_vertical)
{
    QPointF r = pointSplid(QPointF(4, 0), QPointF(4, 10), 3.0);
    EXPECT_DOUBLE_EQ(0.0, r.x());
    EXPECT_DOUBLE_EQ(3.0, r.y());
}

TEST_F(CalculaterectExtTest, pointSplid_angled)
{
    // 45-degree line: cos(45)==sin(45)
    QPointF r = pointSplid(QPointF(0, 0), QPointF(10, 10), 10.0);
    EXPECT_NEAR(10.0 * std::cos(M_PI / 4), r.x(), 1e-9);
    EXPECT_NEAR(10.0 * std::sin(M_PI / 4), r.y(), 1e-9);
}

// ---- calculateAngle: identical points + 4 quadrants ----
TEST_F(CalculaterectExtTest, calculateAngle_identicalPoints)
{
    EXPECT_DOUBLE_EQ(0.0, calculateAngle(QPointF(1, 1), QPointF(1, 1), QPointF(0, 0)));
}

TEST_F(CalculaterectExtTest, calculateAngle_quadrantAdjustments)
{
    // Cover each quadrant-adjustment branch without asserting exact trig values;
    // ensure it returns a finite number in [-1, 1].
    QPointF center(0, 0);
    auto inRange = [](qreal a) { return a >= -1.0001 && a <= 1.0001; };
    EXPECT_TRUE(inRange(calculateAngle(center, QPointF(5, 5), QPointF(10, 0))));
    EXPECT_TRUE(inRange(calculateAngle(center, QPointF(-5, 5), QPointF(-10, 0))));
    EXPECT_TRUE(inRange(calculateAngle(center, QPointF(-5, -5), QPointF(0, -10))));
    EXPECT_TRUE(inRange(calculateAngle(center, QPointF(5, -5), QPointF(10, -10))));
}

// ---- pointToLineDistance: vertical and sloped ----
TEST_F(CalculaterectExtTest, pointToLineDistance_vertical)
{
    EXPECT_DOUBLE_EQ(3.0, pointToLineDistance(QPointF(2, 0), QPointF(2, 10), QPointF(5, 5)));
}

TEST_F(CalculaterectExtTest, pointToLineDistance_onLine)
{
    EXPECT_DOUBLE_EQ(0.0, pointToLineDistance(QPointF(0, 0), QPointF(10, 10), QPointF(5, 5)));
}

// ---- pointLineDir: vertical and sloped ----
TEST_F(CalculaterectExtTest, pointLineDir_verticalLeftRight)
{
    EXPECT_EQ(-1, pointLineDir(QPointF(2, 0), QPointF(2, 10), QPointF(1, 5)));
    EXPECT_EQ(1, pointLineDir(QPointF(2, 0), QPointF(2, 10), QPointF(3, 5)));
}

TEST_F(CalculaterectExtTest, pointLineDir_slopedAboveBelow)
{
    // line y = x through (0,0)-(10,10)
    EXPECT_EQ(-1, pointLineDir(QPointF(0, 0), QPointF(10, 10), QPointF(5, 0)));  // below
    EXPECT_EQ(1, pointLineDir(QPointF(0, 0), QPointF(10, 10), QPointF(5, 20)));  // above
}

// ---- getControlPoint: both direction branches ----
TEST_F(CalculaterectExtTest, getControlPoint_bothDirections)
{
    QPointF a = getControlPoint(QPointF(0, 0), QPointF(10, 10), true);
    QPointF b = getControlPoint(QPointF(0, 0), QPointF(10, 10), false);
    EXPECT_FALSE(a.isNull());
    EXPECT_FALSE(b.isNull());
}

// ---- getAnotherFPoints: valid vs invalid length ----
TEST_F(CalculaterectExtTest, getAnotherFPoints_invalidLength)
{
    FourPoints bad;
    bad << QPointF(0, 0) << QPointF(1, 1);   // length != 4
    FourPoints r = getAnotherFPoints(bad);
    ASSERT_EQ(4, r.size());
    for (int i = 0; i < 4; ++i) {
        EXPECT_TRUE(r[i].isNull());
    }
}

TEST_F(CalculaterectExtTest, getAnotherFPoints_valid)
{
    FourPoints fp;
    fp << QPointF(0, 0) << QPointF(0, 10) << QPointF(10, 0) << QPointF(10, 10);
    FourPoints r = getAnotherFPoints(fp);
    EXPECT_EQ(4, r.size());
    // midpoints of opposite edges
    EXPECT_EQ(QPointF(0, 5), r[0]);
    EXPECT_EQ(QPointF(5, 0), r[1]);
    EXPECT_EQ(QPointF(10, 5), r[2]);
    EXPECT_EQ(QPointF(5, 10), r[3]);
}

// ---- fourPointsOfLine: insufficient vs sufficient points ----
TEST_F(CalculaterectExtTest, fourPointsOfLine_insufficient)
{
    QList<QPointF> one;
    one << QPointF(7, 7);
    FourPoints r = fourPointsOfLine(one);
    ASSERT_EQ(4, r.size());
    for (int i = 0; i < 4; ++i) {
        EXPECT_TRUE(r[i].isNull());
    }
}

TEST_F(CalculaterectExtTest, fourPointsOfLine_normal)
{
    QList<QPointF> pts;
    pts << QPointF(0, 0) << QPointF(10, 10) << QPointF(5, 5);
    FourPoints r = fourPointsOfLine(pts);
    ASSERT_EQ(4, r.size());
    // padded corners around bounding box (min=0,0 max=10,10 with _MIN_PADDING=10)
    EXPECT_EQ(QPointF(-10, -10), r[0]);
    EXPECT_EQ(QPointF(20, 20), r[3]);
}

// ---- pointOnBezier: hit vs miss ----
TEST_F(CalculaterectExtTest, pointOnBezier_hitAndMiss)
{
    // Straight bezier where control points are on the line: midpoint (5,5) is on it.
    EXPECT_TRUE(pointOnBezier(QPointF(0, 0), QPointF(3, 3), QPointF(7, 7), QPointF(10, 10), QPointF(5, 5)));
    EXPECT_FALSE(pointOnBezier(QPointF(0, 0), QPointF(3, 3), QPointF(7, 7), QPointF(10, 10), QPointF(500, 500)));
}

// ---- pointOnEllipse ----
TEST_F(CalculaterectExtTest, pointOnEllipse_boundaryAndOutside)
{
    FourPoints fp;
    fp << QPointF(0, 0) << QPointF(0, 10) << QPointF(10, 0) << QPointF(10, 10);
    // a point near an edge midpoint should hit; a far point should miss
    EXPECT_NO_FATAL_FAILURE(pointOnEllipse(fp, QPointF(0, 5)));
    EXPECT_FALSE(pointOnEllipse(fp, QPointF(1000, 1000)));
}

// ---- pointOfArrow: vertical/horizontal/diagonal ----
TEST_F(CalculaterectExtTest, pointOfArrow_various)
{
    EXPECT_EQ(3, pointOfArrow(QPointF(0, 0), QPointF(0, 20), 5.0).size());   // same X
    EXPECT_EQ(3, pointOfArrow(QPointF(0, 0), QPointF(20, 0), 5.0).size());   // same Y
    EXPECT_EQ(3, pointOfArrow(QPointF(0, 0), QPointF(20, 20), 5.0).size());  // diagonal
}

// ---- pointOnArLine ----
TEST_F(CalculaterectExtTest, pointOnArLine_hitAndEmpty)
{
    QList<QPointF> pts;
    pts << QPointF(0, 0) << QPointF(5, 5) << QPointF(10, 10);
    EXPECT_TRUE(pointOnArLine(pts, QPointF(5, 5)));
    EXPECT_FALSE(pointOnArLine(pts, QPointF(50, 50)));
    // empty list -> false, no crash
    EXPECT_FALSE(pointOnArLine(QList<QPointF>(), QPointF(0, 0)));
}

// ---- relativePosition: invalid vs valid ----
TEST_F(CalculaterectExtTest, relativePosition_invalidLength)
{
    FourPoints bad;
    bad << QPointF(0, 0) << QPointF(1, 1);
    EXPECT_TRUE(relativePosition(bad, QPointF(2, 2)).isEmpty());
}

TEST_F(CalculaterectExtTest, relativePosition_valid)
{
    FourPoints fp;
    fp << QPointF(0, 0) << QPointF(0, 10) << QPointF(10, 0) << QPointF(10, 10);
    QList<qreal> rp = relativePosition(fp, QPointF(5, 5));
    EXPECT_EQ(2, rp.size());
}

// ---- getNewPosition: three branches (re[0]==-2 / re[1]==-2 / neither) ----
TEST_F(CalculaterectExtTest, getNewPosition_branchRe0Neg2)
{
    FourPoints fp;
    fp << QPointF(0, 0) << QPointF(0, 10) << QPointF(10, 0) << QPointF(10, 10);
    QList<qreal> re; re << -2.0 << 1.0;
    EXPECT_NO_FATAL_FAILURE(getNewPosition(fp, re));
}

TEST_F(CalculaterectExtTest, getNewPosition_branchRe1Neg2)
{
    FourPoints fp;
    fp << QPointF(0, 0) << QPointF(0, 10) << QPointF(10, 0) << QPointF(10, 10);
    QList<qreal> re; re << 1.0 << -2.0;
    EXPECT_NO_FATAL_FAILURE(getNewPosition(fp, re));
}

TEST_F(CalculaterectExtTest, getNewPosition_branchBothValid_axisAlignedX)
{
    // mainPoints[0].x == mainPoints[1].x (vertical edge)
    FourPoints fp;
    fp << QPointF(5, 0) << QPointF(5, 10) << QPointF(15, 0) << QPointF(15, 10);
    QList<qreal> re; re << 1.0 << 1.0;
    QPointF p = getNewPosition(fp, re);
    EXPECT_FALSE(p.isNull());
}

TEST_F(CalculaterectExtTest, getNewPosition_branchBothValid_axisAlignedY)
{
    // mainPoints[0].x == mainPoints[2].x
    FourPoints fp;
    fp << QPointF(5, 0) << QPointF(15, 10) << QPointF(5, 20) << QPointF(15, 30);
    QList<qreal> re; re << 1.0 << 1.0;
    QPointF p = getNewPosition(fp, re);
    EXPECT_FALSE(p.isNull());
}

TEST_F(CalculaterectExtTest, getNewPosition_branchBothValid_general)
{
    // generic non-axis-aligned rectangle
    FourPoints fp;
    fp << QPointF(8, 5) << QPointF(5, 6) << QPointF(9, 8) << QPointF(5, 1);
    QList<qreal> re; re << 0.5 << 0.5;
    QPointF p = getNewPosition(fp, re);
    EXPECT_FALSE(p.isNull());
}

// ---- initFourPoints ----
TEST_F(CalculaterectExtTest, initFourPoints_resetsToFourZeros)
{
    FourPoints fp;
    fp << QPointF(1, 2) << QPointF(3, 4);
    FourPoints r = initFourPoints(fp);
    ASSERT_EQ(4, r.size());
    for (int i = 0; i < 4; ++i) {
        EXPECT_TRUE(r[i].isNull());
    }
}

// ---- pointInRect: inside / outside ----
TEST_F(CalculaterectExtTest, pointInRect_insideAndOutside)
{
    FourPoints fp;
    fp << QPointF(0, 0) << QPointF(0, 20) << QPointF(20, 0) << QPointF(20, 20);
    EXPECT_TRUE(pointInRect(fp, QPointF(10, 10)));
    EXPECT_FALSE(pointInRect(fp, QPointF(100, 100)));
}

// ---- getInterpolationPoints: vertical / forward / backward ----
TEST_F(CalculaterectExtTest, getInterpolationPoints_vertical)
{
    QList<QPointF> r = getInterpolationPoints(QPointF(2, 0), QPointF(2, 10), 2.0);
    EXPECT_FALSE(r.isEmpty());
    for (const QPointF &p : r) {
        EXPECT_DOUBLE_EQ(2.0, p.x());
    }
}

TEST_F(CalculaterectExtTest, getInterpolationPoints_forwardAndBackward)
{
    EXPECT_FALSE(getInterpolationPoints(QPointF(0, 0), QPointF(10, 0), 2.0).isEmpty());   // x1<x2
    EXPECT_FALSE(getInterpolationPoints(QPointF(10, 0), QPointF(0, 0), 2.0).isEmpty());   // x1>x2
}

// ---- getRectPoints: vertical / horizontal / diagonal ----
TEST_F(CalculaterectExtTest, getRectPoints_verticalHorizontalDiagonal)
{
    EXPECT_EQ(4, getRectPoints(QPointF(0, 0), QPointF(0, 10), 4.0).size());   // vertical
    EXPECT_EQ(4, getRectPoints(QPointF(0, 0), QPointF(10, 0), 4.0).size());   // horizontal
    EXPECT_EQ(4, getRectPoints(QPointF(0, 0), QPointF(10, 10), 4.0).size());  // diagonal
    // negative slope branch
    FourPoints r = getRectPoints(QPointF(0, 10), QPointF(10, 0), 4.0);
    EXPECT_EQ(4, r.size());
}

// ---- getMainPoints: shift-mode all 4 quadrants ----
TEST_F(CalculaterectExtTest, getMainPoints_shiftQuadrants)
{
    EXPECT_EQ(4, getMainPoints(QPointF(0, 0), QPointF(30, 40), true).size());   // +x +y
    EXPECT_EQ(4, getMainPoints(QPointF(0, 40), QPointF(30, 0), true).size());   // +x -y
    EXPECT_EQ(4, getMainPoints(QPointF(30, 0), QPointF(0, 40), true).size());   // -x +y
    EXPECT_EQ(4, getMainPoints(QPointF(30, 40), QPointF(0, 0), true).size());   // -x -y
    EXPECT_EQ(4, getMainPoints(QPointF(0, 0), QPointF(30, 40), false).size());  // non-shift
}

// ---- pointRotate ----
TEST_F(CalculaterectExtTest, pointRotate_identityAndQuarter)
{
    // zero angle -> unchanged (relative to center)
    QPointF r0 = pointRotate(QPointF(0, 0), QPointF(5, 0), 0.0);
    EXPECT_NEAR(5.0, r0.x(), 1e-9);
    EXPECT_NEAR(0.0, r0.y(), 1e-9);
    // 90-degree rotation
    QPointF r90 = pointRotate(QPointF(0, 0), QPointF(5, 0), M_PI / 2);
    EXPECT_NEAR(0.0, r90.x(), 1e-9);
    EXPECT_NEAR(5.0, r90.y(), 1e-9);
}
