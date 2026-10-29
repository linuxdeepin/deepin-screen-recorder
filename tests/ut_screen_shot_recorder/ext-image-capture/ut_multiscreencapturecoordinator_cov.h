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
//  - onScreenFrameReady：DMA/GBM 提取失败时不得把帧错误标记为就绪
//  - onCompositeFrameReady：向上层转发合成帧
// 注意：ut_multiscreencapturecoordinator.h 与 _ext.h 已声明部分私有函数，
// 同一 TU 内不能重复 ACCESS_PRIVATE_FUN，故此处仅访问字段。
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

TEST_F(MultiScreenCaptureCoordinatorCovTest, compositeFrameReadyIsForwarded)
{
    QSignalSpy compSpy(m_c, &MultiScreenCaptureCoordinator::compositeFrameReady);
    const QByteArray frame(8, 'x');
    QMetaObject::invokeMethod(m_c, "onCompositeFrameReady", Qt::DirectConnection,
                              Q_ARG(QByteArray, frame), Q_ARG(int, 2),
                              Q_ARG(int, 1), Q_ARG(int, 8),
                              Q_ARG(uint64_t, 1234));
    ASSERT_EQ(compSpy.count(), 1);
    EXPECT_EQ(compSpy.first().at(0).toByteArray(), frame);
    EXPECT_EQ(compSpy.first().at(3).toInt(), 8);
}

TEST_F(MultiScreenCaptureCoordinatorCovTest, frameSyncTimeoutWithoutInflightIsSafe)
{
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_c, "onFrameSyncTimeout", Qt::DirectConnection));
}

// onScreenFrameReady：有 layout，fd=-1（extract 早退返回空）-> 不得标记 ready
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

    bool found = false;
    for (const auto &layout : m_c->getScreenLayouts()) {
        if (layout.screen == screen) {
            EXPECT_FALSE(layout.frameReady);
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
