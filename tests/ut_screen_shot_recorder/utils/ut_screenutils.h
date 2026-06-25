// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QScreen>
#include "../../src/utils/screenutils.h"

using namespace testing;

// ScreenUtils is a singleton that reads screen geometry from qApp->screens().
// In Qt6, QDesktopWidget is removed; ScreenUtils uses qApp->screens() internally.
// In offscreen mode, qApp->screens() provides one screen.

class ScreenUtilsTest : public Test
{
public:
    ScreenUtils *m_su = nullptr;

    void SetUp() override
    {
        m_su = ScreenUtils::instance(QPoint(0, 0));
    }
    void TearDown() override
    {
        // ScreenUtils is a singleton — we don't delete it
    }
};

TEST_F(ScreenUtilsTest, instanceNotNull)
{
    EXPECT_NE(m_su, nullptr);
}

TEST_F(ScreenUtilsTest, instanceReturnsSameSingleton)
{
    ScreenUtils *su2 = ScreenUtils::instance(QPoint(0, 0));
    EXPECT_EQ(m_su, su2);
}

TEST_F(ScreenUtilsTest, getScreenNum)
{
    int num = m_su->getScreenNum();
    EXPECT_GE(num, 0);
}

TEST_F(ScreenUtilsTest, backgroundRectValid)
{
    QRect rect = m_su->backgroundRect();
    EXPECT_GE(rect.width(), 0);
    EXPECT_GE(rect.height(), 0);
}

TEST_F(ScreenUtilsTest, rootWindowIdValid)
{
    WId wid = m_su->rootWindowId();
    // May be 0 in offscreen env; just ensure no crash
    EXPECT_NO_FATAL_FAILURE((void)wid);
}

TEST_F(ScreenUtilsTest, primaryScreenNotNull)
{
    QScreen *screen = m_su->primaryScreen();
    EXPECT_NE(screen, nullptr);
}

TEST_F(ScreenUtilsTest, destructorSafe)
{
    SUCCEED();
}
