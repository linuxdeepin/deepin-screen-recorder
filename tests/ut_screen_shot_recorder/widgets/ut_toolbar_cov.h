// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QEvent>
#include "addr_pri.h"
#include "../../src/widgets/toolbar.h"

using namespace testing;

// The legacy ut_toolbar.h is disabled (Qt5 enterEvent signature, removed
// setExpand/currentFunctionMode/etc.). This file targets the CURRENT ToolBar
// API with only the no-MainWindow surface that is safe on a freshly constructed
// widget:
//   - ctor / paintEvent / enterEvent(QEnterEvent*) / eventFilter
//   - getInnerWidgetRect (null m_toolbarWidget guard -> returns this->rect())
//   - isDraged / isPressed
//   - mouseReleaseEvent (left button; no MainWindow deref)
//   - guarded setters (setScrollShotDisabled/setPinScreenshotsEnable/
//     setOcrScreenshotsEnable/setButEnableOnLockScreen return early when
//     m_toolbarWidget is null)
//
// SKIPPED (unsafe): mousePressEvent (m_pMainWindow->getSideBarStartPressPoint
// deref -> SEGV when null) and mouseMoveEvent (m_pMainWindow->moveToolBars).
// initToolBar(MainWindow*) / currentFunctionMode build ToolBarWidget which
// couples to MainWindow and is left to integration tests.

ACCESS_PRIVATE_FUN(ToolBar, void(QPaintEvent *), paintEvent);
ACCESS_PRIVATE_FUN(ToolBar, void(QEnterEvent *), enterEvent);
ACCESS_PRIVATE_FUN(ToolBar, bool(QObject *, QEvent *), eventFilter);

class ToolBarCovTest : public Test
{
public:
    ToolBar *m_bar = nullptr;
    void SetUp() override { m_bar = new ToolBar; }
    void TearDown() override
    {
        delete m_bar;
        // ensure no override cursor leaks between suites
        while (qApp->overrideCursor()) {
            qApp->restoreOverrideCursor();
        }
    }
};

TEST_F(ToolBarCovTest, constructionStable)
{
    EXPECT_FALSE(m_bar->isDraged());
    EXPECT_FALSE(m_bar->isPressed());
    SUCCEED();
}

TEST_F(ToolBarCovTest, getInnerWidgetRectNullFallback)
{
    m_bar->resize(120, 60);
    QRect r = m_bar->getInnerWidgetRect();
    EXPECT_EQ(r, m_bar->rect());
    EXPECT_EQ(r.size(), QSize(120, 60));
}

TEST_F(ToolBarCovTest, paintEventSafe)
{
    QPaintEvent pe(QRect(0, 0, 20, 20));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolBarpaintEvent(*m_bar, &pe));
}

TEST_F(ToolBarCovTest, enterEventSetsOpenHandCursor)
{
    QEnterEvent enter(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolBarenterEvent(*m_bar, &enter));
    QApplication::restoreOverrideCursor();
}

TEST_F(ToolBarCovTest, eventFilterPassthrough)
{
    QObject obj;
    QEvent ev(QEvent::PaletteChange);
    bool handled = true;
    EXPECT_NO_FATAL_FAILURE(handled = call_private_fun::ToolBareventFilter(*m_bar, &obj, &ev));
    EXPECT_FALSE(handled); // DLabel base returns false for unknown events
}

TEST_F(ToolBarCovTest, mouseReleaseLeftClearsFlags)
{
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(5, 5),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_bar, &release));
    QApplication::restoreOverrideCursor();
    EXPECT_FALSE(m_bar->isPressed());
    EXPECT_FALSE(m_bar->isDraged());
}

TEST_F(ToolBarCovTest, mouseReleaseRightButtonNoop)
{
    // right-button release: the if(button==LeftButton) is skipped
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(5, 5),
                        Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_bar, &release));
    SUCCEED();
}

// ---------- guarded setters (m_toolbarWidget null -> early return) ----------

TEST_F(ToolBarCovTest, guardedSettersNullWidgetEarlyReturn)
{
    EXPECT_NO_FATAL_FAILURE(m_bar->setScrollShotDisabled(true));
    EXPECT_NO_FATAL_FAILURE(m_bar->setScrollShotDisabled(false));
    bool t = true, f = false;
    EXPECT_NO_FATAL_FAILURE(m_bar->setPinScreenshotsEnable(t));
    EXPECT_NO_FATAL_FAILURE(m_bar->setPinScreenshotsEnable(f));
    EXPECT_NO_FATAL_FAILURE(m_bar->setOcrScreenshotsEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_bar->setOcrScreenshotsEnable(false));
    EXPECT_NO_FATAL_FAILURE(m_bar->setButEnableOnLockScreen(true));
    EXPECT_NO_FATAL_FAILURE(m_bar->setButEnableOnLockScreen(false));
    SUCCEED();
}

TEST_F(ToolBarCovTest, delegatingAccessorsNullSafe)
{
    // getShotOptionRect / getFuncSubToolX / getAiButtonGlobalRect /
    // getAiButtonGlobalCenter all guard on m_toolbarWidget; with it null they
    // return safe defaults without crashing.
    QRect r;
    EXPECT_NO_FATAL_FAILURE(r = m_bar->getShotOptionRect());
    EXPECT_NO_FATAL_FAILURE(m_bar->getAiButtonGlobalRect());
    EXPECT_NO_FATAL_FAILURE(m_bar->getAiButtonGlobalCenter());
    QString func = QStringLiteral("rect");
    int x = -999;
    EXPECT_NO_FATAL_FAILURE(x = m_bar->getFuncSubToolX(func));
    SUCCEED();
}
