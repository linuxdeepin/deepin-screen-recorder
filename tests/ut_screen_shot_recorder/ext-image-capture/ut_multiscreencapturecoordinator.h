// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QGuiApplication>
#include <QScreen>
#include "addr_pri.h"
#include "../../src/ext-image-capture/multiscreencapturecoordinator.h"

using namespace testing;

// private 辅助：纯几何/图案逻辑
ACCESS_PRIVATE_FUN(MultiScreenCaptureCoordinator, void(const QList<QScreen *> &), calculateVirtualDesktopLayout);
ACCESS_PRIVATE_FUN(MultiScreenCaptureCoordinator, QByteArray(), composeMultiScreenFrame);
ACCESS_PRIVATE_FUN(MultiScreenCaptureCoordinator, void(), resetFrameReadyFlags);
ACCESS_PRIVATE_FUN(MultiScreenCaptureCoordinator, void(), cleanupSessions);
ACCESS_PRIVATE_FUN(MultiScreenCaptureCoordinator, void *(QScreen *), getWaylandOutput);

class MultiScreenCaptureCoordinatorTest : public Test
{
public:
    MultiScreenCaptureCoordinator *m_c;
    void SetUp() override { m_c = new MultiScreenCaptureCoordinator; }
    void TearDown() override { delete m_c; }
};

TEST_F(MultiScreenCaptureCoordinatorTest, defaults)
{
    EXPECT_FALSE(m_c->isCapturing());
    EXPECT_TRUE(m_c->getVirtualDesktopSize().isEmpty());
    EXPECT_TRUE(m_c->getScreenLayouts().isEmpty());
    m_c->getAvailableScreens(); // 仅触发调用
}

TEST_F(MultiScreenCaptureCoordinatorTest, startGuards)
{
    QSignalSpy errSpy(m_c, &MultiScreenCaptureCoordinator::error);
    EXPECT_FALSE(m_c->startMultiScreenCapture({}, false));   // empty screens
    QList<QScreen*> screens = QGuiApplication::screens();
    if (!screens.isEmpty()) {
        EXPECT_FALSE(m_c->startMultiScreenCapture(screens, false)); // 无 manager
    }
    EXPECT_FALSE(m_c->captureMultiScreenFrame());             // not capturing
    EXPECT_NO_FATAL_FAILURE(m_c->stopMultiScreenCapture());   // guard
}

TEST_F(MultiScreenCaptureCoordinatorTest, composePlaceholderWhenEmpty)
{
    // virtualDesktopSize 未设 -> 返回空
    EXPECT_TRUE(call_private_fun::MultiScreenCaptureCoordinatorcomposeMultiScreenFrame(*m_c).isEmpty());
}

TEST_F(MultiScreenCaptureCoordinatorTest, layoutAndCompose)
{
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    call_private_fun::MultiScreenCaptureCoordinatorcalculateVirtualDesktopLayout(*m_c, screens);
    EXPECT_FALSE(m_c->getVirtualDesktopSize().isEmpty());
    EXPECT_EQ(m_c->getScreenLayouts().size(), screens.size());

    QByteArray frame = call_private_fun::MultiScreenCaptureCoordinatorcomposeMultiScreenFrame(*m_c);
    EXPECT_FALSE(frame.isEmpty());
    EXPECT_EQ(frame.size(), m_c->getVirtualDesktopSize().width() *
                            m_c->getVirtualDesktopSize().height() * 4);
}

TEST_F(MultiScreenCaptureCoordinatorTest, frameReadyFlags)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MultiScreenCaptureCoordinatorresetFrameReadyFlags(*m_c));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MultiScreenCaptureCoordinatorcleanupSessions(*m_c));
    EXPECT_EQ(call_private_fun::MultiScreenCaptureCoordinatorgetWaylandOutput(*m_c, nullptr), nullptr);
}
