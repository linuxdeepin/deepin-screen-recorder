#pragma once
#include <QDebug>
#include <QPointF>
#include <gtest/gtest.h>
#include "../../src/utils/calculaterect.h"

using namespace testing;

class CalculaterectTest:public testing::Test{

public:
    virtual void SetUp() override{
        qDebug() << "start CalculaterectTest";
    }

    virtual void TearDown() override{
        std::cout << "end CalculaterectTest" << std::endl;
    }
};

TEST_F(CalculaterectTest, test_getRotatePoint)
{
    QPointF point1(0, 0);
    QPointF point2(10, 10);
    QPointF point3(5, 5);
    QPointF point4(0, 5);
    qDebug() << getRotatePoint(point1, QPointF(5, 15), point3, QPointF(15, 5));
    qDebug() << getRotatePoint(point1, QPointF(5, 15), point3, QPointF(15, 25));
    qDebug() << getRotatePoint(point1, QPointF(25, 15), point3, QPointF(15, 35));
    qDebug() << getRotatePoint(point1, point2, point3, point4);
    //EXPECT_EQ(4, );

    QList<QPointF> points;
    points << point1 << point2 << point3 << point4;
    EXPECT_EQ(4,fourPointsOfLine(points).size());


    pointRotate(point1, point2, 1.0);
    FourPoints rectPoints;
    rectPoints << QPointF(0, 0) << QPointF(10, 0) << QPointF(0, 10) << QPointF(10, 10);
    getEightControlPoint(rectPoints);

    pointOfArrow(point1, point4, 5.0);
}

TEST_F(CalculaterectTest, test_base)
{
    QPointF point1(0, 0);
    QPointF point2(10, 10);
    QPointF point3(5, 5);
    QPointF point4(0, 5);
    QPointF pos(55, 55);
    QList<QPointF> points;
    FourPoints rectFPoints;
    FourPoints fourPoints;
    FourPoints mainPoints;
    rectFPoints << point1 << point2 << point3 << point4;
    fourPoints << point1 << point2 << point3 << point4;
    mainPoints << point1 << point2 << point3 << point4;

    EXPECT_TRUE(pointClickIn(QPointF(0, 0), QPointF(10, 10), 100));
    EXPECT_TRUE(pointOnLine(point1,  point2,  point3));
    EXPECT_EQ(5,  getDistance(point1, point4));

    EXPECT_DOUBLE_EQ(-0.54030230586813977, calculateAngle(point1,  point2,  point3));
    calculateAngle(QPointF(5,5),  QPointF(5,5),  point1);
    calculateAngle(QPointF(15, 25),  QPointF(10, 20),  QPointF(25,15));
    calculateAngle(QPointF(15, 25),  QPointF(10, 20),  QPointF(5,15));
    calculateAngle(QPointF(15, 25),  QPointF(10, 20),  QPointF(5,45));

    /* get the new points after rotate with angle*/
    QPointF  pointRotate(QPointF point1, QPointF point2, qreal angle);

    /* the distance from a point(point3) to a line(point1, point2) */
    EXPECT_EQ(0, pointToLineDistance( point1,  point2,  point3));

    /* judge the direction of point3 of line(point1, point2) */
    EXPECT_EQ(-1,   pointLineDir( point1,  point2,  point3));

    /*get another four points in rectangle (Top/Bottom/Left/Right) */
    FourPoints getAnotherFPoints(FourPoints mainPoints);

    /* calculate the control point of the beizer */
    QPointF getControlPoint(QPointF point1, QPointF point2, bool direction) ;

    /* get eight control points */
    //QList<QPointF> getEightControlPoint(FourPoints rectFPoints);

    /* judge whether the clickOnPoint is on the bezier */
    /* 0 <= pos.x() <= 1*/
    EXPECT_FALSE(pointOnBezier(point1, point2,  point3, point4,  pos));

    /* judge whether the clickOnPoint is on the ellipse */
    EXPECT_FALSE(pointOnEllipse(rectFPoints, pos));

    QList<QPointF> pointsLine;
    pointsLine << point1 << point2 << point3 << point4;
    EXPECT_FALSE(pointOnArLine(pointsLine,  QPointF(55, 55)));

    //    /* resize arbitrary curved */
    EXPECT_EQ(2,  relativePosition(mainPoints, pos).size());
    FourPoints fourPointsvalue;
    fourPointsvalue << QPointF(8,5) << QPointF(5,6) << QPointF(9,8) << QPointF(5, 1);
    QList<qreal> re;
    re << 55 << 7;
    QPointF po = getNewPosition(fourPointsvalue, re);
    qreal x = po.x();
    qreal y = po.y();
    EXPECT_DOUBLE_EQ(3.9017857142857135, x);
    EXPECT_DOUBLE_EQ(1.4553571428571428, y);
    re.clear();
    re << -2 << -2;
    getNewPosition(fourPointsvalue, re);

    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(10, 10), QPoint(20, 30), QPoint(25, 5), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 5), QPoint(20, 15), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(20, 10), QPoint(25, 30), point4, pos, 7).size());


    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(20, 20), QPoint(35, 30), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 10), QPoint(30, 20), QPoint(35, 10), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 30), point4, pos, 7).size());

    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 0).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 1).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 2).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 3).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 4).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 5).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 6).size());
    EXPECT_EQ(4,  resizePointPosition(QPoint(30, 20), QPoint(40, 20), QPoint(30, 10), point4, pos, 7).size());

}
TEST_F(CalculaterectTest, test_point1Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    //    /***************** first point1 *******************/
    //    /* point1 in the first position */
    EXPECT_EQ(4,   point1Resize1(point1,  point2,  point3, point4,  pos).size());
    EXPECT_EQ(4,   point1Resize1(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point1 in the second position*/
    EXPECT_EQ(4,   point1Resize2(QPointF(30, 20),  QPointF(40, 5),  QPointF(20, 15), QPointF(0, 5),  QPointF(55, 55)).size());
    EXPECT_EQ(4,   point1Resize2(QPointF(30, 20),  QPointF(40, 5),  QPointF(20, 15), QPointF(0, 5),  QPointF(55, 55), true).size());
    //    /* point1 in the third position */
    EXPECT_EQ(4,   point1Resize3(QPointF(30, 20),  QPointF(20, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(15, 0)).size());
    EXPECT_EQ(4,   point1Resize3(QPointF(30, 20),  QPointF(20, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(15, 0), true).size());
    //    /* point1 in the fourth position */
    EXPECT_EQ(4,   point1Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 25), QPointF(20, 25),  QPointF(15, 20)).size());
    EXPECT_EQ(4,   point1Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(15, 100)).size());
    EXPECT_EQ(4,   point1Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(15, 100), true).size());
    //    /* point1 in the fifth position */
    EXPECT_EQ(4,   point1Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(5, 5),  QPointF(110, 14)).size());
    EXPECT_EQ(4,   point1Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(5, 5),  QPointF(110, 14), true).size());
    //    /* point1 in the sixth position */
    EXPECT_EQ(4,   point1Resize6(point1,  point2,  point3,  point4,  pos).size());
    EXPECT_EQ(4,   point1Resize6(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point1 in the seventh position */
    EXPECT_EQ(4,   point1Resize7(point1,  point2,  point3, point4,  pos).size());
    EXPECT_EQ(4,   point1Resize7(point1,  point2,  point3, point4,  pos, true).size());
}
TEST_F(CalculaterectTest, test_point2Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    //    /***************** second point2 *******************/
    //    /* point2 in the first position */
    EXPECT_EQ(4,   point2Resize1(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(45, 100)).size());
    //    /* point2 in the second position */
    EXPECT_EQ(4,   point2Resize2(point1,  point2,  point3, point4,  pos).size());
    //    /* point2 in the third position */
    EXPECT_EQ(4,   point2Resize3(QPointF(30, 20),  QPointF(20, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(10, 40)).size());
    //    /* point2 in the fourth position */
    EXPECT_EQ(4,   point2Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 125),  QPointF(15, 100)).size());
    //    /* point2 in the fifth position */
    EXPECT_EQ(4,   point2Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(25, 5),  QPointF(14, 140)).size());
    //    /* point2 in the sixth position */
    EXPECT_EQ(4,   point2Resize6(point1,  point2,  point3, point4,  pos).size());
    //    /* point2 in the seventh position */
    EXPECT_EQ(4,   point2Resize7(point1,  point2,  point3, point4,  pos).size());

    EXPECT_EQ(4,   point2Resize1(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(45, 100), true).size());
    //    /* point2 in the second position */
    EXPECT_EQ(4,   point2Resize2(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point2 in the third position */
    EXPECT_EQ(4,   point2Resize3(QPointF(30, 20),  QPointF(20, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(10, 40), true).size());
    //    /* point2 in the fourth position */
    EXPECT_EQ(4,   point2Resize4(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 125),  QPointF(15, 100), true).size());
    //    /* point2 in the fifth position */
    EXPECT_EQ(4,   point2Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(25, 5),  QPointF(14, 140), true).size());
    //    /* point2 in the sixth position */
    EXPECT_EQ(4,   point2Resize6(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point2 in the seventh position */
    EXPECT_EQ(4,   point2Resize7(point1,  point2,  point3, point4,  pos, true).size());
}
TEST_F(CalculaterectTest, test_point3Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    //    /***************** third point3 *******************/
    //    /* point3 in the first position */
    //

    // TTTTTTTTTTTTTTTTTTDDDDDDDDDDDDDDDDDDDDDDDDDDDd
    EXPECT_EQ(4,   point3Resize1(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(10, -14)).size());
    //    /* point3 in the second position */
    EXPECT_EQ(4,   point3Resize2(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(50, 40)).size());
    //    /* point3 in the third position */
    EXPECT_EQ(4,   point3Resize3(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 0)).size());
    //    /* point3 in the fourth position */
    EXPECT_EQ(4,   point3Resize4(point1,  point2,  point3, point4,  pos).size());
    //    /* point3 in the fifth position */
    EXPECT_EQ(4,   point3Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(5, 150),  QPointF(140, 140)).size());
    //    /* point3 in the sixth position */
    EXPECT_EQ(4,   point3Resize6(point1,  point2,  point3, point4,  pos).size());
    //    /* point3 in the seventh position */
    EXPECT_EQ(4,   point3Resize7(point1,  point2,  point3, point4,  pos).size());

    EXPECT_EQ(4,   point3Resize1(QPointF(30, 20),  QPointF(10, 5),  QPointF(20, 125), QPointF(20, 25),  QPointF(10, -14), true).size());
    //    /* point3 in the second position */
    EXPECT_EQ(4,   point3Resize2(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point3 in the third position */
    EXPECT_EQ(4,   point3Resize3(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 0), true).size());
    //    /* point3 in the fourth position */
    EXPECT_EQ(4,   point3Resize4(QPointF(30, 20),  QPointF(20, 10),  QPointF(25, 30), QPointF(0, 5),  QPointF(55, 55), true).size());
    //    /* point3 in the fifth position */
    EXPECT_EQ(4,   point3Resize5(QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 10), QPointF(5, 150),  QPointF(140, 140), true).size());
    //    /* point3 in the sixth position */
    EXPECT_EQ(4,   point3Resize6(point1,  point2,  point3, point4,  pos, true).size());
    //    /* point3 in the seventh position */
    EXPECT_EQ(4,   point3Resize7(point1,  point2,  point3, point4,  pos, true).size());
}
TEST_F(CalculaterectTest, test_point4Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    //    /***************** fourth point4 *******************/
    //    /* point4 in the first position */
    EXPECT_EQ(4,   point4Resize1( point1,  point2, point3,point4, pos).size());
    //    /* point4 in the second position */
    EXPECT_EQ(4,   point4Resize2( QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(20, 0)).size());
    //    /* point4 in the third position */
    EXPECT_EQ(4,   point4Resize3( QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 40)).size());
    //    /* point4 in the fourth position */
    EXPECT_EQ(4,   point4Resize4( QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 0)).size());
    //    /* point4 in the fifth position */
    EXPECT_EQ(4,   point4Resize5( QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(140, 140)).size());
    //    /* point4 in the sixth position */
    EXPECT_EQ(4,   point4Resize6( point1,  point2, point3,point4, pos).size());
    //    /* point4 in the seventh position */
    EXPECT_EQ(4,   point4Resize7( point1,  point2, point3,point4, pos).size());

    EXPECT_EQ(4,   point4Resize1( QPointF(10, 10), QPointF(20, 30), QPointF(25, 5), QPointF(0, 5), QPointF(55, 55), true).size());
    //    /* point4 in the second position */
    EXPECT_EQ(4,   point4Resize2( QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(20, 0), true).size());
    //    /* point4 in the third position */
    EXPECT_EQ(4,   point4Resize3(QPointF(30, 20),  QPointF(30, 5),  QPointF(20, 15), QPointF(20, 5),  QPointF(40, 40), true).size());
    //    /* point4 in the fourth position */
    EXPECT_EQ(4,   point4Resize4( point1,  point2, point3,point4, pos, true).size());
    //    /* point4 in the fifth position */
    EXPECT_EQ(4,   point4Resize5( QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(140, 140), true).size());
    //    /* point4 in the sixth position */
    EXPECT_EQ(4,   point4Resize6( point1,  point2, point3,point4, pos, true).size());
    //    /* point4 in the seventh position */
    EXPECT_EQ(4,   point4Resize7( point1,  point2, point3,point4, pos, true).size());
}
TEST_F(CalculaterectTest, test_point5Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    //    /********************** fifth point5 ************************/
    //    /* point5 in the first position */
    EXPECT_EQ(4,   point5Resize1( point1,  point2,  point3, point4,  pos).size());
    //    /* point5 in the second position */
    EXPECT_EQ(4,   point5Resize2( QPointF(30, 20), QPointF(20, 10), QPointF(25, 30), QPointF(0, 5), QPointF(55, 55)).size());
    //    /* point5 in the third position */
    EXPECT_EQ(4,   point5Resize3( QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -160)).size());
    //    /* point5 in the fourth position */
    EXPECT_EQ(4,   point5Resize4( QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(30, -25)).size());
    //    /* point5 in the fifth position */
    EXPECT_EQ(4,   point5Resize5( QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(60, 140)).size());
    //    /* point5 in the sixth position */
    EXPECT_EQ(4,   point5Resize6( point1,  point2,  point3, point4,  pos).size());
    //    /* point5 in the seventh position */
    EXPECT_EQ(4,   point5Resize7( point1,  point2,  point3, point4,  pos).size());

    EXPECT_EQ(4,   point5Resize1( point1,  point2,  point3, point4,  pos, true).size());
    //    /* point5 in the second position */
    EXPECT_EQ(4,   point5Resize2( QPointF(30, 20), QPointF(40, 5), QPointF(20, 15), QPointF(0, 5), QPointF(55, 55), true).size());
    //    /* point5 in the third position */
    EXPECT_EQ(4,   point5Resize3( QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -160), true).size());
    //    /* point5 in the fourth position */
    EXPECT_EQ(4,   point5Resize4( QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(30, -25), true).size());
    //    /* point5 in the fifth position */
    EXPECT_EQ(4,   point5Resize5( QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(60, 140), true).size());
    //    /* point5 in the sixth position */
    EXPECT_EQ(4,   point5Resize6( point1,  point2,  point3, point4,  pos, true).size());
    //    /* point5 in the seventh position */
    EXPECT_EQ(4,   point5Resize7( point1,  point2,  point3, point4,  pos, true).size());
}
TEST_F(CalculaterectTest, test_point6Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    /********************** sixth point6 ************************/
    /* point6 in the first position */
    EXPECT_EQ(4,    point6Resize1( QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -25)).size());
    /* point6 in the second position */
    EXPECT_EQ(4,    point6Resize2( QPointF(30, 20), QPointF(40, 5), QPointF(20, 15), QPointF(0, 5), QPointF(55, 55)).size());
    /* point6 in the third position */
    EXPECT_EQ(4,    point6Resize3( QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -50)).size());
    /* point6 in the fourth position */
    EXPECT_EQ(4,    point6Resize4( QPointF(30, 20), QPointF(20, 10), QPointF(25, 30), QPointF(0, 5), QPointF(55, 55)).size());
    /* point6 in the fifth position */
    EXPECT_EQ(4,    point6Resize5( point1, point2, point3, point4, pos).size());
    /* point6 in the sixth position */
    EXPECT_EQ(4,    point6Resize6( point1, point2, point3, point4, pos).size());
    /* point6 in the seventh position */
    EXPECT_EQ(4,    point6Resize7( point1, point2, point3, point4, pos).size());

    EXPECT_EQ(4,    point6Resize1( QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -25), true).size());
    /* point6 in the second position */
    EXPECT_EQ(4,    point6Resize2( QPointF(30, 20), QPointF(40, 5), QPointF(20, 15), QPointF(0, 5), QPointF(55, 55), true).size());
    /* point6 in the third position */
    EXPECT_EQ(4,    point6Resize3( QPointF(10, 10), QPointF(20, 20), QPointF(25, 5), QPointF(20, -20), QPointF(15, -50), true).size());
    /* point6 in the fourth position */
    EXPECT_EQ(4,    point6Resize4( QPointF(30, 20), QPointF(20, 10), QPointF(25, 30), QPointF(0, 5), QPointF(55, 55), true).size());
    /* point6 in the fifth position */
    EXPECT_EQ(4,    point6Resize5( point1, point2, point3, point4, pos, true).size());
    /* point6 in the sixth position */
    EXPECT_EQ(4,    point6Resize6( point1, point2, point3, point4, pos, true).size());
    /* point6 in the seventh position */
    EXPECT_EQ(4,    point6Resize7( point1, point2, point3, point4, pos, true).size());
}
TEST_F(CalculaterectTest, test_point7Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    /* point7 in the first position */
    EXPECT_EQ(4,    point7Resize1( point1, point2, point3, point4, pos).size());
    EXPECT_EQ(4,    point7Resize1( QPointF(10, 10), QPointF(20, 30), QPointF(25, 5), QPointF(0, 5), QPointF(55, 15)).size());
    /* point7 in the second position */
    EXPECT_EQ(4,    point7Resize2( QPointF(-10, -10), QPointF(20, 20), QPointF(25, 5), QPointF(0, -20), QPointF(0, -15)).size());
    /* point7 in the third position */
    EXPECT_EQ(4,    point7Resize3( QPointF(-10, -10), QPointF(20, 20), QPointF(25, 5), QPointF(0, 20), QPointF(10, 15)).size());
    /* point7 in the fourth position */
    EXPECT_EQ(4,    point7Resize4( QPointF(30, 20),  QPointF(20, 10),  QPointF(25, 30), QPointF(0, 5),  QPointF(55, 55)).size());
    /* point7 in the fifth position */
    EXPECT_EQ(4,    point7Resize5( QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(150, 140)).size());
    /* point7 in the sixth position */
    EXPECT_EQ(4,    point7Resize6( point1, point2, point3, point4, pos).size());
    /* point7 in the seventh position */
    EXPECT_EQ(4,    point7Resize7( point1, point2, point3, point4, pos, true).size());

    EXPECT_EQ(4,    point7Resize1( point1, point2, point3, point4, pos, true).size());
    /* point7 in the second position */
    EXPECT_EQ(4,    point7Resize2( QPointF(-10, -10), QPointF(20, 20), QPointF(25, 5), QPointF(0, -20), QPointF(0, -15), true).size());
    /* point7 in the third position */
    EXPECT_EQ(4,    point7Resize3( QPointF(-10, -10), QPointF(20, 20), QPointF(25, 5), QPointF(0, 20), QPointF(10, 15), true).size());
    /* point7 in the fourth position */
    EXPECT_EQ(4,    point7Resize4( QPointF(30, 20),  QPointF(20, 10),  QPointF(25, 30), QPointF(0, 5),  QPointF(55, 55), true).size());
    /* point7 in the fifth position */
    EXPECT_EQ(4,    point7Resize5( QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(150, 140), true).size());
    /* point7 in the sixth position */
    EXPECT_EQ(4,    point7Resize6( point1, point2, point3, point4, pos, true).size());
    /* point7 in the seventh position */
    EXPECT_EQ(4,    point7Resize7( point1, point2, point3, point4, pos, true).size());
}
TEST_F(CalculaterectTest, test_point8Resize)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    /* point8 in the first position */
    EXPECT_EQ(4,    point8Resize1( QPointF(10, 10), QPointF(20, 30), QPointF(25, 5), QPointF(0, 5), QPointF(55, 55)).size());
    /* point8 in the second position */
    EXPECT_EQ(4,    point8Resize2( point1, point2, point3, point4, pos).size());
    /* point8 in the third position */
    EXPECT_EQ(4,    point8Resize3( QPointF(30, 10), QPointF(20, 20), QPointF(35, 30), QPointF(0, 5), QPointF(10, 0)).size());
    /* point8 in the fourth position */
    EXPECT_EQ(4,    point8Resize4( QPointF(20, 30), QPointF(10, 10), QPointF(25, 5), QPointF(0, 5), QPointF(15, 5)).size());
    /* point8 in the fifth position */
    EXPECT_EQ(4,    point8Resize5( QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(150, 140)).size());
    /* point8 in the sixth position */
    EXPECT_EQ(4,    point8Resize6( point1, point2, point3, point4, pos).size());
    /* point8 in the seventh position */
    EXPECT_EQ(4,    point8Resize7( point1, point2, point3, point4, pos).size());

    EXPECT_EQ(4,    point8Resize1( QPointF(10, 10), QPointF(20, 30), QPointF(25, 5), QPointF(0, 5), QPointF(55, 55), true).size());
    /* point8 in the second position */
    EXPECT_EQ(4,    point8Resize2( point1, point2, point3, point4, pos, true).size());
    /* point8 in the third position */
    EXPECT_EQ(4,    point8Resize3( QPointF(30, 10), QPointF(20, 20), QPointF(35, 30), QPointF(0, 5), QPointF(10, 0), true).size());
    /* point8 in the fourth position */
    EXPECT_EQ(4,    point8Resize4( QPointF(20, 30), QPointF(10, 10), QPointF(25, 5), QPointF(0, 5), QPointF(15, 5), true).size());
    /* point8 in the fifth position */
    EXPECT_EQ(4,    point8Resize5( QPointF(130, 130),  QPointF(120, 120),  QPointF(150, 100), QPointF(5, 150),  QPointF(150, 140), true).size());
    /* point8 in the sixth position */
    EXPECT_EQ(4,    point8Resize6( point1, point2, point3, point4, pos, true).size());
    /* point8 in the seventh position */
    EXPECT_EQ(4,    point8Resize7( point1, point2, point3, point4, pos, true).size());
}
TEST_F(CalculaterectTest, test_pointMoveMicro)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);

    FourPoints fourPoints;
    fourPoints << point1 << point2 << point3 << point4;

    //    /************************ micro-adjust  **************************/
    QString dir = "Left"; // Left Right Up Down
    bool isBig = true;
    EXPECT_EQ(4,    pointMoveMicro(fourPoints,   dir).size());
    EXPECT_EQ(4,    pointMoveMicro(fourPoints,   "Right").size());
    EXPECT_EQ(4,    pointMoveMicro(fourPoints,   "Up").size());
    EXPECT_EQ(4,    pointMoveMicro(fourPoints,   "Down").size());
    EXPECT_EQ(4,    pointResizeMicro(fourPoints,  "Ctrl+Shift+Left",  isBig).size());
    EXPECT_EQ(4,    pointResizeMicro(fourPoints,  "Ctrl+Shift+Right",  isBig).size());
    EXPECT_EQ(4,    pointResizeMicro(fourPoints,  "Ctrl+Shift+Up",  isBig).size());
    EXPECT_EQ(4,    pointResizeMicro(fourPoints,  "Ctrl+Shift+Down",  isBig).size());
    fourPoints.clear();
    fourPoints << QPointF(10, 10) << QPointF(20, 20) << QPointF(20, 5) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());
    fourPoints.clear();
    fourPoints << QPointF(10, 10) << QPointF(20, 5) << QPointF(5, 5) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());
    fourPoints.clear();
    fourPoints << QPointF(10, 10) << QPointF(5, 20) << QPointF(20, 15) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());
    fourPoints.clear();
    fourPoints << QPointF(10, 10) << QPointF(20, 15) << QPointF(5, 15) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());

    fourPoints.clear();
    fourPoints << QPointF(30, 30) << QPointF(20, 15) << QPointF(5, 45) << QPointF(10, 10);
    EXPECT_EQ(4,    point5ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point6ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point7ResizeMicro(fourPoints, isBig).size());
    EXPECT_EQ(4,    point8ResizeMicro(fourPoints, isBig).size());
}

TEST_F(CalculaterectTest, test_getMainPoints)
{
    QPointF point1(130, 130);
    QPointF point2(120, 120);
    QPointF point3(5, 10);
    QPointF point4(5, 5);
    QPointF pos(110, 14);

    FourPoints fourPoints;
    fourPoints << point1 << point2 << point3 << point4;

    EXPECT_FALSE(pointInRect(fourPoints, pos));
    EXPECT_EQ(4,    getMainPoints(point1,  point2).size());
    EXPECT_EQ(4,    getMainPoints(point1,  point2, true).size());
    EXPECT_EQ(4,    getMainPoints(QPointF(20, 30),  QPointF(30, 40), true).size());
    EXPECT_EQ(4,    getMainPoints(QPointF(20, 30),  QPointF(30, 20), true).size());
    EXPECT_EQ(4,    getMainPoints(QPointF(20, 30),  QPointF(10, 40), true).size());
    EXPECT_EQ(4,    getMainPoints(QPointF(20, 30),  QPointF(10, 20), true).size());

}
