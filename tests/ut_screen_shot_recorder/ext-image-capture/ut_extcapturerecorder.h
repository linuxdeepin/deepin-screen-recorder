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

// 暴露 recorder 的私有纯函数用于直接覆盖
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, QString(), generateDefaultOutputPath);
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, bool(), adjustVideoDurationIfNeeded);
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, void(int64_t), updateFrameTimestamps);
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, void(), processFrameQueue);
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, void(), initializeCapture);

class ExtCaptureRecorderTest : public Test
{
public:
    ExtCaptureRecorder *m_rec;
    void SetUp() override { m_rec = new ExtCaptureRecorder; }
    void TearDown() override { delete m_rec; }
};

TEST_F(ExtCaptureRecorderTest, initialState)
{
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopped);
    EXPECT_EQ(m_rec->frameCount(), 0);
    EXPECT_EQ(m_rec->recordingDuration(), 0); // m_startTime==0
    EXPECT_TRUE(m_rec->outputPath().isEmpty());
    EXPECT_NE(m_rec->getFrameBuffer(), nullptr);
    // 无 Wayland 协议时 isAvailable 返回 false
    EXPECT_FALSE(m_rec->isAvailable());
}

TEST_F(ExtCaptureRecorderTest, startRecordingRejected)
{
    // 未就绪：isAvailable()==false，应返回 false 并发出 error
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);
    EXPECT_FALSE(m_rec->startRecording(nullptr));
    EXPECT_FALSE(m_rec->startRecording(nullptr, false, QString(), 30));
    EXPECT_GE(errSpy.count(), 1);

    // 提供 screen 仍因 isAvailable false 失败
    QList<QScreen*> screens = QGuiApplication::screens();
    if (!screens.isEmpty()) {
        EXPECT_FALSE(m_rec->startRecording(screens.first(), false));
        EXPECT_GE(errSpy.count(), 2);
    }
}

TEST_F(ExtCaptureRecorderTest, startRecordingInvalidFrameRateClamped)
{
    // 即便最终失败，进入参数设置前的状态校验分支被覆盖
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);
    EXPECT_FALSE(m_rec->startRecording(screens.first(), true, QString(), 999));
    EXPECT_FALSE(m_rec->startRecording(screens.first(), true, QStringLiteral("/tmp/x.mp4"), 0));
    EXPECT_GE(errSpy.count(), 1);
}

TEST_F(ExtCaptureRecorderTest, stopRecordingGuard)
{
    // 停止态调用 stopRecording 应直接返回，无副作用
    EXPECT_NO_FATAL_FAILURE(m_rec->stopRecording());
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopped);
}

TEST_F(ExtCaptureRecorderTest, setOutputPathOnlyWhenStopped)
{
    m_rec->setOutputPath(QStringLiteral("/tmp/foo.mp4"));
    EXPECT_EQ(m_rec->outputPath(), QStringLiteral("/tmp/foo.mp4"));
}

TEST_F(ExtCaptureRecorderTest, privatePureMethods)
{
    // generateDefaultOutputPath：纯函数，返回非空路径
    QString p = call_private_fun::ExtCaptureRecordergenerateDefaultOutputPath(*m_rec);
    EXPECT_FALSE(p.isEmpty());
    EXPECT_TRUE(p.contains("ext-capture-recording"));

    // adjustVideoDurationIfNeeded：frameCount==0 时早退返回 true
    EXPECT_TRUE(call_private_fun::ExtCaptureRecorderadjustVideoDurationIfNeeded(*m_rec));

    // updateFrameTimestamps：纯赋值，无副作用
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderupdateFrameTimestamps(*m_rec, 1000));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderupdateFrameTimestamps(*m_rec, -1));

    // processFrameQueue / initializeCapture：仅日志
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderprocessFrameQueue(*m_rec));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderinitializeCapture(*m_rec));
}

TEST_F(ExtCaptureRecorderTest, privateSlotsViaMetaObject)
{
    QSignalSpy protoSpy(m_rec, &ExtCaptureRecorder::protocolAvailable);
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);

    // onExtCaptureAvailable -> 发 protocolAvailable
    QMetaObject::invokeMethod(m_rec, "onExtCaptureAvailable", Qt::DirectConnection);
    EXPECT_EQ(protoSpy.count(), 1);

    // onExtCaptureUnavailable：非录制态，无副作用
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_rec, "onExtCaptureUnavailable", Qt::DirectConnection));

    // onExtCaptureError -> setState Error + 发 error
    QMetaObject::invokeMethod(m_rec, "onExtCaptureError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("boom")));
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Error);
    EXPECT_GE(errSpy.count(), 1);

    // onCaptureTimer：非录制态 -> 早退
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_rec, "onCaptureTimer", Qt::DirectConnection));
}

TEST_F(ExtCaptureRecorderTest, onRecordingStartedThenStopped)
{
    // 直接驱动槽：onRecordingStarted 启定时器并置 Recording，
    // 紧接 onRecordingStopped 走 finalizeRecording（无 ffmpeg，安全）收尾。
    QSignalSpy startedSpy(m_rec, &ExtCaptureRecorder::recordingStarted);
    QSignalSpy stoppedSpy(m_rec, &ExtCaptureRecorder::recordingStopped);

    QMetaObject::invokeMethod(m_rec, "onRecordingStarted", Qt::DirectConnection);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Recording);
    EXPECT_GE(startedSpy.count(), 1);
    EXPECT_GE(m_rec->recordingDuration(), 0);

    QMetaObject::invokeMethod(m_rec, "onRecordingStopped", Qt::DirectConnection);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopped);
    EXPECT_GE(stoppedSpy.count(), 1);
}

TEST_F(ExtCaptureRecorderTest, frameReadyGuards)
{
    // 停止态：onFrameReady / onDmaFrameReady 走守卫早退，不触发 ffmpeg
    unsigned char data[16] = {0};
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(
        m_rec, "onFrameReady", Qt::DirectConnection,
        Q_ARG(const void *, (const void *)data), Q_ARG(size_t, 16),
        Q_ARG(int, 4), Q_ARG(int, 4), Q_ARG(int, 4), Q_ARG(uint64_t, 100)));
    // null 数据守卫
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(
        m_rec, "onFrameReady", Qt::DirectConnection,
        Q_ARG(const void *, (const void *)nullptr), Q_ARG(size_t, 0),
        Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(uint64_t, 0)));
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(
        m_rec, "onDmaFrameReady", Qt::DirectConnection,
        Q_ARG(int, -1), Q_ARG(void *, nullptr), Q_ARG(size_t, 0),
        Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(uint64_t, 0)));
    EXPECT_EQ(m_rec->frameCount(), 0);
}
