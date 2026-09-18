// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include "addr_pri.h"
#include "../../src/utils/shapesutils.h"
#include "../../src/widgets/shapeswidget.h"

using namespace testing;

// ShapesWidget event handlers (mousePressEvent ~265 lines, mouseMoveEvent ~195
// lines, mouseReleaseEvent) are the largest uncovered block in shapeswidget.cpp.
// We access the protected handlers via ACCESS_PRIVATE_FUN. With an un-initialized
// ShapesWidget (no shapes selected, m_sideBar/m_menuController may be null), the
// early-return branches complete without SEGV.

ACCESS_PRIVATE_FUN(ShapesWidget, bool(QEvent *), event);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *), mousePressEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *), mouseReleaseEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *), mouseMoveEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QKeyEvent *), keyPressEvent);

class ShapesWidgetEventTest : public Test
{
public:
    ShapesWidget *m_w;
    void SetUp() override { m_w = new ShapesWidget; }
    void TearDown() override { delete m_w; }
};

TEST_F(ShapesWidgetEventTest, eventNone)
{
    QEvent ev(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetevent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, mousePressLeftButton)
{
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(50, 50),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetmousePressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, mousePressRightButton)
{
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(50, 50),
        Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetmousePressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, mouseReleaseLeftButton)
{
    QMouseEvent ev(QEvent::MouseButtonRelease, QPointF(50, 50),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetmouseReleaseEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, mouseMoveNoButton)
{
    QMouseEvent ev(QEvent::MouseMove, QPointF(50, 50),
        Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, keyPressEscape)
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, keyPressDelete)
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, keyPressCtrlZ)
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, keyPressLeftArrow)
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &ev));
}
