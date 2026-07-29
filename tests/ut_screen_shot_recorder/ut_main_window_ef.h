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
ACCESS_PRIVATE_FUN(MainWindow, int(const QPoint &) const, getActionAt);
ACCESS_PRIVATE_FUN(MainWindow, bool(QKeyEvent *, bool &), adjustSelectionByKey);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_cursorBound);
ACCESS_PRIVATE_FIELD(MainWindow, QRect, m_backgroundRect);
ACCESS_PRIVATE_FIELD(MainWindow, QRect, rootWindowRect);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_needSaveScreenshot);

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

// ===================== 框选区域调整（V-628）单测 =====================

// getActionAt：八方向边缘命中与内部判定
TEST_F(MainWindowEFTest, getActionAtEdgeAndInterior)
{
    access_private_field::MainWindowrecordX(*m_w) = 100;
    access_private_field::MainWindowrecordY(*m_w) = 100;
    access_private_field::MainWindowrecordWidth(*m_w) = 200;
    access_private_field::MainWindowrecordHeight(*m_w) = 150;
    access_private_field::MainWindowm_cursorBound(*m_w) = 20;
    // ACTION_RESIZE_TOP_LEFT=1, ACTION_RESIZE_LEFT=7, ACTION_MOVE=0, ACTION_RESIZE_BOTTOM_RIGHT=4
    EXPECT_EQ(call_private_fun::MainWindowgetActionAt(*m_w, QPoint(100, 100)), 1);
    EXPECT_EQ(call_private_fun::MainWindowgetActionAt(*m_w, QPoint(95, 175)), 7);
    EXPECT_EQ(call_private_fun::MainWindowgetActionAt(*m_w, QPoint(200, 175)), 0);
    EXPECT_EQ(call_private_fun::MainWindowgetActionAt(*m_w, QPoint(295, 245)), 4);
}

// adjustSelectionByKey：无修饰方向键平移框选区域
TEST_F(MainWindowEFTest, adjustSelectionByKeyArrowMovesSelection)
{
    access_private_field::MainWindowrecordX(*m_w) = 100;
    access_private_field::MainWindowrecordY(*m_w) = 100;
    access_private_field::MainWindowrecordWidth(*m_w) = 200;
    access_private_field::MainWindowrecordHeight(*m_w) = 150;
    access_private_field::MainWindowm_backgroundRect(*m_w) = QRect(0, 0, 1920, 1080);
    access_private_field::MainWindowrootWindowRect(*m_w) = QRect(0, 0, 1920, 1080);
    access_private_field::MainWindowm_needSaveScreenshot(*m_w) = false;

    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowadjustSelectionByKey(*m_w, &keyEvent, needRepaint));
    EXPECT_EQ(access_private_field::MainWindowrecordX(*m_w), 101);
}

// keyPressEF：标注态无选中图形时方向键调框选（D3①）
TEST_F(MainWindowEFTest, keyPressEFAnnotNoSelectionAdjustsSelection)
{
    access_private_field::MainWindowrecordX(*m_w) = 100;
    access_private_field::MainWindowrecordY(*m_w) = 100;
    access_private_field::MainWindowrecordWidth(*m_w) = 200;
    access_private_field::MainWindowrecordHeight(*m_w) = 150;
    access_private_field::MainWindowm_backgroundRect(*m_w) = QRect(0, 0, 1920, 1080);
    access_private_field::MainWindowrootWindowRect(*m_w) = QRect(0, 0, 1920, 1080);
    access_private_field::MainWindowm_needSaveScreenshot(*m_w) = false;
    access_private_field::MainWindowm_isShapesWidgetExist(*m_w) = true;

    // 无选中图形（selectedIndex() == -1）
    ShapesWidget *sw = new ShapesWidget(m_w);
    access_private_field::MainWindowm_shapesWidget(*m_w) = sw;

    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowkeyPressEF(*m_w, &keyEvent, needRepaint));
    EXPECT_EQ(access_private_field::MainWindowrecordX(*m_w), 101);
}
