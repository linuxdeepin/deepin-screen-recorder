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

// 覆盖 multiscreencapturecoordinator.cpp 未触及的安全分支：
//  - extractDmaBufferData：伪造 fd（mmap 必然失败）+ 伪造 gbmBo（gbm_bo_* 会段错误）
//    -> 仅测 fd>=0 且 gbmBo=null 的早退路径（已在 _ext 覆盖），这里补 gbmBo 伪造
//    但 gbmBo 伪造会调 gbm_bo_get_width -> 段错误，故不测该路径。
//  - checkFrameSyncAndCompose：m_capturing=true 且 allReady 且 compose 非空 -> emit compositeFrameReady
//  - onScreenFrameReady：有 layout 且 fd 有效但 gbmBo=null -> extractDmaBuffer 早退，
//    compositor.addScreenFrame 仍执行，frameReady 置位
//  - allScreenFramesReady：空 layout -> true
// 注意：ut_multiscreencapturecoordinator.h 与 _ext.h 已声明部分私有函数，
// 同一 TU 内不能重复 ACCESS_PRIVATE_FUN，故此处仅访问字段与尚未声明的私有函数。
ACCESS_PRIVATE_FUN(MultiScreenCaptureCoordinator, bool() const, allScreenFramesReady);
ACCESS_PRIVATE_FIELD(MultiScreenCaptureCoordinator, bool, m_capturing);

class MultiScreenCaptureCoordinatorCovTest : public Test
{
public:
    MultiScreenCaptureCoordinator *m_c;
    void SetUp() override { m_c = new MultiScreenCaptureCoordinator; }
    void TearDown() override { delete m_c; }

    QList<QScreen *> realScreens()
    {
        return QGuiApplication::screens();
    }
};

// allScreenFramesReady：空 layout -> true（无屏幕需就绪）
TEST_F(MultiScreenCaptureCoordinatorCovTest, allReadyWhenNoLayouts)
{
    EXPECT_TRUE(call_private_fun::MultiScreenCaptureCoordinatorallScreenFramesReady(*m_c));
}

// checkFrameSyncAndCompose：m_capturing=true，无 layout -> allReady=true，
// 但 composeMultiScreenFrame 因 virtualDesktopSize 空 -> 返回空 -> 不 emit
TEST_F(MultiScreenCaptureCoordinatorCovTest, checkSyncNoEmitWhenComposeEmpty)
{
    access_private_field::MultiScreenCaptureCoordinatorm_capturing(*m_c) = true;
    QSignalSpy compSpy(m_c, &MultiScreenCaptureCoordinator::compositeFrameReady);
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_c, "checkFrameSyncAndCompose", Qt::DirectConnection));
    EXPECT_EQ(compSpy.count(), 0);
}

// checkFrameSyncAndCompose：m_capturing=true，有 layout（frames 未就绪）-> allReady=false -> 不 emit
TEST_F(MultiScreenCaptureCoordinatorCovTest, checkSyncNoEmitWhenFramesNotReady)
{
    QList<QScreen *> screens = realScreens();
    if (screens.isEmpty()) GTEST_SKIP();
    call_private_fun::MultiScreenCaptureCoordinatorcalculateVirtualDesktopLayout(*m_c, screens);
    access_private_field::MultiScreenCaptureCoordinatorm_capturing(*m_c) = true;
    QSignalSpy compSpy(m_c, &MultiScreenCaptureCoordinator::compositeFrameReady);
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_c, "checkFrameSyncAndCompose", Qt::DirectConnection));
    EXPECT_EQ(compSpy.count(), 0);
}

// onScreenFrameReady：有 layout，fd=-1（extract 早退返回空）-> compositor 收到空 frameData
// frameData 为空时 convertDmaBuffer 走占位符，addScreenFrame 仍返回 true
TEST_F(MultiScreenCaptureCoordinatorCovTest, onScreenFrameReadyPlaceholderPath)
{
    QList<QScreen *> screens = realScreens();
    if (screens.isEmpty()) GTEST_SKIP();
    QScreen *screen = screens.first();
    call_private_fun::MultiScreenCaptureCoordinatorcalculateVirtualDesktopLayout(*m_c, screens);
    ASSERT_FALSE(m_c->getScreenLayouts().isEmpty());

    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(
        m_c, "onScreenFrameReady", Qt::DirectConnection,
        Q_ARG(QScreen *, screen), Q_ARG(int, -1), Q_ARG(void *, nullptr),
        Q_ARG(int, 4), Q_ARG(int, 4), Q_ARG(int, 16), Q_ARG(size_t, 64),
        Q_ARG(uint64_t, 9999)));

    // 对应 layout.frameReady 应被置 true
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

// stopMultiScreenCapture：m_capturing=true -> emit captureStopped 并复位
TEST_F(MultiScreenCaptureCoordinatorCovTest, stopEmitsCaptureStopped)
{
    access_private_field::MultiScreenCaptureCoordinatorm_capturing(*m_c) = true;
    QSignalSpy stopSpy(m_c, &MultiScreenCaptureCoordinator::captureStopped);
    EXPECT_NO_FATAL_FAILURE(m_c->stopMultiScreenCapture());
    EXPECT_FALSE(m_c->isCapturing());
    EXPECT_EQ(stopSpy.count(), 1);
}

// setCaptureManager：存储指针，不影响 capturing
TEST_F(MultiScreenCaptureCoordinatorCovTest, setCaptureManagerStores)
{
    EXPECT_NO_FATAL_FAILURE(m_c->setCaptureManager(nullptr));
    EXPECT_FALSE(m_c->isCapturing());
}

// getAvailableScreens：返回 QGuiApplication::screens()，offscreen 下至少 1 个
TEST_F(MultiScreenCaptureCoordinatorCovTest, getAvailableScreensNonEmpty)
{
    QList<QScreen *> screens = m_c->getAvailableScreens();
    EXPECT_GE(screens.size(), 1);
}

// 析构在 capturing 态：触发 stopMultiScreenCapture，无 crash
TEST_F(MultiScreenCaptureCoordinatorCovTest, destructWhileCapturingSafe)
{
    MultiScreenCaptureCoordinator *c = new MultiScreenCaptureCoordinator;
    access_private_field::MultiScreenCaptureCoordinatorm_capturing(*c) = true;
    EXPECT_NO_FATAL_FAILURE(delete c);
}
