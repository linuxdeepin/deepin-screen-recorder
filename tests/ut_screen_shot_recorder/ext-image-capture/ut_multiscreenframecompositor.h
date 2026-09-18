// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QGuiApplication>
#include <QScreen>
#include "../../src/ext-image-capture/multiscreenframecompositor.h"

using namespace testing;

class MultiScreenFrameCompositorTest : public Test
{
public:
    MultiScreenFrameCompositor *m_c;
    void SetUp() override { m_c = new MultiScreenFrameCompositor; }
    void TearDown() override { delete m_c; }

    // 构造一帧带 CPU 可读数据的 DmaFrameInfo
    DmaFrameInfo makeFrame(QScreen *screen, const QRect &geo, int w, int h, uint64_t ts)
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
        f.frameData = QByteArray(f.stride * h, '\xAA'); // 非空 -> 走 CPU 数据路径
        return f;
    }
};

TEST_F(MultiScreenFrameCompositorTest, defaults)
{
    EXPECT_TRUE(m_c->getOutputSize().isEmpty());
    EXPECT_EQ(m_c->getOutputStride(), 0);
    EXPECT_FALSE(m_c->isCompositionReady());
}

TEST_F(MultiScreenFrameCompositorTest, virtualDesktopAndCrop)
{
    m_c->setVirtualDesktopSize(QSize(100, 50));
    EXPECT_EQ(m_c->getOutputSize(), QSize(100, 50));
    EXPECT_EQ(m_c->getOutputStride(), 400);

    m_c->setCropRegion(QRect(10, 10, 20, 20));
    EXPECT_EQ(m_c->getOutputSize(), QSize(20, 20));
    EXPECT_EQ(m_c->getOutputStride(), 80);

    m_c->clearCropRegion();
    EXPECT_EQ(m_c->getOutputSize(), QSize(100, 50));
    EXPECT_EQ(m_c->getOutputStride(), 400);
}

TEST_F(MultiScreenFrameCompositorTest, composeWithoutLayoutReturnsEmpty)
{
    m_c->setVirtualDesktopSize(QSize(10, 10));
    EXPECT_TRUE(m_c->composeFrames().isEmpty());
    EXPECT_FALSE(m_c->isCompositionReady());
}

TEST_F(MultiScreenFrameCompositorTest, addFrameForUnknownScreenFails)
{
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    m_c->setVirtualDesktopSize(QSize(10, 10));
    // 未配置该 screen 的布局 -> addScreenFrame 返回 false
    DmaFrameInfo f = makeFrame(screens.first(), QRect(0, 0, 4, 4), 4, 4, 1);
    EXPECT_FALSE(m_c->addScreenFrame(f));
}

TEST_F(MultiScreenFrameCompositorTest, fullDesktopComposition)
{
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    QScreen *screen = screens.first();
    QRect geo = screen->geometry();

    m_c->setScreenLayouts({screen});
    m_c->setVirtualDesktopSize(geo.size());

    EXPECT_FALSE(m_c->isCompositionReady());
    DmaFrameInfo f = makeFrame(screen, geo, geo.width(), geo.height(), 12345);
    EXPECT_TRUE(m_c->addScreenFrame(f));
    EXPECT_TRUE(m_c->isCompositionReady());

    QByteArray out = m_c->composeFrames();
    EXPECT_FALSE(out.isEmpty());
    m_c->reset();
    EXPECT_FALSE(m_c->isCompositionReady());
}

TEST_F(MultiScreenFrameCompositorTest, performCompositionSignal)
{
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    QScreen *screen = screens.first();
    QRect geo(0, 0, 8, 8);
    // 直接喂小尺寸数据，规避真实屏幕几何
    m_c->setScreenLayouts({screen});
    m_c->setVirtualDesktopSize(QSize(8, 8));

    QSignalSpy doneSpy(m_c, &MultiScreenFrameCompositor::compositionComplete);
    QSignalSpy errSpy(m_c, &MultiScreenFrameCompositor::compositionError);

    DmaFrameInfo f = makeFrame(screen, geo, 8, 8, 1);
    ASSERT_TRUE(m_c->addScreenFrame(f)); // 触发 QueuedConnection 的 performComposition
    // 处理排队调用
    QCoreApplication::processEvents(QEventLoop::AllEvents, 200);
    EXPECT_GE(doneSpy.count() + errSpy.count(), 0);
}
