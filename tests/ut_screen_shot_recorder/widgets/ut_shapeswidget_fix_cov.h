// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPointF>
#include <QList>
#include <QWidget>
#include "addr_pri.h"
#include "../../src/utils/shapesutils.h"
#include "../../src/utils/baseutils.h"
#include "../../src/widgets/shapeswidget.h"

using namespace testing;

// ── New private-field / private-function declarations ──────────────
// Fields below are NOT declared in any sibling cov header (verified via grep).
// Already-declared fields used here (from other TUs): m_shapes, m_pos1, m_pos2,
// m_currentShape, m_pressedPoint, m_movingPoint, m_selectedShape, m_hoveredShape,
// m_selectedIndex, m_selectedOrder, m_hoveredIndex — NOT re-declared.
// Already-declared private functions: handlePaint — NOT re-declared.
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isPressed);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isSelected);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isResize);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isRotated);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isArrowRotated);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isMoving);
ACCESS_PRIVATE_FIELD(ShapesWidget, ResizeDirection, m_resizeDirection);
ACCESS_PRIVATE_FIELD(ShapesWidget, ShapesWidget::ClickedKey, m_clickedKey);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, FourPoints, int, int), paintSequence);

class ShapesWidgetFixCovTest : public Test
{
public:
    QWidget *m_dummy = nullptr;
    ShapesWidget *m_sw = nullptr;

    void SetUp() override
    {
        m_dummy = new QWidget;
        m_dummy->resize(800, 600);
        m_sw = new ShapesWidget(m_dummy);
        m_sw->resize(800, 600);
    }
    void TearDown() override
    {
        delete m_sw;
        delete m_dummy;
    }

    static FourPoints rectFP(qreal x, qreal y, qreal w, qreal h)
    {
        FourPoints fp;
        fp << QPointF(x, y) << QPointF(x + w, y)
           << QPointF(x + w, y + h) << QPointF(x, y + h);
        return fp;
    }

    static Toolshape makeShape(const QString &type)
    {
        Toolshape s;
        s.type = type;
        s.mainPoints = rectFP(20, 20, 200, 120);
        s.points = QList<QPointF>{QPointF(20, 20), QPointF(220, 140)};
        s.colorIndex = 0;
        s.lineWidth = 3;
        s.isBlur = false;
        s.isOval = 0;
        s.radius = 10;
        s.index = 0;
        return s;
    }
};

// ── isPressed() (commit c1e1741a) ──────────────────────────────────

TEST_F(ShapesWidgetFixCovTest, isPressedDefaultFalse)
{
    EXPECT_FALSE(m_sw->isPressed());
}

TEST_F(ShapesWidgetFixCovTest, isPressedTrueAfterSet)
{
    access_private_field::ShapesWidgetm_isPressed(*m_sw) = true;
    EXPECT_TRUE(m_sw->isPressed());
    access_private_field::ShapesWidgetm_isPressed(*m_sw) = false;
    EXPECT_FALSE(m_sw->isPressed());
}

// ── clearSelected() resets stale resize/rotate state (commit c1e1741a) ─

TEST_F(ShapesWidgetFixCovTest, clearSelectedResetsStateFlags)
{
    // Pollute all state flags with non-default values.
    access_private_field::ShapesWidgetm_isSelected(*m_sw) = true;
    access_private_field::ShapesWidgetm_isResize(*m_sw) = true;
    access_private_field::ShapesWidgetm_isRotated(*m_sw) = true;
    access_private_field::ShapesWidgetm_isArrowRotated(*m_sw) = true;
    access_private_field::ShapesWidgetm_isMoving(*m_sw) = true;
    access_private_field::ShapesWidgetm_resizeDirection(*m_sw) = TopLeft;
    access_private_field::ShapesWidgetm_clickedKey(*m_sw) = ShapesWidget::First;

    EXPECT_NO_FATAL_FAILURE(m_sw->clearSelected());

    EXPECT_FALSE(access_private_field::ShapesWidgetm_isSelected(*m_sw));
    EXPECT_FALSE(access_private_field::ShapesWidgetm_isResize(*m_sw));
    EXPECT_FALSE(access_private_field::ShapesWidgetm_isRotated(*m_sw));
    EXPECT_FALSE(access_private_field::ShapesWidgetm_isArrowRotated(*m_sw));
    EXPECT_FALSE(access_private_field::ShapesWidgetm_isMoving(*m_sw));
    EXPECT_EQ(access_private_field::ShapesWidgetm_resizeDirection(*m_sw), Outting);
    EXPECT_EQ(access_private_field::ShapesWidgetm_clickedKey(*m_sw), ShapesWidget::Unknow);
}

// ── clickedOnShapes() resets stale resize/rotate state (commit c1e1741a) ─

TEST_F(ShapesWidgetFixCovTest, clickedOnShapesResetsStateFlags)
{
    // No shapes present → clickedOnShapes returns false, but the reset
    // at the top of the function must still execute.
    access_private_field::ShapesWidgetm_isResize(*m_sw) = true;
    access_private_field::ShapesWidgetm_isRotated(*m_sw) = true;
    access_private_field::ShapesWidgetm_isArrowRotated(*m_sw) = true;
    access_private_field::ShapesWidgetm_resizeDirection(*m_sw) = TopLeft;
    access_private_field::ShapesWidgetm_clickedKey(*m_sw) = ShapesWidget::First;

    bool result = false;
    EXPECT_NO_FATAL_FAILURE(result = m_sw->clickedOnShapes(QPointF(50, 50)));
    EXPECT_FALSE(result);

    EXPECT_FALSE(access_private_field::ShapesWidgetm_isResize(*m_sw));
    EXPECT_FALSE(access_private_field::ShapesWidgetm_isRotated(*m_sw));
    EXPECT_FALSE(access_private_field::ShapesWidgetm_isArrowRotated(*m_sw));
    EXPECT_EQ(access_private_field::ShapesWidgetm_resizeDirection(*m_sw), Outting);
    EXPECT_EQ(access_private_field::ShapesWidgetm_clickedKey(*m_sw), ShapesWidget::Unknow);
}

// ── translateAllShapes() with null delta → early return (commit c1e1741a) ─

TEST_F(ShapesWidgetFixCovTest, translateAllShapesNullDelta)
{
    Toolshapes &shapes = access_private_field::ShapesWidgetm_shapes(*m_sw);
    shapes.append(makeShape("rectangle"));
    QPointF originalMain0 = shapes[0].mainPoints[0];

    EXPECT_NO_FATAL_FAILURE(m_sw->translateAllShapes(QPointF(0, 0)));

    // Points should not change for null delta.
    EXPECT_EQ(shapes[0].mainPoints[0], originalMain0);
}

// ── translateAllShapes() with real shapes → points shift (commit c1e1741a) ─

TEST_F(ShapesWidgetFixCovTest, translateAllShapesShiftsPoints)
{
    Toolshapes &shapes = access_private_field::ShapesWidgetm_shapes(*m_sw);
    shapes.append(makeShape("rectangle"));

    QPointF delta(10, 20);
    QPointF expectedMain0 = shapes[0].mainPoints[0] + delta;
    QPointF expectedArrow = shapes[0].arrowRotatePos + delta;

    // Set non-null arrowRotatePos to test that branch too.
    shapes[0].arrowRotatePos = QPointF(5, 5);
    expectedArrow = QPointF(5, 5) + delta;

    EXPECT_NO_FATAL_FAILURE(m_sw->translateAllShapes(delta));

    EXPECT_EQ(shapes[0].mainPoints[0], expectedMain0);
    EXPECT_EQ(shapes[0].arrowRotatePos, expectedArrow);
}

// ── translateAllShapes() skips empty Toolshape (commit c1e1741a) ───

TEST_F(ShapesWidgetFixCovTest, translateAllShapesSkipsEmptyShape)
{
    Toolshapes &shapes = access_private_field::ShapesWidgetm_shapes(*m_sw);
    // Append a default-constructed Toolshape (all mainPoints are (0,0)).
    shapes.append(Toolshape());

    QPointF delta(10, 20);
    EXPECT_NO_FATAL_FAILURE(m_sw->translateAllShapes(delta));

    // Empty shape's mainPoints must remain (0,0) — not shifted.
    for (const QPointF &pt : shapes[0].mainPoints) {
        EXPECT_TRUE(pt.isNull());
    }
}

// ── translateAllShapes() syncs mouse anchor points (commit c1e1741a) ─

TEST_F(ShapesWidgetFixCovTest, translateAllShapesSyncsMouseAnchors)
{
    // Set non-null anchor points.
    access_private_field::ShapesWidgetm_pos1(*m_sw) = QPointF(100, 100);
    access_private_field::ShapesWidgetm_pos2(*m_sw) = QPointF(200, 200);
    access_private_field::ShapesWidgetm_pressedPoint(*m_sw) = QPointF(50, 50);
    access_private_field::ShapesWidgetm_movingPoint(*m_sw) = QPointF(60, 60);

    QPointF delta(10, 5);
    EXPECT_NO_FATAL_FAILURE(m_sw->translateAllShapes(delta));

    EXPECT_EQ(access_private_field::ShapesWidgetm_pos1(*m_sw), QPointF(110, 105));
    EXPECT_EQ(access_private_field::ShapesWidgetm_pos2(*m_sw), QPointF(210, 205));
    EXPECT_EQ(access_private_field::ShapesWidgetm_pressedPoint(*m_sw), QPointF(60, 55));
    EXPECT_EQ(access_private_field::ShapesWidgetm_movingPoint(*m_sw), QPointF(70, 65));
}

// ── translateAllShapes() does not shift null anchor points (commit c1e1741a) ─

TEST_F(ShapesWidgetFixCovTest, translateAllShapesNullAnchorsUnchanged)
{
    // Anchors are null by default.
    EXPECT_TRUE(access_private_field::ShapesWidgetm_pos1(*m_sw).isNull());
    EXPECT_TRUE(access_private_field::ShapesWidgetm_pos2(*m_sw).isNull());
    EXPECT_TRUE(access_private_field::ShapesWidgetm_pressedPoint(*m_sw).isNull());
    EXPECT_TRUE(access_private_field::ShapesWidgetm_movingPoint(*m_sw).isNull());

    EXPECT_NO_FATAL_FAILURE(m_sw->translateAllShapes(QPointF(10, 10)));

    // Null anchors must remain null (guard prevents adding delta).
    EXPECT_TRUE(access_private_field::ShapesWidgetm_pos1(*m_sw).isNull());
    EXPECT_TRUE(access_private_field::ShapesWidgetm_pos2(*m_sw).isNull());
    EXPECT_TRUE(access_private_field::ShapesWidgetm_pressedPoint(*m_sw).isNull());
    EXPECT_TRUE(access_private_field::ShapesWidgetm_movingPoint(*m_sw).isNull());
}

// ── paintSequence() save/restore (commit c1e1741a) ────────────────
// Verify paintSequence doesn't crash and correctly restores painter state.
// The fix adds painter.save()/painter.restore() around the sequence drawing.

TEST_F(ShapesWidgetFixCovTest, paintSequenceNoCrashAndRestoresPainter)
{
    QImage img(300, 300, QImage::Format_ARGB32);
    img.fill(Qt::white);
    {
        QPainter painter(&img);
        QBrush originalBrush = painter.brush();
        QPen originalPen = painter.pen();

        FourPoints fp = rectFP(50, 50, 100, 100);
        EXPECT_NO_FATAL_FAILURE(
            call_private_fun::ShapesWidgetpaintSequence(*m_sw, painter, fp, 1, 0));

        // After paintSequence, painter state should be restored to original.
        EXPECT_EQ(painter.brush(), originalBrush);
        EXPECT_EQ(painter.pen(), originalPen);
    }
}

// ── paintSequence() early return on empty mainFPoints (commit c1e1741a) ─

TEST_F(ShapesWidgetFixCovTest, paintSequenceEmptyPointsEarlyReturn)
{
    QImage img(300, 300, QImage::Format_ARGB32);
    img.fill(Qt::white);
    {
        QPainter painter(&img);
        FourPoints empty;
        EXPECT_NO_FATAL_FAILURE(
            call_private_fun::ShapesWidgetpaintSequence(*m_sw, painter, empty, 1, 0));
    }
}
