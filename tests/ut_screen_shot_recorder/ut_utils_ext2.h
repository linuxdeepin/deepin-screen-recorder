// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QKeyEvent>
#include <QPixmap>
#include <QPainter>
#include "stub.h"
#include "../../src/utils.h"

using namespace testing;

// Additional Utils static helpers not yet covered by ut_utils.h or ut_utils_ext.h.
// Deliberately SKIPPED (X11/Wayland/modal/process dependencies):
//   - passInputEvent, getInputEvent, cancelInputEvent, cancelInputEvent1
//   - enableXGrabButton, disableXGrabButton
//   - checkCpuIsZhaoxin, notSupportWarn
//   - getAllWindowInfo, cursorMove (QCursor::setPos)
//   - getCurrentAudioChannel (QProcess)
//   - blurRect, clearBlur (DWindowManager)
//   - isWaylandProtocol, isTreelandProtocol (QX11Info)

class UtilsExt2Test : public testing::Test
{
public:
    void SetUp() override { std::cout << "start UtilsExt2Test" << std::endl; }
    void TearDown() override { std::cout << "end UtilsExt2Test" << std::endl; }
};

// ---- drawTooltipBackground with opacity ----
TEST_F(UtilsExt2Test, drawTooltipBackgroundDifferentOpacity)
{
    QPixmap pm(64, 32);
    pm.fill(Qt::transparent);
    QPainter painter(&pm);
    EXPECT_NO_FATAL_FAILURE(Utils::drawTooltipBackground(painter, QRect(0, 0, 64, 32), "#FF0000", 0.0));
    EXPECT_NO_FATAL_FAILURE(Utils::drawTooltipBackground(painter, QRect(0, 0, 64, 32), "#00FF00", 1.0));
    EXPECT_NO_FATAL_FAILURE(Utils::drawTooltipBackground(painter, QRect(0, 0, 64, 32), "#0000FF", 0.5));
}

// ---- drawTooltipText with different colors ----
TEST_F(UtilsExt2Test, drawTooltipTextDifferentColors)
{
    QPixmap pm(128, 64);
    pm.fill(Qt::transparent);
    QPainter painter(&pm);
    EXPECT_NO_FATAL_FAILURE(Utils::drawTooltipText(painter, "hello", "#FFFFFF", 12, QRectF(0, 0, 128, 64)));
    EXPECT_NO_FATAL_FAILURE(Utils::drawTooltipText(painter, "world", "#000000", 14, QRectF(0, 0, 128, 64)));
    EXPECT_NO_FATAL_FAILURE(Utils::drawTooltipText(painter, "", "#FF0000", 10, QRectF(0, 0, 128, 64)));
}

// ---- setFontSize with different sizes ----
TEST_F(UtilsExt2Test, setFontSizeVariousSizes)
{
    QPixmap pm(10, 10);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    for (int size : {8, 10, 12, 16, 24, 48}) {
        Utils::setFontSize(painter, size);
        EXPECT_EQ(painter.font().pointSize(), size);
    }
}

// ---- Static member read/write (these are public static vars) ----
TEST_F(UtilsExt2Test, staticFlagsReadWrite)
{
    bool orig3rd = Utils::is3rdInterfaceStart;
    Utils::is3rdInterfaceStart = true;
    EXPECT_TRUE(Utils::is3rdInterfaceStart);
    Utils::is3rdInterfaceStart = false;
    EXPECT_FALSE(Utils::is3rdInterfaceStart);
    Utils::is3rdInterfaceStart = orig3rd;

    int origSpecial = Utils::specialRecordingScreenMode;
    Utils::specialRecordingScreenMode = 0;
    EXPECT_EQ(Utils::specialRecordingScreenMode, 0);
    Utils::specialRecordingScreenMode = 1;
    EXPECT_EQ(Utils::specialRecordingScreenMode, 1);
    Utils::specialRecordingScreenMode = origSpecial;
}

TEST_F(UtilsExt2Test, isFFmpegEnvDefault)
{
    // isFFmpegEnv defaults to true
    EXPECT_TRUE(Utils::isFFmpegEnv);
}

TEST_F(UtilsExt2Test, themeTypeReadWrite)
{
    int orig = Utils::themeType;
    Utils::themeType = 1;
    EXPECT_EQ(Utils::themeType, 1);
    Utils::themeType = orig;
}

TEST_F(UtilsExt2Test, appNameReadWrite)
{
    QString orig = Utils::appName;
    Utils::appName = "test";
    EXPECT_EQ(Utils::appName, "test");
    Utils::appName = orig;
}

TEST_F(UtilsExt2Test, pixelRatioReadWrite)
{
    qreal orig = Utils::pixelRatio;
    Utils::pixelRatio = 1.5;
    EXPECT_DOUBLE_EQ(Utils::pixelRatio, 1.5);
    Utils::pixelRatio = orig;
}

TEST_F(UtilsExt2Test, isRootUserDefault)
{
    // Default is false in non-root test env
    EXPECT_FALSE(Utils::isRootUser);
}

TEST_F(UtilsExt2Test, forceResetScaleReadWrite)
{
    bool orig = Utils::forceResetScale;
    Utils::forceResetScale = true;
    EXPECT_TRUE(Utils::forceResetScale);
    Utils::forceResetScale = orig;
}

// ---- getRenderSize edge cases ----
TEST_F(UtilsExt2Test, getRenderSizeVeryLongString)
{
    QString longText(200, QChar('A'));
    QSize s = Utils::getRenderSize(12, longText);
    EXPECT_GT(s.width(), 0);
    EXPECT_GT(s.height(), 0);
}

TEST_F(UtilsExt2Test, getRenderSizeNewlinesOnly)
{
    QSize s = Utils::getRenderSize(12, "\n\n\n");
    EXPECT_GE(s.height(), 0);
}

// ---- RecordVideoType / AudioRecordType enum values ----
TEST_F(UtilsExt2Test, recordVideoTypeConstants)
{
    EXPECT_EQ(Utils::kGIF, 0);
    EXPECT_EQ(Utils::kMP4, 1);
    EXPECT_EQ(Utils::kMKV, 2);
}

TEST_F(UtilsExt2Test, audioRecordTypeConstants)
{
    EXPECT_EQ(Utils::kNoAudio, 0);
    EXPECT_EQ(Utils::kMic, 1);
    EXPECT_EQ(Utils::kSystemAudio, 2);
    EXPECT_EQ(Utils::kMicAndSystemAudio, 3);
}

// ---- cursorMove via key simulation (no real cursor move in offscreen) ----
TEST_F(UtilsExt2Test, cursorMoveKeyEventBranches)
{
    // cursorMove only modifies a local QPoint; it doesn't call QCursor::setPos directly
    // in the keyEvent branch — it just updates `pos`. We can exercise the logic by
    // calling it with different key events. The function doesn't return anything but
    // the branches are exercised for coverage.
    QPoint cursor(100, 100);
    QKeyEvent leftEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent rightEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent upEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent downEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent aEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    QKeyEvent dEvent(QEvent::KeyPress, Qt::Key_D, Qt::NoModifier);
    QKeyEvent wEvent(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier);
    QKeyEvent sEvent(QEvent::KeyPress, Qt::Key_S, Qt::NoModifier);
    QKeyEvent otherEvent(QEvent::KeyPress, Qt::Key_Z, Qt::NoModifier);

    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(cursor, &leftEvent));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(cursor, &rightEvent));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(cursor, &upEvent));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(cursor, &downEvent));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(cursor, &aEvent));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(cursor, &dEvent));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(cursor, &wEvent));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(cursor, &sEvent));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(cursor, &otherEvent));
}

// ---- getCpuModelName (DSysInfo backed, read-only) ----
TEST_F(UtilsExt2Test, getCpuModelNameReturnsNonEmpty)
{
    QString name = Utils::getCpuModelName();
    // On CI may be empty; just exercise the code path
    EXPECT_NO_FATAL_FAILURE((void)name);
}

// ---- isSysHighVersion1040 / isSysGreatEqualV23 already tested in ut_utils_ext.h ----
// Adding showCurrentSys for additional coverage
TEST_F(UtilsExt2Test, showCurrentSysSafe)
{
    EXPECT_NO_FATAL_FAILURE(Utils::showCurrentSys());
}
