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

// 运行期覆盖套件：在默认构造的 ExtCaptureFrame 上覆盖平台无关函数（getters、
// protected handler、mapBuffer/unmapBuffer/copyFrameData、initialize/capture 的
// 早返回守卫）。所有私有函数的 ACCESS_PRIVATE_FUN 声明已在 ut_extcaptureframe.h
// 与 ut_extcaptureframe_ext.h 中给出，本 TU 内直接复用 call_private_fun，不重复声明。
class ExtCaptureFrameRunCovTest : public Test
{
public:
    ExtCaptureFrame *m_frame;
    void SetUp() override { m_frame = new ExtCaptureFrame; }
    void TearDown() override { delete m_frame; }
};

// 默认构造后所有 getters 处于初值
TEST_F(ExtCaptureFrameRunCovTest, defaultGetters)
{
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Uninitialized);
    EXPECT_TRUE(m_frame->size().isEmpty());
    EXPECT_EQ(m_frame->pixelFormat(), 0u);
    EXPECT_EQ(m_frame->timestamp(), 0ull);
    // 默认 CaptureConfig::useDmaBuffer=true，但 bufferFd=-1 / bo=null
    EXPECT_TRUE(m_frame->isDmaBuffer());
    EXPECT_EQ(m_frame->getDmaBufferFd(), -1);
    EXPECT_EQ(m_frame->getGbmBufferObject(), nullptr);
}

// frameData() 默认字段
TEST_F(ExtCaptureFrameRunCovTest, frameDataDefaults)
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

// handleTransform：写入 transform 字段并影响 frameData()
TEST_F(ExtCaptureFrameRunCovTest, handleTransformUpdatesField)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleTransform(*m_frame, 3)); // Rotate270
    EXPECT_EQ(m_frame->frameData().transform, FrameTransform::Rotate270);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleTransform(*m_frame, 0));
    EXPECT_EQ(m_frame->frameData().transform, FrameTransform::Normal);
}

// handleDamage：追加 damage 区域
TEST_F(ExtCaptureFrameRunCovTest, handleDamageAppendsRegion)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleDamage(*m_frame, 10, 20, 30, 40));
    ASSERT_EQ(m_frame->frameData().damageRegions.size(), 1);
    EXPECT_EQ(m_frame->frameData().damageRegions.last(), QRect(10, 20, 30, 40));
}

// handlePresentationTime：时间戳组合（hi/lo/nsec）
TEST_F(ExtCaptureFrameRunCovTest, handlePresentationTimeCombines)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandlePresentationTime(*m_frame, 0, 1, 0));
    EXPECT_GT(m_frame->timestamp(), 0ull);
    EXPECT_EQ(m_frame->timestamp(), 1000000000ull);
}

// handleReady：setState(Ready) + mapBuffer + emit ready
TEST_F(ExtCaptureFrameRunCovTest, handleReadyEmitsReady)
{
    QSignalSpy readySpy(m_frame, &ExtCaptureFrame::ready);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleReady(*m_frame));
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Ready);
    EXPECT_EQ(readySpy.count(), 1);
}

// handleFailed：setState(Failed) + emit failed(reason)
TEST_F(ExtCaptureFrameRunCovTest, handleFailedEmitsFailed)
{
    QSignalSpy failSpy(m_frame, &ExtCaptureFrame::failed);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFramehandleFailed(*m_frame, 1));
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Failed);
    ASSERT_EQ(failSpy.count(), 1);
    QString reason = failSpy.takeFirst().at(0).toString();
    EXPECT_EQ(reason.toStdString(), std::string("Buffer constraints mismatch"));
}

// failureReasonToString：所有 switch 分支
TEST_F(ExtCaptureFrameRunCovTest, failureReasonToStringAllBranches)
{
    EXPECT_EQ(call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 0).toStdString(),
              std::string("Unknown error"));
    EXPECT_EQ(call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 1).toStdString(),
              std::string("Buffer constraints mismatch"));
    EXPECT_EQ(call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 2).toStdString(),
              std::string("Session stopped"));
    // default 分支
    QString d = call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 42);
    EXPECT_TRUE(d.toStdString().find("42") != std::string::npos);
}

// mapBuffer/unmapBuffer：mappedData 为空时反复调用安全
TEST_F(ExtCaptureFrameRunCovTest, mapUnmapBufferOnDefault)
{
    EXPECT_EQ(m_frame->mapBuffer(), nullptr);
    EXPECT_EQ(m_frame->mapBuffer(), nullptr);
    EXPECT_NO_FATAL_FAILURE(m_frame->unmapBuffer());
    EXPECT_NO_FATAL_FAILURE(m_frame->unmapBuffer());
}

// copyFrameData：state != Ready -> 0
TEST_F(ExtCaptureFrameRunCovTest, copyFrameDataGuards)
{
    char dst[64] = {0};
    EXPECT_EQ(m_frame->copyFrameData(dst, 64), 0u);
    EXPECT_EQ(m_frame->copyFrameData(nullptr, 64), 0u);
}

// initialize：早返回守卫（state != Uninitialized 与 frameObj null）
TEST_F(ExtCaptureFrameRunCovTest, initializeEarlyReturns)
{
    CaptureConfig cfg;
    EXPECT_FALSE(m_frame->initialize(nullptr, cfg));
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Uninitialized);
    // 把 state 提到 Attached 模拟已初始化
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Attached);
    EXPECT_FALSE(m_frame->initialize(nullptr, cfg));
}

// capture：state < Attached -> false（两次 fullDamage）
TEST_F(ExtCaptureFrameRunCovTest, captureLowStateGuard)
{
    EXPECT_FALSE(m_frame->capture(true));
    EXPECT_FALSE(m_frame->capture(false));
}

// setState：跨状态转换
TEST_F(ExtCaptureFrameRunCovTest, setStateTransitions)
{
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Damaged);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Damaged);
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Capturing);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Capturing);
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Ready);
    EXPECT_EQ(m_frame->state(), ExtCaptureFrame::Ready);
}

// 默认构造 + 析构：完整生命周期（无 buffer，安全析构）
TEST_F(ExtCaptureFrameRunCovTest, defaultConstructDestruct)
{
    EXPECT_NO_FATAL_FAILURE({
        ExtCaptureFrame frame;
        EXPECT_EQ(frame.state(), ExtCaptureFrame::Uninitialized);
    });
}
