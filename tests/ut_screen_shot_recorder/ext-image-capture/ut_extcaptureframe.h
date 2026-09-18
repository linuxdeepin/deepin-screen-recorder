// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "addr_pri.h"
#include "../../src/ext-image-capture/frame/extcaptureframe.h"
#include "../../src/ext-image-capture/session/extcapturesession.h"

using namespace testing;

// protected 处理函数 + failureReasonToString：均为安全纯逻辑
ACCESS_PRIVATE_FUN(ExtCaptureFrame, void(uint32_t), handleTransform);
ACCESS_PRIVATE_FUN(ExtCaptureFrame, void(int32_t, int32_t, int32_t, int32_t), handleDamage);
ACCESS_PRIVATE_FUN(ExtCaptureFrame, void(uint32_t, uint32_t, uint32_t), handlePresentationTime);
ACCESS_PRIVATE_FUN(ExtCaptureFrame, void(), handleReady);
ACCESS_PRIVATE_FUN(ExtCaptureFrame, void(uint32_t), handleFailed);
ACCESS_PRIVATE_FUN(ExtCaptureFrame, QString(uint32_t), failureReasonToString);

class ExtCaptureFrameTest : public Test
{
public:
    ExtCaptureFrame *m_frame;
    void SetUp() override { m_frame = new ExtCaptureFrame; }
    void TearDown() override { delete m_frame; }
};

TEST_F(ExtCaptureFrameTest, defaults)
{
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Uninitialized);
    EXPECT_TRUE(m_frame->size().isEmpty());
    EXPECT_EQ(m_frame->pixelFormat(), 0u);
    EXPECT_EQ(m_frame->timestamp(), 0ull);
    // 默认 CaptureConfig::useDmaBuffer=true
    EXPECT_EQ(m_frame->getDmaBufferFd(), -1);
    EXPECT_EQ(m_frame->getGbmBufferObject(), nullptr);
    EXPECT_EQ(m_frame->mapBuffer(), nullptr); // 无 mappedData
}

TEST_F(ExtCaptureFrameTest, captureAndInitGuards)
{
    EXPECT_FALSE(m_frame->capture());              // state < Attached
    EXPECT_FALSE(m_frame->capture(false));
    char dst[16];
    EXPECT_EQ(m_frame->copyFrameData(dst, 16), 0u); // state != Ready
    CaptureConfig cfg;
    EXPECT_FALSE(m_frame->initialize(nullptr, cfg)); // frameObj null
    m_frame->unmapBuffer();                          // 无副作用
}

TEST_F(ExtCaptureFrameTest, protectedHandlers)
{
    call_private_fun::ExtCaptureFramehandleTransform(*m_frame, 2);
    call_private_fun::ExtCaptureFramehandleDamage(*m_frame, 0, 0, 10, 10);
    call_private_fun::ExtCaptureFramehandlePresentationTime(*m_frame, 0, 1, 500);
    EXPECT_NE(m_frame->timestamp(), 0ull);

    QSignalSpy readySpy(m_frame, &ExtCaptureFrame::ready);
    call_private_fun::ExtCaptureFramehandleReady(*m_frame);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Ready);
    EXPECT_EQ(readySpy.count(), 1);

    QSignalSpy failSpy(m_frame, &ExtCaptureFrame::failed);
    call_private_fun::ExtCaptureFramehandleFailed(*m_frame, 1);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Failed);
    EXPECT_EQ(failSpy.count(), 1);
}

TEST_F(ExtCaptureFrameTest, failureReasonToStringSwitch)
{
    EXPECT_EQ(call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 0), QStringLiteral("Unknown error"));
    EXPECT_EQ(call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 1), QStringLiteral("Buffer constraints mismatch"));
    EXPECT_EQ(call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 2), QStringLiteral("Session stopped"));
    EXPECT_TRUE(call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 99).contains("99"));
}

TEST_F(ExtCaptureFrameTest, parentedToSession)
{
    // 构造时 parent 为 session -> d->session 非空路径
    ExtCaptureSession session;
    ExtCaptureFrame frame(&session);
    EXPECT_EQ(frame.state(), ExtCaptureFrame::Uninitialized);
}
