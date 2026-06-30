// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QPointF>
#include <QList>
#include "../../src/utils/shapesutils.h"
#include "../../src/widgets/shapeswidget.h"

using namespace testing;

// 这些 are 纯几何命中检测辅助函数（clickedOn*/hoverOn*/rotateOnPoint 等），
// 入参为 FourPoints / QList<QPointF> / QPointF，返回 bool，可直接在 ShapesWidget 实例上调用。
// FourPoints 约定：[0]=TopLeft [1]=BottomLeft [2]=TopRight [3]=BottomRight。
// getAnotherFPoints 返回 4 个边中点：[0]=Left [1]=Top [2]=Right [3]=Bottom。
// pointClickIn 默认 padding=4，故 pos==控制点 必然命中。

class ShapesWidgetGeoTest : public Test
{
public:
    ShapesWidget *m_w;
    void SetUp() override { m_w = new ShapesWidget; }
    void TearDown() override { delete m_w; }

    // 100x100 的矩形：TL(10,10) BL(10,110) TR(110,10) BR(110,110)
    static FourPoints rectPoints()
    {
        FourPoints fp;
        fp << QPointF(10, 10) << QPointF(10, 110) << QPointF(110, 10) << QPointF(110, 110);
        return fp;
    }
    // 边中点：Left(10,60) Top(60,10) Right(110,60) Bottom(60,110)
    static QList<QPointF> cornerPositions()
    {
        return {QPointF(10, 10), QPointF(10, 110), QPointF(110, 10), QPointF(110, 110)};
    }
    static QList<QPointF> midPositions()
    {
        return {QPointF(10, 60), QPointF(60, 10), QPointF(110, 60), QPointF(60, 110)};
    }
};

// ---------------- clickedOnRect ----------------
TEST_F(ShapesWidgetGeoTest, clickedOnRect_corners)
{
    FourPoints fp = rectPoints();
    for (const auto &p : cornerPositions()) {
        EXPECT_TRUE(m_w->clickedOnRect(fp, p));
    }
}

TEST_F(ShapesWidgetGeoTest, clickedOnRect_mids)
{
    FourPoints fp = rectPoints();
    for (const auto &p : midPositions()) {
        EXPECT_TRUE(m_w->clickedOnRect(fp, p));
    }
}

TEST_F(ShapesWidgetGeoTest, clickedOnRect_edgeAndBlurAndMiss)
{
    FourPoints fp = rectPoints();
    // 顶边上非角/中点 -> pointOnLine 分支
    EXPECT_TRUE(m_w->clickedOnRect(fp, QPointF(40, 10)));
    // 内部 + isBlurMosaic -> pointInRect 分支
    EXPECT_TRUE(m_w->clickedOnRect(fp, QPointF(60, 60), true));
    // 内部 + 非 blur -> 未命中
    EXPECT_FALSE(m_w->clickedOnRect(fp, QPointF(60, 60), false));
    // 远处 -> 未命中
    EXPECT_FALSE(m_w->clickedOnRect(fp, QPointF(1000, 1000)));
}

// ---------------- clickedOnEllipse ----------------
TEST_F(ShapesWidgetGeoTest, clickedOnEllipse_cornersAndMids)
{
    FourPoints fp = rectPoints();
    for (const auto &p : cornerPositions()) {
        EXPECT_TRUE(m_w->clickedOnEllipse(fp, p));
    }
    for (const auto &p : midPositions()) {
        EXPECT_TRUE(m_w->clickedOnEllipse(fp, p));
    }
}

TEST_F(ShapesWidgetGeoTest, clickedOnEllipse_edgeBlurMiss)
{
    FourPoints fp = rectPoints();
    // 椭圆边界附近：用角点本身已在上一用例命中 pointOnEllipse，这里测 blur/miss
    EXPECT_TRUE(m_w->clickedOnEllipse(fp, QPointF(60, 60), true));   // 内部 blur
    EXPECT_FALSE(m_w->clickedOnEllipse(fp, QPointF(60, 60), false)); // 内部非 blur（不在椭圆边界则未命中）
    EXPECT_FALSE(m_w->clickedOnEllipse(fp, QPointF(1000, 1000)));    // 远处
}

// ---------------- clickedOnArrow ----------------
TEST_F(ShapesWidgetGeoTest, clickedOnArrow_branches)
{
    QList<QPointF> pts;
    pts << QPointF(10, 10) << QPointF(110, 110);
    // length != 2 提前返回 false
    EXPECT_FALSE(m_w->clickedOnArrow(QList<QPointF>{QPointF(0, 0)}, QPointF(0, 0)));
    EXPECT_FALSE(m_w->clickedOnArrow(QList<QPointF>{QPointF(0, 0), QPointF(1, 1), QPointF(2, 2)}, QPointF(1, 1)));
    // 起点 / 终点 / 线上 / 未命中
    EXPECT_TRUE(m_w->clickedOnArrow(pts, QPointF(10, 10)));    // point0
    EXPECT_TRUE(m_w->clickedOnArrow(pts, QPointF(110, 110)));  // point1
    EXPECT_TRUE(m_w->clickedOnArrow(pts, QPointF(60, 60)));    // 线上（y=x）
    EXPECT_FALSE(m_w->clickedOnArrow(pts, QPointF(1000, 1000))); // 未命中
}

// ---------------- clickedOnLine ----------------
TEST_F(ShapesWidgetGeoTest, clickedOnLine_cornersAndMids)
{
    FourPoints fp = rectPoints();
    QList<QPointF> linePts;
    linePts << QPointF(10, 10) << QPointF(110, 110);
    for (const auto &p : cornerPositions()) {
        EXPECT_TRUE(m_w->clickedOnLine(fp, linePts, p));
    }
    for (const auto &p : midPositions()) {
        EXPECT_TRUE(m_w->clickedOnLine(fp, linePts, p));
    }
    // 未命中
    EXPECT_FALSE(m_w->clickedOnLine(fp, linePts, QPointF(1000, 1000)));
}

// ---------------- clickedOnText ----------------
TEST_F(ShapesWidgetGeoTest, clickedOnText_insideOutside)
{
    FourPoints fp = rectPoints();
    EXPECT_TRUE(m_w->clickedOnText(fp, QPointF(60, 60)));        // 内部
    EXPECT_FALSE(m_w->clickedOnText(fp, QPointF(1000, 1000)));   // 外部
}

// ---------------- hover 系列 ----------------
TEST_F(ShapesWidgetGeoTest, hoverOnRect_cornersMidsEdgeMiss)
{
    FourPoints fp = rectPoints();
    for (const auto &p : cornerPositions()) {
        EXPECT_TRUE(m_w->hoverOnRect(fp, p));
    }
    for (const auto &p : midPositions()) {
        EXPECT_TRUE(m_w->hoverOnRect(fp, p));
    }
    EXPECT_TRUE(m_w->hoverOnRect(fp, QPointF(40, 10)));          // 顶边
    EXPECT_FALSE(m_w->hoverOnRect(fp, QPointF(1000, 1000)));     // 未命中
}

TEST_F(ShapesWidgetGeoTest, hoverOnEllipse_cornersMidsMiss)
{
    FourPoints fp = rectPoints();
    for (const auto &p : cornerPositions()) {
        EXPECT_TRUE(m_w->hoverOnEllipse(fp, p));
    }
    for (const auto &p : midPositions()) {
        EXPECT_TRUE(m_w->hoverOnEllipse(fp, p));
    }
    EXPECT_FALSE(m_w->hoverOnEllipse(fp, QPointF(1000, 1000)));
}

TEST_F(ShapesWidgetGeoTest, hoverOnArrow_branches)
{
    QList<QPointF> pts;
    pts << QPointF(10, 10) << QPointF(110, 110);
    EXPECT_FALSE(m_w->hoverOnArrow(QList<QPointF>{QPointF(0, 0)}, QPointF(0, 0))); // length!=2
    EXPECT_TRUE(m_w->hoverOnArrow(pts, QPointF(60, 60)));   // 线上、远离端点
    EXPECT_FALSE(m_w->hoverOnArrow(pts, QPointF(1000, 1000))); // 未命中
}

TEST_F(ShapesWidgetGeoTest, hoverOnLine_cornersMidsMiss)
{
    FourPoints fp = rectPoints();
    QList<QPointF> linePts;
    linePts << QPointF(10, 10) << QPointF(110, 110);
    for (const auto &p : cornerPositions()) {
        EXPECT_TRUE(m_w->hoverOnLine(fp, linePts, p));
    }
    for (const auto &p : midPositions()) {
        EXPECT_TRUE(m_w->hoverOnLine(fp, linePts, p));
    }
    EXPECT_FALSE(m_w->hoverOnLine(fp, linePts, QPointF(1000, 1000)));
}

TEST_F(ShapesWidgetGeoTest, hoverOnText_branches)
{
    FourPoints fp = rectPoints();
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnText(0, fp, QPointF(60, 60)));   // 内部
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnText(0, fp, QPointF(1000, 1000))); // 外部
}

TEST_F(ShapesWidgetGeoTest, hoverOnShapes_defaultToolshape)
{
    Toolshape ts;
    ts.mainPoints = rectPoints();
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnShapes(ts, QPointF(60, 60)));
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnShapes(ts, QPointF(1000, 1000)));
}

// ---------------- 旋转点 / 杂项 ----------------
TEST_F(ShapesWidgetGeoTest, rotateAndHoverRotatePoint)
{
    FourPoints fp = rectPoints();
    EXPECT_NO_FATAL_FAILURE(m_w->rotateOnPoint(fp, QPointF(60, -20)));
    EXPECT_NO_FATAL_FAILURE(m_w->rotateOnPoint(fp, QPointF(1000, 1000)));
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnRotatePoint(fp, QPointF(60, -20)));
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnRotatePoint(fp, QPointF(1000, 1000)));
}

TEST_F(ShapesWidgetGeoTest, clickedShapesAndAccessors)
{
    FourPoints fp = rectPoints();
    EXPECT_NO_FATAL_FAILURE(m_w->clickedOnShapes(QPointF(60, 60)));
    EXPECT_NO_FATAL_FAILURE(m_w->textEditIsReadOnly());
    EXPECT_NO_FATAL_FAILURE(m_w->isExistsText());
}
