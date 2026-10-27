// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "../../../src/screenshot/core/selectionmodel.h"

using namespace Dsr::Screenshot;
using namespace testing;

class SelectionModelTest : public Test
{
protected:
    SelectionModel model;
    QRect desktop{0, 0, 1920, 1080};
    void SetUp() override
    {
        model.setDesktopBounds(desktop);
    }
};

TEST_F(SelectionModelTest, DefaultStateIsEmpty)
{
    EXPECT_EQ(model.state(), InteractionState::Empty);
    EXPECT_EQ(model.action(), SelectionAction::NoAction);
    EXPECT_TRUE(model.geometry().isEmpty());
}

TEST_F(SelectionModelTest, CursorIntentForActions)
{
    EXPECT_EQ(cursorIntentFor(SelectionAction::NoAction), CursorIntent::Cross);
    EXPECT_EQ(cursorIntentFor(SelectionAction::CreateFreeform), CursorIntent::Cross);
    EXPECT_EQ(cursorIntentFor(SelectionAction::Move), CursorIntent::Move);
    EXPECT_EQ(cursorIntentFor(SelectionAction::ResizeLeft), CursorIntent::ResizeHorizontal);
    EXPECT_EQ(cursorIntentFor(SelectionAction::ResizeRight), CursorIntent::ResizeHorizontal);
    EXPECT_EQ(cursorIntentFor(SelectionAction::ResizeTop), CursorIntent::ResizeVertical);
    EXPECT_EQ(cursorIntentFor(SelectionAction::ResizeBottom), CursorIntent::ResizeVertical);
    EXPECT_EQ(cursorIntentFor(SelectionAction::ResizeTopLeft), CursorIntent::ResizeDiagonalForward);
    EXPECT_EQ(cursorIntentFor(SelectionAction::ResizeBottomRight), CursorIntent::ResizeDiagonalForward);
    EXPECT_EQ(cursorIntentFor(SelectionAction::ResizeTopRight), CursorIntent::ResizeDiagonalBackward);
    EXPECT_EQ(cursorIntentFor(SelectionAction::ResizeBottomLeft), CursorIntent::ResizeDiagonalBackward);
}

TEST_F(SelectionModelTest, SetGeometryTransitionsToReady)
{
    QSignalSpy stateSpy(&model, &SelectionModel::stateChanged);
    model.setGeometry(QRect(100, 100, 200, 150));
    EXPECT_EQ(model.state(), InteractionState::Ready);
    EXPECT_EQ(model.geometry(), QRect(100, 100, 200, 150));
    EXPECT_GE(stateSpy.count(), 1);
}

TEST_F(SelectionModelTest, SetEmptyGeometryClears)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    model.setGeometry(QRect());
    EXPECT_EQ(model.state(), InteractionState::Empty);
    EXPECT_TRUE(model.geometry().isEmpty());
}

TEST_F(SelectionModelTest, SetTooSmallGeometryClears)
{
    model.setGeometry(QRect(100, 100, 3, 3));
    EXPECT_EQ(model.state(), InteractionState::Empty);
    EXPECT_TRUE(model.geometry().isEmpty());
}

TEST_F(SelectionModelTest, ClearResetsState)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    model.clear();
    EXPECT_EQ(model.state(), InteractionState::Empty);
    EXPECT_TRUE(model.geometry().isEmpty());
}

TEST_F(SelectionModelTest, HitTestNoSelectionReturnsNoAction)
{
    EXPECT_EQ(model.hitTest(QPoint(500, 500)), SelectionAction::NoAction);
}

TEST_F(SelectionModelTest, HitTestInsideReturnsMove)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    EXPECT_EQ(model.hitTest(QPoint(200, 175)), SelectionAction::Move);
}

TEST_F(SelectionModelTest, HitTestOutsideReturnsNoAction)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    EXPECT_EQ(model.hitTest(QPoint(500, 500)), SelectionAction::NoAction);
}

TEST_F(SelectionModelTest, HitTestTopLeftHandle)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    EXPECT_EQ(model.hitTest(QPoint(100, 100)), SelectionAction::ResizeTopLeft);
}

TEST_F(SelectionModelTest, HitTestBottomRightHandle)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    EXPECT_EQ(model.hitTest(QPoint(300, 250)), SelectionAction::ResizeBottomRight);
}

TEST_F(SelectionModelTest, HitTestTopRightHandle)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    EXPECT_EQ(model.hitTest(QPoint(300, 100)), SelectionAction::ResizeTopRight);
}

TEST_F(SelectionModelTest, HitTestBottomLeftHandle)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    EXPECT_EQ(model.hitTest(QPoint(100, 250)), SelectionAction::ResizeBottomLeft);
}

TEST_F(SelectionModelTest, BeginInteractionOutsideCreates)
{
    QSignalSpy stateSpy(&model, &SelectionModel::stateChanged);
    SelectionAction action = model.beginInteraction(QPoint(500, 500));
    EXPECT_EQ(action, SelectionAction::CreateFreeform);
    EXPECT_EQ(model.state(), InteractionState::Creating);
}

TEST_F(SelectionModelTest, BeginInteractionInsideMoves)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    SelectionAction action = model.beginInteraction(QPoint(200, 175));
    EXPECT_EQ(action, SelectionAction::Move);
    EXPECT_EQ(model.state(), InteractionState::Moving);
}

TEST_F(SelectionModelTest, BeginInteractionOnHandleResizes)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    SelectionAction action = model.beginInteraction(QPoint(100, 100));
    EXPECT_EQ(action, SelectionAction::ResizeTopLeft);
    EXPECT_EQ(model.state(), InteractionState::Resizing);
}

TEST_F(SelectionModelTest, UpdateInteractionWhileMoving)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    model.beginInteraction(QPoint(200, 175));
    model.updateInteraction(QPoint(300, 275));
    QRect geom = model.geometry();
    EXPECT_EQ(geom.topLeft(), QPoint(200, 200));
    EXPECT_EQ(geom.size(), QSize(200, 150));
}

TEST_F(SelectionModelTest, EndInteractionTransitionsToReady)
{
    model.beginInteraction(QPoint(500, 500));
    model.updateInteraction(QPoint(700, 700));
    model.endInteraction();
    EXPECT_EQ(model.state(), InteractionState::Ready);
    EXPECT_FALSE(model.geometry().isEmpty());
}

TEST_F(SelectionModelTest, EndInteractionTooSmallCreationClears)
{
    model.beginInteraction(QPoint(500, 500));
    model.updateInteraction(QPoint(503, 503));
    model.endInteraction();
    EXPECT_EQ(model.state(), InteractionState::Empty);
}

TEST_F(SelectionModelTest, CancelInteractionRestoresStart)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    model.beginInteraction(QPoint(200, 175));
    model.updateInteraction(QPoint(300, 275));
    model.cancelInteraction();
    EXPECT_EQ(model.geometry(), QRect(100, 100, 200, 150));
}

TEST_F(SelectionModelTest, SetAnnotationBoundsReappliesGeometry)
{
    model.setGeometry(QRect(100, 100, 200, 150));
    model.setAnnotationBounds(QRectF(50, 50, 80, 80));
    QRect geom = model.geometry();
    EXPECT_TRUE(geom.contains(QRect(50, 50, 80, 80)));
    EXPECT_TRUE(geom.contains(QRect(100, 100, 200, 150)));
}
