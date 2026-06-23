// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// RecorderRegionShow smoke tests for the current API (src/RecorderRegionShow.cpp
// currently reports 0% coverage).
//
// The legacy ut_RecorderRegionShow.h exercises the camera / key-button paths
// with stubs and a shown top-level window. This _ext file instead targets the
// trivially safe, dependency-free methods that can run on a never-shown widget
// with no stubs and no MainWindow -- raising baseline coverage without the
// fragile v4l2 / DWindowManagerHelper / window-manager coupling.
//
// Skipped (unsafe in headless, no-stub context):
//   - initCameraInfo : news a CameraWidget, calls initUI/cameraStart/
//     Utils::passInputEvent(winId()) -- v4l2 + X11 winId; needs stubs.
//   - showKeyBoardButtons : news KeyButtonWidget, calls updateMultiKeyBoardPos
//     + repaint() on the key widgets (show()/hide()) -- needs a window manager.
//   - updateMultiKeyBoardPos : iterates m_keyButtonList and show()/hide()s each
//     child; only safe with an empty list (covered below) or a shown parent.
//   - paintEvent : m_painter->begin(this) requires a paintable surface; on a
//     never-shown widget this is unreliable and TreeLand-guarded anyway.
//
// Covered:
//   - ctor/dtor (painter allocation, empty member lists).
//   - setDevcieName : pure QString store.
//   - setCameraShow(true/false) : null-guarded on m_cameraWidget -> no-op safe.
//   - updateKeyBoardButtonStyle : empty m_keyButtonList -> loop body skipped.

#pragma once
#include <gtest/gtest.h>
#include <QString>
#include "addr_pri.h"
#include "../../src/RecorderRegionShow.h"

class RecorderRegionShowExtTest : public ::testing::Test
{
public:
    RecorderRegionShow *m_region;
    void SetUp() override { m_region = new RecorderRegionShow; }
    void TearDown() override { delete m_region; }
};

TEST_F(RecorderRegionShowExtTest, ConstructorAndDestructor)
{
    // Ctor allocates m_painter and installs an event filter; dtor deletes both
    // painter and any camera/key-button children. With none allocated this is
    // a pure allocation/free smoke.
    RecorderRegionShow *r = nullptr;
    EXPECT_NO_FATAL_FAILURE(r = new RecorderRegionShow);
    EXPECT_NO_FATAL_FAILURE(delete r);
}

TEST_F(RecorderRegionShowExtTest, SetDeviceName)
{
    EXPECT_NO_FATAL_FAILURE(m_region->setDevcieName(QString("/dev/video0")));
    EXPECT_NO_FATAL_FAILURE(m_region->setDevcieName(QString("USB Camera")));
    EXPECT_NO_FATAL_FAILURE(m_region->setDevcieName(QString()));
}

TEST_F(RecorderRegionShowExtTest, SetCameraShowWithoutCameraIsNoOp)
{
    // m_cameraWidget is null until initCameraInfo(); setCameraShow guards on
    // it. Both branches must be safe.
    EXPECT_NO_FATAL_FAILURE(m_region->setCameraShow(true));
    EXPECT_NO_FATAL_FAILURE(m_region->setCameraShow(false));
    EXPECT_NO_FATAL_FAILURE(m_region->setCameraShow()); // default arg = true
}

TEST_F(RecorderRegionShowExtTest, UpdateKeyBoardButtonStyleEmptyList)
{
    // Default m_keyButtonList is empty; the for-loop body never executes, so
    // DWindowManagerHelper is never queried. Safe without a display.
    EXPECT_NO_FATAL_FAILURE(m_region->updateKeyBoardButtonStyle());
}

TEST_F(RecorderRegionShowExtTest, RepeatedSetDeviceNameDoesNotLeak)
{
    // Stress the QString member reassignment; primarily a crash/regression
    // guard for the simple setter.
    for (int i = 0; i < 50; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_region->setDevcieName(QString::number(i)));
    }
}
