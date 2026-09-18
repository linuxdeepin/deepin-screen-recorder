// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QImage>
#include <QPainter>
#include <QPointF>
#include <QList>
#include "addr_pri.h"
#include "../../src/utils/shapesutils.h"
#include "../../src/widgets/shapeswidget.h"

using namespace testing;

// handlePaint 依据 m_shapes 中每个 shape 的 type 分发到 paintRect/paintEllipse/
// paintArrow/paintLine/paintEffectLine/paintText。一次性塞入各种 type 的 shape 再
// 调用 handlePaint，即可覆盖整个绘制子系统。
// 注：ut_shapeswidget.h 因 API drift 被禁用，故 m_shapes/m_pos1/m_currentShape/m_currentType
// 在本 TU 中尚无声明，需在此补齐（其余 paint*/event 访问器由 _ext/_ext2 提供）。
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &), handlePaint);
ACCESS_PRIVATE_FIELD(ShapesWidget, QPointF, m_pos2);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_clearAllTextBorder);
ACCESS_PRIVATE_FIELD(ShapesWidget, Toolshapes, m_shapes);
ACCESS_PRIVATE_FIELD(ShapesWidget, QPointF, m_pos1);
ACCESS_PRIVATE_FIELD(ShapesWidget, Toolshape, m_currentShape);
ACCESS_PRIVATE_FIELD(ShapesWidget, QString, m_currentType);

class ShapesWidgetPaintCovTest : public Test
{
public:
    ShapesWidget *m_w = nullptr;
    void SetUp() override { m_w = new ShapesWidget; }
    void TearDown() override { delete m_w; }

    static FourPoints rectFP()
    {
        FourPoints fp;
        fp << QPointF(10, 10) << QPointF(10, 110) << QPointF(110, 10) << QPointF(110, 110);
        return fp;
    }
    static QList<QPointF> twoPts() { return {QPointF(10, 10), QPointF(110, 110)}; }
    static QList<QPointF> penPts() { return {QPointF(10, 10), QPointF(50, 60), QPointF(110, 110)}; }

    static Toolshape makeShape(const QString &type)
    {
        Toolshape s;
        s.type = type;
        s.mainPoints = rectFP();
        s.points = (type == "pen") ? penPts() : twoPts();
        s.colorIndex = 0;
        s.lineWidth = 3;
        s.isBlur = true;
        s.isOval = 0;
        s.radius = 10;
        s.index = 0;
        return s;
    }

    void paintOnce()
    {
        QImage img(200, 200, QImage::Format_ARGB32);
        img.fill(Qt::white);
        {
            QPainter p(&img);
            EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgethandlePaint(*m_w, p));
        }
    }
};

// 各种 type 的 shape 一次性走完所有 paint* 辅助函数
TEST_F(ShapesWidgetPaintCovTest, paintAllShapeTypes)
{
    Toolshapes &shapes = access_private_field::ShapesWidgetm_shapes(*m_w);
    shapes.append(makeShape("rectangle"));
    shapes.append(makeShape("oval"));
    Toolshape e0 = makeShape("effect"); e0.isOval = 0; shapes.append(e0);
    Toolshape e1 = makeShape("effect"); e1.isOval = 1; shapes.append(e1);
    Toolshape e2 = makeShape("effect"); e2.isOval = 2; shapes.append(e2);
    shapes.append(makeShape("arrow"));
    shapes.append(makeShape("line"));
    shapes.append(makeShape("pen"));
    paintOnce();
}

// 当前正在绘制的图形分支（m_pos1/m_pos2 非零 + m_currentType 各值）
TEST_F(ShapesWidgetPaintCovTest, paintCurrentShapeBranches)
{
    access_private_field::ShapesWidgetm_pos1(*m_w) = QPointF(10, 10);
    access_private_field::ShapesWidgetm_pos2(*m_w) = QPointF(110, 110);
    Toolshape &cur = access_private_field::ShapesWidgetm_currentShape(*m_w);
    cur = makeShape("rectangle");

    for (const QString &t : {"rectangle", "oval", "arrow", "line", "pen"}) {
        access_private_field::ShapesWidgetm_currentType(*m_w) = t;
        cur.type = t;
        EXPECT_NO_FATAL_FAILURE(paintOnce());
    }
    // effect 当前分支：isOval 0/1/2
    access_private_field::ShapesWidgetm_currentType(*m_w) = "effect";
    for (int o : {0, 1, 2}) {
        cur.isOval = o;
        EXPECT_NO_FATAL_FAILURE(paintOnce());
    }
}

// 空形状列表 + 无当前形状：handlePaint 早返回路径
TEST_F(ShapesWidgetPaintCovTest, paintEmpty)
{
    EXPECT_NO_FATAL_FAILURE(paintOnce());
}
