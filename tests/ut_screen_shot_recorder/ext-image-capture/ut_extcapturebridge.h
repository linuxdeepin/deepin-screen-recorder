// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QMetaObject>
#include "addr_pri.h"
#include "../../src/ext-image-capture/extcapturebridge.h"
#include "../../src/ext-image-capture/extcapturerecorder.h"

using namespace testing;

// 私有纯函数：KF5_WAYLAND_FLAGE_OFF 下均为安全分支
ACCESS_PRIVATE_FUN(ExtCaptureBridge, bool(const unsigned char *, int, int, int, unsigned char *, size_t), convertFrameFormat);
ACCESS_PRIVATE_FUN(ExtCaptureBridge, int64_t(), calculateFrameTimestamp);
ACCESS_PRIVATE_FUN(ExtCaptureBridge, bool(), initializeRecordAdmin);
ACCESS_PRIVATE_FUN(ExtCaptureBridge, bool(), processSingleFrame);

class ExtCaptureBridgeTest : public Test
{
public:
    ExtCaptureBridge *m_bridge;
    void SetUp() override { m_bridge = new ExtCaptureBridge; }
    void TearDown() override { delete m_bridge; }
};

TEST_F(ExtCaptureBridgeTest, defaults)
{
    EXPECT_FALSE(m_bridge->isBridging());
    EXPECT_EQ(m_bridge->getProcessedFrameCount(), 0);
    EXPECT_TRUE(m_bridge->getStatusInfo().contains("Bridging"));
}

TEST_F(ExtCaptureBridgeTest, startBridgeWithoutComponentsFails)
{
    QSignalSpy errSpy(m_bridge, &ExtCaptureBridge::bridgeError);
    EXPECT_FALSE(m_bridge->startBridge(100, 100));
    EXPECT_GE(errSpy.count(), 1);
}

TEST_F(ExtCaptureBridgeTest, setRecorderThenStartStillFailsNoAdmin)
{
    ExtCaptureRecorder rec;
    m_bridge->setExtCaptureRecorder(&rec);
    // 缺 RecordAdmin -> 仍失败
    EXPECT_FALSE(m_bridge->startBridge(100, 100));
    // 清空 recorder
    m_bridge->setExtCaptureRecorder(nullptr);
    EXPECT_FALSE(m_bridge->isBridging());
}

TEST_F(ExtCaptureBridgeTest, stopBridgeGuard)
{
    EXPECT_NO_FATAL_FAILURE(m_bridge->stopBridge()); // 未 bridging，直接返回
}

TEST_F(ExtCaptureBridgeTest, privateHelpers)
{
    // convertFrameFormat：源大于目标 -> false；正常 -> true
    unsigned char src[16] = {1};
    unsigned char dst[32] = {0};
    EXPECT_TRUE(call_private_fun::ExtCaptureBridgeconvertFrameFormat(*m_bridge, src, 4, 4, 4, dst, 32));
    EXPECT_FALSE(call_private_fun::ExtCaptureBridgeconvertFrameFormat(*m_bridge, src, 4, 4, 4, dst, 8)); // src 16 > dst 8

    // calculateFrameTimestamp：返回非负
    EXPECT_GE(call_private_fun::ExtCaptureBridgecalculateFrameTimestamp(*m_bridge), 0);

    // initializeRecordAdmin / processSingleFrame：无 Wayland 宏 -> false
    EXPECT_FALSE(call_private_fun::ExtCaptureBridgeinitializeRecordAdmin(*m_bridge));
    EXPECT_FALSE(call_private_fun::ExtCaptureBridgeprocessSingleFrame(*m_bridge));
}

TEST_F(ExtCaptureBridgeTest, recordingSlotsViaMeta)
{
    QSignalSpy errSpy(m_bridge, &ExtCaptureBridge::bridgeError);
    // onRecordingStarted：无 RecordAdmin 初始化 -> 发 bridgeError
    QMetaObject::invokeMethod(m_bridge, "onRecordingStarted", Qt::DirectConnection);
    EXPECT_GE(errSpy.count(), 1);
    // onRecordingStopped / onRecordingError：内部调 stopBridge，安全
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_bridge, "onRecordingStopped", Qt::DirectConnection));
    QMetaObject::invokeMethod(m_bridge, "onRecordingError", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("e")));
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_bridge, "processFrames", Qt::DirectConnection));
}
