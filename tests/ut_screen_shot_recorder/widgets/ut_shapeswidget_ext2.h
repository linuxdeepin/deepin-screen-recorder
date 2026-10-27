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
ACCESS_PRIVATE_FIELD(ShapesWidget, Toolshapes, m_shapes);

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

TEST_F(ShapesWidgetEventTest, rectangleContentAndHandlesHaveDistinctHits)
{
    Toolshape rectangle;
    rectangle.type = QStringLiteral("rectangle");
    rectangle.mainPoints = {
        QPointF(10, 10), QPointF(10, 110),
        QPointF(110, 10), QPointF(110, 110)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {rectangle};

    const ShapesWidget::InteractionHit content =
            m_w->interactionAt(QPointF(60, 60));
    EXPECT_EQ(content.kind, ShapesWidget::InteractionKind::None);

    const ShapesWidget::InteractionHit topEdge =
            m_w->interactionAt(QPointF(35, 10));
    EXPECT_EQ(topEdge.kind, ShapesWidget::InteractionKind::Move);

    const ShapesWidget::InteractionHit topHandle =
            m_w->interactionAt(QPointF(60, 10));
    EXPECT_EQ(topHandle.kind, ShapesWidget::InteractionKind::Resize);
    EXPECT_EQ(topHandle.handle, ShapesWidget::InteractionHandle::Top);

    const ShapesWidget::InteractionHit outside =
            m_w->interactionAt(QPointF(300, 300));
    EXPECT_EQ(outside.kind, ShapesWidget::InteractionKind::None);
}

TEST_F(ShapesWidgetEventTest, interactionDoesNotSelectRectangleInterior)
{
    Toolshape rectangle;
    rectangle.type = QStringLiteral("rectangle");
    rectangle.mainPoints = {
        QPointF(10, 10), QPointF(10, 110),
        QPointF(110, 10), QPointF(110, 110)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {rectangle};

    EXPECT_FALSE(m_w->clickedOnShapes(QPointF(60, 60)));
    EXPECT_TRUE(m_w->clickedOnShapes(QPointF(35, 10)));
}

TEST_F(ShapesWidgetEventTest, interactionDoesNotSelectOvalInteriorOrBoundingBoxCorner)
{
    Toolshape oval;
    oval.type = QStringLiteral("oval");
    oval.mainPoints = {
        QPointF(10, 10), QPointF(10, 110),
        QPointF(110, 10), QPointF(110, 110)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {oval};

    EXPECT_EQ(m_w->interactionAt(QPointF(60, 60)).kind,
              ShapesWidget::InteractionKind::None);
    EXPECT_EQ(m_w->interactionAt(QPointF(35, 35)).kind,
              ShapesWidget::InteractionKind::None);
    EXPECT_FALSE(m_w->clickedOnShapes(QPointF(60, 60)));
    EXPECT_FALSE(m_w->clickedOnShapes(QPointF(35, 35)));

    EXPECT_EQ(m_w->interactionAt(QPointF(60, 10)).kind,
              ShapesWidget::InteractionKind::Resize);
    EXPECT_TRUE(m_w->clickedOnShapes(QPointF(60, 10)));
}
