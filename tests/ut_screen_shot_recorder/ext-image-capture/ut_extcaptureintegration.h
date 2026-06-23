// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QGuiApplication>
#include <QScreen>
#include <QMetaObject>
#include "../../src/ext-image-capture/extcaptureintegration.h"

using namespace testing;

class ExtCaptureIntegrationTest : public Test
{
public:
    ExtCaptureIntegration *m_int;
    void SetUp() override { m_int = new ExtCaptureIntegration; }
    void TearDown() override { delete m_int; }
};

TEST_F(ExtCaptureIntegrationTest, defaults)
{
    EXPECT_FALSE(m_int->isAvailable());   // 无 Wayland 协议
    EXPECT_FALSE(m_int->isRecording());
    EXPECT_FALSE(m_int->isMultiScreenRecording());
    m_int->getAvailableScreens(); // 仅触发调用
}

TEST_F(ExtCaptureIntegrationTest, startScreenRecordingRejected)
{
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    EXPECT_FALSE(m_int->startScreenRecording(nullptr));         // invalid screen
    QList<QScreen*> screens = QGuiApplication::screens();
    if (!screens.isEmpty()) {
        EXPECT_FALSE(m_int->startScreenRecording(screens.first())); // protocol not available
    }
    EXPECT_GE(errSpy.count(), 1);
}

TEST_F(ExtCaptureIntegrationTest, multiScreenAndCaptureGuards)
{
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    EXPECT_FALSE(m_int->startMultiScreenRecording({}));        // empty screens
    EXPECT_FALSE(m_int->captureFrame());                        // not recording
    EXPECT_FALSE(m_int->captureMultiScreenFrame());             // not multi recording
    EXPECT_NO_FATAL_FAILURE(m_int->stopRecording());            // guard
}

TEST_F(ExtCaptureIntegrationTest, multiScreenRecordingWithScreens)
{
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    // 有 screens 但协议不可用 -> 失败
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    EXPECT_FALSE(m_int->startMultiScreenRecording(screens, true));
    EXPECT_GE(errSpy.count(), 1);
}

TEST_F(ExtCaptureIntegrationTest, protocolSlotsViaMeta)
{
    QSignalSpy availSpy(m_int, &ExtCaptureIntegration::available);
    QSignalSpy unavailSpy(m_int, &ExtCaptureIntegration::unavailable);
    QSignalSpy startedSpy(m_int, &ExtCaptureIntegration::recordingStarted);

    QMetaObject::invokeMethod(m_int, "onProtocolAvailable", Qt::DirectConnection);
    EXPECT_EQ(availSpy.count(), 1);
    QMetaObject::invokeMethod(m_int, "onProtocolUnavailable", Qt::DirectConnection);
    EXPECT_EQ(unavailSpy.count(), 1);
    QMetaObject::invokeMethod(m_int, "onSessionReady", Qt::DirectConnection);
    EXPECT_EQ(startedSpy.count(), 1);
}

TEST_F(ExtCaptureIntegrationTest, errorAndFrameSlotsViaMeta)
{
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    QSignalSpy multiSpy(m_int, &ExtCaptureIntegration::multiScreenFrameReady);

    QMetaObject::invokeMethod(m_int, "onSessionError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("e")));
    EXPECT_GE(errSpy.count(), 1);
    QMetaObject::invokeMethod(m_int, "onFrameFailed", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("f")));
    EXPECT_GE(errSpy.count(), 2);
    // onMultiScreen* 系列
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_int, "onMultiScreenCaptureStarted", Qt::DirectConnection));
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_int, "onMultiScreenCaptureStopped", Qt::DirectConnection));
    QMetaObject::invokeMethod(m_int, "onMultiScreenCaptureError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("m")));
    QMetaObject::invokeMethod(m_int, "onMultiScreenFrameReady", Qt::DirectConnection,
                              Q_ARG(QByteArray, QByteArray("data")), Q_ARG(int, 2), Q_ARG(int, 2),
                              Q_ARG(int, 8), Q_ARG(uint64_t, 7));
    EXPECT_GE(multiSpy.count(), 1);
}
