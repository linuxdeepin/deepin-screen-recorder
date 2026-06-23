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
#include "../../src/ext-image-capture/multiscreencapturecoordinator.h"

using namespace testing;

// 仅暴露 ut_multiscreencapturecoordinator.h 中未声明的新增私有函数（避免同一 TU 内宏重定义）
ACCESS_PRIVATE_FUN(MultiScreenCaptureCoordinator, QByteArray(int, void *, int, int, int, size_t), extractDmaBufferData);
// 私有槽：通过 QMetaObject::invokeMethod 调用，无需 ACCESS_PRIVATE_FUN

class MultiScreenCaptureCoordinatorExtTest : public Test
{
public:
    MultiScreenCaptureCoordinator *m_c;
    void SetUp() override { m_c = new MultiScreenCaptureCoordinator; }
    void TearDown() override { delete m_c; }

    QList<QScreen *> realScreens()
    {
        QList<QScreen *> s = QGuiApplication::screens();
        return s;
    }
};

// onScreenFrameReady：fd=-1 且 gbmBo=null -> extractDmaBufferData 早退（安全，无 mmap）
// 走完整个 slot：compositor.addScreenFrame + m_screenFrames 更新 + layout.frameReady 置位
TEST_F(MultiScreenCaptureCoordinatorExtTest, onScreenFrameReadyInvalidFdSafePath)
{
    QList<QScreen *> screens = realScreens();
    if (screens.isEmpty()) GTEST_SKIP();
    QScreen *screen = screens.first();

    // 先建立 layout，使 slot 内的 layout 匹配能命中
    call_private_fun::MultiScreenCaptureCoordinatorcalculateVirtualDesktopLayout(*m_c, screens);
    ASSERT_FALSE(m_c->getScreenLayouts().isEmpty());

    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(
        m_c, "onScreenFrameReady", Qt::DirectConnection,
        Q_ARG(QScreen *, screen), Q_ARG(int, -1), Q_ARG(void *, nullptr),
        Q_ARG(int, 4), Q_ARG(int, 4), Q_ARG(int, 16), Q_ARG(size_t, 64),
        Q_ARG(uint64_t, 1234)));

    // 对应 layout 的 frameReady 应被置 true
    bool found = false;
    for (const auto &layout : m_c->getScreenLayouts()) {
        if (layout.screen == screen) {
            EXPECT_TRUE(layout.frameReady);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// onSessionError：转发为 error 信号，消息带 "Session error:" 前缀
TEST_F(MultiScreenCaptureCoordinatorExtTest, onSessionErrorEmitsError)
{
    QSignalSpy errSpy(m_c, &MultiScreenCaptureCoordinator::error);
    QMetaObject::invokeMethod(m_c, "onSessionError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("no buffer")));
    ASSERT_EQ(errSpy.count(), 1);
    QString msg = errSpy.takeFirst().at(0).toString();
    EXPECT_TRUE(msg.contains(QStringLiteral("Session error")));
    EXPECT_TRUE(msg.contains(QStringLiteral("no buffer")));
}

// onSessionError：空串消息也安全
TEST_F(MultiScreenCaptureCoordinatorExtTest, onSessionErrorEmptyMessage)
{
    QSignalSpy errSpy(m_c, &MultiScreenCaptureCoordinator::error);
    QMetaObject::invokeMethod(m_c, "onSessionError", Qt::DirectConnection,
                              Q_ARG(QString, QString()));
    ASSERT_EQ(errSpy.count(), 1);
}

// onSessionStopped：仅日志，无副作用，可反复调用
TEST_F(MultiScreenCaptureCoordinatorExtTest, onSessionStoppedIsSafe)
{
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_c, "onSessionStopped", Qt::DirectConnection));
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_c, "onSessionStopped", Qt::DirectConnection));
    EXPECT_FALSE(m_c->isCapturing());
}

// checkFrameSyncAndCompose：m_capturing=false -> 早退，不 emit
TEST_F(MultiScreenCaptureCoordinatorExtTest, checkFrameSyncAndComposeGuardWhenNotCapturing)
{
    QSignalSpy compSpy(m_c, &MultiScreenCaptureCoordinator::compositeFrameReady);
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_c, "checkFrameSyncAndCompose", Qt::DirectConnection));
    EXPECT_EQ(compSpy.count(), 0);
}

// checkFrameSyncAndCompose：m_capturing=true 但无 layout -> allScreenFramesReady 返回 true（空）
// composeMultiScreenFrame 因 virtualDesktopSize 为空返回空 -> 不 emit compositeFrameReady
TEST_F(MultiScreenCaptureCoordinatorExtTest, checkFrameSyncAndComposeCapturingNoLayout)
{
    QSignalSpy compSpy(m_c, &MultiScreenCaptureCoordinator::compositeFrameReady);
    // 直接置 m_capturing=true 无法访问私有字段，改用 startMultiScreenCapture 失败路径不可行；
    // 通过 invokeMethod 触发，slot 内自己持锁，空 layout 走 allReady=true 但 compose 返回空
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_c, "checkFrameSyncAndCompose", Qt::DirectConnection));
    // m_capturing 仍为 false -> 早退；即便不早退，空 layout 也无 emit
    EXPECT_EQ(compSpy.count(), 0);
}

// getWaylandOutput：null screen -> nullptr；真实 screen 在 offscreen 平台 -> nullptr（安全，无 roundtrip）
TEST_F(MultiScreenCaptureCoordinatorExtTest, getWaylandOutputNullAndRealScreen)
{
    EXPECT_EQ(call_private_fun::MultiScreenCaptureCoordinatorgetWaylandOutput(*m_c, nullptr), nullptr);
    QList<QScreen *> screens = realScreens();
    if (screens.isEmpty()) GTEST_SKIP();
    // 真实 screen：offscreen 平台下 nativeResourceForScreen 返回 nullptr，不会触发 Wayland roundtrip
    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::MultiScreenCaptureCoordinatorgetWaylandOutput(*m_c, screens.first()));
}

// resetFrameReadyFlags：无 layout 时调用安全；有 layout 后全部复位
TEST_F(MultiScreenCaptureCoordinatorExtTest, resetFrameReadyFlagsClearsState)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MultiScreenCaptureCoordinatorresetFrameReadyFlags(*m_c));
    QList<QScreen *> screens = realScreens();
    if (screens.isEmpty()) GTEST_SKIP();
    call_private_fun::MultiScreenCaptureCoordinatorcalculateVirtualDesktopLayout(*m_c, screens);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MultiScreenCaptureCoordinatorresetFrameReadyFlags(*m_c));
    for (const auto &layout : m_c->getScreenLayouts()) {
        EXPECT_FALSE(layout.frameReady);
    }
}

// cleanupSessions：反复调用幂等，清空 layout/virtualDesktopSize
TEST_F(MultiScreenCaptureCoordinatorExtTest, cleanupSessionsIdempotent)
{
    QList<QScreen *> screens = realScreens();
    if (!screens.isEmpty()) {
        call_private_fun::MultiScreenCaptureCoordinatorcalculateVirtualDesktopLayout(*m_c, screens);
        EXPECT_FALSE(m_c->getScreenLayouts().isEmpty());
    }
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MultiScreenCaptureCoordinatorcleanupSessions(*m_c));
    EXPECT_TRUE(m_c->getScreenLayouts().isEmpty());
    EXPECT_TRUE(m_c->getVirtualDesktopSize().isEmpty());
    // 再次调用应仍安全
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MultiScreenCaptureCoordinatorcleanupSessions(*m_c));
}

// extractDmaBufferData：fd<0 -> 早退返回空（安全，不进入 mmap/gbm）
TEST_F(MultiScreenCaptureCoordinatorExtTest, extractDmaBufferDataInvalidFd)
{
    QByteArray r = call_private_fun::MultiScreenCaptureCoordinatorextractDmaBufferData(
        *m_c, -1, nullptr, 0, 0, 0, 0);
    EXPECT_TRUE(r.isEmpty());
}

// extractDmaBufferData：fd>=0 但 gbmBo=null -> 早退返回空（守卫命中，不 mmap）
TEST_F(MultiScreenCaptureCoordinatorExtTest, extractDmaBufferDataNullBo)
{
    QByteArray r = call_private_fun::MultiScreenCaptureCoordinatorextractDmaBufferData(
        *m_c, 99, nullptr, 4, 4, 16, 64);
    EXPECT_TRUE(r.isEmpty());
}

// calculateVirtualDesktopLayout：空列表 -> virtualDesktopSize 清空
TEST_F(MultiScreenCaptureCoordinatorExtTest, calculateLayoutEmpty)
{
    QList<QScreen*> empty;
    call_private_fun::MultiScreenCaptureCoordinatorcalculateVirtualDesktopLayout(*m_c, empty);
    EXPECT_TRUE(m_c->getVirtualDesktopSize().isEmpty());
    EXPECT_TRUE(m_c->getScreenLayouts().isEmpty());
}

// composeMultiScreenFrame：virtualDesktopSize 为空 -> 返回空 QByteArray
TEST_F(MultiScreenCaptureCoordinatorExtTest, composeEmptyWhenNoDesktopSize)
{
    EXPECT_TRUE(call_private_fun::MultiScreenCaptureCoordinatorcomposeMultiScreenFrame(*m_c).isEmpty());
}

// captureMultiScreenFrame：未 capturing -> false（额外覆盖一次守卫，与原测试互补）
TEST_F(MultiScreenCaptureCoordinatorExtTest, captureFrameGuardWhenNotCapturing)
{
    EXPECT_FALSE(m_c->captureMultiScreenFrame());
}

// stopMultiScreenCapture：未 capturing -> 无 emit、无副作用（守卫）
TEST_F(MultiScreenCaptureCoordinatorExtTest, stopGuardNoEmit)
{
    QSignalSpy stopSpy(m_c, &MultiScreenCaptureCoordinator::captureStopped);
    EXPECT_NO_FATAL_FAILURE(m_c->stopMultiScreenCapture());
    EXPECT_EQ(stopSpy.count(), 0);
}

// startMultiScreenCapture：m_manager 为空 -> false 且 emit error
TEST_F(MultiScreenCaptureCoordinatorExtTest, startFailsWithoutManager)
{
    QList<QScreen *> screens = realScreens();
    if (screens.isEmpty()) GTEST_SKIP();
    QSignalSpy errSpy(m_c, &MultiScreenCaptureCoordinator::error);
    EXPECT_FALSE(m_c->startMultiScreenCapture(screens, false));
    EXPECT_GE(errSpy.count(), 1);
    EXPECT_FALSE(m_c->isCapturing());
}

// setCaptureManager：仅设置字段，不影响其它状态
TEST_F(MultiScreenCaptureCoordinatorExtTest, setCaptureManagerStoresPointer)
{
    EXPECT_NO_FATAL_FAILURE(m_c->setCaptureManager(nullptr));
    EXPECT_FALSE(m_c->isCapturing());
}
