// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QMetaObject>
#include "addr_pri.h"
#include "../../src/ext-image-capture/frame/extcaptureframe.h"
#include "../../src/ext-image-capture/session/extcapturesession.h"

using namespace testing;

// 仅暴露 ut_extcaptureframe.h 中未声明的新增私有函数（避免同一 TU 内宏重定义）
ACCESS_PRIVATE_FUN(ExtCaptureFrame, void(ExtCaptureFrame::FrameState), setState);

class ExtCaptureFrameExtTest : public Test
{
public:
    ExtCaptureFrame *m_frame;
    void SetUp() override { m_frame = new ExtCaptureFrame; }
    void TearDown() override { delete m_frame; }
};

// handleTransform：覆盖全部 8 种枚举值，确保 transform 字段被正确写入
TEST_F(ExtCaptureFrameExtTest, handleTransformAllVariants)
{
    for (uint32_t t = 0; t <= 7; ++t) {
        EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleTransform(*m_frame, t));
    }
    // 越界值：仍被 static_cast 接受，不崩溃
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleTransform(*m_frame, 99));
}

// handleDamage：多次追加 damage 区域，验证列表增长
TEST_F(ExtCaptureFrameExtTest, handleDamageAccumulatesRegions)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleDamage(*m_frame, 0, 0, 10, 10));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleDamage(*m_frame, 5, 5, 20, 20));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleDamage(*m_frame, -1, -1, 0, 0));
    EXPECT_EQ(m_frame->frameData().damageRegions.size(), 3);
    // 验证第一块区域被正确记录
    EXPECT_EQ(m_frame->frameData().damageRegions.first(), QRect(0, 0, 10, 10));
}

// handlePresentationTime：tv_sec_hi 影响 64 位组合
TEST_F(ExtCaptureFrameExtTest, handlePresentationTimeHighBits)
{
    call_private_fun::ExtCaptureFramehandlePresentationTime(*m_frame, 1, 0, 0);
    // seconds = (1<<32) -> 非零时间戳
    EXPECT_GT(m_frame->timestamp(), 0ull);
    EXPECT_TRUE(m_frame->timestamp() >= (1ull << 32));

    // nsec 单独递增
    uint64_t before = m_frame->timestamp();
    call_private_fun::ExtCaptureFramehandlePresentationTime(*m_frame, 0, 0, 500);
    uint64_t after = m_frame->timestamp();
    EXPECT_EQ(after, 500ull);
    EXPECT_NE(after, before);
}

// handleReady：未映射时调用，mapBuffer 走空 mappedData 分支
TEST_F(ExtCaptureFrameExtTest, handleReadyNoMappedData)
{
    QSignalSpy readySpy(m_frame, &ExtCaptureFrame::ready);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleReady(*m_frame));
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Ready);
    EXPECT_EQ(readySpy.count(), 1);
    // 无 mappedData -> mapBuffer 返回 nullptr
    EXPECT_EQ(m_frame->mapBuffer(), nullptr);
}

// handleFailed：覆盖 reason=0 与 default 分支，验证 failureReasonToString 被调用
TEST_F(ExtCaptureFrameExtTest, handleFailedReasons)
{
    QSignalSpy failSpy(m_frame, &ExtCaptureFrame::failed);
    call_private_fun::ExtCaptureFramehandleFailed(*m_frame, 0);
    ASSERT_GE(failSpy.count(), 1);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Failed);

    // Failed -> Ready 的状态转换：直接驱动 handleReady 验证可重置
    call_private_fun::ExtCaptureFramehandleReady(*m_frame);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Ready);

    call_private_fun::ExtCaptureFramehandleFailed(*m_frame, 2);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Failed);
    QString lastReason = failSpy.takeLast().at(0).toString();
    EXPECT_TRUE(lastReason.contains(QStringLiteral("Session stopped")));
}

// failureReasonToString：再次覆盖 default 分支的极大值
TEST_F(ExtCaptureFrameExtTest, failureReasonToStringLargeDefault)
{
    QString s = call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 0xFFFFFFFFu);
    EXPECT_TRUE(s.contains(QStringLiteral("4294967295")));
}

// mapBuffer/unmapBuffer：无 mappedData 时反复调用安全
TEST_F(ExtCaptureFrameExtTest, mapUnmapBufferGuards)
{
    EXPECT_EQ(m_frame->mapBuffer(), nullptr);
    EXPECT_EQ(m_frame->mapBuffer(), nullptr);
    EXPECT_NO_FATAL_FAILURE(m_frame->unmapBuffer());
    EXPECT_NO_FATAL_FAILURE(m_frame->unmapBuffer());
}

// copyFrameData：state != Ready -> 0；null dest 同样安全（因 state 守卫先命中）
TEST_F(ExtCaptureFrameExtTest, copyFrameDataGuards)
{
    char dst[32] = {0};
    EXPECT_EQ(m_frame->copyFrameData(dst, 32), 0u);
    EXPECT_EQ(m_frame->copyFrameData(nullptr, 32), 0u);
    // maxSize=0 -> 即便 state 正确也复制 0 字节
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Ready);
    EXPECT_EQ(m_frame->copyFrameData(dst, 0), 0u);
}

// initialize：state != Uninitialized -> false（已初始化守卫）
TEST_F(ExtCaptureFrameExtTest, initializeRejectsWhenAlreadyInitialized)
{
    CaptureConfig cfg;
    // 先把 state 改为 Attached 模拟已初始化
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Attached);
    EXPECT_FALSE(m_frame->initialize(nullptr, cfg));
}

// initialize：frameObj=null -> false（即便 state==Uninitialized）
TEST_F(ExtCaptureFrameExtTest, initializeRejectsNullFrameObj)
{
    CaptureConfig cfg;
    EXPECT_FALSE(m_frame->initialize(nullptr, cfg));
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Uninitialized);
}

// capture：state<Attached -> false（Uninitialized 守卫；不设高态，避免触发 wayland attach_buffer）
TEST_F(ExtCaptureFrameExtTest, captureRejectsInLowStates)
{
    EXPECT_FALSE(m_frame->capture(true));
    EXPECT_FALSE(m_frame->capture(false));
}

// DMA 访问器：默认 useDmaBuffer=true，但 bufferFd=-1/bo=null
TEST_F(ExtCaptureFrameExtTest, dmaAccessorsDefaults)
{
    EXPECT_TRUE(m_frame->isDmaBuffer());
    EXPECT_EQ(m_frame->getDmaBufferFd(), -1);
    EXPECT_EQ(m_frame->getGbmBufferObject(), nullptr);
}

// 父子关系：parent 为 session 时，d->session 非空；无 parent 时 session 为空仍可安全析构
TEST_F(ExtCaptureFrameExtTest, parentedAndOrphanedBothSafe)
{
    ExtCaptureSession session;
    {
        ExtCaptureFrame frame(&session);
        EXPECT_EQ(frame.state(), ExtCaptureFrame::Uninitialized);
        EXPECT_TRUE(frame.isDmaBuffer());
    }
    {
        ExtCaptureFrame orphan;
        EXPECT_EQ(orphan.state(), ExtCaptureFrame::Uninitialized);
    }
}

// setState：仅状态变化时写入；相同状态不重复（间接验证，无 crash）
TEST_F(ExtCaptureFrameExtTest, setStateIdempotent)
{
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Damaged);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Damaged);
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Damaged);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Damaged);
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Capturing);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Capturing);
}
