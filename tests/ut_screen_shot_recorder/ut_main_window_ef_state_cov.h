// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/main_window.h"

using namespace testing;

// 现有 MainWindowEFTest 以「空状态」调用 EF 辅助函数，多数命中早返回。
// 本文件通过设置 m_functionType(shot/record)、recordButtonStatus 等状态字段，
// 驱动 mousePressEF/mouseMoveEF/mouseReleaseEF 的更深分支（无需改源码）。
// EF 访问器已在 ut_main_window_ef.h 声明；状态字段访问器已在 ut_main_window*.h 声明。

class MainWindowEFStateCovTest : public Test
{
public:
    Stub stub;
    MainWindow *m_w = nullptr;
    static QRect myGeometry_stub() { return QRect(0, 0, 1920, 1080); }
    static qreal myDpr_stub() { return 1.0; }
    static int myWidth_stub() { return 1920; }
    static int myHeight_stub() { return 1080; }
    static void myPassInput_stub(int) {}

    void SetUp() override
    {
        stub.set(ADDR(QScreen, geometry), myGeometry_stub);
        stub.set(ADDR(QScreen, devicePixelRatio), myDpr_stub);
        stub.set(ADDR(QWidget, width), myWidth_stub);
        stub.set(ADDR(QWidget, height), myHeight_stub);
        stub.set(ADDR(Utils, passInputEvent), myPassInput_stub);
        m_w = new MainWindow;
        m_w->initAttributes();
        m_w->initResource();
    }
    void TearDown() override { /* 故意泄漏，避免析构崩溃 */ }

    void setFunc(unsigned int t) { access_private_field::MainWindowm_functionType(*m_w) = t; }
    void setRecordStatus(int s) { access_private_field::MainWindowrecordButtonStatus(*m_w) = s; }
};

// shot 模式下按下/移动/释放左键 -> 走 shot 分支
TEST_F(MainWindowEFStateCovTest, shotModeMouseFlow)
{
    setFunc(MainWindow::status::shot);
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent move(QEvent::MouseMove, QPointF(80, 80), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(90, 90), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmousePressEF(*m_w, &press, needRepaint));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmouseMoveEF(*m_w, &move, needRepaint));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmouseReleaseEF(*m_w, &release, needRepaint));
}

// shot 模式右键 -> 菜单分支（m_menuController 可能为 null -> return 0）
TEST_F(MainWindowEFStateCovTest, shotModeRightClick)
{
    setFunc(MainWindow::status::shot);
    QMouseEvent rpress(QEvent::MouseButtonPress, QPointF(50, 50), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmousePressEF(*m_w, &rpress, needRepaint));
}

// record 模式 + NORMAL 按钮 -> 录屏按下分支
TEST_F(MainWindowEFStateCovTest, recordModeNormalButton)
{
    setFunc(MainWindow::status::record);
    setRecordStatus(0); // RECORD_BUTTON_NORMAL
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmousePressEF(*m_w, &press, needRepaint));
}

// record 模式释放 -> record 分支
TEST_F(MainWindowEFStateCovTest, recordModeRelease)
{
    setFunc(MainWindow::status::record);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(90, 90), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmouseReleaseEF(*m_w, &release, needRepaint));
}

// shot 模式双击
TEST_F(MainWindowEFStateCovTest, shotModeDblClick)
{
    setFunc(MainWindow::status::shot);
    QMouseEvent dbl(QEvent::MouseButtonDblClick, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmouseDblClickEF(*m_w, &dbl, needRepaint));
}

// shot 模式滚轮
TEST_F(MainWindowEFStateCovTest, shotModeWheel)
{
    setFunc(MainWindow::status::shot);
    QWheelEvent wheel(QPointF(50, 50), QPointF(50, 50), QPoint(0, 120), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    bool needRepaint = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowwheelEF(*m_w, &wheel, needRepaint));
}
