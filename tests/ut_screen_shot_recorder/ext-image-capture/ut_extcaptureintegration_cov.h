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
#include "../../src/ext-image-capture/extcaptureintegration.h"

using namespace testing;

// 覆盖 extcaptureintegration.cpp 未触及的安全分支：
//  - onFrameReady：null frame 守卫
//  - captureFrame：not recording / session null / state != Ready 守卫组合
//  - stopRecording：既非单屏也非多屏 -> 直接返回
//  - onProtocolUnavailable：非录制态 -> 仅 emit unavailable
//  - onSessionStopped：非录制态 -> 仅日志
//  - isAvailable：m_manager 存在但协议不可用 -> false
class ExtCaptureIntegrationCovTest : public Test
{
public:
    ExtCaptureIntegration *m_int;
    void SetUp() override { m_int = new ExtCaptureIntegration; }
    void TearDown() override { delete m_int; }
};

// isAvailable：manager 存在但 offscreen 协议不可用 -> false
TEST_F(ExtCaptureIntegrationCovTest, isAvailableFalseUnderOffscreen)
{
    EXPECT_FALSE(m_int->isAvailable());
    EXPECT_FALSE(m_int->isRecording());
    EXPECT_FALSE(m_int->isMultiScreenRecording());
}

// captureFrame：未录制 -> false（不触发任何信号）
TEST_F(ExtCaptureIntegrationCovTest, captureFrameGuards)
{
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    EXPECT_FALSE(m_int->captureFrame()); // 未录制
    EXPECT_EQ(errSpy.count(), 0);
}

// onFrameReady：null frame -> 仅日志，无 emit
TEST_F(ExtCaptureIntegrationCovTest, onFrameReadyNullFrameSafe)
{
    QSignalSpy frameSpy(m_int, &ExtCaptureIntegration::frameReady);
    QSignalSpy dmaSpy(m_int, &ExtCaptureIntegration::dmaFrameReady);
    QMetaObject::invokeMethod(m_int, "onFrameReady", Qt::DirectConnection,
                              Q_ARG(ExtCaptureFrame *, nullptr));
    EXPECT_EQ(frameSpy.count(), 0);
    EXPECT_EQ(dmaSpy.count(), 0);
}

// onProtocolUnavailable：非录制态 -> 仅 emit unavailable（不调 stopRecording）
TEST_F(ExtCaptureIntegrationCovTest, onProtocolUnavailableEmitsWhenIdle)
{
    QSignalSpy unavailSpy(m_int, &ExtCaptureIntegration::unavailable);
    QSignalSpy stoppedSpy(m_int, &ExtCaptureIntegration::recordingStopped);
    QMetaObject::invokeMethod(m_int, "onProtocolUnavailable", Qt::DirectConnection);
    EXPECT_EQ(unavailSpy.count(), 1);
    EXPECT_EQ(stoppedSpy.count(), 0);
}

// onSessionStopped：非录制态 -> 仅日志，不调 stopRecording
TEST_F(ExtCaptureIntegrationCovTest, onSessionStoppedIdleSafe)
{
    QSignalSpy stoppedSpy(m_int, &ExtCaptureIntegration::recordingStopped);
    QMetaObject::invokeMethod(m_int, "onSessionStopped", Qt::DirectConnection);
    EXPECT_EQ(stoppedSpy.count(), 0);
}

// onSessionError：非录制态 -> 仅 emit error（不调 stopRecording）
TEST_F(ExtCaptureIntegrationCovTest, onSessionErrorIdleEmitsOnly)
{
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    QSignalSpy stoppedSpy(m_int, &ExtCaptureIntegration::recordingStopped);
    QMetaObject::invokeMethod(m_int, "onSessionError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("e")));
    EXPECT_GE(errSpy.count(), 1);
    EXPECT_EQ(stoppedSpy.count(), 0);
}

// stopRecording：既非单屏也非多屏 -> 早退，无 emit
TEST_F(ExtCaptureIntegrationCovTest, stopRecordingIdleNoEmit)
{
    QSignalSpy stoppedSpy(m_int, &ExtCaptureIntegration::recordingStopped);
    EXPECT_NO_FATAL_FAILURE(m_int->stopRecording());
    EXPECT_EQ(stoppedSpy.count(), 0);
}

// startMultiScreenRecording：已在单屏录制态（人为无法设置，但可用 startScreenRecording 失败后）-> 失败
// 这里仅验证协议不可用路径在传入 screens 时仍失败
TEST_F(ExtCaptureIntegrationCovTest, startMultiScreenRecordingProtocolUnavailable)
{
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    EXPECT_FALSE(m_int->startMultiScreenRecording(screens, true));
    EXPECT_GE(errSpy.count(), 1);
    EXPECT_FALSE(m_int->isMultiScreenRecording());
}

// captureMultiScreenFrame：未多屏录制 -> false
TEST_F(ExtCaptureIntegrationCovTest, captureMultiScreenFrameGuard)
{
    EXPECT_FALSE(m_int->captureMultiScreenFrame());
}
