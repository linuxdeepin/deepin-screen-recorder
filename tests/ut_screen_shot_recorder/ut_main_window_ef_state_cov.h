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

// 回归（bug 349319）：录屏跨合成器切换不再停止，改为迁移摄像头/创建录制框
ACCESS_PRIVATE_FUN(MainWindow, void(), ensureRecorderRegionAndMigrateCameraFor2D);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_hasComposite);

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
    static bool mw_hasNoBlurWindow_stub() { return false; }
    static bool mw_hasBlurWindow_stub() { return true; }

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


// 回归（bug 349319）：录屏中 3D→2D 切换不再停止录屏，改为创建录制框并迁移摄像头
TEST_F(MainWindowEFStateCovTest, recordModeCompositeSwitchTo2DCreatesRegion)
{
    setFunc(MainWindow::status::record);
    access_private_field::MainWindowm_hasComposite(*m_w) = true;
    access_private_field::MainWindowm_screenCount(*m_w) = 1;
    access_private_field::MainWindowm_pixelRatio(*m_w) = 1.0;
    stub.set(ADDR(DWindowManagerHelper, hasBlurWindow), mw_hasNoBlurWindow_stub);

    EXPECT_EQ(access_private_field::MainWindowm_pRecorderRegion(*m_w), nullptr);
    EXPECT_NO_FATAL_FAILURE(m_w->compositeChanged());
    // 切换后未停止录屏，且录制框已创建（旧逻辑会 stopRecord/exitApp，不会创建录制框）
    EXPECT_NE(access_private_field::MainWindowm_pRecorderRegion(*m_w), nullptr);
}

// 回归（bug 349319）：ensureRecorderRegionAndMigrateCameraFor2D 空时创建录制框
TEST_F(MainWindowEFStateCovTest, ensureRecorderRegionAndMigrateCameraFor2DCreatesRegion)
{
    setFunc(MainWindow::status::record);
    access_private_field::MainWindowm_screenCount(*m_w) = 1;
    access_private_field::MainWindowm_pixelRatio(*m_w) = 1.0;
    EXPECT_EQ(access_private_field::MainWindowm_pRecorderRegion(*m_w), nullptr);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowensureRecorderRegionAndMigrateCameraFor2D(*m_w));
    EXPECT_NE(access_private_field::MainWindowm_pRecorderRegion(*m_w), nullptr);
}


// 回归（bug 349319）：录屏中 2D→3D 切换恢复主窗口、隐藏录制框，且不重新创建录制框
TEST_F(MainWindowEFStateCovTest, recordModeCompositeSwitchTo3DHidesRegion)
{
    setFunc(MainWindow::status::record);
    access_private_field::MainWindowm_hasComposite(*m_w) = false; // 当前 2D
    access_private_field::MainWindowm_screenCount(*m_w) = 1;
    access_private_field::MainWindowm_pixelRatio(*m_w) = 1.0;
    // 预置已存在的录制框并显示，验证 2D→3D 分支会隐藏它且不重新 new
    auto *region = new RecorderRegionShow();
    region->resize(800, 600);
    region->move(50, 50);
    region->show();
    access_private_field::MainWindowm_pRecorderRegion(*m_w) = region;
    stub.set(ADDR(DWindowManagerHelper, hasBlurWindow), mw_hasBlurWindow_stub); // -> 3D

    EXPECT_NO_FATAL_FAILURE(m_w->compositeChanged());
    // 录制框指针不变（未被重新 new）
    EXPECT_EQ(access_private_field::MainWindowm_pRecorderRegion(*m_w), region);
    // 录制框被隐藏
    EXPECT_FALSE(region->isVisible());
}

// 回归（bug 349319）：ensureRecorderRegionAndMigrateCameraFor2D 复用已存在录制框，不新建
TEST_F(MainWindowEFStateCovTest, ensureRecorderRegionAndMigrateCameraFor2DReusesExisting)
{
    setFunc(MainWindow::status::record);
    access_private_field::MainWindowm_screenCount(*m_w) = 1;
    access_private_field::MainWindowm_pixelRatio(*m_w) = 1.0;
    // 预置已存在的录制框（无摄像头 -> 不触发迁移，仅验证复用/不新建路径）
    auto *region = new RecorderRegionShow();
    region->resize(800, 600);
    region->move(50, 50);
    access_private_field::MainWindowm_pRecorderRegion(*m_w) = region;

    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowensureRecorderRegionAndMigrateCameraFor2D(*m_w));
    // 指针不变、未新建
    EXPECT_EQ(access_private_field::MainWindowm_pRecorderRegion(*m_w), region);
}
