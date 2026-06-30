// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QGuiApplication>
#include <QScreen>
#include <QCoreApplication>
#include "addr_pri.h"
#include "../../src/ext-image-capture/multiscreenframecompositor.h"

using namespace testing;

// 覆盖 ut_multiscreenframecompositor.h 未触及的分支：
//  - convertDmaBufferToQImage：frameData 为空 -> 占位符路径
//  - calculateDrawRect：裁剪区域相交/不相交/平移坐标
//  - addScreenFrame：已配置布局后正常更新 + 触发 performComposition（QueuedConnection）
//  - getLatestTimestamp：取最大时间戳
ACCESS_PRIVATE_FUN(MultiScreenFrameCompositor, QImage(const DmaFrameInfo &), convertDmaBufferToQImage);
ACCESS_PRIVATE_FUN(MultiScreenFrameCompositor, QRect(const QRect &) const, calculateDrawRect);
ACCESS_PRIVATE_FUN(MultiScreenFrameCompositor, uint64_t() const, getLatestTimestamp);
ACCESS_PRIVATE_FUN(MultiScreenFrameCompositor, void(), resetFrameReadyFlags);

class MultiScreenFrameCompositorCovTest : public Test
{
public:
    MultiScreenFrameCompositor *m_c;
    void SetUp() override { m_c = new MultiScreenFrameCompositor; }
    void TearDown() override { delete m_c; }

    // 构造一帧“无 CPU 可读数据”的 DmaFrameInfo（frameData 为空 -> 走占位符路径）
    DmaFrameInfo makeEmptyFrame(QScreen *screen, const QRect &geo, int w, int h, uint64_t ts)
    {
        DmaFrameInfo f;
        f.screen = screen;
        f.screenGeometry = geo;
        f.width = w;
        f.height = h;
        f.stride = w * 4;
        f.size = static_cast<size_t>(w * h * 4);
        f.timestamp = ts;
        f.ready = false;
        // frameData 保持空
        return f;
    }
};

// convertDmaBufferToQImage：frameData 为空 -> 返回非空占位图（品红半透明）
TEST_F(MultiScreenFrameCompositorCovTest, convertDmaBufferPlaceholderWhenNoCpuData)
{
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    DmaFrameInfo f = makeEmptyFrame(screens.first(), QRect(0, 0, 4, 4), 4, 4, 1);
    QImage img = call_private_fun::MultiScreenFrameCompositorconvertDmaBufferToQImage(*m_c, f);
    EXPECT_FALSE(img.isNull());
    EXPECT_EQ(img.width(), 4);
    EXPECT_EQ(img.height(), 4);
}

// calculateDrawRect：无裁剪区域 -> 原样返回 screenGeometry
TEST_F(MultiScreenFrameCompositorCovTest, calculateDrawRectNoCropReturnsOriginal)
{
    QRect geo(10, 20, 100, 50);
    QRect r = call_private_fun::MultiScreenFrameCompositorcalculateDrawRect(*m_c, geo);
    EXPECT_EQ(r, geo);
}

// calculateDrawRect：有裁剪区域且相交 -> 平移到裁剪坐标系
TEST_F(MultiScreenFrameCompositorCovTest, calculateDrawRectCropIntersected)
{
    m_c->setCropRegion(QRect(50, 50, 200, 200));
    // screen 完全在裁剪区内
    QRect r = call_private_fun::MultiScreenFrameCompositorcalculateDrawRect(*m_c, QRect(60, 70, 100, 100));
    EXPECT_EQ(r, QRect(10, 20, 100, 100)); // 平移 (60-50, 70-50)
}

// calculateDrawRect：有裁剪区域但 screen 与裁剪区不相交 -> 空 QRect
TEST_F(MultiScreenFrameCompositorCovTest, calculateDrawRectCropNoIntersection)
{
    m_c->setCropRegion(QRect(0, 0, 10, 10));
    QRect r = call_private_fun::MultiScreenFrameCompositorcalculateDrawRect(*m_c, QRect(100, 100, 50, 50));
    EXPECT_TRUE(r.isEmpty());
}

// calculateDrawRect：裁剪区域部分相交 -> 返回相交子矩形（已平移）
TEST_F(MultiScreenFrameCompositorCovTest, calculateDrawRectCropPartialIntersection)
{
    m_c->setCropRegion(QRect(0, 0, 100, 100));
    // screen 跨越裁剪边界
    QRect r = call_private_fun::MultiScreenFrameCompositorcalculateDrawRect(*m_c, QRect(50, 50, 100, 100));
    // 相交 = (50,50,50,50)，平移后仍是 (50,50,50,50)
    EXPECT_EQ(r, QRect(50, 50, 50, 50));
}

// getLatestTimestamp：无任何就绪帧 -> 0；有多帧 -> 取最大
TEST_F(MultiScreenFrameCompositorCovTest, getLatestTimestampPicksMax)
{
    EXPECT_EQ(call_private_fun::MultiScreenFrameCompositorgetLatestTimestamp(*m_c), 0ull);

    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    QScreen *screen = screens.first();
    m_c->setScreenLayouts({screen});
    m_c->setVirtualDesktopSize(QSize(8, 8));

    // 直接通过 addScreenFrame 注入两屏同屏（同一 screen 只保留最新）
    // 这里仅验证单屏的时间戳透传
    DmaFrameInfo f = makeEmptyFrame(screen, QRect(0, 0, 8, 8), 8, 8, 4242);
    ASSERT_TRUE(m_c->addScreenFrame(f));
    // addScreenFrame 在全部就绪时会排队 performComposition，及时排空避免跨测试干扰
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_EQ(call_private_fun::MultiScreenFrameCompositorgetLatestTimestamp(*m_c), 4242ull);
}

// resetFrameReadyFlags：清空所有 ready 标记
TEST_F(MultiScreenFrameCompositorCovTest, resetFrameReadyFlagsClears)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MultiScreenFrameCompositorresetFrameReadyFlags(*m_c));
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    QScreen *screen = screens.first();
    m_c->setScreenLayouts({screen});
    m_c->setVirtualDesktopSize(QSize(8, 8));
    DmaFrameInfo f = makeEmptyFrame(screen, QRect(0, 0, 8, 8), 8, 8, 1);
    ASSERT_TRUE(m_c->addScreenFrame(f));
    EXPECT_TRUE(m_c->isCompositionReady());
    // 排空 addScreenFrame 排队的 performComposition，避免跨测试干扰
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    // resetFrameReadyFlags 在 performComposition 后调用，验证仍可清空
    call_private_fun::MultiScreenFrameCompositorresetFrameReadyFlags(*m_c);
    EXPECT_FALSE(m_c->isCompositionReady());
}

// performComposition：合成成功 -> 发 compositionComplete；合成失败（空输出） -> 发 compositionError
TEST_F(MultiScreenFrameCompositorCovTest, performCompositionEmitsOnReady)
{
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    QScreen *screen = screens.first();
    m_c->setScreenLayouts({screen});
    m_c->setVirtualDesktopSize(QSize(8, 8));

    QSignalSpy doneSpy(m_c, &MultiScreenFrameCompositor::compositionComplete);
    QSignalSpy errSpy(m_c, &MultiScreenFrameCompositor::compositionError);
    DmaFrameInfo f = makeEmptyFrame(screen, QRect(0, 0, 8, 8), 8, 8, 1);
    ASSERT_TRUE(m_c->addScreenFrame(f)); // 触发 QueuedConnection performComposition
    QCoreApplication::processEvents(QEventLoop::AllEvents, 200);
    EXPECT_GE(doneSpy.count() + errSpy.count(), 1);
}

// composeFrames：未全部就绪 -> 返回空（不进入 stitching）
TEST_F(MultiScreenFrameCompositorCovTest, composeFramesNotReadyReturnsEmpty)
{
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    m_c->setScreenLayouts({screens.first()});
    m_c->setVirtualDesktopSize(QSize(8, 8));
    EXPECT_TRUE(m_c->composeFrames().isEmpty());
}

// addScreenFrame：未配置任何布局 -> 返回 false（找不到对应 buffer）
TEST_F(MultiScreenFrameCompositorCovTest, addScreenFrameNoLayoutFails)
{
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    m_c->setVirtualDesktopSize(QSize(8, 8)); // 只设尺寸，不设布局
    DmaFrameInfo f = makeEmptyFrame(screens.first(), QRect(0, 0, 8, 8), 8, 8, 1);
    EXPECT_FALSE(m_c->addScreenFrame(f));
}
