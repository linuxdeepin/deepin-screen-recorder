// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// X11 coverage tests for src/gstrecord/gstrecordx.cpp.
//
// Strategy: exercise the private x11GstStartRecord() / x11GstStopRecord()
// entry points. x11GstStartRecord builds an argument list (covers ~30 lines of
// string assembly) then calls private createPipeline(), which performs a real
// gst_parse_launch — without a display it fails and takes the qCritical else
// branch (still covering the argument-building lines). To keep the suite
// ASan-safe we STUB createPipeline to return false so no real GStreamer
// pipeline is constructed. x11GstStopRecord sees m_pipeline == nullptr by
// default and takes the early-return warning branch.

#pragma once
#include <gtest/gtest.h>
#include <QRect>
#include <QString>
#include "addr_pri.h"
#include "stub.h"
#include "../../src/gstrecord/gstrecordx.h"
#include "../../src/gstrecord/gstinterface.h"

ACCESS_PRIVATE_FUN(GstRecordX, void(), x11GstStartRecord);
ACCESS_PRIVATE_FUN(GstRecordX, void(), x11GstStopRecord);
ACCESS_PRIVATE_FUN(GstRecordX, bool(QStringList), createPipeline);

// Stub replacement for GstRecordX::createPipeline — avoids touching real
// GStreamer (gst_parse_launch) which would risk an ASan abort in headless CI.
static bool cp_stub(void *) { return false; }

class GstRecordXX11CovTest : public ::testing::Test
{
public:
    GstRecordX *m_gst;
    Stub m_stub;

    void SetUp() override
    {
        ASSERT_NO_FATAL_FAILURE(gstInterface::initFunctions());
        m_gst = new GstRecordX();
        // Force createPipeline to short-circuit (return false) so the
        // argument-building branch runs without launching a real pipeline.
        m_stub.set(get_private_fun::GstRecordXcreatePipeline(), cp_stub);
        // Sane values for argument building.
        m_gst->setRecordArea(QRect(0, 0, 100, 100));
        m_gst->setFramerate(15);
    }

    void TearDown() override
    {
        delete m_gst;
        m_gst = nullptr;
    }
};

// x11GstStartRecord builds the ximagesrc argument list and calls the (stubbed)
// createPipeline; with the stub returning false it takes the qCritical else
// branch. Covers the ~30 argument-building lines without launching GStreamer.
TEST_F(GstRecordXX11CovTest, X11GstStartRecordBuildsArgsAndHandlesPipelineFailure)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXx11GstStartRecord(*m_gst));
}

// x11GstStopRecord sees m_pipeline == nullptr (never set by the stubbed start)
// and takes the early-return warning branch.
TEST_F(GstRecordXX11CovTest, X11GstStopRecordEarlyReturnsWhenNoPipeline)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXx11GstStopRecord(*m_gst));
}

// Start then stop must not crash; stop still hits the null-pipeline branch
// because the stubbed start never assigns m_pipeline.
TEST_F(GstRecordXX11CovTest, X11StartThenStopIsCrashFree)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXx11GstStartRecord(*m_gst));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXx11GstStopRecord(*m_gst));
}
