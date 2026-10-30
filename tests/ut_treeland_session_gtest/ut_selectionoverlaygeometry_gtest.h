// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "screenshot/core/selectionoverlaygeometry.h"

#include <gtest/gtest.h>
#include <QRect>
#include <QLineF>
#include <QPoint>
#include <QList>

using namespace Dsr::Screenshot::SelectionOverlayGeometry;

// ---------------------------------------------------------------------------
// outlineForOutput
// ---------------------------------------------------------------------------

TEST(SelectionOverlayGeometryTest, OutlineForOutput_SelectionFullyInside)
{
    QRect selection(10, 10, 50, 50);
    QRect output(0, 0, 200, 200);
    Outline outline = outlineForOutput(selection, output);

    EXPECT_EQ(outline.localRect, QRect(10, 10, 50, 50));
    EXPECT_TRUE(outline.top);
    EXPECT_TRUE(outline.right);
    EXPECT_TRUE(outline.bottom);
    EXPECT_TRUE(outline.left);
}

TEST(SelectionOverlayGeometryTest, OutlineForOutput_SelectionSpanningTwoOutputs)
{
    QRect selection(150, 10, 100, 50);
    QRect output(0, 0, 200, 200);
    Outline outline = outlineForOutput(selection, output);

    EXPECT_FALSE(outline.localRect.isEmpty());
    EXPECT_EQ(outline.localRect.x(), 150);
    EXPECT_EQ(outline.localRect.width(), 50);
    EXPECT_TRUE(outline.top);
    EXPECT_TRUE(outline.bottom);
    EXPECT_TRUE(outline.left);
    EXPECT_FALSE(outline.right);
}

TEST(SelectionOverlayGeometryTest, OutlineForOutput_EmptySelection)
{
    Outline outline = outlineForOutput(QRect(), QRect(0, 0, 100, 100));
    EXPECT_TRUE(outline.localRect.isEmpty());
    EXPECT_FALSE(outline.top);
    EXPECT_FALSE(outline.right);
    EXPECT_FALSE(outline.bottom);
    EXPECT_FALSE(outline.left);
}

TEST(SelectionOverlayGeometryTest, OutlineForOutput_EmptyOutput)
{
    Outline outline = outlineForOutput(QRect(10, 10, 50, 50), QRect());
    EXPECT_TRUE(outline.localRect.isEmpty());
    EXPECT_FALSE(outline.top);
    EXPECT_FALSE(outline.right);
    EXPECT_FALSE(outline.bottom);
    EXPECT_FALSE(outline.left);
}

TEST(SelectionOverlayGeometryTest, OutlineForOutput_SelectionOutsideOutput)
{
    QRect selection(300, 300, 50, 50);
    QRect output(0, 0, 200, 200);
    Outline outline = outlineForOutput(selection, output);
    EXPECT_TRUE(outline.localRect.isEmpty());
}

TEST(SelectionOverlayGeometryTest, OutlineForOutput_NegativeGeometryNormalized)
{
    QRect selection(60, 60, -50, -50);
    QRect output(0, 0, 200, 200);
    Outline outline = outlineForOutput(selection, output);
    EXPECT_EQ(outline.localRect, QRect(10, 10, 50, 50));
    EXPECT_TRUE(outline.top);
    EXPECT_TRUE(outline.bottom);
    EXPECT_TRUE(outline.left);
    EXPECT_TRUE(outline.right);
}

// ---------------------------------------------------------------------------
// strokedOutlineForOutput
// ---------------------------------------------------------------------------

TEST(SelectionOverlayGeometryTest, StrokedOutline_AllFourEdges)
{
    QRect selection(10, 10, 50, 50);
    QRect output(0, 0, 200, 200);
    qreal strokeWidth = 4.0;
    QList<QLineF> lines = strokedOutlineForOutput(selection, output, strokeWidth);

    EXPECT_EQ(lines.size(), 4);
}

TEST(SelectionOverlayGeometryTest, StrokedOutline_InsetCalculation)
{
    QRect selection(10, 10, 50, 50);
    QRect output(0, 0, 200, 200);
    qreal strokeWidth = 4.0;
    QList<QLineF> lines = strokedOutlineForOutput(selection, output, strokeWidth);

    qreal expectedInset = strokeWidth / 2.0;
    qreal expectedLeft = 10.0 + expectedInset;
    qreal expectedTop = 10.0 + expectedInset;
    qreal expectedRightExclusive = 10 + 50;
    qreal expectedBottomExclusive = 10 + 50;

    bool foundTop = false, foundRight = false, foundBottom = false, foundLeft = false;
    for (const QLineF &line : lines) {
        if (qFuzzyCompare(line.p1().y(), expectedTop) && qFuzzyCompare(line.p2().y(), expectedTop))
            foundTop = true;
        if (qFuzzyCompare(line.p1().x(), expectedRightExclusive - expectedInset) &&
            qFuzzyCompare(line.p2().x(), expectedRightExclusive - expectedInset))
            foundRight = true;
        if (qFuzzyCompare(line.p1().y(), expectedBottomExclusive - expectedInset) &&
            qFuzzyCompare(line.p2().y(), expectedBottomExclusive - expectedInset))
            foundBottom = true;
        if (qFuzzyCompare(line.p1().x(), expectedLeft) &&
            qFuzzyCompare(line.p2().x(), expectedLeft))
            foundLeft = true;
    }
    EXPECT_TRUE(foundTop);
    EXPECT_TRUE(foundRight);
    EXPECT_TRUE(foundBottom);
    EXPECT_TRUE(foundLeft);
}

TEST(SelectionOverlayGeometryTest, StrokedOutline_EmptyOutlineReturnsNoLines)
{
    QList<QLineF> lines = strokedOutlineForOutput(QRect(), QRect(0, 0, 100, 100), 2.0);
    EXPECT_TRUE(lines.isEmpty());
}

TEST(SelectionOverlayGeometryTest, StrokedOutline_SelectionOutsideOutput)
{
    QList<QLineF> lines = strokedOutlineForOutput(
        QRect(500, 500, 50, 50), QRect(0, 0, 100, 100), 2.0);
    EXPECT_TRUE(lines.isEmpty());
}

// ---------------------------------------------------------------------------
// handlesForOutput
// ---------------------------------------------------------------------------

TEST(SelectionOverlayGeometryTest, Handles_AllEightInsideOutput)
{
    QRect selection(10, 10, 50, 50);
    QRect output(0, 0, 200, 200);
    QList<QPoint> handles = handlesForOutput(selection, output);

    EXPECT_EQ(handles.size(), 8);
    for (const QPoint &h : handles) {
        EXPECT_GE(h.x(), 0);
        EXPECT_GE(h.y(), 0);
        EXPECT_LE(h.x(), 200);
        EXPECT_LE(h.y(), 200);
    }
}

TEST(SelectionOverlayGeometryTest, Handles_SomeOutsideOutput)
{
    QRect selection(150, 150, 100, 100);
    QRect output(0, 0, 200, 200);
    QList<QPoint> handles = handlesForOutput(selection, output);

    EXPECT_LT(handles.size(), 8);
    EXPECT_GT(handles.size(), 0);
    for (const QPoint &h : handles) {
        EXPECT_GE(h.x(), 0);
        EXPECT_GE(h.y(), 0);
    }
}

TEST(SelectionOverlayGeometryTest, Handles_EmptySelection)
{
    QList<QPoint> handles = handlesForOutput(QRect(), QRect(0, 0, 100, 100));
    EXPECT_TRUE(handles.isEmpty());
}

TEST(SelectionOverlayGeometryTest, Handles_EmptyOutput)
{
    QList<QPoint> handles = handlesForOutput(QRect(10, 10, 50, 50), QRect());
    EXPECT_TRUE(handles.isEmpty());
}

// ---------------------------------------------------------------------------
// snapRegionForOutputs
// ---------------------------------------------------------------------------

TEST(SelectionOverlayGeometryTest, SnapRegion_NoIntersection)
{
    QRect region(300, 300, 50, 50);
    QList<QRect> outputs = { QRect(0, 0, 200, 200) };
    QRect result = snapRegionForOutputs(region, outputs);
    EXPECT_EQ(result, region);
}

TEST(SelectionOverlayGeometryTest, SnapRegion_SingleIntersection)
{
    QRect region(150, 150, 100, 100);
    QList<QRect> outputs = { QRect(0, 0, 200, 200) };
    QRect result = snapRegionForOutputs(region, outputs);
    EXPECT_EQ(result, region.intersected(outputs.first()));
}

TEST(SelectionOverlayGeometryTest, SnapRegion_MultiOutputLargestWins)
{
    QRect region(0, 0, 300, 100);
    QList<QRect> outputs = {
        QRect(0, 0, 200, 200),
        QRect(200, 0, 200, 200)
    };
    QRect result = snapRegionForOutputs(region, outputs);
    int ownerIndex = largestSelectionIntersectionIndex(region, outputs);
    EXPECT_EQ(result, outputs.at(ownerIndex).normalized());
}

TEST(SelectionOverlayGeometryTest, SnapRegion_EmptyOutputList)
{
    QRect region(10, 10, 50, 50);
    QList<QRect> outputs;
    QRect result = snapRegionForOutputs(region, outputs);
    EXPECT_EQ(result, region);
}

TEST(SelectionOverlayGeometryTest, SnapRegion_EmptyRegion)
{
    QRect region;
    QList<QRect> outputs = { QRect(0, 0, 200, 200) };
    QRect result = snapRegionForOutputs(region, outputs);
    EXPECT_EQ(result, region);
}

// ---------------------------------------------------------------------------
// largestSelectionIntersectionIndex
// ---------------------------------------------------------------------------

TEST(SelectionOverlayGeometryTest, LargestIntersection_MultipleOutputs)
{
    QRect selection(0, 0, 300, 100);
    QList<QRect> outputs = {
        QRect(0, 0, 100, 100),
        QRect(100, 0, 200, 100)
    };
    int index = largestSelectionIntersectionIndex(selection, outputs);
    EXPECT_EQ(index, 1);
}

TEST(SelectionOverlayGeometryTest, LargestIntersection_NoIntersection)
{
    QRect selection(500, 500, 50, 50);
    QList<QRect> outputs = { QRect(0, 0, 100, 100) };
    int index = largestSelectionIntersectionIndex(selection, outputs);
    EXPECT_EQ(index, 0);
}

TEST(SelectionOverlayGeometryTest, LargestIntersection_SingleOutput)
{
    QRect selection(10, 10, 50, 50);
    QList<QRect> outputs = { QRect(0, 0, 200, 200) };
    int index = largestSelectionIntersectionIndex(selection, outputs);
    EXPECT_EQ(index, 0);
}

TEST(SelectionOverlayGeometryTest, LargestIntersection_EmptyList)
{
    QRect selection(10, 10, 50, 50);
    QList<QRect> outputs;
    int index = largestSelectionIntersectionIndex(selection, outputs);
    EXPECT_EQ(index, -1);
}

// ---------------------------------------------------------------------------
// selectionIntersectionArea
// ---------------------------------------------------------------------------

TEST(SelectionOverlayGeometryTest, IntersectionArea_FullOverlap)
{
    QRect selection(0, 0, 100, 100);
    QRect output(0, 0, 100, 100);
    qint64 area = selectionIntersectionArea(selection, output);
    EXPECT_EQ(area, 10000);
}

TEST(SelectionOverlayGeometryTest, IntersectionArea_NoOverlap)
{
    QRect selection(200, 200, 50, 50);
    QRect output(0, 0, 100, 100);
    qint64 area = selectionIntersectionArea(selection, output);
    EXPECT_EQ(area, 0);
}

TEST(SelectionOverlayGeometryTest, IntersectionArea_PartialOverlap)
{
    QRect selection(50, 50, 100, 100);
    QRect output(0, 0, 100, 100);
    qint64 area = selectionIntersectionArea(selection, output);
    EXPECT_EQ(area, 2500);
}

TEST(SelectionOverlayGeometryTest, IntersectionArea_EmptySelection)
{
    qint64 area = selectionIntersectionArea(QRect(), QRect(0, 0, 100, 100));
    EXPECT_EQ(area, 0);
}
