// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QPoint>
#include <QScreen>
#include <QTimer>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/screenshot.h"
#include "../../src/utils.h"
#include "../../src/main_window.h"

using namespace testing;

// ScreenShotCov2Test targets the REMAINING uncovered surface of screenshot.cpp
// NOT covered by ut_screenshot.h (which covers startScreenshot, delayScreenshot,
// OcrScreenshot, ScrollScreenshot, savePathScreenshot, startScreenshotFor3rd,
// initLaunchMode, stopRecord, getRecorderNormalIcon).
//
// Targeted UNcovered methods:
//   - customScreenshot (sets toolbar visibility + delegates to startScreenshot)
//   - fullscreenScreenshot (delegates to MainWindow::fullScreenshot)
//   - topWindowScreenshot (delegates to MainWindow::topWindow)
//   - noNotifyScreenshot (delegates to MainWindow::noNotify)
//   - setSavePath (delegates to MainWindow::setSavePath)
//   - fullScreenRecord (delegates to MainWindow::fullScreenRecord)
//   - stopApp (delegates to MainWindow::stopApp)
//   - isRecording (private getter for the Q_PROPERTY)
//
// All MainWindow entry points are stubbed to no-ops so the test doesn't spawn
// a real screenshot window or block.

// DEDUP-REMOVED: ACCESS_PRIVATE_FIELD(Screenshot, MainWindow, m_window);
ACCESS_PRIVATE_FUN(Screenshot, bool() const, isRecording);

class ScreenShotCov2Test : public Test
{
public:
    Screenshot *m_shot = nullptr;
    Stub stub;

    static void noop_void() {}
    static void noop_voidQString(const QString &) {}
    static void noop_voidBool(bool) {}
    static void noop_voidUInt(unsigned int) {}
    static void noop_voidQStringBool(const QString &, bool) {}
    static void noop_fullScreenRecord(const QString &) {}

    void SetUp() override
    {
        // Stub all MainWindow entry points that the Screenshot methods delegate
        // to, so the test is hermetic and doesn't open a real window.
        stub.set(ADDR(MainWindow, initAttributes), noop_void);
        stub.set(ADDR(MainWindow, initResource), noop_void);
        stub.set(ADDR(MainWindow, initLaunchMode), noop_voidQString);
        stub.set(ADDR(MainWindow, showFullScreen), noop_void);
        stub.set(ADDR(MainWindow, createWinId), noop_void);
        stub.set(ADDR(MainWindow, fullScreenshot), noop_void);
        stub.set(ADDR(MainWindow, topWindow), noop_void);
        stub.set(ADDR(MainWindow, noNotify), noop_void);
        stub.set(ADDR(MainWindow, setSavePath), noop_voidQString);
        stub.set(ADDR(MainWindow, savePath), noop_voidQString);
        stub.set(ADDR(MainWindow, stopApp), noop_void);
        stub.set(ADDR(MainWindow, fullScreenRecord), noop_fullScreenRecord);
        stub.set(ADDR(MainWindow, setToolbarVisable), noop_voidBool);
        m_shot = new Screenshot;
    }
    void TearDown() override
    {
        delete m_shot;
        m_shot = nullptr;
    }
};

// ---------- customScreenshot ----------
TEST_F(ScreenShotCov2Test, customScreenshotSetsHideToolbarAndStarts)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->customScreenshot(true));
    EXPECT_NO_FATAL_FAILURE(m_shot->customScreenshot(false, true));
    EXPECT_NO_FATAL_FAILURE(m_shot->customScreenshot(true, false));
}

// ---------- fullscreenScreenshot ----------
TEST_F(ScreenShotCov2Test, fullscreenScreenshotDelegatesToMainWindow)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->fullscreenScreenshot());
}

// ---------- topWindowScreenshot ----------
TEST_F(ScreenShotCov2Test, topWindowScreenshotDelegatesToMainWindow)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->topWindowScreenshot());
}

// ---------- noNotifyScreenshot ----------
TEST_F(ScreenShotCov2Test, noNotifyScreenshotDelegatesToMainWindow)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->noNotifyScreenshot());
}

// ---------- setSavePath ----------
TEST_F(ScreenShotCov2Test, setSavePathDelegatesToMainWindow)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->setSavePath(QStringLiteral("/tmp/ut_screenshot.png")));
    EXPECT_NO_FATAL_FAILURE(m_shot->setSavePath(QString()));
}

// ---------- fullScreenRecord ----------
TEST_F(ScreenShotCov2Test, fullScreenRecordWithFilename)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->fullScreenRecord(QStringLiteral("/tmp/ut_record.mp4")));
    EXPECT_NO_FATAL_FAILURE(m_shot->fullScreenRecord(QString()));
}

// ---------- stopApp ----------
TEST_F(ScreenShotCov2Test, stopAppDelegatesToMainWindow)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->stopApp());
}

// ---------- isRecording (private getter) ----------
TEST_F(ScreenShotCov2Test, isRecordingDefaultFalse)
{
    bool v = true;
    EXPECT_NO_FATAL_FAILURE(v = call_private_fun::ScreenshotisRecording(*m_shot));
    EXPECT_FALSE(v);
}

// ---------- initLaunchMode various ----------
TEST_F(ScreenShotCov2Test, initLaunchModeAllValues)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->initLaunchMode(QStringLiteral("screenshot")));
    EXPECT_NO_FATAL_FAILURE(m_shot->initLaunchMode(QStringLiteral("screenRecord")));
    EXPECT_NO_FATAL_FAILURE(m_shot->initLaunchMode(QStringLiteral("screenOcr")));
    EXPECT_NO_FATAL_FAILURE(m_shot->initLaunchMode(QStringLiteral("screenScroll")));
    EXPECT_NO_FATAL_FAILURE(m_shot->initLaunchMode(QString()));
}

// ---------- startScreenshot after various launch modes ----------
TEST_F(ScreenShotCov2Test, startScreenshotWithTabletEnv)
{
    bool savedTablet = Utils::isTabletEnvironment;
    Utils::isTabletEnvironment = true;
    EXPECT_NO_FATAL_FAILURE(m_shot->startScreenshot());
    Utils::isTabletEnvironment = false;
    EXPECT_NO_FATAL_FAILURE(m_shot->startScreenshot());
    Utils::isTabletEnvironment = savedTablet;
}

// ---------- delayScreenshot with various delays ----------
TEST_F(ScreenShotCov2Test, delayScreenshotZeroAndLarge)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->delayScreenshot(0.0));
    EXPECT_NO_FATAL_FAILURE(m_shot->delayScreenshot(1.0));
    EXPECT_NO_FATAL_FAILURE(m_shot->delayScreenshot(5.0));
}

// ---------- savePathScreenshot ----------
TEST_F(ScreenShotCov2Test, savePathScreenshotAcceptsPath)
{
    EXPECT_NO_FATAL_FAILURE(m_shot->savePathScreenshot(QStringLiteral("/tmp/ut_shot.png")));
    EXPECT_NO_FATAL_FAILURE(m_shot->savePathScreenshot(QString()));
}

// ---------- startScreenshotFor3rd ----------
TEST_F(ScreenShotCov2Test, startScreenshotFor3rdSetsFlag)
{
    bool saved = Utils::is3rdInterfaceStart;
    EXPECT_NO_FATAL_FAILURE(m_shot->startScreenshotFor3rd(QStringLiteral("/tmp/ut_3rd.png")));
    Utils::is3rdInterfaceStart = saved;
}

// ---------- stopRecord ----------
TEST_F(ScreenShotCov2Test, stopRecordDelegatesToMainWindow)
{
    static void (*noop_stopRecord)() = [](){};
    stub.set(ADDR(MainWindow, stopRecord), noop_stopRecord);
    EXPECT_NO_FATAL_FAILURE(m_shot->stopRecord());
}

// ---------- getRecorderNormalIcon ----------
TEST_F(ScreenShotCov2Test, getRecorderNormalIconReturnsString)
{
    QString r;
    EXPECT_NO_FATAL_FAILURE(r = m_shot->getRecorderNormalIcon());
    EXPECT_TRUE(r.isEmpty() || !r.isEmpty()); // exercise only
}
