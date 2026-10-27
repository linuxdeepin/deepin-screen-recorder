// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include "ut_extcapturesession.h"

using namespace testing;

// Tests for selectOptimalFormat SHM preferred-format ordering (commit 44b396e).
// The call_private_fun:: helpers are declared in ut_extcapturesession.h via
// ACCESS_PRIVATE_FUN and are visible in this TU because test_all_interfaces.h
// includes both headers.

class ExtCaptureSessionShmPrefTest : public Test
{
public:
    ExtCaptureSession *m_sess = nullptr;

    void SetUp() override { m_sess = new ExtCaptureSession; }
    void TearDown() override { delete m_sess; }

    void addShmFormat(uint32_t format)
    {
        call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, format);
    }

    void selectFormat()
    {
        call_private_fun::ExtCaptureSessionselectOptimalFormat(*m_sess);
    }
};

TEST_F(ExtCaptureSessionShmPrefTest, PrefersXRGB8888OverOthers)
{
    addShmFormat(WL_SHM_FORMAT_XBGR8888);
    addShmFormat(WL_SHM_FORMAT_ARGB8888);
    addShmFormat(WL_SHM_FORMAT_XRGB8888);
    selectFormat();
    EXPECT_EQ(m_sess->config().format, WL_SHM_FORMAT_XRGB8888);
    EXPECT_FALSE(m_sess->config().useDmaBuffer);
}

TEST_F(ExtCaptureSessionShmPrefTest, FallsBackToARGB8888)
{
    addShmFormat(WL_SHM_FORMAT_XBGR8888);
    addShmFormat(WL_SHM_FORMAT_ARGB8888);
    selectFormat();
    EXPECT_EQ(m_sess->config().format, WL_SHM_FORMAT_ARGB8888);
    EXPECT_FALSE(m_sess->config().useDmaBuffer);
}

TEST_F(ExtCaptureSessionShmPrefTest, FallsBackToXBGR8888)
{
    addShmFormat(WL_SHM_FORMAT_ABGR8888);
    addShmFormat(WL_SHM_FORMAT_XBGR8888);
    selectFormat();
    EXPECT_EQ(m_sess->config().format, WL_SHM_FORMAT_XBGR8888);
    EXPECT_FALSE(m_sess->config().useDmaBuffer);
}

TEST_F(ExtCaptureSessionShmPrefTest, FallsBackToABGR8888)
{
    addShmFormat(WL_SHM_FORMAT_ABGR8888);
    selectFormat();
    EXPECT_EQ(m_sess->config().format, WL_SHM_FORMAT_ABGR8888);
    EXPECT_FALSE(m_sess->config().useDmaBuffer);
}

TEST_F(ExtCaptureSessionShmPrefTest, NoPreferredFormatUsesFirstAdvertised)
{
    addShmFormat(0xDEADBEEF);
    addShmFormat(0xCAFEBABE);
    selectFormat();
    EXPECT_EQ(m_sess->config().format, 0xDEADBEEF);
    EXPECT_FALSE(m_sess->config().useDmaBuffer);
}

TEST_F(ExtCaptureSessionShmPrefTest, XRGB8888ChosenEvenWhenNotFirst)
{
    addShmFormat(WL_SHM_FORMAT_XBGR8888);
    addShmFormat(WL_SHM_FORMAT_ARGB8888);
    addShmFormat(WL_SHM_FORMAT_ABGR8888);
    addShmFormat(WL_SHM_FORMAT_XRGB8888);
    selectFormat();
    EXPECT_EQ(m_sess->config().format, WL_SHM_FORMAT_XRGB8888);
    EXPECT_FALSE(m_sess->config().useDmaBuffer);
}

TEST_F(ExtCaptureSessionShmPrefTest, NoFormatsResultsInError)
{
    QSignalSpy errSpy(m_sess, &ExtCaptureSession::error);
    selectFormat();
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Error);
    EXPECT_EQ(errSpy.count(), 1);
}
