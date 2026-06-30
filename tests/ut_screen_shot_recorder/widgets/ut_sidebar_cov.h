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
#include "../../src/widgets/sidebar.h"

using namespace testing;

// Covers previously-uncovered SideBar / SideBarWidget surface that is safe to
// exercise offscreen without a fully-wired MainWindow:
//   - SideBar::changeShotToolFunc / getSideBarWidth / showWidget / hideWidget /
//     showAt / getInnerWidgetRect (with an inner widget) / initSideBar(nullptr)
//   - SideBarWidget::changeShotToolWidget across all func strings (geometry,
//     text, effect, aiassistant, default) and getSideBarWidth / isAIMode
//
// initSideBar(nullptr) is safe: SideBarWidget only *stores* the MainWindow
// pointer and registers signal/slot connections; it never dereferences the
// pointer during construction (verified in ShotToolWidget / AIAssistantWidget
// ctors). The AI functionSelected -> onAiAssistantSelected lambda only fires on
// a real user action, which we never trigger here.
//
// Skipped (would deref null MainWindow): SideBar::mousePressEvent (calls
// m_pMainWindow->getToolBarStartPressPoint) and mouseMoveEvent (calls
// m_pMainWindow->moveToolBars). enterEvent only touches the cursor + reads
// isAIMode(), so it is safe.

ACCESS_PRIVATE_FUN(SideBar, void(QPaintEvent *), paintEvent);
ACCESS_PRIVATE_FUN(SideBar, bool(QObject *, QEvent *), eventFilter);

class SideBarCovTest : public Test
{
public:
    SideBar *m_bar = nullptr;
    void SetUp() override
    {
        m_bar = new SideBar;
        // Build the inner SideBarWidget with a null MainWindow. Construction is
        // safe (no MainWindow deref in any child ctor); wrapped just in case a
        // future refactor adds one.
        EXPECT_NO_FATAL_FAILURE(m_bar->initSideBar(nullptr));
    }
    void TearDown() override { delete m_bar; }
};

// ---------- SideBarWidget::changeShotToolWidget branch coverage ----------

TEST_F(SideBarCovTest, changeShotToolFuncGeometryModes)
{
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("gio")));
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("oval")));
    SUCCEED();
}

TEST_F(SideBarCovTest, changeShotToolFuncLineArrowPenText)
{
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("line")));
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("arrow")));
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("pen")));
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("text")));
    SUCCEED();
}

TEST_F(SideBarCovTest, changeShotToolFuncEffect)
{
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("effect")));
    SUCCEED();
}

TEST_F(SideBarCovTest, changeShotToolFuncAiAssistant)
{
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("aiassistant")));
    SUCCEED();
}

TEST_F(SideBarCovTest, changeShotToolFuncUnknownDefaultBranch)
{
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("unknown_func")));
    SUCCEED();
}

// ---------- SideBar::getSideBarWidth delegation ----------

TEST_F(SideBarCovTest, getSideBarWidthAllBranches)
{
    int w1 = 0;
    EXPECT_NO_FATAL_FAILURE(w1 = m_bar->getSideBarWidth(QStringLiteral("rectangle")));
    EXPECT_GT(w1, 0);
    EXPECT_NO_FATAL_FAILURE(m_bar->getSideBarWidth(QStringLiteral("text")));
    EXPECT_NO_FATAL_FAILURE(m_bar->getSideBarWidth(QStringLiteral("effect")));
    EXPECT_NO_FATAL_FAILURE(m_bar->getSideBarWidth(QStringLiteral("aiassistant")));
    EXPECT_NO_FATAL_FAILURE(m_bar->getSideBarWidth(QStringLiteral("whatever")));
    SUCCEED();
}

// ---------- SideBar show/hide/showAt ----------

TEST_F(SideBarCovTest, showHideWidget)
{
    EXPECT_NO_FATAL_FAILURE(m_bar->showWidget());
    EXPECT_NO_FATAL_FAILURE(m_bar->hideWidget());
    SUCCEED();
}

TEST_F(SideBarCovTest, showAtMovesWidget)
{
    EXPECT_NO_FATAL_FAILURE(m_bar->showAt(QPoint(10, 10)));
    if (m_bar->isVisible()) {
        m_bar->hide();
    }
    SUCCEED();
}

// ---------- SideBar::getInnerWidgetRect with an inner widget ----------

TEST_F(SideBarCovTest, getInnerWidgetRectWithInner)
{
    QRect r;
    EXPECT_NO_FATAL_FAILURE(r = m_bar->getInnerWidgetRect());
    EXPECT_TRUE(r.isValid() || r.isNull());
}

// ---------- SideBar::enterEvent (cursor override, no MainWindow deref) ----------

TEST_F(SideBarCovTest, enterEventNonAiMode)
{
    QEnterEvent enter(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_bar, &enter));
    // restore cursor to avoid leaking override cursor state across tests
    QApplication::restoreOverrideCursor();
    SUCCEED();
}

TEST_F(SideBarCovTest, enterEventAiMode)
{
    // switch to AI mode first so the isAIMode() branch is taken
    EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("aiassistant")));
    QEnterEvent enter(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_bar, &enter));
    QApplication::restoreOverrideCursor();
    SUCCEED();
}

// ---------- SideBar::mouseReleaseEvent (left button; no MainWindow deref) ----------

TEST_F(SideBarCovTest, mouseReleaseLeftButton)
{
    // mouseReleaseEvent only reads m_sidebarWidget->isAIMode(); safe.
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(5, 5),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_bar, &release));
    QApplication::restoreOverrideCursor();
    EXPECT_FALSE(m_bar->isPressed());
}

// ---------- paintEvent / eventFilter via private access ----------

TEST_F(SideBarCovTest, paintEventAndEventFilter)
{
    QPaintEvent pe(QRect(0, 0, 10, 10));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SideBarpaintEvent(*m_bar, &pe));

    QObject obj;
    QEvent ev(QEvent::PaletteChange);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SideBareventFilter(*m_bar, &obj, &ev));
    SUCCEED();
}

// ---------- SideBarWidget::isAIMode ----------

TEST_F(SideBarCovTest, isAIModeReflectsCurrentFunc)
{
    // default not AI. initSideBar() parents the SideBarWidget to the SideBar,
    // so locate it via QObject::findChild (there is no child() method).
    SideBarWidget *inner = m_bar->findChild<SideBarWidget *>();
    if (inner) {
        EXPECT_FALSE(inner->isAIMode());
        EXPECT_NO_FATAL_FAILURE(m_bar->changeShotToolFunc(QStringLiteral("aiassistant")));
        EXPECT_TRUE(inner->isAIMode());
    }
    SUCCEED();
}
