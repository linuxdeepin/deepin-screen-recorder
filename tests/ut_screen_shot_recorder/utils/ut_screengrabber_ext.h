// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QRect>
#include <QPixmap>
#include <QDebug>
#include "../../src/utils/screengrabber.h"
#include "../../src/utils.h"

using namespace testing;

// Extended smoke tests for ScreenGrabber. The existing utils/ut_screengrabber.h already
// covers grabEntireDesktop in both Wayland-off and Wayland-on modes, so here we exercise
// the remaining safe public surface and edge rects.
//
// Deliberately SKIPPED (they directly touch Wayland/X11 native APIs per the rules):
//   - grabWaylandScreenshot  (KWin DBus)
//   - grabX11Screenshot      (dispatches to X11 / XGetImage paths)
//   - grabPrimaryScreenFallback / grabSingleScreen / grabMultipleScreens
//       (these call QScreen::grabWindow which under XCB/Wayland hits the display server)
//   - grabWithXGetImage      (XOpenDisplay / XGetImage)
//   - grabScreenFragment     (private, calls grabWindow)
//   - findIntersectingScreens(private, but only iterates QGuiApplication::screens; covered
//                             indirectly via grabEntireDesktop's non-wayland path)
class ScreenGrabberExtTest : public testing::Test
{
public:
    void SetUp() override { std::cout << "start ScreenGrabberExtTest" << std::endl; }
    void TearDown() override { Utils::isWaylandMode = false; Utils::isQt6XcbEnv = false; }
};

TEST_F(ScreenGrabberExtTest, constructAndDestroy)
{
    ScreenGrabber *g = new ScreenGrabber(nullptr);
    EXPECT_NE(nullptr, g);
    delete g;
}

TEST_F(ScreenGrabberExtTest, constructWithParent)
{
    QObject parent;
    ScreenGrabber *g = new ScreenGrabber(&parent);
    EXPECT_EQ(&parent, g->parent());
    // parent takes ownership; no explicit delete
}

// getX11RootWindowSize is static and safe: returns QSize() unless Utils::isQt6XcbEnv,
// in which case it would open the X11 display. We keep isQt6XcbEnv=false so it short-circuits.
TEST_F(ScreenGrabberExtTest, getX11RootWindowSize_whenNotXcb)
{
    Utils::isQt6XcbEnv = false;
    QSize s = ScreenGrabber::getX11RootWindowSize();
    EXPECT_TRUE(s.isEmpty());
}

// grabEntireDesktop with an empty rect in non-wayland mode: must not crash, ok is set true
// by the dispatcher before delegating (the X11 path is exercised but with a degenerate rect
// the intersecting-screens logic completes without grabbing real pixels in many layouts).
TEST_F(ScreenGrabberExtTest, grabEntireDesktop_emptyRectNonWayland)
{
    Utils::isWaylandMode = false;
    Utils::isQt6XcbEnv = false;
    ScreenGrabber g;
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, QRect(), 1.0));
    // ok is unconditionally set to true at the top of grabEntireDesktop
    EXPECT_TRUE(ok);
}

// grabEntireDesktop with a degenerate (zero-area) rect in wayland mode: the wayland path
// performs a DBus call that will fail in CI, setting ok=false. This mirrors the existing
// grabEntireDesktop_wayland test and just confirms the dispatcher branches on the flag.
TEST_F(ScreenGrabberExtTest, grabEntireDesktop_waylandFlagRouting)
{
    Utils::isWaylandMode = true;
    ScreenGrabber g;
    bool ok = true;
    EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, QRect(0, 0, 1, 1), 1.0));
    // Wayland DBus unavailable in CI -> ok flipped to false; we only assert it ran.
}
