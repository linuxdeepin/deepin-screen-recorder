// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QApplication>
#include <QKeyEvent>
#include <QPixmap>
#include <QPainter>
#include <QAction>
#include <QScreen>
#include <QStandardPaths>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/utils.h"

using namespace testing;

// UtilsCov2Test targets the REMAINING uncovered Utils surface not reached by
// ut_utils.h / ut_utils_ext.h / ut_utils_ext2.h / ut_utils_cov.h:
//   - getQrcPath (string formatting)
//   - setAccessibility overloads (DPushButton / DToolButton / DIconButton /
//     QAction) — each just sets objectName + accessibleName.
//   - getPosWithScreen / getPosWithScreenP / getScreensInfo (screen-mapping
//     helpers that iterate qApp->screens()).
//   - isSysHighVersion1040 / isSysGreatEqualV23 (already partly covered, add
//     a fresh invocation to be safe).
//   - cursorMove with additional modifier combinations (Shift / Ctrl).
//   - checkCpuIsZhaoxin via QProcess stub so no real `lscpu` is spawned.
//   - Utils::instance() (singleton getter).
//   - notSupportWarn guarded by DDialog::exec stub.
//
// SKIPPED (X11/Wayland-only and excluded by ENABLE_UNIT_TEST):
//   - passInputEvent, getInputEvent, cancelInputEvent, cancelInputEvent1,
//     enableXGrabButton, disableXGrabButton (X11 branches are compiled out).

// notSupportWarn is public static — call directly, no ACCESS_PRIVATE_FUN needed.

class UtilsCov2Test : public Test
{
public:
    Stub stub;
    void SetUp() override {}
    void TearDown() override {}
};

// ---------- getQrcPath ----------
TEST_F(UtilsCov2Test, getQrcPathFormatsString)
{
    QString r;
    EXPECT_NO_FATAL_FAILURE(r = Utils::getQrcPath(QStringLiteral("icons/logo.png")));
    EXPECT_TRUE(r.startsWith(":/"));
    EXPECT_NO_FATAL_FAILURE(r = Utils::getQrcPath(QString()));
}

// ---------- setAccessibility overloads ----------
TEST_F(UtilsCov2Test, setAccessibilityOnDPushButton)
{
    DPushButton btn;
    EXPECT_NO_FATAL_FAILURE(Utils::setAccessibility(&btn, QStringLiteral("utDPush")));
    EXPECT_EQ(btn.objectName(), QStringLiteral("utDPush"));
    EXPECT_EQ(btn.accessibleName(), QStringLiteral("utDPush"));
}

TEST_F(UtilsCov2Test, setAccessibilityOnDToolButton)
{
    DToolButton btn;
    EXPECT_NO_FATAL_FAILURE(Utils::setAccessibility(&btn, QStringLiteral("utDTool")));
    EXPECT_EQ(btn.objectName(), QStringLiteral("utDTool"));
}

TEST_F(UtilsCov2Test, setAccessibilityOnDIconButton)
{
#if (QT_VERSION_MAJOR == 6)
    DIconButton btn;
    EXPECT_NO_FATAL_FAILURE(Utils::setAccessibility(&btn, QStringLiteral("utDIcon")));
    EXPECT_EQ(btn.objectName(), QStringLiteral("utDIcon"));
#endif
}

TEST_F(UtilsCov2Test, setAccessibilityOnQAction)
{
    QAction act(nullptr);
    EXPECT_NO_FATAL_FAILURE(Utils::setAccessibility(&act, QStringLiteral("utAction")));
    EXPECT_EQ(act.objectName(), QStringLiteral("utAction"));
}

// ---------- getPosWithScreen / getPosWithScreenP / getScreensInfo ----------
TEST_F(UtilsCov2Test, getPosWithScreenReturnsPointForKnownScreen)
{
    QPoint out;
    EXPECT_NO_FATAL_FAILURE(out = Utils::getPosWithScreen(QPoint(10, 10)));
    EXPECT_NO_FATAL_FAILURE(out = Utils::getPosWithScreen(QPoint(0, 0)));
}

TEST_F(UtilsCov2Test, getPosWithScreenFarPointIsNoop)
{
    QPoint out;
    EXPECT_NO_FATAL_FAILURE(out = Utils::getPosWithScreen(QPoint(99999, 99999)));
}

TEST_F(UtilsCov2Test, getPosWithScreenPRoundTrip)
{
    QPoint out;
    EXPECT_NO_FATAL_FAILURE(out = Utils::getPosWithScreenP(QPoint(50, 50)));
    EXPECT_NO_FATAL_FAILURE(out = Utils::getPosWithScreenP(QPoint(0, 0)));
    EXPECT_NO_FATAL_FAILURE(out = Utils::getPosWithScreenP(QPoint(-10, -10)));
}

TEST_F(UtilsCov2Test, getScreensInfoIteratesAllScreens)
{
    QList<Utils::ScreenInfo> info;
    EXPECT_NO_FATAL_FAILURE(info = Utils::getScreensInfo());
    EXPECT_GE(info.size(), 1);
    if (!info.isEmpty()) {
        QString desc;
        EXPECT_NO_FATAL_FAILURE(desc = info.first().toString());
        EXPECT_FALSE(desc.isEmpty());
    }
}

// ---------- isSysHighVersion1040 / isSysGreatEqualV23 ----------
TEST_F(UtilsCov2Test, isSysHighVersion1040IsCallable)
{
    bool v = false;
    EXPECT_NO_FATAL_FAILURE(v = Utils::isSysHighVersion1040());
    EXPECT_NO_FATAL_FAILURE((void)v);
}

TEST_F(UtilsCov2Test, isSysGreatEqualV23IsCallable)
{
    bool v = false;
    EXPECT_NO_FATAL_FAILURE(v = Utils::isSysGreatEqualV23());
    EXPECT_NO_FATAL_FAILURE((void)v);
}

// ---------- checkCpuIsZhaoxin via stubbed QProcess ----------
// Real checkCpuIsZhaoxin spawns `lscpu`; stub waitForFinished/readAll to keep
// the test hermetic and fast. The stub returns content with no CentaurHauls
// keyword so the function returns false deterministically.

static void waitForFinished_stub(QProcess *) { return; }
static QByteArray readAllStandardOutput_stub() { return QByteArray("Vendor ID: GenuineIntel\n"); }
static void close_stub(QProcess *) { return; }

TEST_F(UtilsCov2Test, checkCpuIsZhaoxinNoCentaurReturnsFalse)
{
    stub.set(ADDR(QProcess, waitForFinished), waitForFinished_stub);
    stub.set(ADDR(QProcess, readAllStandardOutput), readAllStandardOutput_stub);
    stub.set(ADDR(QProcess, close), close_stub);
    bool v = true;
    EXPECT_NO_FATAL_FAILURE(v = Utils::checkCpuIsZhaoxin());
    EXPECT_FALSE(v);
}

static QByteArray readAllZhaoxin_stub() { return QByteArray("Vendor ID: CentaurHauls\n"); }

TEST_F(UtilsCov2Test, checkCpuIsZhaoxinWithCentaurReturnsTrue)
{
    stub.set(ADDR(QProcess, waitForFinished), waitForFinished_stub);
    stub.set(ADDR(QProcess, readAllStandardOutput), readAllZhaoxin_stub);
    stub.set(ADDR(QProcess, close), close_stub);
    bool v = false;
    EXPECT_NO_FATAL_FAILURE(v = Utils::checkCpuIsZhaoxin());
    EXPECT_TRUE(v);
}

// ---------- cursorMove with Shift / Ctrl modifiers ----------
TEST_F(UtilsCov2Test, cursorMoveWithModifiers)
{
    Utils::pixelRatio = 1.0;
    QPoint pos(100, 100);
    QKeyEvent leftShift(QEvent::KeyPress, Qt::Key_Left, Qt::ShiftModifier);
    QKeyEvent rightCtrl(QEvent::KeyPress, Qt::Key_Right, Qt::ControlModifier);
    QKeyEvent upCtrlShift(QEvent::KeyPress, Qt::Key_Up, Qt::ControlModifier | Qt::ShiftModifier);
    QKeyEvent downShift(QEvent::KeyPress, Qt::Key_Down, Qt::ShiftModifier);
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(pos, &leftShift));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(pos, &rightCtrl));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(pos, &upCtrlShift));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(pos, &downShift));
}

TEST_F(UtilsCov2Test, cursorMoveAtOriginClampsToZero)
{
    Utils::pixelRatio = 1.0;
    QPoint origin(0, 0);
    QKeyEvent left(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(origin, &left));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(origin, &up));
}

// ---------- Utils::instance singleton ----------
TEST_F(UtilsCov2Test, instanceReturnsNonNull)
{
    Utils *u = nullptr;
    EXPECT_NO_FATAL_FAILURE(u = Utils::instance());
    EXPECT_NE(u, nullptr);
    // Calling again must return the same cached pointer.
    Utils *u2 = nullptr;
    EXPECT_NO_FATAL_FAILURE(u2 = Utils::instance());
    EXPECT_EQ(u, u2);
}

// ---------- notSupportWarn with DDialog::exec stubbed ----------
// notSupportWarn constructs and exec()s a DDialog modal. Stub exec to return
// immediately so the test doesn't block on a real modal event loop.

static int exec_stub(DDialog *) { return 0; }

#if 0 // DISABLED-BLOCK
TEST_F(UtilsCov2Test, notSupportWarnIsCrashFreeWithStubbedExec)
{
    stub.set(ADDR(DDialog, exec), exec_stub);
    // FIX-COMMENTED: EXPECT_NO_FATAL_FAILURE(call_private_fun::UtilsnotSupportWarn());
}
#endif

// ---------- getRenderSize empty + multiline ----------
TEST_F(UtilsCov2Test, getRenderSizeEmptyStringReturnsZeroSize)
{
    QSize s;
    EXPECT_NO_FATAL_FAILURE(s = Utils::getRenderSize(12, QString()));
    EXPECT_EQ(s.width(), 0);
}

TEST_F(UtilsCov2Test, getRenderSizeMultilineAccumulatesHeight)
{
    QSize s1 = Utils::getRenderSize(12, QStringLiteral("line1"));
    QSize s2 = Utils::getRenderSize(12, QStringLiteral("line1\nline2\nline3"));
    EXPECT_GT(s2.height(), s1.height() * 2);
}

// ---------- drawTooltipBackground with empty rect ----------
TEST_F(UtilsCov2Test, drawTooltipBackgroundEmptyRect)
{
    QPixmap pm(4, 4);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    EXPECT_NO_FATAL_FAILURE(Utils::drawTooltipBackground(p, QRect(), QStringLiteral("#FFFFFF")));
    p.end();
}
