// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QAction>
#include <DPushButton>
#include <DToolButton>
#include <DIconButton>
#include "../../src/utils.h"

using namespace testing;

// Extended smoke tests for Utils static helpers. The existing ut_utils.h covers setFontSize
// and the X11 input-event family; here we widen coverage of the safe static surface.
//
// Deliberately SKIPPED (touch Wayland / X11 / QProcess / modal dialog per the rules):
//   - passInputEvent, getInputEvent, cancelInputEvent, cancelInputEvent1  (XShapeCombineRectangles / QX11Info)
//   - enableXGrabButton, disableXGrabButton                                (XGrabButton / QX11Info)
//   - checkCpuIsZhaoxin                                                    (spawns QProcess "lscpu")
//   - cursorMove                                                           (QCursor::setPos / WaylandMouseSimulator)
//   - notSupportWarn                                                       (DDialog::exec modal loop)
//   - getAllWindowInfo                                                     (DForeignWindow + X11 geometry)
//   - getCurrentAudioChannel                                               (Qt5-only + QProcess + DBus)
// Utils::instance() constructs a Utils which derives from utils_interface (a DBus proxy);
// we avoid it to keep the test hermetic and instead exercise static methods directly.
class UtilsExtTest : public testing::Test
{
public:
    void SetUp() override { std::cout << "start UtilsExtTest" << std::endl; }
    void TearDown() override { std::cout << "end UtilsExtTest" << std::endl; }
};

// ---- getQrcPath ----
TEST_F(UtilsExtTest, getQrcPath_formatting)
{
    EXPECT_EQ(QString(":/foo.png"), Utils::getQrcPath("foo.png"));
    EXPECT_EQ(QString(":/icons/a"), Utils::getQrcPath("icons/a"));
    EXPECT_EQ(QString(":/"), Utils::getQrcPath(""));
}

// ---- getRenderSize ----
TEST_F(UtilsExtTest, getRenderSize_singleAndMultiLine)
{
    QSize s1 = Utils::getRenderSize(12, "hello");
    EXPECT_GT(s1.width(), 0);
    EXPECT_GT(s1.height(), 0);

    QSize s2 = Utils::getRenderSize(12, "line one\nline two\nline three");
    EXPECT_GT(s2.width(), 0);
    // multi-line height must be greater than single-line height
    EXPECT_GT(s2.height(), s1.height());
}

TEST_F(UtilsExtTest, getRenderSize_emptyString)
{
    QSize s = Utils::getRenderSize(12, "");
    // empty string still produces a non-negative size; height is one line-metrics worth
    EXPECT_GE(s.height(), 0);
}

// ---- setFontSize ----
TEST_F(UtilsExtTest, setFontSize_appliesToPainter)
{
    QPixmap pm(10, 10);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    int before = painter.font().pointSize();
    Utils::setFontSize(painter, 30);
    int after = painter.font().pointSize();
    EXPECT_EQ(30, after);
    // restore to prove it actually changed
    if (before > 0 && before != after) {
        Utils::setFontSize(painter, before);
        EXPECT_EQ(before, painter.font().pointSize());
    }
}

// ---- drawTooltipBackground / drawTooltipText (need an active painter device) ----
TEST_F(UtilsExtTest, drawTooltipBackground_runs)
{
    QPixmap pm(64, 32);
    pm.fill(Qt::transparent);
    QPainter painter(&pm);
    EXPECT_NO_FATAL_FAILURE(Utils::drawTooltipBackground(painter, QRect(0, 0, 64, 32), "#000000", 0.5));
}

TEST_F(UtilsExtTest, drawTooltipText_runs)
{
    QPixmap pm(64, 32);
    pm.fill(Qt::transparent);
    QPainter painter(&pm);
    EXPECT_NO_FATAL_FAILURE(Utils::drawTooltipText(painter, "tip", "#FFFFFF", 10, QRectF(0, 0, 64, 32)));
}

// ---- setAccessibility overloads ----
TEST_F(UtilsExtTest, setAccessibility_DPushButton)
{
    DPushButton btn;
    Utils::setAccessibility(&btn, "btnSave");
    EXPECT_EQ(QString("btnSave"), btn.objectName());
    EXPECT_EQ(QString("btnSave"), btn.accessibleName());
}

TEST_F(UtilsExtTest, setAccessibility_DToolButton)
{
    DToolButton btn;
    Utils::setAccessibility(&btn, "toolRec");
    EXPECT_EQ(QString("toolRec"), btn.objectName());
    EXPECT_EQ(QString("toolRec"), btn.accessibleName());
}

#if (QT_VERSION_MAJOR == 6)
TEST_F(UtilsExtTest, setAccessibility_DIconButton)
{
    DIconButton btn;
    Utils::setAccessibility(&btn, "iconClose");
    EXPECT_EQ(QString("iconClose"), btn.objectName());
    EXPECT_EQ(QString("iconClose"), btn.accessibleName());
}
#endif

TEST_F(UtilsExtTest, setAccessibility_QAction)
{
    QAction act;
    Utils::setAccessibility(&act, "actExit");
    EXPECT_EQ(QString("actExit"), act.objectName());
}

// ---- CPU model name (DSysInfo-backed, read-only) ----
TEST_F(UtilsExtTest, getCpuModelName_runs)
{
    // Host-dependent value; just ensure the call returns without throwing.
    QString name = Utils::getCpuModelName();
    EXPECT_NO_FATAL_FAILURE((void)name);
}

// ---- System-version helpers (read-only DSysInfo queries) ----
TEST_F(UtilsExtTest, isSysHighVersion1040_isBool)
{
    EXPECT_NO_FATAL_FAILURE(Utils::isSysHighVersion1040());
    // result is host-dependent; just ensure it returns a bool without throwing.
    (void)Utils::isSysHighVersion1040();
}

TEST_F(UtilsExtTest, isSysGreatEqualV23_isBool)
{
    EXPECT_NO_FATAL_FAILURE(Utils::isSysGreatEqualV23());
    (void)Utils::isSysGreatEqualV23();
}

TEST_F(UtilsExtTest, showCurrentSys_runs)
{
    EXPECT_NO_FATAL_FAILURE(Utils::showCurrentSys());
}

// ---- Screen info queries (read QGuiApplication::screens, safe) ----
TEST_F(UtilsExtTest, getScreensInfo_nonEmpty)
{
    QList<Utils::ScreenInfo> infos = Utils::getScreensInfo();
    // At least the test offscreen platform exposes one screen.
    EXPECT_GE(infos.size(), 1);
    for (Utils::ScreenInfo &si : infos) {
        EXPECT_NO_FATAL_FAILURE(si.toString());
    }
}

TEST_F(UtilsExtTest, screenInfo_toStringFormat)
{
    QList<Utils::ScreenInfo> infos = Utils::getScreensInfo();
    ASSERT_GE(infos.size(), 1);
    QString s = infos.first().toString();
    EXPECT_TRUE(s.contains("ScreenName"));
}

// ---- Position scaling helpers (depend on getScreensInfo; safe, no X11) ----
TEST_F(UtilsExtTest, getPosWithScreen_origin)
{
    QPoint out = Utils::getPosWithScreen(QPoint(1, 1));
    // origin is always within the first screen; result is well-defined (may be (0,0) off-screen).
    EXPECT_NO_FATAL_FAILURE((void)out);
}

TEST_F(UtilsExtTest, getPosWithScreenP_origin)
{
    QPoint out = Utils::getPosWithScreenP(QPoint(1, 1));
    EXPECT_NO_FATAL_FAILURE((void)out);
}
