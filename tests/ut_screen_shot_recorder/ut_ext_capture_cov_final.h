// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include "addr_pri.h"
#include <wayland-client.h>
#include "../../src/ext-image-capture/session/extcapturesession.h"
#include "../../src/ext-image-capture/frame/extcaptureframe.h"
#include "../../src/ext-image-capture/manager/extcapturemanager.h"
#include "../../src/ext-image-capture/manager/extoutputsourcemanager.h"

using namespace testing;

ACCESS_PRIVATE_FUN(ExtCaptureFrame, bool(), createBuffer);
ACCESS_PRIVATE_FUN(ExtCaptureFrame, bool(), createShmBuffer);
ACCESS_PRIVATE_FUN(ExtCaptureFrame, bool(), createDmaBuffer);
ACCESS_PRIVATE_FUN(ExtCaptureManager, void(wl_registry *, int, int), bind);
ACCESS_PRIVATE_FUN(ExtOutputSourceManager, void(wl_registry *, int, int), bind);

class ExtCaptureCovFinalTest : public Test
{
};

TEST_F(ExtCaptureCovFinalTest, sessionInitializeStubReturnsFalse)
{
    ExtCaptureSession sess;
    EXPECT_FALSE(sess.initialize(nullptr, nullptr, false));
    EXPECT_FALSE(sess.initialize(reinterpret_cast<void *>(1), reinterpret_cast<void *>(2), true));
    EXPECT_EQ(sess.state(), ExtCaptureSession::Uninitialized);
}

TEST_F(ExtCaptureCovFinalTest, frameCreateBufferStubReturnsFalse)
{
    ExtCaptureFrame frame;
    EXPECT_FALSE(call_private_fun::ExtCaptureFramecreateBuffer(frame));
    EXPECT_EQ(frame.state(), ExtCaptureFrame::Uninitialized);
}

TEST_F(ExtCaptureCovFinalTest, frameCreateShmBufferStubReturnsFalse)
{
    ExtCaptureFrame frame;
    EXPECT_FALSE(call_private_fun::ExtCaptureFramecreateShmBuffer(frame));
}

TEST_F(ExtCaptureCovFinalTest, frameCreateDmaBufferStubReturnsFalse)
{
    ExtCaptureFrame frame;
    EXPECT_FALSE(call_private_fun::ExtCaptureFramecreateDmaBuffer(frame));
}

TEST_F(ExtCaptureCovFinalTest, frameCreateBuffersRepeatedCallsStable)
{
    ExtCaptureFrame frame;
    EXPECT_FALSE(call_private_fun::ExtCaptureFramecreateBuffer(frame));
    EXPECT_FALSE(call_private_fun::ExtCaptureFramecreateShmBuffer(frame));
    EXPECT_FALSE(call_private_fun::ExtCaptureFramecreateDmaBuffer(frame));
    EXPECT_FALSE(call_private_fun::ExtCaptureFramecreateBuffer(frame));
}

TEST_F(ExtCaptureCovFinalTest, managerBindNoopInTestBuild)
{
    ExtCaptureManager mgr;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureManagerbind(mgr, nullptr, 1, 1));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureManagerbind(mgr, nullptr, 0, 0));
    EXPECT_GE(mgr.protocolVersion(), 0);
}

TEST_F(ExtCaptureCovFinalTest, outputSourceManagerBindNoopInTestBuild)
{
    ExtOutputSourceManager mgr;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtOutputSourceManagerbind(mgr, nullptr, 2, 1));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtOutputSourceManagerbind(mgr, nullptr, 5, 4));
    EXPECT_GE(mgr.protocolVersion(), 0);
}
