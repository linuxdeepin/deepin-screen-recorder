#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QMap>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/widgets/shapeswidget.h"
#include "../../src/main_window.h"

ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *event), mousePressEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *event), mouseReleaseEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *event), mouseMoveEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPaintEvent *e), paintEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QEvent *e), enterEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, bool(QPointF pos), clickedShapes);
ACCESS_PRIVATE_FIELD(ShapesWidget,bool, m_isSlectedText)
ACCESS_PRIVATE_FIELD(ShapesWidget,Toolshape, m_selectedShape)
ACCESS_PRIVATE_FIELD(ShapesWidget,int, m_selectedIndex)
ACCESS_PRIVATE_FIELD(ShapesWidget,int, m_selectedOrder)
ACCESS_PRIVATE_FIELD(ShapesWidget,Toolshapes, m_shapes)
ACCESS_PRIVATE_FIELD(ShapesWidget,QString, m_currentType)
ACCESS_PRIVATE_FIELD(ShapesWidget,QPointF, m_pos1)
ACCESS_PRIVATE_FIELD(ShapesWidget,Toolshape, m_currentShape)

using namespace testing;
class ShapesWidgetTest:public testing::Test{

public:
    Stub stub;
    MainWindow *m_mainWindow;
    ShapesWidget *shapesWidget;
    virtual void SetUp() override{
        m_mainWindow = new MainWindow;
        shapesWidget = new ShapesWidget(m_mainWindow);
    }

    virtual void TearDown() override{
        if(nullptr != shapesWidget)
            delete shapesWidget;
    }
};

TEST_F(ShapesWidgetTest, enterEvent)
{
    QEvent *e = new QEvent(QEvent::Enter);
    call_private_fun::ShapesWidgetenterEvent(*shapesWidget,e);
}

TEST_F(ShapesWidgetTest, paintEvent)
{
    QPaintEvent *e = new QPaintEvent(QRect());
    call_private_fun::ShapesWidgetpaintEvent(*shapesWidget,e);
}

TEST_F(ShapesWidgetTest, mousePressEvent)
{
    QMouseEvent *ev0 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev0);
    //1220
    QMouseEvent *ev1 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("line");
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev1);

    QMouseEvent *ev2 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("arrow");
    access_private_field::ShapesWidgetm_currentShape(*shapesWidget).isStraight = true;
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev2);

    QMouseEvent *ev21 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("arrow");
    access_private_field::ShapesWidgetm_currentShape(*shapesWidget).isStraight = false;
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev21);

    QMouseEvent *ev3 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("rectangle");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev3);

    QMouseEvent *ev4 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("oval");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev4);

    QMouseEvent *ev5 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("text");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev5);

    //1162
    QMouseEvent *ev6 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev6);

    //1178
    QMouseEvent *ev7 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = -1;
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev7);

    //1220
    QMouseEvent *ev8 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = -1;
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("line");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev8);

    QMouseEvent *ev9 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("arrow");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget,ev9);

}

TEST_F(ShapesWidgetTest, mouseMoveEvent)
{
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget,ev);
}

TEST_F(ShapesWidgetTest, mouseReleaseEvent)
{
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::ShapesWidgetmouseReleaseEvent(*shapesWidget,ev);
}

TEST_F(ShapesWidgetTest, updateSelectedShape)
{
    QString group = QString("line");
    QString key = QString("linewidth_index");
    int index = 0;
    shapesWidget->updateSelectedShape(group,key,index);

    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
    QList <Toolshape> toolShapes;
    toolShapes << Toolshape();
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("arrow");
    key = QString("arrow_linewidth_index");
    shapesWidget->updateSelectedShape(group,key,index);

    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).isStraight = true;
    key = QString("straightline_linewidth_index");
    shapesWidget->updateSelectedShape(group,key,index);

    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("line");
    key = QString("linewidth_index");
    shapesWidget->updateSelectedShape(group,key,index);

    group = QString("text");
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = group;
    key = QString("color_index");
    shapesWidget->updateSelectedShape(group,key,index);

    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    group = QString("text");
    key = QString("fontsize");
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = group;
    shapesWidget->updateSelectedShape(group,key,index);

    group = QString("line");
    key = QString("color_index");
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = group;
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

    access_private_field::ShapesWidgetm_isSlectedText(*shapesWidget) = true;
    shapesWidget->updateSelectedShape(group,key,index);

    group = QString("common");
    key = QString("color_index");
    shapesWidget->updateSelectedShape(group,key,index);

    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("arrow");

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

    //
//    QList <Toolshape> toolShapes;
//    Toolshape toolShape1;
//    toolShape1.type = QString("text");
//    toolShape1.index = 0;
//    toolShapes << toolShape1;
//    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
//    shapesWidget->setNoChangedTextEditRemove();
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

    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    shapesWidget->clickedOnShapes(pos);
}

TEST_F(ShapesWidgetTest, clickedOnRect)
{
    FourPoints rectPoints0;
    rectPoints0 << QPointF(189,199);
    rectPoints0 << QPointF(189,396);
    rectPoints0 << QPointF(488,199);
    rectPoints0 << QPointF(488,396);
    QPointF pos0 = QPointF(484,398);
    shapesWidget->clickedOnRect(rectPoints0,pos0,false);

    FourPoints rectPoints1;
    rectPoints1 << QPointF(446.545,154.746);
    rectPoints1 << QPointF(249.965,141.891);
    rectPoints1 << QPointF(427.035,453.109);
    rectPoints1 << QPointF(230.455,440.254);
    QPointF pos1 = QPointF(372,338);
    shapesWidget->clickedOnRect(rectPoints1,pos1,false);

    FourPoints rectPoints2;
    rectPoints2 << QPointF(446.545,154.746);
    rectPoints2 << QPointF(249.965,141.891);
    rectPoints2 << QPointF(427.035,453.109);
    rectPoints2 << QPointF(230.455,440.254);
    QPointF pos2 = QPointF(236,291);
    shapesWidget->clickedOnRect(rectPoints2,pos2,false);

    FourPoints rectPoints3;
    rectPoints3 << QPointF(446.545,154.746);
    rectPoints3 << QPointF(249.965,141.891);
    rectPoints3 << QPointF(427.035,453.109);
    rectPoints3 << QPointF(230.455,440.254);
    QPointF pos3 = QPointF(425,455);
    shapesWidget->clickedOnRect(rectPoints3,pos3,false);

    FourPoints rectPoints4;
    rectPoints4 << QPointF(421.027,153.077);
    rectPoints4 << QPointF(249.965,141.891);
    rectPoints4 << QPointF(409,337);
    rectPoints4 << QPointF(237.938,325.814);
    QPointF pos4 = QPointF(413,250);
    shapesWidget->clickedOnRect(rectPoints4,pos4,false);

    FourPoints rectPoints5;
    rectPoints5 << QPointF(421.027,153.077);
    rectPoints5 << QPointF(249.965,141.891);
    rectPoints5 << QPointF(409,337);
    rectPoints5 << QPointF(237.938,325.814);
    QPointF pos5 = QPointF(436,249);
    shapesWidget->clickedOnRect(rectPoints5,pos5,false);

    FourPoints rectPoints6;
    rectPoints6 << QPointF(189,199);
    rectPoints6 << QPointF(189,396);
    rectPoints6 << QPointF(488,199);
    rectPoints6 << QPointF(488,396);
    QPointF pos6 = QPointF(484,398);
    shapesWidget->clickedOnRect(rectPoints6,pos6,false);

    FourPoints rectPoints7;
    rectPoints7 << QPointF(296.346,360.862);
    rectPoints7 << QPointF(448.182,281.281);
    rectPoints7 << QPointF(210.783,197.61);
    rectPoints7 << QPointF(362.619,118.03);
    QPointF pos7 = QPointF(524,239);
    shapesWidget->clickedOnRect(rectPoints7,pos7,false);

    FourPoints rectPoints8;
    rectPoints8 << QPointF(216,105);
    rectPoints8 << QPointF(216,192);
    rectPoints8 << QPointF(334,105);
    rectPoints8 << QPointF(334,192);
    QPointF pos8 = QPointF(213,106);
    shapesWidget->clickedOnRect(rectPoints8,pos8,false);

    FourPoints rectPoints9;
    rectPoints9 << QPointF(80,131);
    rectPoints9 << QPointF(80,288);
    rectPoints9 << QPointF(351,131);
    rectPoints9 << QPointF(351,288);
    QPointF pos9 = QPointF(219,131);
    shapesWidget->clickedOnRect(rectPoints9,pos9,false);

    FourPoints rectPoints10;
    rectPoints10 << QPointF(252,42);
    rectPoints10 << QPointF(252,192);
    rectPoints10 << QPointF(334,42);
    rectPoints10 << QPointF(334,192);
    QPointF pos10 = QPointF(331,116);
    shapesWidget->clickedOnRect(rectPoints10,pos10,false);

    FourPoints rectPoints11;
    rectPoints11 << QPointF(420,85);
    rectPoints11 << QPointF(420,330);
    rectPoints11 << QPointF(730,85);
    rectPoints11 << QPointF(730,330);
    QPointF pos11 = QPointF(651,292);
    shapesWidget->clickedOnRect(rectPoints11,pos11,true);

    FourPoints rectPoints12;
    rectPoints12 << QPointF(182.329,146.634);
    rectPoints12 << QPointF(126.683,177.773);
    rectPoints12 << QPointF(201,180);
    rectPoints12 << QPointF(145.353,211.139);
    QPointF pos12 = QPointF(151,166);
    shapesWidget->clickedOnRect(rectPoints12,pos12,false);

    FourPoints rectPoints13;
    rectPoints13 << QPointF(89,86);
    rectPoints13 << QPointF(89,321);
    rectPoints13 << QPointF(355,86);
    rectPoints13 << QPointF(355,321);
    QPointF pos13 = QPointF(90,322);
    shapesWidget->clickedOnRect(rectPoints13,pos13,false);
}

TEST_F(ShapesWidgetTest, hoverOnText)
{
    FourPoints rectPoints;
    rectPoints << QPointF(96,235);
    rectPoints << QPointF(96,335);
    rectPoints << QPointF(265,235);
    rectPoints << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    EXPECT_TRUE(shapesWidget->hoverOnText(rectPoints,pos));
}

TEST_F(ShapesWidgetTest, hoverOnRotatePoint)
{
    FourPoints rectPoints;
    rectPoints << QPointF(96,235);
    rectPoints << QPointF(96,335);
    rectPoints << QPointF(265,235);
    rectPoints << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    EXPECT_FALSE(shapesWidget->hoverOnRotatePoint(rectPoints,pos));
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

    FourPoints rectPoints0;
    rectPoints0 << QPointF(189,199);
    rectPoints0 << QPointF(189,396);
    rectPoints0 << QPointF(488,199);
    rectPoints0 << QPointF(488,396);
    QPointF pos0 = QPointF(484,398);
    shapesWidget->clickedOnEllipse(rectPoints0,pos0,false);

    FourPoints rectPoints1;
    rectPoints1 << QPointF(446.545,154.746);
    rectPoints1 << QPointF(249.965,141.891);
    rectPoints1 << QPointF(427.035,453.109);
    rectPoints1 << QPointF(230.455,440.254);
    QPointF pos1 = QPointF(372,338);
    shapesWidget->clickedOnEllipse(rectPoints1,pos1,false);

    FourPoints rectPoints2;
    rectPoints2 << QPointF(446.545,154.746);
    rectPoints2 << QPointF(249.965,141.891);
    rectPoints2 << QPointF(427.035,453.109);
    rectPoints2 << QPointF(230.455,440.254);
    QPointF pos2 = QPointF(236,291);
    shapesWidget->clickedOnEllipse(rectPoints2,pos2,false);

    FourPoints rectPoints3;
    rectPoints3 << QPointF(446.545,154.746);
    rectPoints3 << QPointF(249.965,141.891);
    rectPoints3 << QPointF(427.035,453.109);
    rectPoints3 << QPointF(230.455,440.254);
    QPointF pos3 = QPointF(425,455);
    shapesWidget->clickedOnEllipse(rectPoints3,pos3,false);

    FourPoints rectPoints4;
    rectPoints4 << QPointF(421.027,153.077);
    rectPoints4 << QPointF(249.965,141.891);
    rectPoints4 << QPointF(409,337);
    rectPoints4 << QPointF(237.938,325.814);
    QPointF pos4 = QPointF(413,250);
    shapesWidget->clickedOnEllipse(rectPoints4,pos4,false);

    FourPoints rectPoints5;
    rectPoints5 << QPointF(421.027,153.077);
    rectPoints5 << QPointF(249.965,141.891);
    rectPoints5 << QPointF(409,337);
    rectPoints5 << QPointF(237.938,325.814);
    QPointF pos5 = QPointF(436,249);
    shapesWidget->clickedOnEllipse(rectPoints5,pos5,false);

    FourPoints rectPoints6;
    rectPoints6 << QPointF(189,199);
    rectPoints6 << QPointF(189,396);
    rectPoints6 << QPointF(488,199);
    rectPoints6 << QPointF(488,396);
    QPointF pos6 = QPointF(484,398);
    shapesWidget->clickedOnEllipse(rectPoints6,pos6,false);

    FourPoints rectPoints7;
    rectPoints7 << QPointF(296.346,360.862);
    rectPoints7 << QPointF(448.182,281.281);
    rectPoints7 << QPointF(210.783,197.61);
    rectPoints7 << QPointF(362.619,118.03);
    QPointF pos7 = QPointF(524,239);
    shapesWidget->clickedOnEllipse(rectPoints7,pos7,false);

    FourPoints rectPoints8;
    rectPoints8 << QPointF(216,105);
    rectPoints8 << QPointF(216,192);
    rectPoints8 << QPointF(334,105);
    rectPoints8 << QPointF(334,192);
    QPointF pos8 = QPointF(213,106);
    shapesWidget->clickedOnEllipse(rectPoints8,pos8,false);

    FourPoints rectPoints9;
    rectPoints9 << QPointF(80,131);
    rectPoints9 << QPointF(80,288);
    rectPoints9 << QPointF(351,131);
    rectPoints9 << QPointF(351,288);
    QPointF pos9 = QPointF(219,131);
    shapesWidget->clickedOnEllipse(rectPoints9,pos9,false);

    FourPoints rectPoints10;
    rectPoints10 << QPointF(252,42);
    rectPoints10 << QPointF(252,192);
    rectPoints10 << QPointF(334,42);
    rectPoints10 << QPointF(334,192);
    QPointF pos10 = QPointF(331,116);
    shapesWidget->clickedOnEllipse(rectPoints10,pos10,false);

    FourPoints rectPoints11;
    rectPoints11 << QPointF(420,85);
    rectPoints11 << QPointF(420,330);
    rectPoints11 << QPointF(730,85);
    rectPoints11 << QPointF(730,330);
    QPointF pos11 = QPointF(651,292);
    shapesWidget->clickedOnEllipse(rectPoints11,pos11,true);

    FourPoints rectPoints12;
    rectPoints12 << QPointF(182.329,146.634);
    rectPoints12 << QPointF(126.683,177.773);
    rectPoints12 << QPointF(201,180);
    rectPoints12 << QPointF(145.353,211.139);
    QPointF pos12 = QPointF(151,166);
    shapesWidget->clickedOnEllipse(rectPoints12,pos12,false);

    FourPoints rectPoints13;
    rectPoints13 << QPointF(89,86);
    rectPoints13 << QPointF(89,321);
    rectPoints13 << QPointF(355,86);
    rectPoints13 << QPointF(355,321);
    QPointF pos13 = QPointF(90,322);
    shapesWidget->clickedOnEllipse(rectPoints13,pos13,false);
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

    QList<QPointF> points0;
    points0 << QPointF(567,138);
    points0 << QPointF(427,289);
    QPointF pos0 = QPointF(565,138);
    shapesWidget->clickedOnArrow(points0,pos0);

    QList<QPointF> points1;
    points1 << QPointF(554,240);
    points1 << QPointF(432,397);
    QPointF pos1 = QPointF(430,399);
    shapesWidget->clickedOnArrow(points1,pos1);

    QList<QPointF> points2;
    points2 << QPointF(567,138);
    points2 << QPointF(427,289);
    QPointF pos2 = QPointF(524,187);
    shapesWidget->clickedOnArrow(points2,pos2);

    QList<QPointF> points3;
    points3 << QPointF(554,240);
    points3 << QPointF(572,370);
    QPointF pos3 = QPointF(565,138);
    shapesWidget->clickedOnArrow(points3,pos3);

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

TEST_F(ShapesWidgetTest, hoverOnLine)
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
    EXPECT_FALSE(shapesWidget->hoverOnLine(mainPoints,points,pos));
}

TEST_F(ShapesWidgetTest, hoverOnArrow)
{
    QList<QPointF> points;
    points << QPointF(96,235);
    points << QPointF(96,335);
    points << QPointF(265,235);
    points << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    EXPECT_FALSE(shapesWidget->hoverOnArrow(points,pos));
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

TEST_F(ShapesWidgetTest, rotateOnPoint)
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

TEST_F(ShapesWidgetTest, hoverOnEllipse)
{
    FourPoints rectPoints;
    rectPoints << QPointF(96,235);
    rectPoints << QPointF(96,335);
    rectPoints << QPointF(265,235);
    rectPoints << QPointF(265,335);
    QPointF pos = QPointF(220,233);
    EXPECT_TRUE(shapesWidget->hoverOnEllipse(rectPoints,pos));
}

TEST_F(ShapesWidgetTest, hoverOnShapes)
{
    Toolshape toolshape;
    EXPECT_FALSE(shapesWidget->hoverOnShapes(toolshape,QPointF(96,235)));
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

TEST_F(ShapesWidgetTest, handleResize)
{
    shapesWidget->handleResize(QPointF(503,462),1);
}

TEST_F(ShapesWidgetTest, menuSaveSlot)
{
    shapesWidget->menuSaveSlot();
}

TEST_F(ShapesWidgetTest, clickedShapes)
{
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    call_private_fun::ShapesWidgetclickedShapes(*shapesWidget,QPointF(10,10));
}



