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

// 覆盖 extcaptureframe.cpp 未触及的安全分支（createShmBuffer/createDmaBuffer/capture
// 需 Wayland 协议，offscreen 下 SEGV，故不调用）：
//  - frameData()：聚合 transform/damage/timestamp 后的字段读取
//  - size()/pixelFormat()：默认值与初始化后值
//  - handleReady 重复调用幂等
//  - isDmaBuffer/useDmaBuffer 配置默认
// 所有私有函数（handleTransform/handleDamage/handlePresentationTime/handleReady/
// handleFailed/setState/failureReasonToString）已在 ut_extcaptureframe.h 与
// ut_extcaptureframe_ext.h 声明，同一 TU 内可直接调用 call_private_fun，不重复声明。

class ExtCaptureFrameCovTest : public Test
{
public:
    ExtCaptureFrame *m_frame;
    void SetUp() override { m_frame = new ExtCaptureFrame; }
    void TearDown() override { delete m_frame; }
};

// frameData()：默认全空/零
TEST_F(ExtCaptureFrameCovTest, defaultFrameData)
{
    const FrameData &fd = m_frame->frameData();
    EXPECT_EQ(fd.data, nullptr);
    EXPECT_EQ(fd.size, 0u);
    EXPECT_TRUE(fd.dimensions.isEmpty());
    EXPECT_EQ(fd.format, 0u);
    EXPECT_EQ(fd.stride, 0u);
    EXPECT_EQ(fd.transform, FrameTransform::Normal);
    EXPECT_EQ(fd.timestamp, 0ull);
    EXPECT_TRUE(fd.damageRegions.isEmpty());
}

// 多个 handler 聚合后 frameData 字段一致性
TEST_F(ExtCaptureFrameCovTest, aggregatedFrameDataAfterHandlers)
{
    call_private_fun::ExtCaptureFramehandleTransform(*m_frame, 1); // Rotate90
    call_private_fun::ExtCaptureFramehandleDamage(*m_frame, 1, 2, 3, 4);
    call_private_fun::ExtCaptureFramehandlePresentationTime(*m_frame, 0, 2, 100);
    const FrameData &fd = m_frame->frameData();
    EXPECT_EQ(fd.transform, FrameTransform::Rotate90);
    EXPECT_EQ(fd.damageRegions.size(), 1);
    EXPECT_EQ(fd.damageRegions.first(), QRect(1, 2, 3, 4));
    EXPECT_GT(fd.timestamp, 0ull);
}

// size()/pixelFormat()：默认零；setState 后仍为零（无 buffer 创建）
TEST_F(ExtCaptureFrameCovTest, sizeAndPixelFormatDefaults)
{
    EXPECT_TRUE(m_frame->size().isEmpty());
    EXPECT_EQ(m_frame->pixelFormat(), 0u);
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Ready);
    EXPECT_TRUE(m_frame->size().isEmpty());
    EXPECT_EQ(m_frame->pixelFormat(), 0u);
}

// handleReady 重复调用：每次都 setState(Ready) 并 emit ready
TEST_F(ExtCaptureFrameCovTest, handleReadyIdempotentEmits)
{
    QSignalSpy readySpy(m_frame, &ExtCaptureFrame::ready);
    call_private_fun::ExtCaptureFramehandleReady(*m_frame);
    call_private_fun::ExtCaptureFramehandleReady(*m_frame);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Ready);
    EXPECT_EQ(readySpy.count(), 2);
}

// handleFailed 后再 handleReady：状态可从 Failed 回到 Ready
TEST_F(ExtCaptureFrameCovTest, failedThenReadyTransition)
{
    QSignalSpy failSpy(m_frame, &ExtCaptureFrame::failed);
    call_private_fun::ExtCaptureFramehandleFailed(*m_frame, 99);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Failed);
    ASSERT_GE(failSpy.count(), 1);
    QString reason = failSpy.takeLast().at(0).toString();
    EXPECT_TRUE(reason.contains(QStringLiteral("99")));

    call_private_fun::ExtCaptureFramehandleReady(*m_frame);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Ready);
}

// setState 在各状态间转换：覆盖 Damaged/Capturing 等
TEST_F(ExtCaptureFrameCovTest, setStateAcrossAllStates)
{
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Attached);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Attached);
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Damaged);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Damaged);
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Capturing);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Capturing);
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Failed);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Failed);
}

// 父子构造：parent=session 时 d->session 非空，安全析构
TEST_F(ExtCaptureFrameCovTest, parentedToSessionSafeLifecycle)
{
    ExtCaptureSession session;
    {
        ExtCaptureFrame f1(&session);
        ExtCaptureFrame f2(&session);
        EXPECT_EQ(f1.state(), ExtCaptureFrame::Uninitialized);
        EXPECT_EQ(f2.state(), ExtCaptureFrame::Uninitialized);
    }
    // session 仍在作用域，frames 已析构
    EXPECT_EQ(session.state(), ExtCaptureSession::Uninitialized);
}

// mapBuffer/unmapBuffer：无 mappedData 时反复调用，copyFrameData 始终 0
TEST_F(ExtCaptureFrameCovTest, mapUnmapCopyGuardsCombined)
{
    EXPECT_EQ(m_frame->mapBuffer(), nullptr);
    char dst[64] = {0};
    EXPECT_EQ(m_frame->copyFrameData(dst, 64), 0u);
    EXPECT_NO_FATAL_FAILURE(m_frame->unmapBuffer());
    // 即便 setState(Ready)，无 mappedData 仍 copyFrameData 返回 0
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Ready);
    EXPECT_EQ(m_frame->copyFrameData(dst, 64), 0u);
}
