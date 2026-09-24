// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QPixmap>
#include <QString>
#include <QtDBus/QDBusMessage>
#include <QVariantMap>
#include <QStringList>
#include <QByteArray>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/main_window.h"

using namespace testing;

// MainWindow eventFilter helper methods (mousePressEF/mouseMoveEF/etc) — the
// largest uncovered block in main_window.cpp (~960 lines). These are protected;
// we access via ACCESS_PRIVATE_FUN. They mutate member state but have null-checks
// on m_toolBar/m_sideBar/m_shapesWidget, so with an un-initialized MainWindow they
// complete without SEGV in most branches.

ACCESS_PRIVATE_FUN(MainWindow, bool(QObject *, QEvent *), eventFilter);
ACCESS_PRIVATE_FUN(MainWindow, int(QMouseEvent *, bool &), mouseDblClickEF);
ACCESS_PRIVATE_FUN(MainWindow, int(QMouseEvent *, bool &), mousePressEF);
ACCESS_PRIVATE_FUN(MainWindow, int(QMouseEvent *, bool &), mouseReleaseEF);
ACCESS_PRIVATE_FUN(MainWindow, int(QMouseEvent *, bool &), mouseMoveEF);
ACCESS_PRIVATE_FUN(MainWindow, int(QKeyEvent *, bool &), keyPressEF);
ACCESS_PRIVATE_FUN(MainWindow, int(QKeyEvent *, bool &), keyReleaseEF);
ACCESS_PRIVATE_FUN(MainWindow, int(QWheelEvent *, bool &), wheelEF);
ACCESS_PRIVATE_FUN(MainWindow, bool(), expandSelectionToContents);
ACCESS_PRIVATE_FIELD(MainWindow, QRect, m_backgroundRect);

class MainWindowEFTest : public Test
{
public:
    Stub stub;
    MainWindow *m_w = nullptr;
    static qreal devicePixelRatio_stub() { return 1; }
    static int width_stub() { return 1920; }
    static int height_stub() { return 1080; }

    void SetUp() override
    {
        stub.set(ADDR(QScreen, geometry), geometry_stub);
        stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
        stub.set(ADDR(QScreen, devicePixelRatio), devicePixelRatio_stub);
        stub.set(ADDR(QWidget, width), width_stub);
        stub.set(ADDR(QWidget, height), height_stub);
        m_w = new MainWindow;
        m_w->initAttributes();
        m_w->initResource();
    }
    void TearDown() override { /* 故意泄漏 m_w，避免析构崩溃 */ }
};

TEST_F(MainWindowEFTest, eventFilterNullEventSafe)
{
    QEvent dummy(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindoweventFilter(*m_w, m_w, &dummy));
}

TEST_F(MainWindowEFTest, eventFilterKeyPressEsc)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowkeyPressEF(*m_w, &keyEvent, needRepaint));
}

TEST_F(MainWindowEFTest, eventFilterKeyReleaseEsc)
{
    QKeyEvent keyEvent(QEvent::KeyRelease, Qt::Key_Escape, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowkeyReleaseEF(*m_w, &keyEvent, needRepaint));
}

TEST_F(MainWindowEFTest, eventFilterKeyPressEnter)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowkeyPressEF(*m_w, &keyEvent, needRepaint));
}

TEST_F(MainWindowEFTest, eventFilterKeyPressSpace)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowkeyPressEF(*m_w, &keyEvent, needRepaint));
}

TEST_F(MainWindowEFTest, eventFilterKeyPressCtrl)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Control, Qt::ControlModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowkeyPressEF(*m_w, &keyEvent, needRepaint));
}

TEST_F(MainWindowEFTest, mouseDblClickEFLeftButton)
{
    QMouseEvent mouseEvent(QEvent::MouseButtonDblClick, QPointF(50, 50),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmouseDblClickEF(*m_w, &mouseEvent, needRepaint));
}

TEST_F(MainWindowEFTest, mouseDblClickEFRightButton)
{
    QMouseEvent mouseEvent(QEvent::MouseButtonDblClick, QPointF(50, 50),
        Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmouseDblClickEF(*m_w, &mouseEvent, needRepaint));
}

TEST_F(MainWindowEFTest, mousePressEFLeftButton)
{
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPointF(50, 50),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmousePressEF(*m_w, &mouseEvent, needRepaint));
}

TEST_F(MainWindowEFTest, mousePressEFRightButton)
{
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPointF(50, 50),
        Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmousePressEF(*m_w, &mouseEvent, needRepaint));
}

TEST_F(MainWindowEFTest, mouseReleaseEFLeftButton)
{
    QMouseEvent mouseEvent(QEvent::MouseButtonRelease, QPointF(50, 50),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmouseReleaseEF(*m_w, &mouseEvent, needRepaint));
}

TEST_F(MainWindowEFTest, mouseMoveEFNoButton)
{
    QMouseEvent mouseEvent(QEvent::MouseMove, QPointF(50, 50),
        Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmouseMoveEF(*m_w, &mouseEvent, needRepaint));
}

TEST_F(MainWindowEFTest, wheelEFVertical)
{
    QWheelEvent wheelEvent(QPointF(50, 50), QPointF(50, 50), QPoint(0, 120), QPoint(0, 120),
        Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowwheelEF(*m_w, &wheelEvent, needRepaint));
}

// expandSelectionToContents：图形编辑越界时，选区实时扩张到覆盖图形外接矩形。
// 这里是纯状态计算，不依赖真实鼠标事件，可稳定回归"扩张方向/边界/幂等"。
TEST_F(MainWindowEFTest, expandSelectionGrowsToCoverAnnotationOnRight)
{
    access_private_field::MainWindowrecordX(*m_w) = 400;
    access_private_field::MainWindowrecordY(*m_w) = 300;
    access_private_field::MainWindowrecordWidth(*m_w) = 500;
    access_private_field::MainWindowrecordHeight(*m_w) = 400;
    access_private_field::MainWindowm_backgroundRect(*m_w) = QRect(0, 0, 1920, 1080);

    ShapesWidget *sw = new ShapesWidget(m_w);
    sw->setFixedSize(496, 396);      // 选区四周内缩 2px
    sw->move(402, 302);
    access_private_field::MainWindowm_shapesWidget(*m_w) = sw;
    access_private_field::MainWindowm_isShapesWidgetExist(*m_w) = true;

    // 局部坐标 x:300..700 => 屏幕坐标 x:702..1102，超出选区右边界 900
    Toolshape shape;
    shape.type = "rectangle";
    shape.lineWidth = 3;
    // 注意：Toolshape 构造函数默认已有 4 个 (0,0) 点，这里必须整体赋值
    shape.mainPoints = {QPointF(300, 50), QPointF(300, 200),
                        QPointF(700, 50), QPointF(700, 200)};
    access_private_field::ShapesWidgetm_shapes(*sw).append(shape);

    EXPECT_TRUE(call_private_fun::MainWindowexpandSelectionToContents(*m_w));

    // 只往右长：左/上边界不动
    EXPECT_EQ(400, access_private_field::MainWindowrecordX(*m_w));
    EXPECT_EQ(300, access_private_field::MainWindowrecordY(*m_w));
    // 右边界必须覆盖图形屏幕外接矩形（702+700=1102）
    EXPECT_GE(access_private_field::MainWindowrecordX(*m_w)
              + access_private_field::MainWindowrecordWidth(*m_w), 1102);

    // 幂等：几何没再变化时不应继续增长
    EXPECT_FALSE(call_private_fun::MainWindowexpandSelectionToContents(*m_w));
}

// 左/上扩张会移动 ShapesWidget 原点，图形局部坐标必须同步平移，
// 保证图形在屏幕上的位置不变（否则会跳变）。
TEST_F(MainWindowEFTest, expandSelectionGrowsLeftWithoutMovingShapeOnScreen)
{
    access_private_field::MainWindowrecordX(*m_w) = 400;
    access_private_field::MainWindowrecordY(*m_w) = 300;
    access_private_field::MainWindowrecordWidth(*m_w) = 500;
    access_private_field::MainWindowrecordHeight(*m_w) = 400;
    access_private_field::MainWindowm_backgroundRect(*m_w) = QRect(0, 0, 1920, 1080);

    ShapesWidget *sw = new ShapesWidget(m_w);
    sw->setFixedSize(496, 396);
    sw->move(402, 302);
    access_private_field::MainWindowm_shapesWidget(*m_w) = sw;
    access_private_field::MainWindowm_isShapesWidgetExist(*m_w) = true;

    // 局部 x:-50..100 => 屏幕 x:352..502，左侧超出选区左边界 400
    Toolshape shape;
    shape.type = "rectangle";
    shape.lineWidth = 3;
    shape.mainPoints = {QPointF(-50, 50), QPointF(-50, 200),
                        QPointF(100, 50), QPointF(100, 200)};
    access_private_field::ShapesWidgetm_shapes(*sw).append(shape);

    const qreal screenLeftBefore = shape.mainPoints[0].x() + sw->x();

    EXPECT_TRUE(call_private_fun::MainWindowexpandSelectionToContents(*m_w));

    // 左边界收缩（选区左边界变小），右/上/下不变
    EXPECT_LT(access_private_field::MainWindowrecordX(*m_w), 400);
    EXPECT_EQ(300, access_private_field::MainWindowrecordY(*m_w));
    EXPECT_EQ(900, access_private_field::MainWindowrecordX(*m_w)
              + access_private_field::MainWindowrecordWidth(*m_w));

    // widget 已移动到新的选区原点
    EXPECT_EQ(access_private_field::MainWindowrecordX(*m_w) + 2, sw->x());

    // 图形屏幕位置保持不变（局部坐标已反向平移）
    const Toolshape &moved = access_private_field::ShapesWidgetm_shapes(*sw).at(0);
    EXPECT_EQ(screenLeftBefore, moved.mainPoints[0].x() + sw->x());
}
