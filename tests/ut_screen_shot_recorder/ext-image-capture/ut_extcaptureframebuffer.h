// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include "../../src/ext-image-capture/extcaptureframebuffer.h"

using namespace testing;

// ExtCaptureFrameBuffer：纯帧队列/缓冲区管理，全部为可测的数据逻辑。
class ExtCaptureFrameBufferTest : public Test
{
public:
    ExtCaptureFrameBuffer *m_fb;
    void SetUp() override { m_fb = new ExtCaptureFrameBuffer; }
    void TearDown() override { delete m_fb; }
};

TEST_F(ExtCaptureFrameBufferTest, defaultState)
{
    EXPECT_FALSE(m_fb->hasFrames());
    EXPECT_EQ(m_fb->frameCount(), 0);
    EXPECT_TRUE(m_fb->isGetFrameEnabled());
}

TEST_F(ExtCaptureFrameBufferTest, initializeInvalidArgs)
{
    EXPECT_FALSE(m_fb->initialize(0, 0, 0));
    EXPECT_FALSE(m_fb->initialize(-1, 10, 10));
    EXPECT_FALSE(m_fb->initialize(10, 0, 10));
    EXPECT_FALSE(m_fb->initialize(10, 10, 0));
}

TEST_F(ExtCaptureFrameBufferTest, initializeAndGetters)
{
    EXPECT_TRUE(m_fb->initialize(4, 2, 8, 3));
    // double init rejected
    EXPECT_FALSE(m_fb->initialize(4, 2, 8, 3));
    EXPECT_FALSE(m_fb->hasFrames());
    EXPECT_EQ(m_fb->frameCount(), 0);
}

TEST_F(ExtCaptureFrameBufferTest, appendBufferRejectsBadInput)
{
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 3));
    unsigned char data[16] = {0};
    // 未初始化场景由单独实例验证；这里测参数校验
    EXPECT_NO_FATAL_FAILURE(m_fb->appendBuffer(nullptr, 16, 2, 2, 4, 1));   // null data
    EXPECT_NO_FATAL_FAILURE(m_fb->appendBuffer(data, 0, 2, 2, 4, 1));        // size 0
    EXPECT_NO_FATAL_FAILURE(m_fb->appendBuffer(data, 16, 3, 2, 4, 1));       // width mismatch
    EXPECT_NO_FATAL_FAILURE(m_fb->appendBuffer(data, 16, 2, 3, 4, 1));       // height mismatch
    EXPECT_NO_FATAL_FAILURE(m_fb->appendBuffer(data, 16, 2, 2, 8, 1));       // stride mismatch
    EXPECT_NO_FATAL_FAILURE(m_fb->appendBuffer(data, 7, 2, 2, 4, 1));        // size mismatch (expected 8)
    EXPECT_EQ(m_fb->frameCount(), 0);
}

TEST_F(ExtCaptureFrameBufferTest, appendAndGetFrame)
{
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 3));
    unsigned char data[16] = {1, 2, 3, 4, 5, 6, 7, 8};
    QSignalSpy availSpy(m_fb, &ExtCaptureFrameBuffer::frameAvailable);
    QSignalSpy statusSpy(m_fb, &ExtCaptureFrameBuffer::bufferStatusChanged);

    m_fb->appendBuffer(data, 8, 2, 2, 4, 100);
    EXPECT_TRUE(m_fb->hasFrames());
    EXPECT_EQ(m_fb->frameCount(), 1);
    EXPECT_EQ(availSpy.count(), 1);
    EXPECT_GE(statusSpy.count(), 1);

    ExtFrameData frame;
    ASSERT_TRUE(m_fb->getFrame(frame));
    EXPECT_EQ(frame.width, 2);
    EXPECT_EQ(frame.height, 2);
    EXPECT_EQ(frame.stride, 4);
    EXPECT_EQ(frame.timestamp, 100);
    EXPECT_NE(frame.data, nullptr);
    EXPECT_FALSE(m_fb->hasFrames());

    // empty queue -> false
    EXPECT_FALSE(m_fb->getFrame(frame));
}

TEST_F(ExtCaptureFrameBufferTest, circularBufferOverflow)
{
    // bufferSize=2，append 超过容量，验证循环回收老帧
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 2));
    unsigned char data[8] = {0};
    for (int i = 0; i < 10; ++i) {
        m_fb->appendBuffer(data, 8, 2, 2, 4, i);
    }
    EXPECT_EQ(m_fb->frameCount(), 2);
}

TEST_F(ExtCaptureFrameBufferTest, setGetFrameToggle)
{
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 3));
    EXPECT_TRUE(m_fb->isGetFrameEnabled());
    m_fb->setGetFrame(false);
    EXPECT_FALSE(m_fb->isGetFrameEnabled());
    // 关闭后 append 直接返回，不入队
    unsigned char data[8] = {0};
    m_fb->appendBuffer(data, 8, 2, 2, 4, 1);
    EXPECT_EQ(m_fb->frameCount(), 0);
    m_fb->setGetFrame(true);
    EXPECT_TRUE(m_fb->isGetFrameEnabled());
}

TEST_F(ExtCaptureFrameBufferTest, clearAndReset)
{
    ASSERT_TRUE(m_fb->initialize(2, 2, 4, 3));
    unsigned char data[8] = {0};
    m_fb->appendBuffer(data, 8, 2, 2, 4, 1);
    m_fb->appendBuffer(data, 8, 2, 2, 4, 2);
    ASSERT_EQ(m_fb->frameCount(), 2);

    m_fb->clear();
    EXPECT_EQ(m_fb->frameCount(), 0);

    m_fb->appendBuffer(data, 8, 2, 2, 4, 3);
    ASSERT_EQ(m_fb->frameCount(), 1);
    m_fb->reset();
    EXPECT_EQ(m_fb->frameCount(), 0);
    EXPECT_TRUE(m_fb->isGetFrameEnabled());
}

TEST_F(ExtCaptureFrameBufferTest, unIntializedAppendAndGet)
{
    // 未调用 initialize 直接操作，验证防护分支
    unsigned char data[8] = {0};
    EXPECT_NO_FATAL_FAILURE(m_fb->appendBuffer(data, 8, 2, 2, 4, 1));
    ExtFrameData frame;
    EXPECT_FALSE(m_fb->getFrame(frame));
    EXPECT_NO_FATAL_FAILURE(m_fb->clear());
    EXPECT_NO_FATAL_FAILURE(m_fb->reset());
}
