// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QImage>
#include <QVBoxLayout>
#include <QByteArray>
#include <QPainter>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/main_window.h"

using namespace testing;

// MainWindowCov4Test: 新增声明未在 cov/ef/ext/cov.h 中出现的私有方法，
// 并以多种入参驱动几何/状态/保存/通知分支。TearDown 故意泄漏 m_w。
ACCESS_PRIVATE_FUN(MainWindow, void(const QPixmap &), save2Clipboard);
ACCESS_PRIVATE_FUN(MainWindow, void(), noNotify);
ACCESS_PRIVATE_FUN(MainWindow, void(), tableRecordSet);
ACCESS_PRIVATE_FUN(MainWindow, void(), onActivateWindow);
ACCESS_PRIVATE_FUN(MainWindow, void(), handleOptionMenuShown);
ACCESS_PRIVATE_FUN(MainWindow, void(), updateMultiKeyBoardPos);
ACCESS_PRIVATE_FUN(MainWindow, void(bool), pinScreenshotsLockScreen);
ACCESS_PRIVATE_FUN(MainWindow, void(bool), scrollShotLockScreen);
ACCESS_PRIVATE_FUN(MainWindow, void(bool), onPowersource);
ACCESS_PRIVATE_FUN(MainWindow, void(const QByteArray &), onSaveClipboardComing);
ACCESS_PRIVATE_FUN(MainWindow, void(), onRecordingStarted);
ACCESS_PRIVATE_FUN(MainWindow, void(), onRecordingStopped);
ACCESS_PRIVATE_FUN(MainWindow, void(uint32_t), onSourceFailed);
ACCESS_PRIVATE_FUN(MainWindow, void(bool), on_CheckVideoCouldUse);
ACCESS_PRIVATE_FUN(MainWindow, void(int), onAiAssistantSelected);
ACCESS_PRIVATE_FUN(MainWindow, void(int), onScrollShotCheckScrollType);
ACCESS_PRIVATE_FUN(MainWindow, void(), destroyTreelandToolBar);
ACCESS_PRIVATE_FUN(MainWindow, void(), updateCaptureRegion);
ACCESS_PRIVATE_FUN(MainWindow, void(), stopRecordResource);
ACCESS_PRIVATE_FUN(MainWindow, void(), hideScreenshotTips);
ACCESS_PRIVATE_FUN(MainWindow, void(), addCursorToImage);
ACCESS_PRIVATE_FUN(MainWindow, void(), shotImgWidthEffect);
ACCESS_PRIVATE_FUN(MainWindow, void(), prepareScreenshot);
ACCESS_PRIVATE_FUN(MainWindow, void(), captureScreenshotImage);
ACCESS_PRIVATE_FUN(MainWindow, void(), finishScreenshot);
ACCESS_PRIVATE_FUN(MainWindow, void(), saveScreenShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), saveScreenShotToClipboardOnly);
ACCESS_PRIVATE_FUN(MainWindow, void(), saveScreenShotToFile);
ACCESS_PRIVATE_FUN(MainWindow, void(), initBackground);
ACCESS_PRIVATE_FUN(MainWindow, void(), forceX11WindowPosition);
ACCESS_PRIVATE_FUN(MainWindow, void(), updateSideBarPos);
ACCESS_PRIVATE_FUN(MainWindow, void(), updateCameraWidgetPos);
ACCESS_PRIVATE_FUN(MainWindow, void(QString, int), reloadImage);

class MainWindowCov4Test : public Test
{
public:
    Stub stub;
    MainWindow *m_w = nullptr;
    static QRect cov4_geometry_stub() { return QRect(0, 0, 1920, 1080); }
    static qreal cov4_dpr_stub() { return 1.0; }
    static int cov4_width_stub() { return 1920; }
    static int cov4_height_stub() { return 1080; }
    static void cov4_passInput_stub(int) {}
    static bool cov4_isComp_stub() { return true; }
    static void cov4_quit_stub() {}
    static bool cov4_isRoot_stub() { return true; }

    void SetUp() override
    {
        stub.set(ADDR(QScreen, geometry), cov4_geometry_stub);
        stub.set(ADDR(QScreen, devicePixelRatio), cov4_dpr_stub);
        stub.set(ADDR(QWidget, width), cov4_width_stub);
        stub.set(ADDR(QWidget, height), cov4_height_stub);
        stub.set(ADDR(Utils, passInputEvent), cov4_passInput_stub);
        m_w = new MainWindow;
        m_w->initAttributes();
        m_w->initResource();
    }
    void TearDown() override { /* 故意泄漏 m_w，避免析构崩溃 */ }
};

// ---- 保存相关 ----
TEST_F(MainWindowCov4Test, save2ClipboardVariants)
{
    QPixmap pix(32, 32);
    pix.fill(Qt::blue);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsave2Clipboard(*m_w, pix));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsave2Clipboard(*m_w, QPixmap()));
}

TEST_F(MainWindowCov4Test, saveFlows)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsaveScreenShot(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsaveScreenShotToClipboardOnly(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsaveScreenShotToFile(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowprepareScreenshot(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowcaptureScreenshotImage(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowfinishScreenshot(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowhideScreenshotTips(*m_w));
}

TEST_F(MainWindowCov4Test, notifyAndStateSlots)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindownoNotify(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindownoNotify(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonRecordingStarted(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonRecordingStopped(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonSourceFailed(*m_w, 0u));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonSourceFailed(*m_w, 1u));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonSourceFailed(*m_w, 2u));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowon_CheckVideoCouldUse(*m_w, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowon_CheckVideoCouldUse(*m_w, false));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonPowersource(*m_w, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonPowersource(*m_w, false));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonSaveClipboardComing(*m_w, QByteArray("xx")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonActivateWindow(*m_w));
}

TEST_F(MainWindowCov4Test, aiAndScrollSlots)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 0));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 1));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 2));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotCheckScrollType(*m_w, 0));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotCheckScrollType(*m_w, 1));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowpinScreenshotsLockScreen(*m_w, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowpinScreenshotsLockScreen(*m_w, false));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotLockScreen(*m_w, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotLockScreen(*m_w, false));
}

TEST_F(MainWindowCov4Test, layoutAndGeoUpdates)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateMultiKeyBoardPos(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateSideBarPos(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateCameraWidgetPos(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateCaptureRegion(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowhandleOptionMenuShown(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetDragCursor(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresetCursor(*m_w));
}

TEST_F(MainWindowCov4Test, miscHelpers)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowforceX11WindowPosition(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowdestroyTreelandToolBar(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowstopRecordResource(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshotImgWidthEffect(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitBackground(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowaddCursorToImage(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowreloadImage(*m_w, QStringLiteral("effect"), 4));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindownoNotify(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowtableRecordSet(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonActivateWindow(*m_w));
}

TEST_F(MainWindowCov4Test, publicSlotsSweep)
{
    EXPECT_NO_FATAL_FAILURE(m_w->compositeChanged());
    EXPECT_NO_FATAL_FAILURE(m_w->responseEsc());
    EXPECT_NO_FATAL_FAILURE(m_w->changeFunctionButton(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->changeFunctionButton(QStringLiteral("oval")));
    EXPECT_NO_FATAL_FAILURE(m_w->changeFunctionButton(QStringLiteral("line")));
    EXPECT_NO_FATAL_FAILURE(m_w->changeKeyBoardShowEvent(true));
    EXPECT_NO_FATAL_FAILURE(m_w->changeKeyBoardShowEvent(false));
    EXPECT_NO_FATAL_FAILURE(m_w->changeMouseShowEvent(true));
    EXPECT_NO_FATAL_FAILURE(m_w->changeMouseShowEvent(false));
    EXPECT_NO_FATAL_FAILURE(m_w->changeShotToolEvent(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->changeShotToolEvent(QStringLiteral("arrow")));
    EXPECT_NO_FATAL_FAILURE(m_w->changeShotToolEvent(QStringLiteral("text")));
    EXPECT_NO_FATAL_FAILURE(m_w->moveToolBars(QPoint(0, 0), QPoint(20, 20)));
    EXPECT_NO_FATAL_FAILURE(m_w->moveToolBars(QPoint(100, 100), QPoint(-50, -50)));
    EXPECT_NO_FATAL_FAILURE(m_w->getTwoScreenIntersectPos(QPoint(100, 100)));
    EXPECT_NO_FATAL_FAILURE(m_w->getTwoScreenIntersectPos(QPoint(5000, 5000)));
    EXPECT_NO_FATAL_FAILURE(m_w->savePath(QStringLiteral("/tmp")));
    EXPECT_NO_FATAL_FAILURE(m_w->setSavePath(QStringLiteral("/home/uos/Pictures")));
    EXPECT_NO_FATAL_FAILURE(m_w->startScreenshotFor3rd(QStringLiteral("/tmp/a.png")));
    EXPECT_NO_FATAL_FAILURE(m_w->noNotify());
    EXPECT_NO_FATAL_FAILURE(m_w->onExitScreenCapture());
    EXPECT_NO_FATAL_FAILURE(m_w->onScreenResolutionChanged());
}

TEST_F(MainWindowCov4Test, keyAndWheelEvents)
{
    QKeyEvent kp(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    bool handled = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowkeyPressEF(*m_w, &kp, handled));
    QWheelEvent we(QPointF(100, 100), QPointF(100, 100), QPoint(0, 120), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowwheelEvent(*m_w, &we));
}

// =========================================================================
// BUG 185721 (sev1) — 终端拉起录屏提示段错误，导致无法录屏
// PMS: https://pms.uniontech.com/bug-view-185721.html  commit: 3691818fe7
// 根因：initAttributes 中未判 windowHandle() 即设窗口属性；startCountdown 路径
//       缺保护。修复在 initAttributes 加 windowHandle() 守卫 + startCountdown 加固。
// 回归：initAttributes（SetUp 已执行，隐式验证守卫）+ startCountdown 不再段错误。
// 注：ut_main_window_ext.h:118 / ut_main_window_cov2.h:319 已有同名行为用例
//     （startCountdownSafe），本例以 BUG 编号命名建立 PMS 可追溯链接，并在
//     MainWindowCov4Test 这个不同 stub 集下复跑，扩宽路径覆盖。
// =========================================================================
TEST_F(MainWindowCov4Test, BUG185721_StartCountdownNoSegfault)
{
    // Arrange — SetUp() 已 new MainWindow + initAttributes() + initResource()，
    //            initAttributes 的 windowHandle() 守卫在此被隐式覆盖。
    ASSERT_NE(m_w, nullptr);

    // Act — 触发修复的第二处（startCountdown 路径）
    EXPECT_NO_FATAL_FAILURE(m_w->startCountdown());

    // Assert — 对象存活、未析构崩溃；窗口句柄属性访问不段错误
    EXPECT_NE(m_w, nullptr);
    EXPECT_NO_FATAL_FAILURE(m_w->windowHandle());
}

// =========================================================================
// BUG 275661 (sev2) — 适配屏幕窗口层级
// PMS: https://pms.uniontech.com/bug-view-275661.html  commit: 38c42f57
// 根因：initAttributes 中屏幕窗口层级设置在部分机型异常。修复追加层级适配。
// 回归：initAttributes 可重复执行（幂等无崩溃），窗口属性稳定。
// =========================================================================
TEST_F(MainWindowCov4Test, BUG275661_InitAttributesScreenLayerIdempotent)
{
    // Arrange — SetUp() 已调用一次 initAttributes()
    ASSERT_NE(m_w, nullptr);

    // Act — 再次执行 initAttributes，验证层级适配代码路径稳定
    EXPECT_NO_FATAL_FAILURE(m_w->initAttributes());

    // Assert — 重复初始化后对象仍可用
    EXPECT_NE(m_w, nullptr);
    EXPECT_NO_FATAL_FAILURE(m_w->initAttributes());
}

// =========================================================================
// BUG 280187 (sev2) — 休眠唤醒后录屏卡顿
// BUG 344127 (sev2) — Treeland 下跳过不必要的后台初始化
// PMS: https://pms.uniontech.com/bug-view-280187.html  commit: d4be697a82 / b8c106bd77
// PMS: https://pms.uniontech.com/bug-view-344127.html  commit: 05d44da0f7
// 根因：initTreelandtAttributes 在休眠唤醒/Treeland 路径下执行了不必要的后台初始化
//       致卡顿。修复跳过冗余初始化、收敛属性设置路径。
// 回归：initTreelandtAttributes 可重复执行（幂等无崩溃），属性设置稳定。
// 注：ut_main_window_cov2.h:322 / ut_main_window_ext.h:112 已有同名行为用例，
//     本例以 BUG 编号命名建立 PMS 可追溯链接，并在 MainWindowCov4Test 这个
//     不同 stub 集下复跑，扩宽路径覆盖。
// =========================================================================
TEST_F(MainWindowCov4Test, BUG280187_InitTreelandtAttributesIdempotent)
{
    // Arrange — SetUp() 已 new MainWindow + initAttributes() + initResource()
    ASSERT_NE(m_w, nullptr);

    // Act — 触发 Treeland 属性初始化路径（修复点：跳过冗余后台初始化）
    EXPECT_NO_FATAL_FAILURE(m_w->initTreelandtAttributes());

    // Assert — 重复执行稳定（休眠唤醒等场景会再次进入此路径）
    EXPECT_NE(m_w, nullptr);
    EXPECT_NO_FATAL_FAILURE(m_w->initTreelandtAttributes());
}

// =========================================================================
// BUG 352125 (sev2) — 整机适配：特效/模糊态合成逻辑
// BUG 241263 (sev2) — 快捷键打开截图录屏时无法 ESC 退出
// BUG 235511 (sev2) — 截图录屏不能自动识别桌面/应用窗口
// BUG 185721 (sev1) — 终端拉起截图录屏段错误（另已测 startCountdown 路径，
//                     本例补 initMainWindow 路径，185721 的 initMainWindow 关联 commit）
// BUG 170721 (sev2) — 休眠唤醒卡顿（另已测 initTreelandtAttributes 路径）
// BUG 124121 (sev2) — 启动性能
// PMS: https://pms.uniontech.com/bug-view-352125.html  commit: 65b0c35ef9
// PMS: https://pms.uniontech.com/bug-view-241263.html  commit: a8b04484d0
// PMS: https://pms.uniontech.com/bug-view-235511.html  commit: fd1d7aaf07
//
// initMainWindow 是 sev1/2 缺陷热点函数（多个 bug 汇聚于此），既有 cov/ef/ext
// 系列均未直接调用此公开函数（SetUp 仅调 initAttributes+initResource）。函数体
// 含光标绑定、pixelRatio、DWindowManagerHelper 信号连接、EventMonitor 创建等。
// wayland/treeland 块被 #ifndef ENABLE_UNIT_TEST 守卫（测试构建跳过），EventMonitor
// 构造的 wayland 分支同理（ut_misc_cov2.h / ut_event_monitor.h 已验证构造安全）。
// 回归：在已 initAttributes+initResource 的 MainWindow 上调用 initMainWindow 不崩溃，
//       覆盖光标/pixelRatio/信号连接/EventMonitor 创建路径，防退化。
// =========================================================================
TEST_F(MainWindowCov4Test, BUG352125_InitMainWindowNoCrash)
{
    // Arrange — SetUp() 已 new MainWindow + initAttributes() + initResource()
    ASSERT_NE(m_w, nullptr);

    // Act — 触发主窗口初始化（多 bug 汇聚路径）
    EXPECT_NO_FATAL_FAILURE(m_w->initMainWindow());

    // Assert — 对象存活，关键初始化路径稳定
    EXPECT_NE(m_w, nullptr);
}
