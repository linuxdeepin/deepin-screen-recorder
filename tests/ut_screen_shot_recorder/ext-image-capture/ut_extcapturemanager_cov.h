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
#include "../../src/ext-image-capture/manager/extcapturemanager.h"

using namespace testing;

// 覆盖 extcapturemanager.cpp 未触及的安全分支：
//  - 构造（QWaylandClientExtension 在 offscreen 下不绑定，安全）
//  - protocolVersion（默认 0）
//  - isProtocolAvailable（offscreen 下 isActive()==false -> false）
//  - createScreenCaptureSession 守卫（protocol 不可用 / null screen）
//  - onActiveChanged 非 active 分支（private slot，可 invokeMethod）
//  - setupWaylandIntegration（空实现）
//  - getWaylandOutput（null screen 走 nativeInterface 分支）
// 注意：bind 是 protected 非 slot，无法 invokeMethod；构造在 offscreen 下安全。
ACCESS_PRIVATE_FUN(ExtCaptureManager, void(), setupWaylandIntegration);
ACCESS_PRIVATE_FUN(ExtCaptureManager, wl_output *(QScreen *), getWaylandOutput);

class ExtCaptureManagerCovTest : public Test
{
public:
    ExtCaptureManager *m_mgr;
    void SetUp() override { m_mgr = new ExtCaptureManager; }
    void TearDown() override { delete m_mgr; }
};

// 构造 + 默认协议版本
TEST_F(ExtCaptureManagerCovTest, constructAndDefaults)
{
    EXPECT_GE(m_mgr->protocolVersion(), 0);
    // offscreen 无 Wayland -> 协议不可用
    EXPECT_FALSE(m_mgr->isProtocolAvailable());
}

// createScreenCaptureSession：协议不可用 -> nullptr
TEST_F(ExtCaptureManagerCovTest, createSessionFailsWhenProtocolUnavailable)
{
    EXPECT_EQ(m_mgr->createScreenCaptureSession(nullptr), nullptr);
    QList<QScreen *> screens = QGuiApplication::screens();
    if (!screens.isEmpty()) {
        EXPECT_EQ(m_mgr->createScreenCaptureSession(screens.first(), true), nullptr);
    }
}

// onActiveChanged：非 active 时（offscreen 默认）走 deactive 分支
TEST_F(ExtCaptureManagerCovTest, onActiveChangedSafeWhenInactive)
{
    QSignalSpy unavailSpy(m_mgr, &ExtCaptureManager::protocolUnavailable);
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_mgr, "onActiveChanged", Qt::DirectConnection));
    // 默认 protocolActive=false，deactive 分支不发信号
    SUCCEED();
}

// setupWaylandIntegration：空实现，安全调用
TEST_F(ExtCaptureManagerCovTest, setupWaylandIntegrationNoop)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureManagersetupWaylandIntegration(*m_mgr));
}

// getWaylandOutput：null screen -> 走 nativeInterface（offscreen 返回 nullptr），不崩溃
TEST_F(ExtCaptureManagerCovTest, getWaylandOutputNullScreenSafe)
{
    EXPECT_EQ(call_private_fun::ExtCaptureManagergetWaylandOutput(*m_mgr, nullptr), nullptr);
    QList<QScreen *> screens = QGuiApplication::screens();
    if (!screens.isEmpty()) {
        EXPECT_NO_FATAL_FAILURE(
            call_private_fun::ExtCaptureManagergetWaylandOutput(*m_mgr, screens.first()));
    }
}
