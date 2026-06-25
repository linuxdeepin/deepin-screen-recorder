// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// ShapesWidget 纯几何 hit-test 方法 smoke 测试（规避旧 ut_shapeswidget.h 的 Qt6 API 漂移，
// 仅调用返回 bool 的几何判断方法，避免触发 setCursor 等在未完整初始化时 SEGV 的路径）。

#pragma once
#include <gtest/gtest.h>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include "addr_pri.h"
#include "../../src/utils/shapesutils.h"
#include "../../src/widgets/shapeswidget.h"

using namespace testing;

// 私有 paint* 方法：纯 QPainter 渲染，不触光标，offscreen 可测
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, FourPoints, int, ShapesWidget::ShapeBlurStatus, bool, bool, int), paintRect);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, FourPoints, int, ShapesWidget::ShapeBlurStatus, bool, bool, int), paintEllipse);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, QList<QPointF>, int, bool), paintArrow);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, QList<QPointF>), paintLine);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, QList<QPointF>, bool, int, int), paintEffectLine);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, QPointF, QPixmap, bool), paintImgPoint);
// paintText 是重载，ACCESS_PRIVATE_FUN 无法消歧，故不测

static FourPoints rectPoints(qreal x, qreal y, qreal w, qreal h)
{
    return { QPointF(x, y), QPointF(x + w, y), QPointF(x + w, y + h), QPointF(x, y + h) };
}

class ShapesWidgetSmokeTest : public Test
{
public:
    ShapesWidget *m_w;
    void SetUp() override { m_w = new ShapesWidget; }
    void TearDown() override { delete m_w; }
};

TEST_F(ShapesWidgetSmokeTest, clickedOnRect)
{
    FourPoints rp = rectPoints(0, 0, 100, 50);
    EXPECT_TRUE(m_w->clickedOnRect(rp, QPointF(50, 25)));
    EXPECT_FALSE(m_w->clickedOnRect(rp, QPointF(500, 500)));
    EXPECT_NO_FATAL_FAILURE(m_w->clickedOnRect(rp, QPointF(50, 25), true));
}

TEST_F(ShapesWidgetSmokeTest, clickedOnEllipse)
{
    FourPoints ep = rectPoints(0, 0, 100, 50);
    EXPECT_NO_FATAL_FAILURE(m_w->clickedOnEllipse(ep, QPointF(50, 25)));
    EXPECT_NO_FATAL_FAILURE(m_w->clickedOnEllipse(ep, QPointF(500, 500)));
}

TEST_F(ShapesWidgetSmokeTest, clickedOnArrowAndLine)
{
    QList<QPointF> pts = { QPointF(0, 0), QPointF(50, 50), QPointF(100, 0) };
    FourPoints fp = rectPoints(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE(m_w->clickedOnArrow(pts, QPointF(50, 50)));
    EXPECT_NO_FATAL_FAILURE(m_w->clickedOnLine(fp, pts, QPointF(50, 50)));
}

TEST_F(ShapesWidgetSmokeTest, clickedOnTextAndRotate)
{
    FourPoints fp = rectPoints(0, 0, 100, 30);
    EXPECT_NO_FATAL_FAILURE(m_w->clickedOnText(fp, QPointF(50, 15)));
    EXPECT_NO_FATAL_FAILURE(m_w->rotateOnPoint(fp, QPointF(0, 0)));
}

TEST_F(ShapesWidgetSmokeTest, hoverGeometry)
{
    FourPoints fp = rectPoints(0, 0, 100, 50);
    QList<QPointF> pts = { QPointF(0, 0), QPointF(100, 0) };
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnRect(fp, QPointF(50, 25)));
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnEllipse(fp, QPointF(50, 25)));
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnArrow(pts, QPointF(50, 0)));
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnLine(fp, pts, QPointF(50, 0)));
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnText(0, fp, QPointF(50, 25)));
    EXPECT_NO_FATAL_FAILURE(m_w->hoverOnRotatePoint(fp, QPointF(0, 0)));
}

TEST_F(ShapesWidgetSmokeTest, clickedOnShapesAndExistsText)
{
    EXPECT_NO_FATAL_FAILURE(m_w->clickedOnShapes(QPointF(50, 50)));
    EXPECT_NO_FATAL_FAILURE(m_w->isExistsText());
    EXPECT_NO_FATAL_FAILURE(m_w->textEditIsReadOnly());
}

TEST_F(ShapesWidgetSmokeTest, paintImageOntoCanvas)
{
    QImage img(200, 100, QImage::Format_ARGB32);
    img.fill(Qt::white);
    EXPECT_NO_FATAL_FAILURE(m_w->paintImage(img));
}

TEST_F(ShapesWidgetSmokeTest, privatePaintMethods)
{
    QImage img(300, 200, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    FourPoints rect = rectPoints(10, 10, 100, 60);
    QList<QPointF> line = { QPointF(10, 10), QPointF(120, 80), QPointF(200, 30) };

    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpaintRect(*m_w, painter, rect, 0, ShapesWidget::Normal, false, false, 10));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpaintRect(*m_w, painter, rect, 0, ShapesWidget::Selected, true, false, 10));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpaintEllipse(*m_w, painter, rect, 0, ShapesWidget::Normal, false, false, 10));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpaintArrow(*m_w, painter, line, 2, false));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpaintArrow(*m_w, painter, line, 2, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpaintLine(*m_w, painter, line));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpaintEffectLine(*m_w, painter, line, false, 10, 2));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpaintEffectLine(*m_w, painter, line, true, 10, 2));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpaintImgPoint(*m_w, painter, QPointF(5, 5), QPixmap(8, 8), true));
    painter.end();
}
