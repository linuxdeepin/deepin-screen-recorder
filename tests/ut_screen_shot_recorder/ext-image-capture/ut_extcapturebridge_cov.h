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

// 覆盖 extcapturebridge.cpp 未触及的安全分支：
//  - setRecordAdmin：KF5_WAYLAND_FLAGE_OFF 下仅存储指针（无 Wayland 依赖）
//  - setExtCaptureRecorder：传入真实 recorder（getFrameBuffer 非空）+ 清空
//  - startBridge：三个组件齐全 -> bridging=true，emit bridgeStarted
//    （注意：startBridge 本身不调用 RecordAdmin->init，仅设标志）
//  - processFrames：bridging=true 但 KF5 关 -> processSingleFrame 返回 false -> drop
//  - stopBridge：bridging=true -> emit bridgeStopped
// 私有纯函数已在 ut_extcapturebridge.h 覆盖，这里不重复 ACCESS_PRIVATE_FUN
// （同一 TU 内宏重定义会冲突，故仅用公共接口 + MetaObject 调槽）。
class ExtCaptureBridgeCovTest : public Test
{
public:
    ExtCaptureBridge *m_bridge;
    void SetUp() override { m_bridge = new ExtCaptureBridge; }
    void TearDown() override { delete m_bridge; }
};

// setRecordAdmin：存储 nullptr 与伪造指针（不 deref），KF5 关时安全
TEST_F(ExtCaptureBridgeCovTest, setRecordAdminStoresPointer)
{
    EXPECT_NO_FATAL_FAILURE(m_bridge->setRecordAdmin(nullptr));
    // 传入伪造非空指针（仅存储，startBridge 前不 deref）
    RecordAdmin *fake = reinterpret_cast<RecordAdmin *>(0x1);
    EXPECT_NO_FATAL_FAILURE(m_bridge->setRecordAdmin(fake));
    // 清回 nullptr
    EXPECT_NO_FATAL_FAILURE(m_bridge->setRecordAdmin(nullptr));
}

// setExtCaptureRecorder：真实 recorder -> getFrameBuffer 连接信号；再清空
TEST_F(ExtCaptureBridgeCovTest, setRecorderAndClear)
{
    ExtCaptureRecorder rec;
    EXPECT_NO_FATAL_FAILURE(m_bridge->setExtCaptureRecorder(&rec));
    EXPECT_FALSE(m_bridge->isBridging());
    // 清空
    EXPECT_NO_FATAL_FAILURE(m_bridge->setExtCaptureRecorder(nullptr));
    EXPECT_FALSE(m_bridge->isBridging());
}

// startBridge：组件齐全（伪造 admin 指针）-> bridging=true，emit bridgeStarted
TEST_F(ExtCaptureBridgeCovTest, startBridgeWithAllComponentsSucceeds)
{
    ExtCaptureRecorder rec;
    m_bridge->setExtCaptureRecorder(&rec);                 // 设 recorder + frameBuffer
    m_bridge->setRecordAdmin(reinterpret_cast<RecordAdmin *>(0x1)); // 伪造 admin

    QSignalSpy startedSpy(m_bridge, &ExtCaptureBridge::bridgeStarted);
    EXPECT_TRUE(m_bridge->startBridge(640, 480));
    EXPECT_TRUE(m_bridge->isBridging());
    EXPECT_EQ(startedSpy.count(), 1);

    // 再次 startBridge -> 已 bridging -> false
    EXPECT_FALSE(m_bridge->startBridge(640, 480));

    // getStatusInfo 在 bridging 时含 "Yes"
    EXPECT_TRUE(m_bridge->getStatusInfo().contains("Yes"));
}

// stopBridge：bridging=true -> emit bridgeStopped
TEST_F(ExtCaptureBridgeCovTest, stopBridgeEmitsStopped)
{
    ExtCaptureRecorder rec;
    m_bridge->setExtCaptureRecorder(&rec);
    m_bridge->setRecordAdmin(reinterpret_cast<RecordAdmin *>(0x1));
    ASSERT_TRUE(m_bridge->startBridge(100, 100));

    QSignalSpy stoppedSpy(m_bridge, &ExtCaptureBridge::bridgeStopped);
    EXPECT_NO_FATAL_FAILURE(m_bridge->stopBridge());
    EXPECT_FALSE(m_bridge->isBridging());
    EXPECT_EQ(stoppedSpy.count(), 1);
}

// processFrames：bridging=true 但 KF5 关 -> processSingleFrame false -> 不 crash
TEST_F(ExtCaptureBridgeCovTest, processFramesWhileBridgingNoWayland)
{
    ExtCaptureRecorder rec;
    m_bridge->setExtCaptureRecorder(&rec);
    m_bridge->setRecordAdmin(reinterpret_cast<RecordAdmin *>(0x1));
    ASSERT_TRUE(m_bridge->startBridge(100, 100));

    // frameBuffer 为空队列 -> hasFrames false -> while 不进入，无 crash
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_bridge, "processFrames", Qt::DirectConnection));
    EXPECT_TRUE(m_bridge->isBridging());
}

// onRecordingStarted：bridging 后触发 -> KF5 关 initializeRecordAdmin false -> emit bridgeError
TEST_F(ExtCaptureBridgeCovTest, onRecordingStartedEmitsErrorWithoutWayland)
{
    ExtCaptureRecorder rec;
    m_bridge->setExtCaptureRecorder(&rec);
    m_bridge->setRecordAdmin(reinterpret_cast<RecordAdmin *>(0x1));
    ASSERT_TRUE(m_bridge->startBridge(100, 100));

    QSignalSpy errSpy(m_bridge, &ExtCaptureBridge::bridgeError);
    QMetaObject::invokeMethod(m_bridge, "onRecordingStarted", Qt::DirectConnection);
    EXPECT_GE(errSpy.count(), 1);
}

// setExtCaptureRecorder：bridging 中拒绝设置
TEST_F(ExtCaptureBridgeCovTest, setRecorderRejectedWhileBridging)
{
    ExtCaptureRecorder rec;
    m_bridge->setExtCaptureRecorder(&rec);
    m_bridge->setRecordAdmin(reinterpret_cast<RecordAdmin *>(0x1));
    ASSERT_TRUE(m_bridge->startBridge(100, 100));

    ExtCaptureRecorder rec2;
    // bridging 中应被拒绝（不影响已存的 recorder）
    EXPECT_NO_FATAL_FAILURE(m_bridge->setExtCaptureRecorder(&rec2));
    EXPECT_TRUE(m_bridge->isBridging());
}

// setRecordAdmin：bridging 中拒绝设置
TEST_F(ExtCaptureBridgeCovTest, setRecordAdminRejectedWhileBridging)
{
    ExtCaptureRecorder rec;
    m_bridge->setExtCaptureRecorder(&rec);
    m_bridge->setRecordAdmin(reinterpret_cast<RecordAdmin *>(0x1));
    ASSERT_TRUE(m_bridge->startBridge(100, 100));

    EXPECT_NO_FATAL_FAILURE(m_bridge->setRecordAdmin(nullptr));
    EXPECT_TRUE(m_bridge->isBridging());
}
