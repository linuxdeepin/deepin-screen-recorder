// SPDX-FileCopyrightText: 2026 svan71
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>

#include <QList>
#include <QPoint>
#include <QRect>
#include <QSize>

#include "../../../src/utils/treelandtoolbarplacement.h"

using namespace testing;
using TreelandToolBarPlacement::clampToScreen;
using TreelandToolBarPlacement::pickScreenGeometry;
using TreelandToolBarPlacement::placeToolBar;

namespace {

const QSize kBar(400, 68);
const int kGap = 5;

bool fullyOnScreen(const QPoint &pos, const QSize &size, const QRect &screen)
{
    return screen.contains(QRect(pos, size));
}

bool outsideCapture(const QPoint &pos, const QSize &size, const QRect &capture)
{
    return !QRect(pos, size).intersects(capture);
}

} // namespace

class TreelandToolBarPlacementTest : public testing::Test
{
};

// Top-right selection on a 1920x1080 primary: toolbar must sit fully on-screen,
// prefer below when that fits, and not hang past the right/top edges.
TEST_F(TreelandToolBarPlacementTest, topRightSelectionClampsAndPrefersBelow)
{
    const QRect screen(0, 0, 1920, 1080);
    const QRect capture(1700, 40, 180, 120); // near top-right
    const QPoint pos = placeToolBar(capture, kBar, screen, kGap);

    EXPECT_TRUE(fullyOnScreen(pos, kBar, screen));
    // Right-aligned to capture: 1700+180-400=1480; fully on-screen (not past 1520).
    EXPECT_EQ(pos.x(), capture.x() + capture.width() - kBar.width());
    EXPECT_LE(pos.x() + kBar.width(), screen.x() + screen.width());
    // Below fits (40+120+5+68 < 1080)
    EXPECT_EQ(pos.y(), capture.y() + capture.height() + kGap);
    EXPECT_TRUE(outsideCapture(pos, kBar, capture));
}

// Narrow strip against the right edge: X clamped; full width still on screen.
TEST_F(TreelandToolBarPlacementTest, rightEdgeNarrowSelection)
{
    const QRect screen(0, 0, 1920, 1080);
    const QRect capture(1880, 400, 40, 80);
    const QPoint pos = placeToolBar(capture, kBar, screen, kGap);

    EXPECT_TRUE(fullyOnScreen(pos, kBar, screen));
    EXPECT_EQ(pos.x(), 1920 - kBar.width());
    EXPECT_EQ(pos.y(), capture.y() + capture.height() + kGap);
    EXPECT_TRUE(outsideCapture(pos, kBar, capture));
}

// Bottom-right: below does not fit → place above when it fits.
TEST_F(TreelandToolBarPlacementTest, bottomRightPrefersAbove)
{
    const QRect screen(0, 0, 1920, 1080);
    const QRect capture(1600, 980, 280, 90);
    const QPoint pos = placeToolBar(capture, kBar, screen, kGap);

    EXPECT_TRUE(fullyOnScreen(pos, kBar, screen));
    EXPECT_EQ(pos.y(), capture.y() - kBar.height() - kGap);
    EXPECT_TRUE(outsideCapture(pos, kBar, capture));
    EXPECT_LE(pos.x() + kBar.width(), screen.x() + screen.width());
}

// Flush against the top edge with no room above: below preferred when available.
TEST_F(TreelandToolBarPlacementTest, topEdgePlacesBelow)
{
    const QRect screen(0, 0, 1920, 1080);
    const QRect capture(200, 0, 300, 100);
    const QPoint pos = placeToolBar(capture, kBar, screen, kGap);

    EXPECT_TRUE(fullyOnScreen(pos, kBar, screen));
    EXPECT_EQ(pos.y(), capture.y() + capture.height() + kGap);
    EXPECT_TRUE(outsideCapture(pos, kBar, capture));
    // Right-aligned to capture, not overflowing left.
    EXPECT_EQ(pos.x(), capture.x() + capture.width() - kBar.width());
}

// Selection wider than toolbar: right-align keeps bar inside capture's right edge.
TEST_F(TreelandToolBarPlacementTest, selectionWiderThanToolbarRightAligns)
{
    const QRect screen(0, 0, 1920, 1080);
    const QRect capture(100, 200, 900, 300);
    const QPoint pos = placeToolBar(capture, kBar, screen, kGap);

    EXPECT_TRUE(fullyOnScreen(pos, kBar, screen));
    EXPECT_EQ(pos.x(), capture.x() + capture.width() - kBar.width());
    EXPECT_EQ(pos.y(), capture.y() + capture.height() + kGap);
    EXPECT_TRUE(outsideCapture(pos, kBar, capture));
}

// Selection narrower than toolbar: X may extend left of capture, still on screen.
TEST_F(TreelandToolBarPlacementTest, selectionNarrowerThanToolbarStillOnScreen)
{
    const QRect screen(0, 0, 1920, 1080);
    const QRect capture(500, 300, 80, 60);
    const QPoint pos = placeToolBar(capture, kBar, screen, kGap);

    EXPECT_TRUE(fullyOnScreen(pos, kBar, screen));
    EXPECT_EQ(pos.x(), capture.x() + capture.width() - kBar.width());
    EXPECT_LT(pos.x(), capture.x());
    EXPECT_TRUE(outsideCapture(pos, kBar, capture));
}

// Multi-screen with non-zero origin: pick screen containing capture center.
TEST_F(TreelandToolBarPlacementTest, multiScreenNonZeroOrigin)
{
    const QRect left(0, 0, 1920, 1080);
    const QRect right(1920, 0, 2560, 1440);
    const QList<QRect> screens{left, right};
    const QRect capture(1920 + 2300, 20, 200, 100); // top-right of second screen

    const QRect picked = pickScreenGeometry(capture, screens, left);
    EXPECT_EQ(picked, right);

    const QPoint pos = placeToolBar(capture, kBar, picked, kGap);
    EXPECT_TRUE(fullyOnScreen(pos, kBar, right));
    EXPECT_FALSE(left.intersects(QRect(pos, kBar))); // must not spill onto left screen
    EXPECT_EQ(pos.x(), capture.x() + capture.width() - kBar.width());
    EXPECT_GE(pos.x(), right.x());
    EXPECT_LE(pos.x() + kBar.width(), right.x() + right.width());
    EXPECT_EQ(pos.y(), capture.y() + capture.height() + kGap);
    EXPECT_TRUE(outsideCapture(pos, kBar, capture));
}

// Neither above nor below fits (capture nearly full screen height): place inside and clamp.
TEST_F(TreelandToolBarPlacementTest, fullHeightCapturePlacesInsideClamped)
{
    const QRect screen(0, 0, 1920, 1080);
    const QRect capture(100, 10, 800, 1060);
    const QPoint pos = placeToolBar(capture, kBar, screen, kGap);

    EXPECT_TRUE(fullyOnScreen(pos, kBar, screen));
    // Inside candidate is capture.y()+gap; must remain on screen.
    EXPECT_GE(pos.y(), screen.y());
    EXPECT_LE(pos.y() + kBar.height(), screen.y() + screen.height());
}

// clampToScreen must pull a stale edge position back fully on-screen.
TEST_F(TreelandToolBarPlacementTest, clampRestoredEdgePosition)
{
    const QRect screen(0, 0, 1920, 1080);
    const QPoint stale(1900, -20); // past right and top
    const QPoint clamped = clampToScreen(stale, kBar, screen);

    EXPECT_TRUE(fullyOnScreen(clamped, kBar, screen));
    EXPECT_EQ(clamped.x(), 1920 - kBar.width());
    EXPECT_EQ(clamped.y(), 0);
}

// Screen taller secondary monitor origin below primary (vertical stack).
TEST_F(TreelandToolBarPlacementTest, verticalStackScreenOrigin)
{
    const QRect top(0, 0, 1920, 1080);
    const QRect bottom(0, 1080, 1920, 1200);
    const QList<QRect> screens{top, bottom};
    const QRect capture(1500, 1080 + 50, 300, 100);

    const QRect picked = pickScreenGeometry(capture, screens, top);
    EXPECT_EQ(picked, bottom);

    const QPoint pos = placeToolBar(capture, kBar, picked, kGap);
    EXPECT_TRUE(fullyOnScreen(pos, kBar, bottom));
    EXPECT_EQ(pos.y(), capture.y() + capture.height() + kGap);
    EXPECT_TRUE(outsideCapture(pos, kBar, capture));
}
