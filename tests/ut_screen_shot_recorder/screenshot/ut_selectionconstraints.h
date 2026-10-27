// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include "../../../src/screenshot/core/selectionconstraints.h"

using namespace Dsr::Screenshot;
using namespace testing;

class SelectionConstraintsTest : public Test
{
protected:
    SelectionConstraints constraints;
    void SetUp() override
    {
        constraints.desktopBounds = QRect(0, 0, 1920, 1080);
        constraints.minimumSize = QSize(8, 8);
    }
};

TEST_F(SelectionConstraintsTest, EmptyCandidateReturnsEmpty)
{
    QRect result = resolveSelectionGeometry(QRect(), constraints);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(SelectionConstraintsTest, ValidCandidateWithinBounds)
{
    QRect candidate(100, 100, 200, 200);
    QRect result = resolveSelectionGeometry(candidate, constraints);
    EXPECT_EQ(result, QRect(100, 100, 200, 200));
}

TEST_F(SelectionConstraintsTest, CandidateClampedToDesktop)
{
    QRect candidate(1800, 1000, 300, 200);
    QRect result = resolveSelectionGeometry(candidate, constraints);
    EXPECT_EQ(result, QRect(1800, 1000, 120, 80));
}

TEST_F(SelectionConstraintsTest, NegativeOriginClamped)
{
    QRect candidate(-100, -100, 300, 200);
    QRect result = resolveSelectionGeometry(candidate, constraints);
    EXPECT_EQ(result, QRect(0, 0, 200, 100));
}

TEST_F(SelectionConstraintsTest, BelowMinimumSizeEnlarged)
{
    QRect candidate(100, 100, 3, 3);
    QRect result = resolveSelectionGeometry(candidate, constraints);
    EXPECT_GE(result.width(), 8);
    EXPECT_GE(result.height(), 8);
}

TEST_F(SelectionConstraintsTest, ExpandForAnnotations)
{
    constraints.annotationBounds = QRectF(50, 50, 100, 100);
    QRect candidate(200, 200, 100, 100);
    QRect result = resolveSelectionGeometry(candidate, constraints);
    EXPECT_TRUE(result.contains(QRect(50, 50, 100, 100)));
    EXPECT_TRUE(result.contains(QRect(200, 200, 100, 100)));
}

TEST_F(SelectionConstraintsTest, NoExpandWhenDisabled)
{
    constraints.expandSelectionForAnnotations = false;
    constraints.annotationBounds = QRectF(50, 50, 100, 100);
    QRect candidate(200, 200, 100, 100);
    QRect result = resolveSelectionGeometry(candidate, constraints);
    EXPECT_EQ(result, QRect(200, 200, 100, 100));
}

TEST_F(SelectionConstraintsTest, NoClampWhenDesktopInvalid)
{
    constraints.desktopBounds = QRect();
    QRect candidate(100, 100, 200, 200);
    QRect result = resolveSelectionGeometry(candidate, constraints);
    EXPECT_EQ(result, QRect(100, 100, 200, 200));
}

TEST_F(SelectionConstraintsTest, NoClampWhenKeepInsideDisabled)
{
    constraints.keepInsideDesktop = false;
    QRect candidate(1800, 1000, 300, 200);
    QRect result = resolveSelectionGeometry(candidate, constraints);
    // Source always intersects with desktop bounds at the end:
    // QRect(1800,1000,300,200) ∩ QRect(0,0,1920,1080) = QRect(1800,1000,120,80)
    EXPECT_EQ(result, QRect(1800, 1000, 120, 80));
}

TEST_F(SelectionConstraintsTest, FullyOutsideDesktopReturnsEmpty)
{
    QRect candidate(5000, 5000, 100, 100);
    QRect result = resolveSelectionGeometry(candidate, constraints);
    EXPECT_TRUE(result.isEmpty());
}
