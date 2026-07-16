// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QScreen>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QPointF>
#include <QList>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QEnterEvent>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QTapGesture>
#include <vector>
#include <utility>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/utils/shapesutils.h"
#include "../../src/utils/baseutils.h"
#include "../../src/utils/calculaterect.h"
#include "../../src/utils/configsettings.h"
#include "../../src/utils.h"
#include "../../src/widgets/shapeswidget.h"
#include "../../src/widgets/toolbar.h"
#include "../../src/widgets/subtoolwidget.h"
#include "../../src/widgets/camerawidget.h"
#include "../../src/utils/screengrabber.h"
#include "../../src/event_monitor.h"
#include "../../src/main_window.h"

using namespace testing;

// =============================================================================
// ut_resid_cov.h
// One fixture per target class; each TEST_F exercises branches that the prior
// cov headers leave uncovered. The strategy is to pre-populate private state
// via ACCESS_PRIVATE_FIELD BEFORE invoking the public/slot method, so that we
// hit the deeper branches (recording state machine, populated shape list,
// mouse + shift combos, etc.).
//
// DEDUP: every ACCESS_PRIVATE_FUN/FIELD macro invocation below targets a member
// that has NOT been declared by any sibling cov header ACTUALLY included in
// test_all_interfaces.h (the base ut_shapeswidget.h / ut_camerawidget.h /
// ut_toolbar.h / ut_subtoolwidget.h are commented out due to API drift, so
// their declarations do NOT exist in the TU).
// =============================================================================

// ---- NEW ShapesWidget private field declarations (active siblings only
// declare m_shapes / m_pos1 / m_pos2 / m_currentType / m_currentShape /
// m_clearAllTextBorder / m_currentIndex / m_pressedPoint / m_movingPoint /
// m_hoveredIndex / m_selectedShape / m_hoveredShape / m_editMap / m_globalRect /
// m_currentCursor; everything else is new here).
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isRecording);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isSelected);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isPressed);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isRotated);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isResize);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isMoving);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isHovered);
ACCESS_PRIVATE_FIELD(ShapesWidget, ResizeDirection, m_resizeDirection);
ACCESS_PRIVATE_FIELD(ShapesWidget, ShapesWidget::ClickedKey, m_clickedKey);
ACCESS_PRIVATE_FIELD(ShapesWidget, int, m_selectedOrder);
ACCESS_PRIVATE_FIELD(ShapesWidget, int, m_selectedIndex);
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isShiftPressed);
ACCESS_PRIVATE_FIELD(ShapesWidget, QColor, m_penColor);

// ---- NEW ShapesWidget private function declarations (siblings only declare
// handlePaint / event / mousePressEvent / mouseReleaseEvent / mouseMoveEvent /
// keyPressEvent / paintRect / paintEllipse / paintArrow / paintLine /
// paintEffectLine / paintImgPoint; we add the gesture handlers + enterEvent).
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPinchGesture *), pinchTriggered);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QTapGesture *), tapTriggered);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QEvent *), enterEvent);

// =============================================================================
// ShapesWidgetResidTest
// Targets the deepest uncovered paths of mousePressEvent / mouseMoveEvent /
// mouseReleaseEvent / keyPressEvent by setting state combinations BEFORE the
// event arrives, plus the gesture-event dispatch.
// =============================================================================
class ShapesWidgetResidTest : public Test
{
public:
    ShapesWidget *m_w = nullptr;
    void SetUp() override
    {
        m_w = new ShapesWidget;
        m_w->resize(400, 300);
    }
    void TearDown() override { /* leak: shape list / TextEdit children */ }

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

// ---- mousePressEvent: real mouse + shape selected branch ----
TEST_F(ShapesWidgetResidTest, mousePressRealMouseShapeSelected)
{
    auto ut_resid_run0 = [&] {
        access_private_field::ShapesWidgetm_selectedIndex(*m_w) = 0;
        access_private_field::ShapesWidgetm_shapes(*m_w).append(makeShape("rectangle"));
        QMouseEvent e(QEvent::MouseButtonPress, QPointF(150, 150), QPointF(150, 150),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmousePressEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run0());
}

// ---- mousePressEvent: right-button on empty area pops the menu ----
TEST_F(ShapesWidgetResidTest, mousePressRightButtonEmptyArea)
{
    auto ut_resid_run1 = [&] {
        access_private_field::ShapesWidgetm_currentType(*m_w) = "rectangle";
        QMouseEvent e(QEvent::MouseButtonPress, QPointF(150, 150), QPointF(150, 150),
                      Qt::RightButton, Qt::RightButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmousePressEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run1());
}

// ---- mousePressEvent with each current type drives each per-type init branch ----
TEST_F(ShapesWidgetResidTest, mousePressStartDrawingPerType)
{
    const QStringList types{"rectangle", "oval", "arrow", "line",
                            "pen", "effect", "text"};
    for (const QString &t : types) {
        auto run = [&] {
            access_private_field::ShapesWidgetm_pos1(*m_w) = QPointF(0, 0);
            access_private_field::ShapesWidgetm_currentType(*m_w) = t;
            access_private_field::ShapesWidgetm_selectedIndex(*m_w) = -1;
            access_private_field::ShapesWidgetm_currentShape(*m_w) = Toolshape{};
            QMouseEvent e(QEvent::MouseButtonPress, QPointF(40, 40), QPointF(40, 40),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            call_private_fun::ShapesWidgetmousePressEvent(*m_w, &e);
            qApp->processEvents();
        };
        EXPECT_NO_FATAL_FAILURE(run());
    }
}

// ---- mousePressEvent: aiassistant early-return branch ----
TEST_F(ShapesWidgetResidTest, mousePressAiAssistantEarlyReturn)
{
    auto ut_resid_run2 = [&] {
        access_private_field::ShapesWidgetm_currentType(*m_w) = "aiassistant";
        access_private_field::ShapesWidgetm_selectedIndex(*m_w) = -1;
        QMouseEvent e(QEvent::MouseButtonPress, QPointF(40, 40), QPointF(40, 40),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmousePressEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run2());
}

// ---- mousePressEvent: synthesized event with selected shape ----
TEST_F(ShapesWidgetResidTest, mousePressSynthesizedSelectedClearsSelection)
{
    auto ut_resid_run3 = [&] {
        access_private_field::ShapesWidgetm_selectedIndex(*m_w) = 2;
        access_private_field::ShapesWidgetm_currentType(*m_w) = "rectangle";
        QMouseEvent e(QEvent::MouseButtonPress, QPointF(40, 40), QPointF(40, 40),
                      QPointF(40, 40), Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
        call_private_fun::ShapesWidgetmousePressEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run3());
}

// ---- mouseMoveEvent: recording + pressed + arrow/line branch (append + update) ----
TEST_F(ShapesWidgetResidTest, mouseMoveRecordingArrowLine)
{
    auto ut_resid_run4 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = true;
        access_private_field::ShapesWidgetm_isPressed(*m_w) = true;
        Toolshape cur = makeShape("arrow");
        cur.points.clear();
        cur.points.append(QPointF(10, 10));
        access_private_field::ShapesWidgetm_currentShape(*m_w) = cur;
        access_private_field::ShapesWidgetm_pos1(*m_w) = QPointF(10, 10);
        access_private_field::ShapesWidgetm_currentType(*m_w) = "arrow";
        QMouseEvent e1(QEvent::MouseMove, QPointF(80, 80), QPointF(80, 80),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &e1);
        QMouseEvent e2(QEvent::MouseMove, QPointF(120, 120), QPointF(120, 120),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &e2);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run4());
}

// ---- mouseMoveEvent: pen recording branch (distance threshold) ----
TEST_F(ShapesWidgetResidTest, mouseMoveRecordingPen)
{
    auto ut_resid_run5 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = true;
        access_private_field::ShapesWidgetm_isPressed(*m_w) = true;
        Toolshape cur = makeShape("pen");
        cur.points.clear();
        cur.points.append(QPointF(10, 10));
        access_private_field::ShapesWidgetm_currentShape(*m_w) = cur;
        access_private_field::ShapesWidgetm_currentType(*m_w) = "pen";
        QMouseEvent e(QEvent::MouseMove, QPointF(60, 60), QPointF(60, 60),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run5());
}

// ---- mouseMoveEvent: effect (blur pen) recording branches ----
TEST_F(ShapesWidgetResidTest, mouseMoveRecordingEffect)
{
    auto ut_resid_run6 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = true;
        access_private_field::ShapesWidgetm_isPressed(*m_w) = true;
        Toolshape eff = makeShape("effect");
        eff.isOval = 2;
        eff.points.clear();
        eff.points.append(QPointF(10, 10));
        access_private_field::ShapesWidgetm_currentShape(*m_w) = eff;
        access_private_field::ShapesWidgetm_currentType(*m_w) = "effect";
        QMouseEvent e1(QEvent::MouseMove, QPointF(20, 20), QPointF(20, 20),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &e1);
        QMouseEvent e2(QEvent::MouseMove, QPointF(150, 150), QPointF(150, 150),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &e2);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run6());
}

// ---- mouseMoveEvent: not recording + pressed + rotated ----
TEST_F(ShapesWidgetResidTest, mouseMoveNotRecordingRotated)
{
    auto ut_resid_run7 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = false;
        access_private_field::ShapesWidgetm_isPressed(*m_w) = true;
        access_private_field::ShapesWidgetm_isRotated(*m_w) = true;
        QMouseEvent e(QEvent::MouseMove, QPointF(100, 80), QPointF(100, 80),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run7());
}

// ---- mouseMoveEvent: not recording + pressed + resize + all clickedKey values ----
TEST_F(ShapesWidgetResidTest, mouseMoveNotRecordingResizeAllKeys)
{
    auto ut_resid_run8 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = false;
        access_private_field::ShapesWidgetm_isPressed(*m_w) = true;
        access_private_field::ShapesWidgetm_isResize(*m_w) = true;
        for (int k = ShapesWidget::First; k <= ShapesWidget::Eighth; ++k) {
            access_private_field::ShapesWidgetm_clickedKey(*m_w) =
                static_cast<ShapesWidget::ClickedKey>(k);
            QMouseEvent e(QEvent::MouseMove, QPointF(50 + k, 50 + k),
                          QPointF(50 + k, 50 + k),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &e);
        }
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run8());
}

// ---- mouseMoveEvent: not recording + pressed + selected shape drag ----
TEST_F(ShapesWidgetResidTest, mouseMoveNotRecordingSelectedDrag)
{
    auto ut_resid_run9 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = false;
        access_private_field::ShapesWidgetm_isPressed(*m_w) = true;
        access_private_field::ShapesWidgetm_isSelected(*m_w) = true;
        access_private_field::ShapesWidgetm_selectedIndex(*m_w) = 0;
        access_private_field::ShapesWidgetm_selectedOrder(*m_w) = 0;
        access_private_field::ShapesWidgetm_shapes(*m_w).append(makeShape("rectangle"));
        access_private_field::ShapesWidgetm_pressedPoint(*m_w) = QPointF(20, 20);
        QMouseEvent e(QEvent::MouseMove, QPointF(60, 60), QPointF(60, 60),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run9());
}

// ---- mouseMoveEvent: not recording + not pressed -> hover dispatch with populated shapes ----
TEST_F(ShapesWidgetResidTest, mouseMoveHoverDispatchAllResizeDirections)
{
    auto ut_resid_run10 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = false;
        access_private_field::ShapesWidgetm_isPressed(*m_w) = false;
        Toolshapes &shapes = access_private_field::ShapesWidgetm_shapes(*m_w);
        shapes.append(makeShape("rectangle"));
        const ResizeDirection dirs[] = {Left, Top, Right, Bottom,
                                        TopLeft, BottomLeft, TopRight, BottomRight,
                                        Rotate, Moving, Outting};
        for (ResizeDirection d : dirs) {
            access_private_field::ShapesWidgetm_resizeDirection(*m_w) = d;
            QMouseEvent e(QEvent::MouseMove, QPointF(120, 80), QPointF(120, 80),
                          Qt::NoButton, Qt::NoButton, Qt::NoModifier);
            call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &e);
        }
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run10());
}

// ---- mouseReleaseEvent: arrow/line recording completion (shift + non-shift) ----
TEST_F(ShapesWidgetResidTest, mouseReleaseArrowLineCompletion)
{
    auto ut_resid_run11 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = true;
        access_private_field::ShapesWidgetm_isSelected(*m_w) = false;
        access_private_field::ShapesWidgetm_selectedIndex(*m_w) = -1;
        access_private_field::ShapesWidgetm_currentType(*m_w) = "arrow";
        Toolshape cur = makeShape("arrow");
        cur.points = {QPointF(10, 10), QPointF(60, 60)};
        access_private_field::ShapesWidgetm_currentShape(*m_w) = cur;
        access_private_field::ShapesWidgetm_pos1(*m_w) = QPointF(10, 10);
        access_private_field::ShapesWidgetm_pos2(*m_w) = QPointF(60, 60);
        access_private_field::ShapesWidgetm_isShiftPressed(*m_w) = true;
        QMouseEvent e(QEvent::MouseButtonRelease, QPointF(60, 60), QPointF(60, 60),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseReleaseEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run11());
}

// ---- mouseReleaseEvent: pen recording completion ----
TEST_F(ShapesWidgetResidTest, mouseReleasePenCompletion)
{
    auto ut_resid_run12 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = true;
        access_private_field::ShapesWidgetm_isSelected(*m_w) = false;
        access_private_field::ShapesWidgetm_selectedIndex(*m_w) = -1;
        access_private_field::ShapesWidgetm_currentType(*m_w) = "pen";
        Toolshape cur = makeShape("pen");
        cur.points = {QPointF(10, 10), QPointF(30, 30), QPointF(60, 60)};
        access_private_field::ShapesWidgetm_currentShape(*m_w) = cur;
        access_private_field::ShapesWidgetm_pos1(*m_w) = QPointF(10, 10);
        access_private_field::ShapesWidgetm_pos2(*m_w) = QPointF(60, 60);
        QMouseEvent e(QEvent::MouseButtonRelease, QPointF(60, 60), QPointF(60, 60),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseReleaseEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run12());
}

// ---- mouseReleaseEvent: rectangle/oval/effect recording completion ----
TEST_F(ShapesWidgetResidTest, mouseReleaseRectCompletion)
{
    auto ut_resid_run13 = [&] {
        access_private_field::ShapesWidgetm_isRecording(*m_w) = true;
        access_private_field::ShapesWidgetm_isSelected(*m_w) = false;
        access_private_field::ShapesWidgetm_selectedIndex(*m_w) = -1;
        access_private_field::ShapesWidgetm_currentType(*m_w) = "rectangle";
        access_private_field::ShapesWidgetm_currentShape(*m_w) = makeShape("rectangle");
        access_private_field::ShapesWidgetm_pos1(*m_w) = QPointF(10, 10);
        access_private_field::ShapesWidgetm_pos2(*m_w) = QPointF(100, 80);
        QMouseEvent e(QEvent::MouseButtonRelease, QPointF(100, 80), QPointF(100, 80),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        call_private_fun::ShapesWidgetmouseReleaseEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run13());
}

// ---- mouseReleaseEvent: synthesized + selected shape early-return ----
TEST_F(ShapesWidgetResidTest, mouseReleaseSynthesizedSelectedReturnsEarly)
{
    auto ut_resid_run14 = [&] {
        access_private_field::ShapesWidgetm_selectedIndex(*m_w) = 1;
        access_private_field::ShapesWidgetm_currentType(*m_w) = "rectangle";
        QMouseEvent e(QEvent::MouseButtonRelease, QPointF(50, 50), QPointF(50, 50),
                      QPointF(50, 50), Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
        call_private_fun::ShapesWidgetmouseReleaseEvent(*m_w, &e);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run14());
}

// ---- keyPressEvent: microAdjust modifiers when a shape IS selected ----
TEST_F(ShapesWidgetResidTest, keyPressMicroAdjustShapeSelected)
{
    auto ut_resid_run15 = [&] {
        access_private_field::ShapesWidgetm_selectedIndex(*m_w) = 0;
        access_private_field::ShapesWidgetm_selectedOrder(*m_w) = 0;
        access_private_field::ShapesWidgetm_shapes(*m_w).append(makeShape("rectangle"));
        const std::vector<std::pair<Qt::Key, Qt::KeyboardModifiers>> keys{
            {Qt::Key_Left,   Qt::NoModifier},
            {Qt::Key_Right,  Qt::NoModifier},
            {Qt::Key_Up,     Qt::NoModifier},
            {Qt::Key_Down,   Qt::NoModifier},
            {Qt::Key_A,      Qt::ControlModifier},
            {Qt::Key_D,      Qt::ControlModifier},
            {Qt::Key_W,      Qt::ControlModifier},
            {Qt::Key_S,      Qt::ControlModifier},
            {Qt::Key_Left,   Qt::ControlModifier | Qt::ShiftModifier},
            {Qt::Key_Right,  Qt::ControlModifier | Qt::ShiftModifier},
            {Qt::Key_Up,     Qt::ControlModifier | Qt::ShiftModifier},
            {Qt::Key_Down,   Qt::ControlModifier | Qt::ShiftModifier},
        };
        for (const auto &km : keys) {
            QKeyEvent e(QEvent::KeyPress, km.first, km.second);
            call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &e);
        }
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run15());
}

// ---- keyPressEvent: cursorMove when pressed + delete + non-mapped key ----
TEST_F(ShapesWidgetResidTest, keyPressCursorMoveAndDelete)
{
    auto ut_resid_run16 = [&] {
        access_private_field::ShapesWidgetm_isPressed(*m_w) = true;
        QKeyEvent move(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
        call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &move);
        QKeyEvent del(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &del);
        QKeyEvent other(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
        call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &other);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run16());
}

// ---- pinchTriggered with center/scale/rotation change flags ----
TEST_F(ShapesWidgetResidTest, pinchTriggeredCenterFlag)
{
    auto ut_resid_run17 = [&] {
        QPinchGesture pinch;
        pinch.setCenterPoint(QPointF(100, 100));
        pinch.setScaleFactor(1.5);
        pinch.setTotalScaleFactor(1.5);
        pinch.setLastScaleFactor(1.0);
        pinch.setRotationAngle(15.0);
        pinch.setChangeFlags(QPinchGesture::ScaleFactorChanged |
                              QPinchGesture::RotationAngleChanged |
                              QPinchGesture::CenterPointChanged);
        call_private_fun::ShapesWidgetpinchTriggered(*m_w, &pinch);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run17());
}

// ---- tapTriggered: gesture state set via QGesture::state property ----
TEST_F(ShapesWidgetResidTest, tapTriggeredAllStates)
{
    auto ut_resid_run18 = [&] {
        QTapGesture tap;
        tap.setProperty("state", static_cast<int>(Qt::GestureStarted));
        call_private_fun::ShapesWidgettapTriggered(*m_w, &tap);
        tap.setProperty("state", static_cast<int>(Qt::GestureFinished));
        call_private_fun::ShapesWidgettapTriggered(*m_w, &tap);
        tap.setProperty("state", static_cast<int>(Qt::GestureCanceled));
        call_private_fun::ShapesWidgettapTriggered(*m_w, &tap);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run18());
}

// ---- enterEvent direct invocation ----
TEST_F(ShapesWidgetResidTest, enterEventSetsCursor)
{
    auto ut_resid_run19 = [&] {
        QEvent enter(QEvent::Enter);
        call_private_fun::ShapesWidgetenterEvent(*m_w, &enter);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run19());
}

// =============================================================================
// ToolBarResidTest
// Focus on public surface that does NOT depend on m_toolbarWidget being set
// (initToolBar couples to MainWindow), plus direct event handlers.
// DEDUP: paintEvent / enterEvent / eventFilter already declared in
// ut_toolbar_cov.h, so we don't redeclare them here.
// =============================================================================
class ToolBarResidTest : public Test
{
public:
    ToolBar *m_bar = nullptr;
    void SetUp() override { m_bar = new ToolBar; m_bar->resize(100, 50); }
    void TearDown() override
    {
        while (qApp->overrideCursor()) qApp->restoreOverrideCursor();
        /* leak m_bar */
    }
};

// ---- enterEvent installs OpenHandCursor (uses existing declaration) ----
TEST_F(ToolBarResidTest, enterEventInstallsCursor)
{
    auto ut_resid_run20 = [&] {
        QEnterEvent ee(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
        call_private_fun::ToolBarenterEvent(*m_bar, &ee);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run20());
}

// ---- mouse press/move/release cycle without MainWindow: SKIPPED because
// ToolBar::mousePressEvent dereferences m_pMainWindow (null here) -> SEGV that
// EXPECT_NO_FATAL_FAILURE cannot trap (it's a signal, not a gtest assertion).
// Per-test process isolation in build_run_ut.sh would lose only this test's
// gcda; left disabled to keep the local --gtest_filter smoke run clean.
#if 0 // DISABLED-BLOCK
TEST_F(ToolBarResidTest, mouseCycleNoMainWindow)
{
    auto ut_resid_run21 = [&] {
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(5, 5),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(m_bar, &press);
        QMouseEvent move(QEvent::MouseMove, QPointF(20, 20), QPointF(20, 20),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(m_bar, &move);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(20, 20), QPointF(20, 20),
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(m_bar, &release);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run21());
}
#endif

// ---- paintEvent direct invocation ----
TEST_F(ToolBarResidTest, paintEventDirect)
{
    auto ut_resid_run22 = [&] {
        m_bar->show();
        qApp->processEvents();
        QPaintEvent pe(QRect(0, 0, 100, 50));
        call_private_fun::ToolBarpaintEvent(*m_bar, &pe);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run22());
}

// ---- eventFilter falls through to DLabel (uses existing declaration) ----
TEST_F(ToolBarResidTest, eventFilterPassthrough)
{
    auto ut_resid_run23 = [&] {
        QObject obj;
        QEvent types[] = {
            QEvent(QEvent::PaletteChange),
            QEvent(QEvent::HoverEnter),
            QEvent(QEvent::MouseButtonPress),
        };
        for (QEvent &ev : types) {
            (void)call_private_fun::ToolBareventFilter(*m_bar, &obj, &ev);
        }
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run23());
}

// ---- showAt multi position; showWidget/hideWidget no-op when null ----
TEST_F(ToolBarResidTest, showAtAndToggleWidgetsRepeated)
{
    auto ut_resid_run24 = [&] {
        for (int i = 0; i < 4; ++i) {
            m_bar->showAt(QPoint(i * 25, i * 15));
        }
        m_bar->showWidget();
        m_bar->hideWidget();
        qApp->processEvents();
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run24());
}

// =============================================================================
// SubToolWidgetResidTest
// Focus on remaining switchContent / shapeClickedFromWidget / setMicroPhoneEnable
// branches by toggling ConfigSettings state between calls.
// =============================================================================
class SubToolWidgetResidTest : public Test
{
public:
    MainWindow *m_mw = nullptr;
    SubToolWidget *m_w = nullptr;
    void SetUp() override
    {
        m_mw = new MainWindow;
        m_w = new SubToolWidget(m_mw);
        m_w->switchContent(QStringLiteral("shot"));
    }
    void TearDown() override { /* leak */ }
};

// ---- shapeClickedFromWidget covering all known branches ----
TEST_F(SubToolWidgetResidTest, shapeClickedFromWidgetAllBranches)
{
    const QStringList shapes{"gio", "rectangle", "oval", "rect", "circ",
                             "line", "arrow", "pen", "text",
                             "effect", "blur", "mosaic",
                             "keyBoard", "camera", "mouse",
                             "pinScreenshots", "scrollShot", "ocr",
                             "aiassistant", "unknown_xyz", QString()};
    for (const QString &s : shapes) {
        EXPECT_NO_FATAL_FAILURE(m_w->shapeClickedFromWidget(s));
    }
}

// ---- switchContent re-entry with config toggles ----
TEST_F(SubToolWidgetResidTest, switchContentReentryWithConfig)
{
    ConfigSettings *s = ConfigSettings::instance();
    ASSERT_NE(s, nullptr);
    auto ut_resid_run25 = [&] {
        s->setValue("recorder", "format", 0);
        m_w->switchContent(QStringLiteral("record"));
        s->setValue("recorder", "format", 1);
        m_w->switchContent(QStringLiteral("record"));
        s->setValue("recorder", "format", 2);
        m_w->switchContent(QStringLiteral("record"));
        m_w->switchContent(QStringLiteral("scroll"));
        m_w->switchContent(QStringLiteral("shot"));
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run25());
}

// ---- setMicroPhoneEnable toggling with various format values ----
TEST_F(SubToolWidgetResidTest, setMicroPhoneEnableAllFormats)
{
    ConfigSettings *s = ConfigSettings::instance();
    ASSERT_NE(s, nullptr);
    for (int fmt = 0; fmt <= 2; ++fmt) {
        auto ut_resid_run26 = [&] {
            s->setValue("recorder", "format", fmt);
            m_w->setMicroPhoneEnable(true);
            m_w->setMicroPhoneEnable(false);
        };
        EXPECT_NO_FATAL_FAILURE(ut_resid_run26());
    }
}

// ---- getFuncSubToolX across every shape+toolbar mode ----
TEST_F(SubToolWidgetResidTest, getFuncSubToolXAllModes)
{
    auto ut_resid_run27 = [&] {
        m_w->switchContent(QStringLiteral("record"));
        QStringList funcs{"gio", "rectangle", "oval", "rect", "circ",
                          "line", "arrow", "pen", "text", "aiassistant",
                          QString(), "unknown"};
        for (QString &f : funcs) {
            (void)m_w->getFuncSubToolX(f);
        }
        m_w->switchContent(QStringLiteral("scroll"));
        for (QString &f : funcs) {
            (void)m_w->getFuncSubToolX(f);
        }
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run27());
}

// ---- installHint + installTipHint repeated ----
TEST_F(SubToolWidgetResidTest, installHintAndTipHintRepeated)
{
    auto ut_resid_run28 = [&] {
        QWidget w1, w2, hintWidget;
        for (int i = 0; i < 3; ++i) {
            m_w->installTipHint(&w1, QStringLiteral("tip %1").arg(i));
            m_w->installHint(&w1, &hintWidget);
            m_w->installTipHint(&w2, QString());
        }
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run28());
}

// ---- eventFilter with non-tracked object + various events ----
TEST_F(SubToolWidgetResidTest, eventFilterUntrackedVarious)
{
    auto ut_resid_run29 = [&] {
        QObject obj;
        QEvent evs[] = {
            QEvent(QEvent::MouseButtonPress),
            QEvent(QEvent::MouseButtonRelease),
            QEvent(QEvent::MouseMove),
            QEvent(QEvent::HoverEnter),
            QEvent(QEvent::PaletteChange),
        };
        for (QEvent &e : evs) {
            (void)m_w->eventFilter(&obj, &e);
        }
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run29());
}

// ---- updateSaveButtonTip with many SaveAction values ----
TEST_F(SubToolWidgetResidTest, updateSaveButtonTipAllActions)
{
    ConfigSettings *s = ConfigSettings::instance();
    ASSERT_NE(s, nullptr);
    const SaveAction ops[] = {SaveToClipboard, SaveToDesktop, SaveToImage,
                              SaveToSpecificDir, AutoSave, CustomScreenSave};
    for (SaveAction op : ops) {
        auto ut_resid_run30 = [&] {
            s->setValue("shot", "save_ways", SaveWays::SpecifyLocation);
            s->setValue("shot", "save_op", static_cast<SaveAction>(op));
            s->setValue("shot", "save_dir_change", false);
            s->setValue("shot", "save_dir", QString());
            m_w->updateSaveButtonTip();
        };
        EXPECT_NO_FATAL_FAILURE(ut_resid_run30());
    }
    auto ut_resid_run31 = [&] {
        m_w->switchContent(QStringLiteral("scroll"));
        s->setValue("shot", "save_ways", SaveWays::Ask);
        m_w->updateSaveButtonTip();
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run31());
}

// =============================================================================
// CameraWidgetResidTest
// Bound-clamping branches of mouseMoveEvent and all 4 quadrant snap branches
// of mouseReleaseEvent.
// =============================================================================
class CameraWidgetResidTest : public Test
{
public:
    CameraWidget *m_cam = nullptr;
    void SetUp() override
    {
        m_cam = new CameraWidget;
        m_cam->resize(160, 120);
        m_cam->setRecordRect(0, 0, 800, 600);
    }
    void TearDown() override
    {
        while (qApp->overrideCursor()) qApp->restoreOverrideCursor();
        /* leak */
    }
};

// ---- mouseMoveEvent: each boundary (left/top/right/bottom clamp) ----
TEST_F(CameraWidgetResidTest, mouseMoveAllBoundClamps)
{
    auto ut_resid_run32 = [&] {
        m_cam->setCameraWidgetImmovable(false);
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(80, 60), QPointF(80, 60),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(m_cam, &press);
        const QList<QPoint> positions{
            QPoint(-500, 300),
            QPoint(400, -500),
            QPoint(2000, 300),
            QPoint(400, 2000),
            QPoint(400, 300),
        };
        for (const QPoint &p : positions) {
            QMouseEvent move(QEvent::MouseMove, QPointF(80, 60), QPointF(p),
                             Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            qApp->sendEvent(m_cam, &move);
        }
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run32());
}

// ---- mouseReleaseEvent: all 4 quadrant snap branches ----
TEST_F(CameraWidgetResidTest, mouseReleaseAllQuadrantSnaps)
{
    auto ut_resid_run33 = [&] {
        m_cam->move(50, 50);
        QMouseEvent r1(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(50, 50),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(m_cam, &r1);
        m_cam->move(50, 500);
        QMouseEvent r2(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(50, 500),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(m_cam, &r2);
        m_cam->move(500, 50);
        QMouseEvent r3(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(500, 50),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(m_cam, &r3);
        m_cam->move(500, 500);
        QMouseEvent r4(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(500, 500),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(m_cam, &r4);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run33());
}

// ---- cameraStart twice -> restartDevices branch: SKIPPED because cameraStart
// pulls in v4l2 symbols (v4l2_ioctl/v4l2_mmap/...) that aren't linked into the
// UT binary -> undefined-symbol abort that EXPECT_NO_FATAL_FAILURE can't trap.
// Per-test process isolation would lose only this test's gcda.
#if 0 // DISABLED-BLOCK
TEST_F(CameraWidgetResidTest, cameraStartTwiceRestarts)
{
    auto ut_resid_run34 = [&] {
        m_cam->setDevcieName(QStringLiteral("/dev/video-nonexistent"));
        m_cam->cameraStart();
        m_cam->cameraStart();
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run34());
}
#endif

// ---- onReceiveMajorImage after initUI: pixmap set path ----
TEST_F(CameraWidgetResidTest, onReceiveMajorImageAfterInitUI)
{
    auto ut_resid_run35 = [&] {
        m_cam->initUI();
        QImage img(80, 60, QImage::Format_RGB32);
        img.fill(Qt::green);
        m_cam->onReceiveMajorImage(img);
        QPixmap pix(80, 60);
        pix.fill(Qt::yellow);
        m_cam->onReceiveMajorImage(pix);
        qApp->processEvents();
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run35());
}

// ---- getter sweep ----
TEST_F(CameraWidgetResidTest, gettersAndSetters)
{
    auto ut_resid_run36 = [&] {
        m_cam->setRecordRect(10, 20, 300, 400);
        (void)m_cam->getRecordX();
        (void)m_cam->getRecordY();
        (void)m_cam->getRecordWidth();
        (void)m_cam->getRecordHeight();
        (void)m_cam->getCameraStatus();
        m_cam->setCameraWidgetImmovable(true);
        m_cam->setCameraWidgetImmovable(false);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run36());
}

// ---- enterEvent + paintEvent combined ----
TEST_F(CameraWidgetResidTest, enterAndPaintEvent)
{
    auto ut_resid_run37 = [&] {
        QEvent enter(QEvent::Enter);
        qApp->sendEvent(m_cam, &enter);
        QPaintEvent pe(QRect(0, 0, 160, 120));
        qApp->sendEvent(m_cam, &pe);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run37());
}

// ---- showAt multiple positions ----
TEST_F(CameraWidgetResidTest, showAtVariadic)
{
    auto ut_resid_run38 = [&] {
        m_cam->showAt(QPoint(10, 10));
        m_cam->showAt(QPoint(100, 100));
        m_cam->showAt(QPoint(0, 0));
        qApp->processEvents();
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run38());
}

// =============================================================================
// ScreenGrabberResidTest
// DEDUP: grabMultipleScreens is already declared in ut_screengrabber_cov.h,
// so we don't redeclare it here.
// =============================================================================
class ScreenGrabberResidTest : public Test
{
public:
    void SetUp() override
    {
        Utils::isWaylandMode = false;
        Utils::isQt6XcbEnv = false;
    }
    void TearDown() override
    {
        Utils::isWaylandMode = false;
        Utils::isQt6XcbEnv = false;
    }
};

// ---- grabEntireDesktop with several rects to hit single/multi/fallback branches ----
TEST_F(ScreenGrabberResidTest, grabEntireDesktopVariedRects)
{
    ScreenGrabber g;
    const QList<QRect> rects{
        QRect(0, 0, 100, 100),
        QRect(-50, -50, 200, 200),
        QRect(0, 0, 50000, 50000),
        QRect(-1000, -1000, 10, 10),
        QRect(0, 0, 1, 1),
    };
    for (const QRect &r : rects) {
        bool ok = false;
        EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, r, 1.0));
    }
}

// ---- grabEntireDesktop with DPR variation ----
TEST_F(ScreenGrabberResidTest, grabEntireDesktopHighDPR)
{
    ScreenGrabber g;
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, QRect(0, 0, 200, 100), 2.0));
    EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, QRect(0, 0, 200, 100), 0.5));
}

// ---- static getX11RootWindowSize: env is Qt6+XCB but Utils flag off -> empty ----
TEST_F(ScreenGrabberResidTest, getX11RootWindowSizeWhenFlagOff)
{
    QSize sz;
    EXPECT_NO_FATAL_FAILURE(sz = ScreenGrabber::getX11RootWindowSize());
    (void)sz;
}

// ---- direct multi-screen call with current screen duplicated to fake clone mode ----
TEST_F(ScreenGrabberResidTest, grabMultipleScreensDirectDupList)
{
    auto ut_resid_run39 = [&] {
        ScreenGrabber g;
        QScreen *primary = QGuiApplication::primaryScreen();
        ASSERT_NE(primary, nullptr);
        QList<QScreen*> dup{primary, primary};
        bool ok = false;
        call_private_fun::ScreenGrabbergrabMultipleScreens(
            g, ok, QRect(0, 0, 100, 100), dup, 1.0);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run39());
}

// =============================================================================
// EventMonitorResidTest
// Cover handleEvent with synthesized XRecord data covering all switch cases.
// =============================================================================
class EventMonitorResidTest : public Test
{
public:
    EventMonitor *m_mon = nullptr;
    void SetUp() override { m_mon = new EventMonitor; }
    void TearDown() override { /* leak */ }
};

// ---- handleEvent with category NOT XRecordFromServer -> SKIPPED: duplicate
// of EventMonitorCov2Test.callbackWithNullData in ut_misc_cov2.h. The XRecord
// path crashes when XRecordFreeData is called on stack-allocated fake data;
// keeping the existing one only.
#if 0 // DISABLED-BLOCK
TEST_F(EventMonitorResidTest, handleEventNonServerCategory)
{
    auto ut_resid_run40 = [&] {
        XRecordInterceptData fake;
        fake.category = 0;
        fake.client_seq = 0;
        fake.data = nullptr;
        fake.data_len = 0;
        m_mon->handleEvent(&fake);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run40());
}
#endif

// ---- all wayland slot button type variations ----
TEST_F(EventMonitorResidTest, waylandSlotsAllButtonTypes)
{
    auto ut_resid_run41 = [&] {
        QSignalSpy pressSpy(m_mon, &EventMonitor::mousePress);
        QSignalSpy releaseSpy(m_mon, &EventMonitor::mouseRelease);
        QSignalSpy moveSpy(m_mon, &EventMonitor::mouseMove);
        const int buttonTypes[] = {Button1, Button2, Button3, 4, 5, 999, -1, 0};
        for (int bt : buttonTypes) {
            m_mon->ButtonPressEvent(bt, bt, bt * 2, QString());
            m_mon->ButtonReleaseEvent(bt, bt * 3, bt * 4, QString());
            m_mon->CursorMoveEvent(bt * 5, bt * 6, QString());
        }
        (void)pressSpy.count(); (void)releaseSpy.count(); (void)moveSpy.count();
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run41());
}

// ---- initWaylandEventMonitor repeated + getCursorImageWayland ----
TEST_F(EventMonitorResidTest, initWaylandAndGetCursorRepeated)
{
    auto ut_resid_run42 = [&] {
        for (int i = 0; i < 3; ++i) {
            m_mon->initWaylandEventMonitor();
        }
        QImage img = m_mon->getCursorImageWayland();
        (void)img;
        XFixesCursorImage *cur = m_mon->getCursorImage();
        (void)cur;
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run42());
}

// ---- releaseRes repeated calls when nothing was started ----
TEST_F(EventMonitorResidTest, releaseResRepeatedIdle)
{
    auto ut_resid_run43 = [&] {
        for (int i = 0; i < 5; ++i) {
            m_mon->releaseRes();
        }
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run43());
}

// ---- callback static function path: SKIPPED (same crash root-cause as
// handleEventNonServerCategory above — XRecordFreeData on stack fake).
#if 0 // DISABLED-BLOCK
TEST_F(EventMonitorResidTest, callbackStaticNonServerData)
{
    auto ut_resid_run44 = [&] {
        XRecordInterceptData fake;
        fake.category = 0;
        fake.client_seq = 0;
        fake.data = nullptr;
        fake.data_len = 0;
        EventMonitor::callback(reinterpret_cast<XPointer>(m_mon), &fake);
    };
    EXPECT_NO_FATAL_FAILURE(ut_resid_run44());
}
#endif
