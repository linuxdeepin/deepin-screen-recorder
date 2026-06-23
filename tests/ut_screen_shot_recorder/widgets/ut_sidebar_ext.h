// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// SideBar smoke tests for the current (Qt6) API.
//
// The legacy ut_sidebar.h is disabled in test_all_interfaces.h because the old
// API (SideBar::initSideBar() with no-arg variant, QEvent-typed enterEvent) has
// drifted. This file targets the SideBar class (the DLabel-based container),
// exercising only the no-dependency getters and the null-guarded geometry
// helper that are safe on a freshly constructed, never-shown widget.
//
// Skipped (unsafe / need a fully wired MainWindow + child SideBarWidget):
//   - changeShotToolFunc / getSideBarWidth : dereference m_sidebarWidget (null
//     unless initSideBar(MainWindow*) ran -> SEGV).
//   - showWidget / hideWidget : dereference m_sidebarWidget (SEGV when null).
//   - showAt : calls this->show() (needs a real window).
//   - initSideBar(MainWindow*) : builds SideBarWidget which wires MainWindow
//     signals/slots and AI assistant -- heavy, needs a fully working MainWindow.
//   - enterEvent / mousePress/Move/ReleaseEvent : cursor override + MainWindow
//     deref; cursor path caused the prior shapeswidget SEGV.
//   - paintEvent : DLabel::paintEvent is benign, but kept out to stay strictly
//     within "no shown window" guarantees.
//   - eventFilter : trivial passthrough; no value to assert beyond a no-crash
//     smoke which is covered indirectly.
//
// SideBarWidget (the DFloatingWidget inner panel) is intentionally NOT covered
// here: its constructor calls initSideBarWidget() which instantiates
// ShapeToolWidget/ColorToolWidget/ShotToolWidget/AIAssistantWidget and connects
// to MainWindow -- too coupled to exercise as a smoke test.

#pragma once
#include <gtest/gtest.h>
#include <QEvent>
#include <QObject>
#include "../../src/widgets/sidebar.h"

class SideBarExtTest : public ::testing::Test
{
public:
    SideBar *m_bar;
    void SetUp() override { m_bar = new SideBar; }
    void TearDown() override { delete m_bar; }
};

TEST_F(SideBarExtTest, DragAndPressDefaultFalse)
{
    // Fresh SideBar has never been dragged or pressed.
    EXPECT_FALSE(m_bar->isDraged());
    EXPECT_FALSE(m_bar->isPressed());
}

TEST_F(SideBarExtTest, InnerWidgetRectFallsBackToOwnRect)
{
    // m_sidebarWidget is null until initSideBar(); getInnerWidgetRect() guards
    // on that and returns this->rect().
    m_bar->resize(120, 80);
    QRect r = m_bar->getInnerWidgetRect();
    EXPECT_EQ(r, m_bar->rect());
    EXPECT_EQ(r.size(), QSize(120, 80));
}

TEST_F(SideBarExtTest, InnerWidgetRectOnDefaultGeometry)
{
    // No resize; just confirm it does not crash and returns a valid QRect.
    QRect r;
    EXPECT_NO_FATAL_FAILURE(r = m_bar->getInnerWidgetRect());
    EXPECT_TRUE(r.isValid() || r.isNull()); // default widget rect is null/empty
}

TEST_F(SideBarExtTest, ConstructionIsStable)
{
    // Re-enter the type to catch ctor regressions; nothing to assert beyond
    // the object surviving construction.
    SideBar *extra = nullptr;
    EXPECT_NO_FATAL_FAILURE(extra = new SideBar);
    delete extra;
    SUCCEED();
}
