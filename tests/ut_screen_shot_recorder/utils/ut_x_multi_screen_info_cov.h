// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <gtest/gtest.h>
#include "../../src/utils/x_multi_screen_info.h"

using namespace testing;

// Coverage tests for XMultiScreenInfo.
//
// The class has exactly one meaningful method, screenNeedResetScale(), and it
// is a thin wrapper around raw X11/Xinerama calls:
//     XOpenDisplay -> XineramaQueryExtension -> XineramaIsActive ->
//     XineramaQueryScreens -> XCloseDisplay
//
// Per the task rules, pure X11/Wayland/hardware methods must NOT be invoked
// from unit tests (they would either fail to find a display under offscreen Qt
// or touch the real display server). It is therefore SKIPPED here.
//
// The only safe line is the constructor, which just emits a debug log. We
// exercise it so the file is not 0% covered.
class XMultiScreenInfoCovTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(XMultiScreenInfoCovTest, constructorRunsClean)
{
    EXPECT_NO_FATAL_FAILURE(XMultiScreenInfo info;);
}

TEST_F(XMultiScreenInfoCovTest, constructOnHeapAndDelete)
{
    XMultiScreenInfo *info = nullptr;
    EXPECT_NO_FATAL_FAILURE(info = new XMultiScreenInfo(); delete info;);
}

// NOTE: screenNeedResetScale() is intentionally NOT tested. It performs direct
// X11/Xinerama calls (XOpenDisplay, XineramaQueryScreens) which violate the
// "no X11/Wayland/hardware" rule and would misbehave under the offscreen
// platform. There is no safe, hardware-free path through that function.
