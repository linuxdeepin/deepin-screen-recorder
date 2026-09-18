// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QGuiApplication>
#include <QScreen>
#include <QMetaObject>
#include "addr_pri.h"
#include "../../src/ext-image-capture/extcapturerecorder.h"

using namespace testing;

// 覆盖 extcapturerecorder.cpp 未触及的安全分支：
//  - setState 各状态转换
//  - stopRecording 从 Recording/Starting/Stopping 态执行（走 m_frameBuffer->setGetFrame 等）
//  - onExtCaptureUnavailable 在 Recording 态 -> 调 stopRecording + emit error
//  - getFrameBuffer 非空
//  - onRecordingStarted 缓冲模式（m_streamingMode=false）分支
// 注意：createVideoFile/startFFmpegProcess/startDmaFFmpegProcess/adjust 校正分支会
// spawn 进程，按约束不调用。
// setState 是新增私有函数声明（既有头未声明）；m_state/m_streamingMode/m_frameRate
// 字段已在 ut_extcapturerecorder_ext.h 声明，同一 TU 内可直接复用，不重复声明。
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, void(ExtCaptureRecorder::RecordState), setState);

class ExtCaptureRecorderCovTest : public Test
{
public:
    ExtCaptureRecorder *m_rec;
    void SetUp() override { m_rec = new ExtCaptureRecorder; }
    void TearDown() override { delete m_rec; }
};

// setState：各状态转换 + state() 读取
TEST_F(ExtCaptureRecorderCovTest, setStateAllTransitions)
{
    call_private_fun::ExtCaptureRecordersetState(*m_rec, ExtCaptureRecorder::Starting);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Starting);
    call_private_fun::ExtCaptureRecordersetState(*m_rec, ExtCaptureRecorder::Recording);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Recording);
    call_private_fun::ExtCaptureRecordersetState(*m_rec, ExtCaptureRecorder::Stopping);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopping);
    call_private_fun::ExtCaptureRecordersetState(*m_rec, ExtCaptureRecorder::Error);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Error);
    call_private_fun::ExtCaptureRecordersetState(*m_rec, ExtCaptureRecorder::Stopped);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopped);
}

// getFrameBuffer：构造即非空
TEST_F(ExtCaptureRecorderCovTest, frameBufferAlwaysExists)
{
    EXPECT_NE(m_rec->getFrameBuffer(), nullptr);
}

// isAvailable：offscreen 下 ext-capture 不可用 -> false
TEST_F(ExtCaptureRecorderCovTest, isAvailableFalseUnderOffscreen)
{
    EXPECT_FALSE(m_rec->isAvailable());
}

// startRecording：null screen + 协议不可用 -> 多重守卫，仍 false
TEST_F(ExtCaptureRecorderCovTest, startRecordingMultiGuard)
{
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);
    EXPECT_FALSE(m_rec->startRecording(nullptr, true, QStringLiteral("/tmp/x.mp4"), 30));
    EXPECT_GE(errSpy.count(), 1);
}

// startRecording：frameRate 越界（负数/极大）最终仍因协议不可用失败
TEST_F(ExtCaptureRecorderCovTest, startRecordingExtremeFrameRate)
{
    EXPECT_FALSE(m_rec->startRecording(nullptr, false, QString(), -5));
    EXPECT_FALSE(m_rec->startRecording(nullptr, false, QString(), 1000));
}

// stopRecording：从 Recording 态执行（disconnect 信号、停定时器、setGetFrame(false)）
TEST_F(ExtCaptureRecorderCovTest, stopRecordingFromRecordingState)
{
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Recording;
    EXPECT_NO_FATAL_FAILURE(m_rec->stopRecording());
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopping);
}

// stopRecording：从 Starting 态执行
TEST_F(ExtCaptureRecorderCovTest, stopRecordingFromStartingState)
{
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Starting;
    EXPECT_NO_FATAL_FAILURE(m_rec->stopRecording());
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopping);
}

// onExtCaptureUnavailable：Recording 态 -> stopRecording + emit error
TEST_F(ExtCaptureRecorderCovTest, onExtCaptureUnavailableWhileRecording)
{
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Recording;
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);
    QMetaObject::invokeMethod(m_rec, "onExtCaptureUnavailable", Qt::DirectConnection);
    EXPECT_GE(errSpy.count(), 1);
    // 状态变为 Stopping（stopRecording 内部 setState(Stopping)）
    EXPECT_NE(m_rec->state(), ExtCaptureRecorder::Recording);
}

// onRecordingStarted：缓冲模式（streamingMode=false）分支
TEST_F(ExtCaptureRecorderCovTest, onRecordingStartedBufferMode)
{
    access_private_field::ExtCaptureRecorderm_streamingMode(*m_rec) = false;
    access_private_field::ExtCaptureRecorderm_frameRate(*m_rec) = 15;
    QSignalSpy startedSpy(m_rec, &ExtCaptureRecorder::recordingStarted);
    QMetaObject::invokeMethod(m_rec, "onRecordingStarted", Qt::DirectConnection);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Recording);
    EXPECT_GE(startedSpy.count(), 1);
    // 收尾：进入 Stopping 再 stopRecording，避免定时器残留
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Stopping;
    EXPECT_NO_FATAL_FAILURE(m_rec->stopRecording());
}

// onRecordingStarted：流式模式默认分支
TEST_F(ExtCaptureRecorderCovTest, onRecordingStartedStreamingMode)
{
    // 默认 m_streamingMode=true
    QSignalSpy startedSpy(m_rec, &ExtCaptureRecorder::recordingStarted);
    QMetaObject::invokeMethod(m_rec, "onRecordingStarted", Qt::DirectConnection);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Recording);
    EXPECT_GE(startedSpy.count(), 1);
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Stopping;
    EXPECT_NO_FATAL_FAILURE(m_rec->stopRecording());
}

// setOutputPath：Stopped 态写入；构造默认即 Stopped
TEST_F(ExtCaptureRecorderCovTest, setOutputPathInStopped)
{
    m_rec->setOutputPath(QStringLiteral("/tmp/ut_cov.mp4"));
    EXPECT_EQ(m_rec->outputPath(), QStringLiteral("/tmp/ut_cov.mp4"));
    // 清空
    m_rec->setOutputPath(QString());
    EXPECT_TRUE(m_rec->outputPath().isEmpty());
}

// recordingDuration：m_startTime==0 -> 0（默认态）
TEST_F(ExtCaptureRecorderCovTest, recordingDurationZeroWhenNotStarted)
{
    EXPECT_EQ(m_rec->recordingDuration(), 0);
}

// 析构在 Recording 态：触发 stopRecording 收尾，无 crash
TEST_F(ExtCaptureRecorderCovTest, destructWhileRecordingSafe)
{
    ExtCaptureRecorder *rec = new ExtCaptureRecorder;
    access_private_field::ExtCaptureRecorderm_state(*rec) = ExtCaptureRecorder::Recording;
    EXPECT_NO_FATAL_FAILURE(delete rec);
}
