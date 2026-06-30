// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <gtest/gtest.h>
#include "../../src/utils/screengrabber.h"
#include "../../src/utils.h"
#include "addr_pri.h"

using namespace testing;

// Coverage tests for ScreenGrabber. The existing ut_screengrabber.h and
// ut_screengrabber_ext.h cover grabEntireDesktop (both flags), findIntersecting
// screens, grabPrimaryScreenFallback, grabSingleScreen, and the static
// getX11RootWindowSize short-circuit.
//
// Here we cover the multi-screen dispatch path and the grabMultipleScreens
// geometry/mapping logic. On the offscreen Qt platform screen->grabWindow(0)
// returns a null pixmap; grabMultipleScreens handles null fragments with a
// `continue`, so it completes safely. Everything is wrapped in
// EXPECT_NO_FATAL_FAILURE for robustness.
//
// DELIBERATELY SKIPPED (pure hardware / native calls):
//   - grabWaylandScreenshot    (KWin DBus + temp file)
//   - grabWithXGetImage        (XOpenDisplay / XGetImage)
//   - getX11RootWindowSize with isQt6XcbEnv=true (XOpenDisplay)
//   - grabScreenFragment       (declared but not defined in the .cpp)
class ScreenGrabberCovTest : public testing::Test
{
public:
    void SetUp() override { Utils::isWaylandMode = false; Utils::isQt6XcbEnv = false; }
    void TearDown() override { Utils::isWaylandMode = false; Utils::isQt6XcbEnv = false; }
};

// Non-wayland, non-xcb dispatcher delegates to grabX11Screenshot -> findIntersectingScreens.
// With a wide rect at origin it intersects all offscreen screens and routes to
// grabMultipleScreens; with a far-off rect it routes to grabPrimaryScreenFallback.
TEST_F(ScreenGrabberCovTest, grabEntireDesktopWideRectDispatchesToMultiple)
{
    ScreenGrabber g;
    bool ok = false;
    // A huge rect should intersect every screen -> multi-screen path.
    EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, QRect(-50000, -50000, 100000, 100000), 1.0));
    EXPECT_TRUE(ok);
}

TEST_F(ScreenGrabberCovTest, grabEntireDesktopOriginRect)
{
    ScreenGrabber g;
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, QRect(0, 0, 100, 100), 1.0));
    EXPECT_TRUE(ok);
}

// === grabMultipleScreens via private accessor ===
ACCESS_PRIVATE_FUN(ScreenGrabber, QPixmap(bool &, const QRect &, const QList<QScreen*> &, const qreal), grabMultipleScreens);

// Empty screen list: returns a black canvas sized by rect; layout analysis is
// skipped (screens.size() <= 1). Safe and side-effect-free.
TEST_F(ScreenGrabberCovTest, grabMultipleScreensEmptyList)
{
    ScreenGrabber g;
    bool ok = false;
    QPixmap pm;
    EXPECT_NO_FATAL_FAILURE(pm = call_private_fun::ScreenGrabbergrabMultipleScreens(
        g, ok, QRect(0, 0, 64, 48), QList<QScreen*>(), 1.0));
    EXPECT_TRUE(ok);
    EXPECT_FALSE(pm.isNull());
    EXPECT_EQ(QSize(64, 48), pm.size());
}

// Single-screen list: still skips layout analysis, produces a canvas.
TEST_F(ScreenGrabberCovTest, grabMultipleScreensSingleScreen)
{
    ScreenGrabber g;
    QScreen *primary = QGuiApplication::primaryScreen();
    ASSERT_NE(nullptr, primary);
    bool ok = false;
    QPixmap pm;
    EXPECT_NO_FATAL_FAILURE(pm = call_private_fun::ScreenGrabbergrabMultipleScreens(
        g, ok, QRect(0, 0, 32, 32), QList<QScreen*>{primary}, 1.0));
    EXPECT_TRUE(ok);
}

// Multi-screen list with clone-mode geometry (identical geometries) exercises
// the isCloneMode branch of the layout analysis. The offscreen platform
// typically reports a single screen; if it reports multiple, this still runs.
TEST_F(ScreenGrabberCovTest, grabMultipleScreensAllScreensClonePath)
{
    ScreenGrabber g;
    QList<QScreen*> all = QGuiApplication::screens();
    if (all.size() < 2) {
        GTEST_SKIP() << "offscreen host has fewer than 2 screens; clone path not exercisable";
    }
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ScreenGrabbergrabMultipleScreens(
        g, ok, QRect(0, 0, 50, 50), all, 1.0));
}

// Degenerate rect (zero size) in non-wayland mode: must not crash.
TEST_F(ScreenGrabberCovTest, grabEntireDesktopZeroSizeRect)
{
    ScreenGrabber g;
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, QRect(0, 0, 0, 0), 2.0));
}

// High DPR value exercises the devicePixelRatio scaling in grabMultipleScreens.
TEST_F(ScreenGrabberCovTest, grabMultipleScreensHighDPR)
{
    ScreenGrabber g;
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ScreenGrabbergrabMultipleScreens(
        g, ok, QRect(0, 0, 100, 100), QList<QScreen*>(), 2.5));
}
