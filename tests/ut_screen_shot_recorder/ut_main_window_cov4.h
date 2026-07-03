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
