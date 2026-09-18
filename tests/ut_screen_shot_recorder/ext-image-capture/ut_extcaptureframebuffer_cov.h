// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "addr_pri.h"
#include "../../src/ext-image-capture/extcaptureframebuffer.h"

using namespace testing;

// 覆盖 ut_extcaptureframebuffer.h 未触及的分支：
//  - initialize 在 bufferSize<=1 时被 qMax 钳到 2
//  - appendBuffer 在 bufferSize 容量恰好填满时的 bufferStatusChanged(isFull=true)
//  - getFrame 在 workingBuffer 存在但队列为空 -> false 并清空输出帧字段
//  - recycleMemory 私有方法（仅日志）
ACCESS_PRIVATE_FUN(ExtCaptureFrameBuffer, void(), recycleMemory);

class ExtCaptureFrameBufferCovTest : public Test
{
public:
    ExtCaptureFrameBuffer *m_fb;
    void SetUp() override { m_fb = new ExtCaptureFrameBuffer; }
    void TearDown() override { delete m_fb; }
};

// initialize：bufferSize 传入 1 -> qMax(2,1)=2；传入 0 -> 2
TEST_F(ExtCaptureFrameBufferCovTest, initializeClampsBufferSizeFloor)
{
    EXPECT_TRUE(m_fb->initialize(2, 2, 4, 1));
    EXPECT_EQ(m_fb->frameCount(), 0);
    // 销毁后重建新实例测 0
    ExtCaptureFrameBuffer fb2;
    EXPECT_TRUE(fb2.initialize(2, 2, 4, 0));
}

// bufferSize=1（钳到 2）后填满：触发 bufferStatusChanged 的 isFull=true 路径
TEST_F(ExtCaptureFrameBufferCovTest, fillToCapacityEmitsFullStatus)
{
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 2)); // 容量 2
    unsigned char data[8] = {0};
    QSignalSpy statusSpy(m_fb, &ExtCaptureFrameBuffer::bufferStatusChanged);

    m_fb->appendBuffer(data, 8, 2, 2, 4, 1);
    m_fb->appendBuffer(data, 8, 2, 2, 4, 2); // 队列达容量 -> isFull=true
    ASSERT_EQ(statusSpy.count(), 2);
    QList<QVariant> last = statusSpy.takeLast();
    EXPECT_EQ(last.at(0).toInt(), 2);          // frameCount
    EXPECT_TRUE(last.at(1).toBool());          // isFull
}

// 循环缓冲：容量 2 时 append 第 3 帧回收最老帧，回收后 isFull 仍 true
TEST_F(ExtCaptureFrameBufferCovTest, circularRecycleKeepsCapacity)
{
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 2));
    unsigned char data[8] = {0};
    for (int i = 0; i < 5; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_fb->appendBuffer(data, 8, 2, 2, 4, i));
    }
    EXPECT_EQ(m_fb->frameCount(), 2);
    EXPECT_TRUE(m_fb->hasFrames());
}

// getFrame：已初始化但队列为空 -> false，且输出帧字段被清零
TEST_F(ExtCaptureFrameBufferCovTest, getFrameEmptyQueueClearsOutput)
{
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 2));
    ExtFrameData frame;
    frame.width = 99;
    frame.height = 99;
    frame.data = reinterpret_cast<unsigned char *>(0x1);
    EXPECT_FALSE(m_fb->getFrame(frame));
    EXPECT_EQ(frame.width, 0);
    EXPECT_EQ(frame.height, 0);
    EXPECT_EQ(frame.data, nullptr);
}

// getFrame：成功取出后，bufferStatusChanged 以 isFull=false 发射
TEST_F(ExtCaptureFrameBufferCovTest, getFrameEmitsNotFullStatus)
{
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 2));
    unsigned char data[8] = {0};
    m_fb->appendBuffer(data, 8, 2, 2, 4, 1);
    m_fb->appendBuffer(data, 8, 2, 2, 4, 2); // 满

    QSignalSpy statusSpy(m_fb, &ExtCaptureFrameBuffer::bufferStatusChanged);
    ExtFrameData frame;
    ASSERT_TRUE(m_fb->getFrame(frame));
    ASSERT_GE(statusSpy.count(), 1);
    QList<QVariant> last = statusSpy.takeLast();
    EXPECT_EQ(last.at(0).toInt(), 1);           // 剩余 1
    EXPECT_FALSE(last.at(1).toBool());          // isFull=false
}

// recycleMemory：私有方法，仅持锁打印日志，无副作用
TEST_F(ExtCaptureFrameBufferCovTest, recycleMemorySafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFrameBufferrecycleMemory(*m_fb));
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 2));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureFrameBufferrecycleMemory(*m_fb));
}

// reset 后再次 initialize 应成功（reset 清理 m_initialized）
TEST_F(ExtCaptureFrameBufferCovTest, resetAllowsReinitialize)
{
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 2));
    m_fb->reset();
    // reset 后 initialize 不再被“已初始化”守卫拒绝
    EXPECT_TRUE(m_fb->initialize(2, 2, 4, 2));
}

// appendBuffer：未初始化时即便参数合法也直接返回（守卫）
TEST_F(ExtCaptureFrameBufferCovTest, appendBufferUninitializedGuard)
{
    unsigned char data[8] = {0};
    EXPECT_NO_FATAL_FAILURE(m_fb->appendBuffer(data, 8, 2, 2, 4, 1));
    EXPECT_EQ(m_fb->frameCount(), 0);
}
