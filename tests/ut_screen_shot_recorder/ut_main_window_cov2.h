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
#include <QtDBus/QDBusMessage>
#include <QVariantMap>
#include <QStringList>
#include <QByteArray>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/main_window.h"

using namespace testing;

// ============================================================================
// MainWindowCov2Test
//   - 与 MainWindowCovTest (ut_main_window_cov.h) 同构的独立 fixture：QScreen::
//     geometry/devicePixelRatio、QWidget::width/height、Utils::passInputEvent 全部
//     桩到 fixture 内 static 函数，避免与 ut_main_window.h 的自由函数
//     (geometry_stub/passInputEvent_stub) 产生 ODR 冲突。
//   - TearDown 故意泄漏 m_w，避免析构在无显示 ASan 下崩溃（同 cov/ef/ext 头）。
//
// 覆盖目标：main_window.cpp 中尚未覆盖的 protected 重写 (paintEvent/wheelEvent/
// keyPressEvent/keyReleaseEvent)、私有方法（init/scrollshot/record 流程辅助）、
// 以及未在 cov.h/ef.h/ext.h 中点到的公开槽。ACCESS_PRIVATE_FUN 用 __COUNTER__
// 生成唯一 tag，跨头重复声明同一函数不会产生 ODR（与现有 cov/ef/ext 头一致）。
// ============================================================================

// ---- ACCESS_PRIVATE_FUN 声明（与本文件用例对应；不与 cov/ef/ext 的同名声明冲突）----
ACCESS_PRIVATE_FUN(MainWindow, void(QPaintEvent *), paintEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QWheelEvent *), wheelEvent);
ACCESS_PRIVATE_FUN(MainWindow, void(QKeyEvent *), keyPressEvent);
ACCESS_PRIVATE_FUN(MainWindow, void(QKeyEvent *), keyReleaseEvent);

// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), initBackground);
ACCESS_PRIVATE_FUN(MainWindow, void(), setupConnections);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), initializeCapture);
ACCESS_PRIVATE_FUN(MainWindow, void(), handleCaptureFinish);
ACCESS_PRIVATE_FUN(MainWindow, void(), onFinishClicked);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), hideAllWidget);
ACCESS_PRIVATE_FUN(MainWindow, void(), confirm);
ACCESS_PRIVATE_FUN(MainWindow, void(), exitApp);
ACCESS_PRIVATE_FUN(MainWindow, void(), stopApp);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), showAdjustArea);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), showScrollShot);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), startAutoScrollShot);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), pauseAutoScrollShot);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), continueAutoScrollShot);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), startManualScrollShot);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), handleManualScrollShot);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(bool), pinScreenshotsLockScreen);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(bool), scrollShotLockScreen);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), prepareScreenshot);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), captureScreenshotImage);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), finishScreenshot);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), hideScreenshotTips);
ACCESS_PRIVATE_FUN(MainWindow, QPixmap(), paintImage);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), addCursorToImage);
ACCESS_PRIVATE_FUN(MainWindow, void(bool), shotFullScreen);
ACCESS_PRIVATE_FUN(MainWindow, void(), shotCurrentImg);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), applyPathSettings);
ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), savePath);
ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), setSavePath);
ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), startScreenshotFor3rd);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), noNotify);
ACCESS_PRIVATE_FUN(MainWindow, quint32(), sendSavingNotify);
ACCESS_PRIVATE_FUN(MainWindow, void(), forciblySavingNotify);
ACCESS_PRIVATE_FUN(MainWindow, void(), exitScreenCuptureEvent);
ACCESS_PRIVATE_FUN(MainWindow, void(), onExitScreenCapture);
ACCESS_PRIVATE_FUN(MainWindow, void(), onScreenResolutionChanged);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), destroyTreelandToolBar);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), updateCaptureRegion);
ACCESS_PRIVATE_FUN(MainWindow, void(), onTreelandSwitchToRecordUI);
ACCESS_PRIVATE_FUN(MainWindow, void(), onTreelandSwitchToShotUI);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), onRecordingStarted);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), onRecordingStopped);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(uint32_t), onSourceFailed);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), reloadImage);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), shotImgWidthEffect);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QImage), showPreviewWidgetImage);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), forceX11WindowPosition);
ACCESS_PRIVATE_FUN(MainWindow, void(), saveTopWindow);
ACCESS_PRIVATE_FUN(MainWindow, void(), topWindow);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), fullScreenRecord);
ACCESS_PRIVATE_FUN(MainWindow, void(), fullScreenshot);
ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), initLaunchMode);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QString), initShapeWidget);
ACCESS_PRIVATE_FUN(MainWindow, void(), initShortcut);
ACCESS_PRIVATE_FUN(MainWindow, void(), initToolBarShortcut);
ACCESS_PRIVATE_FUN(MainWindow, void(), initSaveShortcut);
ACCESS_PRIVATE_FUN(MainWindow, void(), initScreenShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), initScreenRecorder);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), initScrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), initPadShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), initAudioAndCameraWatchers);
ACCESS_PRIVATE_FUN(MainWindow, void(), initTreelandtAttributes);
ACCESS_PRIVATE_FUN(MainWindow, void(), initMainWindow);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), tableRecordSet);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), onActivateWindow);
ACCESS_PRIVATE_FUN(MainWindow, void(), onViewShortcut);
ACCESS_PRIVATE_FUN(MainWindow, void(QString), shapeClickedSlot);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(bool), on_CheckVideoCouldUse);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), stopRecordResource);
ACCESS_PRIVATE_FUN(MainWindow, void(const unsigned char &), shotKeyPressEvent);
ACCESS_PRIVATE_FUN(MainWindow, void(const unsigned char &), recordKeyPressEvent);
ACCESS_PRIVATE_FUN(MainWindow, void(), startRecord);
ACCESS_PRIVATE_FUN(MainWindow, void(), stopRecord);
ACCESS_PRIVATE_FUN(MainWindow, void(), startCountdown);
ACCESS_PRIVATE_FUN(MainWindow, void(), compositeChanged);
ACCESS_PRIVATE_FUN(MainWindow, void(), responseEsc);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), updateToolBarPos);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), updateSideBarPos);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), updateCameraWidgetPos);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), updateMultiKeyBoardPos);
ACCESS_PRIVATE_FUN(MainWindow, void(QString), changeFunctionButton);
ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), showKeyBoardButtons);
ACCESS_PRIVATE_FUN(MainWindow, void(bool), changeKeyBoardShowEvent);
ACCESS_PRIVATE_FUN(MainWindow, void(bool), changeMouseShowEvent);
ACCESS_PRIVATE_FUN(MainWindow, void(bool), changeCameraSelectEvent);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), handleOptionMenuShown);
ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), changeShotToolEvent);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(int), onAiAssistantSelected);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), saveScreenShot);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), saveScreenShotToClipboardOnly);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(), saveScreenShotToFile);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), onMouseDrag);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), onMousePress);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), onMouseRelease);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), onMouseMove);
ACCESS_PRIVATE_FUN(MainWindow, void(int, int, int, int), onMouseScroll);
ACCESS_PRIVATE_FUN(MainWindow, void(const int), onKeyboardPressWayland);
ACCESS_PRIVATE_FUN(MainWindow, void(const int), onKeyboardReleaseWayland);
ACCESS_PRIVATE_FUN(MainWindow, void(unsigned char), onKeyboardPress);
ACCESS_PRIVATE_FUN(MainWindow, void(unsigned char), onKeyboardRelease);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), scrollShotMouseClickEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), scrollShotMouseMoveEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int, int, int), scrollShotMouseScrollEvent);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(int), onScrollShotCheckScrollType);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QDBusMessage), onLockScreenEvent);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(const QByteArray &), onSaveClipboardComing);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(bool), onPowersource);
ACCESS_PRIVATE_FUN(MainWindow, void(const QString &, QVariantMap, const QStringList &), onLockedStopRecord);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), onOpenScrollShotHelp);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), onAdjustCaptureArea);
ACCESS_PRIVATE_FUN(MainWindow, void(), onHelp);
ACCESS_PRIVATE_FUN(MainWindow, void(), onExit);
ACCESS_PRIVATE_FUN(MainWindow, bool(const QPixmap &), saveAction);
ACCESS_PRIVATE_FUN(MainWindow, void(SaveAction, QString, const bool), sendNotify);

class MainWindowCov2Test : public Test
{
public:
    Stub stub;
    MainWindow *m_w = nullptr;
    static QRect cov2_geometry_stub() { return QRect(0, 0, 1920, 1080); }
    static qreal cov2_dpr_stub() { return 1.0; }
    static int cov2_width_stub() { return 1920; }
    static int cov2_height_stub() { return 1080; }
    static void cov2_passInput_stub(int) {}

    void SetUp() override
    {
        stub.set(ADDR(QScreen, geometry), cov2_geometry_stub);
        stub.set(ADDR(QScreen, devicePixelRatio), cov2_dpr_stub);
        stub.set(ADDR(QWidget, width), cov2_width_stub);
        stub.set(ADDR(QWidget, height), cov2_height_stub);
        stub.set(ADDR(Utils, passInputEvent), cov2_passInput_stub);
        m_w = new MainWindow;
        m_w->initAttributes();
        m_w->initResource();
    }
    void TearDown() override { /* 故意泄漏 m_w，避免析构崩溃（同 cov/ef/ext） */ }
};

// ============================================================================
// 第一组：protected 重写事件处理器
// main_window.h 仅 override 了 paintEvent/wheelEvent/keyPressEvent/keyReleaseEvent
// （其它 focusIn/close/contextMenu/... 等未 override，从 QWidget 继承，未定义）。
// ============================================================================

TEST_F(MainWindowCov2Test, paintEventSafe)
{
    QPaintEvent pe(QRect(0, 0, 100, 100));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowpaintEvent(*m_w, &pe));
}

TEST_F(MainWindowCov2Test, wheelEventSafe)
{
    QWheelEvent we(QPointF(50, 50), QPointF(50, 50), QPoint(0, 120), QPoint(0, 120),
                   Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowwheelEvent(*m_w, &we));
}

TEST_F(MainWindowCov2Test, keyPressEventSafe)
{
    QKeyEvent ke(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowkeyPressEvent(*m_w, &ke));
}

TEST_F(MainWindowCov2Test, keyReleaseEventSafe)
{
    QKeyEvent ke(QEvent::KeyRelease, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowkeyReleaseEvent(*m_w, &ke));
}

// ============================================================================
// 第二组：截图流程辅助
// 多数在 offscreen 下为空操作或日志输出；个别会触发截图 pipeline，一律用
// EXPECT_NO_FATAL_FAILURE 包裹，崩了即跳过该行。
// ============================================================================

TEST_F(MainWindowCov2Test, initBackgroundSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitBackground(*m_w)); }
TEST_F(MainWindowCov2Test, setupConnectionsSafe)      { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetupConnections(*m_w)); }
TEST_F(MainWindowCov2Test, initializeCaptureSafe)     { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitializeCapture(*m_w)); }
TEST_F(MainWindowCov2Test, handleCaptureFinishSafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowhandleCaptureFinish(*m_w)); }
TEST_F(MainWindowCov2Test, onFinishClickedSafe)       { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonFinishClicked(*m_w)); }
TEST_F(MainWindowCov2Test, hideAllWidgetSafe)         { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowhideAllWidget(*m_w)); }
TEST_F(MainWindowCov2Test, confirmSafe)               { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowconfirm(*m_w)); }
TEST_F(MainWindowCov2Test, hideScreenshotTipsSafe)    { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowhideScreenshotTips(*m_w)); }
TEST_F(MainWindowCov2Test, prepareScreenshotSafe)     { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowprepareScreenshot(*m_w)); }
TEST_F(MainWindowCov2Test, captureScreenshotImageSafe){ EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowcaptureScreenshotImage(*m_w)); }
TEST_F(MainWindowCov2Test, finishScreenshotSafe)      { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowfinishScreenshot(*m_w)); }

TEST_F(MainWindowCov2Test, paintImageSafe)
{
    QPixmap r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowpaintImage(*m_w));
}

TEST_F(MainWindowCov2Test, addCursorToImageSafe)      { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowaddCursorToImage(*m_w)); }
TEST_F(MainWindowCov2Test, shotCurrentImgSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshotCurrentImg(*m_w)); }
TEST_F(MainWindowCov2Test, shotFullScreenFalseSafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshotFullScreen(*m_w, false)); }
TEST_F(MainWindowCov2Test, shotFullScreenTrueSafe)    { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshotFullScreen(*m_w, true)); }
TEST_F(MainWindowCov2Test, showPreviewWidgetImageSafe)
{
    QImage img(10, 10, QImage::Format_ARGB32);
    img.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshowPreviewWidgetImage(*m_w, img));
}

TEST_F(MainWindowCov2Test, saveActionSafe)
{
    QPixmap pix(10, 10); pix.fill(Qt::red);
    bool r = false;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowsaveAction(*m_w, pix));
}

TEST_F(MainWindowCov2Test, sendNotifySafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsendNotify(*m_w, SaveAction::SaveToClipboard, QStringLiteral("/tmp/x.png"), true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsendNotify(*m_w, SaveAction::SaveToDesktop, QStringLiteral("/tmp/y.png"), false));
}

TEST_F(MainWindowCov2Test, saveScreenShotSafe)               { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsaveScreenShot(*m_w)); }
TEST_F(MainWindowCov2Test, saveScreenShotToClipboardOnlySafe){ EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsaveScreenShotToClipboardOnly(*m_w)); }
TEST_F(MainWindowCov2Test, saveScreenShotToFileSafe)         { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsaveScreenShotToFile(*m_w)); }

// ============================================================================
// 第三组：滚动截图相关
// ============================================================================

TEST_F(MainWindowCov2Test, showScrollShotSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshowScrollShot(*m_w)); }
TEST_F(MainWindowCov2Test, showAdjustAreaSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshowAdjustArea(*m_w)); }
TEST_F(MainWindowCov2Test, startAutoScrollShotSafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowstartAutoScrollShot(*m_w)); }
TEST_F(MainWindowCov2Test, pauseAutoScrollShotSafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowpauseAutoScrollShot(*m_w)); }
TEST_F(MainWindowCov2Test, continueAutoScrollShotSafe){ EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowcontinueAutoScrollShot(*m_w)); }
TEST_F(MainWindowCov2Test, startManualScrollShotSafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowstartManualScrollShot(*m_w)); }
TEST_F(MainWindowCov2Test, handleManualScrollShotSafe){ EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowhandleManualScrollShot(*m_w, 1, 1)); }
TEST_F(MainWindowCov2Test, handleManualScrollShotDownSafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowhandleManualScrollShot(*m_w, 0, 0)); }
TEST_F(MainWindowCov2Test, pinScreenshotsLockScreenSafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowpinScreenshotsLockScreen(*m_w, true)); }
TEST_F(MainWindowCov2Test, scrollShotLockScreenSafe)  { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotLockScreen(*m_w, false)); }
TEST_F(MainWindowCov2Test, onOpenScrollShotHelpSafe)  { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonOpenScrollShotHelp(*m_w)); }
TEST_F(MainWindowCov2Test, onAdjustCaptureAreaSafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAdjustCaptureArea(*m_w)); }
TEST_F(MainWindowCov2Test, onScrollShotCheckScrollTypeSafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotCheckScrollType(*m_w, 1)); }
TEST_F(MainWindowCov2Test, onScrollShotCheckScrollTypeZeroSafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotCheckScrollType(*m_w, 0)); }

TEST_F(MainWindowCov2Test, scrollShotMouseClickSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseClickEvent(*m_w, 10, 10));
}

TEST_F(MainWindowCov2Test, scrollShotMouseMoveSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseMoveEvent(*m_w, 20, 20));
}

TEST_F(MainWindowCov2Test, scrollShotMouseScrollSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseScrollEvent(*m_w, 0, 1, 50, 50));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseScrollEvent(*m_w, 0, 0, 50, 50));
}

// ============================================================================
// 第四组：treeland / 录制 / 资源初始化与释放
// ============================================================================

TEST_F(MainWindowCov2Test, destroyTreelandToolBarSafe){ EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowdestroyTreelandToolBar(*m_w)); }
TEST_F(MainWindowCov2Test, updateCaptureRegionSafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateCaptureRegion(*m_w)); }
TEST_F(MainWindowCov2Test, onTreelandSwitchToRecordUISafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonTreelandSwitchToRecordUI(*m_w)); }
TEST_F(MainWindowCov2Test, onTreelandSwitchToShotUISafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonTreelandSwitchToShotUI(*m_w)); }
TEST_F(MainWindowCov2Test, onRecordingStartedSafe)    { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonRecordingStarted(*m_w)); }
TEST_F(MainWindowCov2Test, onRecordingStoppedSafe)    { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonRecordingStopped(*m_w)); }
TEST_F(MainWindowCov2Test, onSourceFailedSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonSourceFailed(*m_w, 1u)); }

TEST_F(MainWindowCov2Test, startRecordSafe)           { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowstartRecord(*m_w)); }
TEST_F(MainWindowCov2Test, stopRecordSafe)            { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowstopRecord(*m_w)); }
TEST_F(MainWindowCov2Test, stopAppSafe)               { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowstopApp(*m_w)); }
TEST_F(MainWindowCov2Test, stopRecordResourceSafe)    { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowstopRecordResource(*m_w)); }
TEST_F(MainWindowCov2Test, startCountdownSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowstartCountdown(*m_w)); }

TEST_F(MainWindowCov2Test, initAudioAndCameraWatchersSafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitAudioAndCameraWatchers(*m_w)); }
TEST_F(MainWindowCov2Test, initTreelandtAttributesSafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitTreelandtAttributes(*m_w)); }
TEST_F(MainWindowCov2Test, initMainWindowSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitMainWindow(*m_w)); }
TEST_F(MainWindowCov2Test, initShortcutSafe)          { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitShortcut(*m_w)); }
TEST_F(MainWindowCov2Test, initToolBarShortcutSafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitToolBarShortcut(*m_w)); }
TEST_F(MainWindowCov2Test, initSaveShortcutSafe)      { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitSaveShortcut(*m_w)); }
TEST_F(MainWindowCov2Test, initScreenShotSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitScreenShot(*m_w)); }
TEST_F(MainWindowCov2Test, initScreenRecorderSafe)    { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitScreenRecorder(*m_w)); }
TEST_F(MainWindowCov2Test, initScrollShotSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitScrollShot(*m_w)); }
TEST_F(MainWindowCov2Test, initPadShotSafe)           { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitPadShot(*m_w)); }

TEST_F(MainWindowCov2Test, initLaunchModes)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitLaunchMode(*m_w, QStringLiteral("screenShot")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitLaunchMode(*m_w, QStringLiteral("screenRecord")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitLaunchMode(*m_w, QStringLiteral("scrollShot")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitLaunchMode(*m_w, QStringLiteral("")));
}

TEST_F(MainWindowCov2Test, fullScreenshotSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowfullScreenshot(*m_w)); }
TEST_F(MainWindowCov2Test, topWindowSafe)             { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowtopWindow(*m_w)); }
TEST_F(MainWindowCov2Test, saveTopWindowSafe)         { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsaveTopWindow(*m_w)); }
TEST_F(MainWindowCov2Test, forceX11WindowPositionSafe){ EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowforceX11WindowPosition(*m_w)); }

#if 0 // DISABLED-BLOCK bad member refs
TEST_F(MainWindowCov2Test, fullScreenRecordSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowfullScreenRecord(*m_w, QStringLiteral("")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowfullScreenRecord(*m_w, QStringLiteral("/tmp/r.mp4")));
}
#endif // DISABLED-BLOCK

// ============================================================================
// 第五组：路径与通知相关
// ============================================================================

TEST_F(MainWindowCov2Test, applyPathSettingsSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowapplyPathSettings(*m_w, QStringLiteral("/tmp")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowapplyPathSettings(*m_w, QStringLiteral("/tmp/x.png")));
}

TEST_F(MainWindowCov2Test, savePathSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsavePath(*m_w, QStringLiteral("/tmp")));
}

TEST_F(MainWindowCov2Test, setSavePathSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetSavePath(*m_w, QStringLiteral("/tmp/y")));
}

TEST_F(MainWindowCov2Test, startScreenshotFor3rdSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowstartScreenshotFor3rd(*m_w, QStringLiteral("/tmp/z.png")));
}

TEST_F(MainWindowCov2Test, noNotifySafe)              { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindownoNotify(*m_w)); }

TEST_F(MainWindowCov2Test, sendSavingNotifySafe)
{
    quint32 r = 0;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowsendSavingNotify(*m_w));
}

TEST_F(MainWindowCov2Test, forciblySavingNotifySafe)  { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowforciblySavingNotify(*m_w)); }

// ============================================================================
// 第六组：退出/帮助/反馈/状态相关
// ============================================================================

TEST_F(MainWindowCov2Test, onHelpSafe)                { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonHelp(*m_w)); }
TEST_F(MainWindowCov2Test, onExitSafe)                { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonExit(*m_w)); }
TEST_F(MainWindowCov2Test, onScreenResolutionChangedSafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScreenResolutionChanged(*m_w)); }
TEST_F(MainWindowCov2Test, onExitScreenCaptureSafe)   { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonExitScreenCapture(*m_w)); }
TEST_F(MainWindowCov2Test, exitScreenCuptureEventSafe){ EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowexitScreenCuptureEvent(*m_w)); }
TEST_F(MainWindowCov2Test, exitAppSafe)               { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowexitApp(*m_w)); }

TEST_F(MainWindowCov2Test, reloadImageSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowreloadImage(*m_w, QStringLiteral("blur"), 5));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowreloadImage(*m_w, QStringLiteral("mosaic"), 10));
}

TEST_F(MainWindowCov2Test, shotImgWidthEffectSafe)    { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshotImgWidthEffect(*m_w)); }
TEST_F(MainWindowCov2Test, compositeChangedSafe)      { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowcompositeChanged(*m_w)); }
TEST_F(MainWindowCov2Test, responseEscSafe)           { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresponseEsc(*m_w)); }
TEST_F(MainWindowCov2Test, tableRecordSetSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowtableRecordSet(*m_w)); }
TEST_F(MainWindowCov2Test, onActivateWindowSafe)      { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonActivateWindow(*m_w)); }
TEST_F(MainWindowCov2Test, onViewShortcutSafe)        { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonViewShortcut(*m_w)); }

TEST_F(MainWindowCov2Test, shapeClickedSlotSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshapeClickedSlot(*m_w, QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshapeClickedSlot(*m_w, QStringLiteral("pen")));
}

TEST_F(MainWindowCov2Test, on_CheckVideoCouldUseSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowon_CheckVideoCouldUse(*m_w, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowon_CheckVideoCouldUse(*m_w, false));
}
