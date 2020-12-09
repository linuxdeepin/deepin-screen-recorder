#pragma once
#include <gtest/gtest.h>
#include <QTest>

#define private public
#define protected public
#include "../../src/widgets/shapeswidget.h"

using namespace testing;
class ShapesWidgetTest:public testing::Test{

public:
    ShapesWidget *shapesWidget;
    virtual void SetUp() override{
        shapesWidget = new ShapesWidget(nullptr);
    }

    virtual void TearDown() override{
        if(nullptr != shapesWidget)
            delete shapesWidget;
    }
};

TEST_F(ShapesWidgetTest, updateSelectedShape)
{
    QString group = QString("line");
    QString key = QString("linewidth_index");
    int index = 0;
    shapesWidget->updateSelectedShape(group,key,index);
    group = QString("rectangle");
    key = QString("linewidth_index");
    shapesWidget->updateSelectedShape(group,key,index);
    group = QString("oval");
    key = QString("linewidth_index");
    shapesWidget->updateSelectedShape(group,key,index);
    group = QString("arrow");
    key = QString("arrow_linewidth_index");
    shapesWidget->updateSelectedShape(group,key,index);
}

TEST_F(ShapesWidgetTest, setCurrentShape)
{
    QString shapeType = QString("arrow");
    shapesWidget->setCurrentShape(shapeType);
    shapeType = QString("shapeType");
    shapesWidget->setCurrentShape(shapeType);
}

TEST_F(ShapesWidgetTest, clearSelected)
{
    shapesWidget->clearSelected();
}

TEST_F(ShapesWidgetTest, setAllTextEditReadOnly)
{
    shapesWidget->setAllTextEditReadOnly();
}

TEST_F(ShapesWidgetTest, setNoChangedTextEditRemove)
{
    shapesWidget->setNoChangedTextEditRemove();
}

TEST_F(ShapesWidgetTest, saveActionTriggered)
{
    shapesWidget->saveActionTriggered();
}

TEST_F(ShapesWidgetTest, handleDrag)
{
    QPointF oldPoint(500,500);
    QPointF newPoint(600,500);
    shapesWidget->handleDrag(oldPoint,newPoint);
}

TEST_F(ShapesWidgetTest, handleRotate)
{
    QPointF pos(500,500);
    shapesWidget->handleRotate(pos);
}

TEST_F(ShapesWidgetTest, clickedOnShapes)
{
    QPointF pos(500,500);
    shapesWidget->clickedOnShapes(pos);
}

TEST_F(ShapesWidgetTest, clickedOnRect)
{
    FourPoints rectPoints;
    rectPoints << QPointF(96,235);
    rectPoints << QPointF(96,335);
    rectPoints << QPointF(265,235);
    rectPoints << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    bool isBlurMosaic = false;
    EXPECT_TRUE(shapesWidget->clickedOnRect(rectPoints,pos,isBlurMosaic));
    pos = QPointF(500,500);
    EXPECT_FALSE(shapesWidget->clickedOnRect(rectPoints,pos,isBlurMosaic));
}

TEST_F(ShapesWidgetTest, clickedOnEllipse)
{
    FourPoints mainPoints;
    mainPoints << QPointF(96,235);
    mainPoints << QPointF(96,335);
    mainPoints << QPointF(265,235);
    mainPoints << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    bool isBlurMosaic = false;
    EXPECT_TRUE(shapesWidget->clickedOnEllipse(mainPoints,pos,isBlurMosaic));
    pos = QPointF(500,500);
    EXPECT_FALSE(shapesWidget->clickedOnEllipse(mainPoints,pos,isBlurMosaic));
}

TEST_F(ShapesWidgetTest, clickedOnArrow)
{
    QList<QPointF> points;
    points << QPointF(96,235);
    points << QPointF(96,335);
    points << QPointF(265,235);
    points << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    EXPECT_FALSE(shapesWidget->clickedOnArrow(points,pos));
}

TEST_F(ShapesWidgetTest, clickedOnLine)
{
    FourPoints mainPoints;
    mainPoints << QPointF(96,235);
    mainPoints << QPointF(96,335);
    mainPoints << QPointF(265,235);
    mainPoints << QPointF(265,335);
    QList<QPointF> points;
    points << QPointF(96,235);
    points << QPointF(96,335);
    points << QPointF(265,235);
    points << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    EXPECT_FALSE(shapesWidget->clickedOnLine(mainPoints,points,pos));
}

TEST_F(ShapesWidgetTest, clickedOnText)
{
    FourPoints mainPoints;
    mainPoints << QPointF(96,235);
    mainPoints << QPointF(96,335);
    mainPoints << QPointF(265,235);
    mainPoints << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    EXPECT_FALSE(shapesWidget->clickedOnText(mainPoints,pos));
}

TEST_F(ShapesWidgetTest, hoverOnRect)
{
    FourPoints rectPoints;
    rectPoints << QPointF(96,235);
    rectPoints << QPointF(96,335);
    rectPoints << QPointF(265,235);
    rectPoints << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    bool isTextBorder = false;
    EXPECT_TRUE(shapesWidget->hoverOnRect(rectPoints,pos,isTextBorder));
}

TEST_F(ShapesWidgetTest, textEditIsReadOnly)
{
    shapesWidget->textEditIsReadOnly();
}

TEST_F(ShapesWidgetTest, undoDrawShapes)
{
    shapesWidget->undoDrawShapes();
}

TEST_F(ShapesWidgetTest, undoAllDrawShapes)
{
    shapesWidget->undoAllDrawShapes();
}

TEST_F(ShapesWidgetTest, deleteCurrentShape)
{
    shapesWidget->deleteCurrentShape();
}

TEST_F(ShapesWidgetTest, updateCursorShape)
{
    shapesWidget->updateCursorShape();
}

TEST_F(ShapesWidgetTest, menuCloseSlot)
{
    shapesWidget->menuCloseSlot();
}

TEST_F(ShapesWidgetTest, setGlobalRect)
{
    shapesWidget->setGlobalRect(QRect(896,445,782,512));
}

TEST_F(ShapesWidgetTest, setShiftKeyPressed)
{
    shapesWidget->setShiftKeyPressed(false);
}

TEST_F(ShapesWidgetTest, microAdjust)
{
    shapesWidget->microAdjust("Left");
}

