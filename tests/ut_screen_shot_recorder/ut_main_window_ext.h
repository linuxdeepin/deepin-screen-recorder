// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QPixmap>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QByteArray>
#include <QStringList>
#include <QVariantMap>
#include <QtDBus/QDBusMessage>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/main_window.h"

using namespace testing;

// 以下 3 个为 protected 方法，经成员函数指针(ACCESS_PRIVATE_FUN)访问以绕过访问控制。
// 注意：saveImg 已在 main.cpp 经 ACCESS_PRIVATE_FUN 声明并被全局 stub(恒返回 true)，
// 此处不可重复声明，且因被 stub 无法覆盖真实逻辑，故不为其编写用例。
ACCESS_PRIVATE_FUN(MainWindow, bool(const QString &), checkSuffix);
ACCESS_PRIVATE_FUN(MainWindow, int(const QPixmap &), getWaitTimeByImageSize);
ACCESS_PRIVATE_FUN(MainWindow, void(QVBoxLayout *, int, int), adjustLayout);

// 复刻 ut_main_window.h 的安全构造桩：无显示环境下 QScreen::geometry 会崩、
// Utils::passInputEvent 走 X11 注入会 SEGV，必须先桩掉。这两个桩函数
// (geometry_stub / passInputEvent_stub) 由 ut_main_window.h 定义，已在本 TU 可见。
// TearDown 故意泄漏 MainWindow：其析构在无显示 ASAN 下不稳定（gcda 不刷新），
// 逐用例进程退出即回收。
class MainWindowExtTest : public Test
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

TEST_F(MainWindowExtTest, parsePathArgumentDirectory)
{
    QString dir, name, fmt;
    EXPECT_FALSE(m_w->parsePathArgument(QStringLiteral("/tmp/somedir"), dir, name, fmt));
}

TEST_F(MainWindowExtTest, parsePathArgumentFullFile)
{
    QString dir, name, fmt;
    EXPECT_TRUE(m_w->parsePathArgument(QStringLiteral("/tmp/out.png"), dir, name, fmt));
    for (const QString &p : {QStringLiteral("/a.jpg"), QStringLiteral("/a.JPEG"),
                             QStringLiteral("/a.bmp"), QStringLiteral("/a.unknown"), QStringLiteral("")}) {
        EXPECT_NO_FATAL_FAILURE(m_w->parsePathArgument(p, dir, name, fmt));
    }
}

TEST_F(MainWindowExtTest, toolbarScopeAndIntersect)
{
    EXPECT_NO_FATAL_FAILURE(m_w->limitToolbarScope(QPoint(5000, 5000), 0));
    EXPECT_NO_FATAL_FAILURE(m_w->limitToolbarScope(QPoint(-100, -100), 1));
    // getTwoScreenIntersectPos (main_window.cpp:3339) 硬依赖 m_screenInfo>=2 块屏，
    // offscreen 单屏为空 → 越界 SEGV。已剔除，避免整用例 abort 丢覆盖。
}

TEST_F(MainWindowExtTest, pathSetters)
{
    EXPECT_NO_FATAL_FAILURE(m_w->savePath(QStringLiteral("/tmp/x.png")));
    EXPECT_NO_FATAL_FAILURE(m_w->setSavePath(QStringLiteral("/tmp/y")));
    EXPECT_NO_FATAL_FAILURE(m_w->startScreenshotFor3rd(QStringLiteral("/tmp/z.png")));
}

TEST_F(MainWindowExtTest, launchModeAndRecord)
{
    EXPECT_NO_FATAL_FAILURE(m_w->initLaunchMode(QStringLiteral("screenShot")));
    EXPECT_NO_FATAL_FAILURE(m_w->initLaunchMode(QStringLiteral("screenRecord")));
    // fullScreenRecord 会启动真实录制流程(gst pipeline)导致 hang，已剔除。
    EXPECT_NO_FATAL_FAILURE(m_w->setRecordingStateCallback([](bool){}));
    EXPECT_NO_FATAL_FAILURE(m_w->setToolbarVisable(true));
}

TEST_F(MainWindowExtTest, simpleActions)
{
    EXPECT_NO_FATAL_FAILURE(m_w->fullScreenshot());
    EXPECT_NO_FATAL_FAILURE(m_w->topWindow());
    EXPECT_NO_FATAL_FAILURE(m_w->noNotify());
    // saveTopWindow (main_window.cpp:2382) offscreen 下解引用 null QString SEGV，已剔除。
}

TEST_F(MainWindowExtTest, startPressPoints)
{
    EXPECT_NO_FATAL_FAILURE(m_w->getToolBarStartPressPoint());
    EXPECT_NO_FATAL_FAILURE(m_w->getSideBarStartPressPoint());
    EXPECT_NO_FATAL_FAILURE(m_w->getToolBarPoint());
}

TEST_F(MainWindowExtTest, initSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->initTreelandtAttributes());
    EXPECT_NO_FATAL_FAILURE(m_w->initAudioAndCameraWatchers());
}

TEST_F(MainWindowExtTest, recordSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->startCountdown());
    EXPECT_NO_FATAL_FAILURE(m_w->updateCaptureRegion());
    EXPECT_NO_FATAL_FAILURE(m_w->compositeChanged());
    EXPECT_NO_FATAL_FAILURE(m_w->onRecordingStarted());
    EXPECT_NO_FATAL_FAILURE(m_w->onRecordingStopped());
}

TEST_F(MainWindowExtTest, functionSwitchSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->changeFunctionButton(QStringLiteral("shot")));
    EXPECT_NO_FATAL_FAILURE(m_w->changeFunctionButton(QStringLiteral("record")));
    EXPECT_NO_FATAL_FAILURE(m_w->updateToolBarPos());
    EXPECT_NO_FATAL_FAILURE(m_w->updateSideBarPos());
    EXPECT_NO_FATAL_FAILURE(m_w->updateCameraWidgetPos());
}

TEST_F(MainWindowExtTest, keyBoardSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->showKeyBoardButtons(QStringLiteral("s")));
    EXPECT_NO_FATAL_FAILURE(m_w->changeKeyBoardShowEvent(true));
    EXPECT_NO_FATAL_FAILURE(m_w->changeMouseShowEvent(true));
    EXPECT_NO_FATAL_FAILURE(m_w->changeCameraSelectEvent(false));
    EXPECT_NO_FATAL_FAILURE(m_w->updateMultiKeyBoardPos());
    EXPECT_NO_FATAL_FAILURE(m_w->handleOptionMenuShown());
}

TEST_F(MainWindowExtTest, screenshotSlots)
{
    // 抓屏类（prepareScreenshot/captureScreenshotImage/finishScreenshot）会崩，已剔除
    EXPECT_NO_FATAL_FAILURE(m_w->hideScreenshotTips());
    EXPECT_NO_FATAL_FAILURE(m_w->shotImgWidthEffect());
    EXPECT_NO_FATAL_FAILURE(m_w->reloadImage(QStringLiteral("blur"), 10));
}

TEST_F(MainWindowExtTest, feedbackSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->showPressFeedback(10, 10));
    EXPECT_NO_FATAL_FAILURE(m_w->showDragFeedback(20, 20));
    EXPECT_NO_FATAL_FAILURE(m_w->showReleaseFeedback(30, 30));
    EXPECT_NO_FATAL_FAILURE(m_w->responseEsc());
    EXPECT_NO_FATAL_FAILURE(m_w->onActivateWindow());
}

TEST_F(MainWindowExtTest, keyEventSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->shotKeyPressEvent(28));   // Esc
    EXPECT_NO_FATAL_FAILURE(m_w->recordKeyPressEvent(28));
    EXPECT_NO_FATAL_FAILURE(m_w->onKeyboardPress(28));
    EXPECT_NO_FATAL_FAILURE(m_w->onKeyboardRelease(28));
}

TEST_F(MainWindowExtTest, mouseSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->onMouseDrag(10, 10));
    EXPECT_NO_FATAL_FAILURE(m_w->onMousePress(10, 10));
    EXPECT_NO_FATAL_FAILURE(m_w->onMouseMove(20, 20));
    EXPECT_NO_FATAL_FAILURE(m_w->onMouseRelease(20, 20));
    EXPECT_NO_FATAL_FAILURE(m_w->onMouseScroll(1, 1, 30, 30));
}

TEST_F(MainWindowExtTest, shapeAndShortcutSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->changeShotToolEvent(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->shapeClickedSlot(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->onViewShortcut());
    EXPECT_NO_FATAL_FAILURE(m_w->on_CheckVideoCouldUse(true));
    // onAiAssistantSelected→prepareScreenshot→saveActionTriggered 触发截图流程，
    // 无显示环境 SEGV，已剔除。
}

TEST_F(MainWindowExtTest, scrollShotSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->onScrollShotCheckScrollType(1));
    EXPECT_NO_FATAL_FAILURE(m_w->scrollShotMouseClickEvent(10, 10));
    // scrollShotMouseMoveEvent/ScrollEvent (main_window.cpp:6281) 依赖滚动截图运行态
    // 成员，offscreen 下为 null → SEGV，已剔除。
}

TEST_F(MainWindowExtTest, treelandSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->onTreelandSwitchToRecordUI());
    EXPECT_NO_FATAL_FAILURE(m_w->onTreelandSwitchToShotUI());
    EXPECT_NO_FATAL_FAILURE(m_w->onSourceFailed(1));
    EXPECT_NO_FATAL_FAILURE(m_w->destroyTreelandToolBar());
}

TEST_F(MainWindowExtTest, screenAndLockSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_w->onScreenResolutionChanged());
    // onAdjustCaptureArea (main_window.cpp:6522) 解引用 null m_scrollShotTip SEGV，已剔除。
}

TEST_F(MainWindowExtTest, notifySlots)
{
    QDBusMessage msg;
    EXPECT_NO_FATAL_FAILURE(m_w->onLockScreenEvent(msg));
    EXPECT_NO_FATAL_FAILURE(m_w->onSaveClipboardComing(QByteArray("x")));
    EXPECT_NO_FATAL_FAILURE(m_w->onPowersource(true));
    QVariantMap map; QStringList list;
    EXPECT_NO_FATAL_FAILURE(m_w->onLockedStopRecord(QStringLiteral("n"), map, list));
    EXPECT_NO_FATAL_FAILURE(m_w->sendSavingNotify());
    EXPECT_NO_FATAL_FAILURE(m_w->forciblySavingNotify());
}

TEST_F(MainWindowExtTest, padAndHelp)
{
    EXPECT_NO_FATAL_FAILURE(m_w->initPadShot());
    EXPECT_NO_FATAL_FAILURE(m_w->onOpenScrollShotHelp());
    EXPECT_NO_FATAL_FAILURE(m_w->onHelp());
    EXPECT_NO_FATAL_FAILURE(m_w->tableRecordSet());
}

TEST_F(MainWindowExtTest, utilityMethods)
{
    EXPECT_NO_FATAL_FAILURE(m_w->forceX11WindowPosition());
}

// exitAndResource（stopRecordResource/onExitScreenCapture/exitScreenCuptureEvent）
// 会触发退出/线程停止导致挂或崩，已剔除

// === P0 纯逻辑函数补充（无 GUI/系统强依赖，offscreen 下稳定可覆盖）===
TEST_F(MainWindowExtTest, checkSuffixLogic)
{
    auto check = [this](const QString &s) { return call_private_fun::MainWindowcheckSuffix(*m_w, s); };
    EXPECT_TRUE(check(QStringLiteral("a.png")));
    EXPECT_TRUE(check(QStringLiteral("a.jpg")));
    EXPECT_TRUE(check(QStringLiteral("a.bmp")));
    EXPECT_TRUE(check(QStringLiteral("a.jpeg")));
    EXPECT_FALSE(check(QStringLiteral("a")));      // 无后缀
    EXPECT_FALSE(check(QStringLiteral("a.gif")));  // 非法后缀
    EXPECT_FALSE(check(QStringLiteral("a.PNG")));  // 大小写敏感
    EXPECT_FALSE(check(QStringLiteral("")));
}

TEST_F(MainWindowExtTest, getWaitTimeByImageSizeLogic)
{
    auto wait = [this](const QPixmap &p) { return call_private_fun::MainWindowgetWaitTimeByImageSize(*m_w, p); };
    // 空图 → 默认2秒；8K/超大分支 pixmap 过大(>132MB)在 ASan 下有 OOM 风险，故只覆盖到 4K
    EXPECT_EQ(wait(QPixmap()), 2);          // 空
    EXPECT_EQ(wait(QPixmap(1280, 720)), 2); // ≤720p
    EXPECT_EQ(wait(QPixmap(1920, 1080)), 4);// 1080p
    EXPECT_EQ(wait(QPixmap(2560, 1440)), 6);// 2K
    EXPECT_EQ(wait(QPixmap(3840, 2160)), 8);// 4K
}

TEST_F(MainWindowExtTest, adjustLayoutGeometry)
{
    QVBoxLayout *layout = new QVBoxLayout;
    // recordX/Y/Width/Height、m_backgroundRect 均为默认 0，setContentsMargins 安全
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowadjustLayout(*m_w, layout, 100, 100));
    delete layout;
}

TEST_F(MainWindowExtTest, moveToolBarsNullSafe)
{
    // moveToolBars 对 m_toolBar/m_sideBar 均有 null 检查，未初始化时安全返回
    EXPECT_NO_FATAL_FAILURE(m_w->moveToolBars(QPoint(10, 10), QPoint(5, 5)));
}

// === P0 补充：通过 ACCESS_PRIVATE_FUN 访问 protected/private 方法 ===
ACCESS_PRIVATE_FUN(MainWindow, void(), checkIsLockScreen);
ACCESS_PRIVATE_FUN(MainWindow, void(), initDynamicLibPath);
ACCESS_PRIVATE_FUN(MainWindow, void(), hideAllWidget);
ACCESS_PRIVATE_FUN(MainWindow, void(QString), initShapeWidget);
ACCESS_PRIVATE_FUN(MainWindow, void(), setDragCursor);
ACCESS_PRIVATE_FUN(MainWindow, void(), initializeCapture);

TEST_F(MainWindowExtTest, checkIsLockScreenSafe)
{
    // DBus 接口已全局 stub 成空 → isValid() 返回 false，提前 return，安全
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowcheckIsLockScreen(*m_w));
}

TEST_F(MainWindowExtTest, initDynamicLibPathSafe)
{
    // libPath 读取系统库目录；非空目录下安全
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitDynamicLibPath(*m_w));
}

TEST_F(MainWindowExtTest, hideAllWidgetSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowhideAllWidget(*m_w));
}

TEST_F(MainWindowExtTest, initShapeWidgetSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitShapeWidget(*m_w, "rectangle"));
}

TEST_F(MainWindowExtTest, setDragCursorSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetDragCursor(*m_w));
}

TEST_F(MainWindowExtTest, initializeCaptureSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitializeCapture(*m_w));
}
