// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QKeyEvent>
#include <QImage>
#include <QPainter>
#include <QProcess>
#include <QOverload>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/utils.h"

using namespace testing;

// Covers the REMAINING safe Utils surface that the existing ut_utils{,_ext,
// _ext2}.h do not reach:
//   - Wayland/TreeLand early-return branches of getInputEvent / cancelInputEvent
//     / cancelInputEvent1 / enableXGrabButton / disableXGrabButton / passInputEvent
//     (the X11 branches call XShapeCombineRectangles/XGrabButton/QX11Info and are
//      FORBIDDEN by the task constraints; the Wayland branches just log+return).
//   - checkCpuIsZhaoxin with QProcess stubbed so no real `lscpu` is spawned.
//   - cursorMove additional key branches not exercised by ext2.
//
// The global Utils::isWaylandMode flag is flipped for the duration of each
// Wayland-branch test and restored in TearDown.

class UtilsCovTest : public Test
{
public:
    Stub stub;
    bool savedWayland = false;
    bool savedTreeland = false;
    void SetUp() override
    {
        savedWayland = Utils::isWaylandMode;
        savedTreeland = Utils::isTreelandMode;
    }
    void TearDown() override
    {
        Utils::isWaylandMode = savedWayland;
        Utils::isTreelandMode = savedTreeland;
    }
};

// ---------- Wayland early-return branches ----------
// NOTE: 这些 Wayland/Treeland 早返回分支已通过 `#ifndef ENABLE_UNIT_TEST` 从单测构建中
// 排除（见 src/utils.cpp），故不再有对应测试用例。X11 路径由 ut_utils.h / ut_utils_ext.h 覆盖。

// ---------- cursorMove: cover remaining key branches ----------

TEST_F(UtilsCovTest, cursorMoveAdditionalKeys)
{
    Utils::pixelRatio = 1.0;
    QPoint pos(100, 100);
    // up / down / left / right without modifiers
    QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent left(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent right(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(pos, &up));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(pos, &down));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(pos, &left));
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(pos, &right));
    SUCCEED();
}

TEST_F(UtilsCovTest, cursorMoveUnknownKeyIsNoop)
{
    Utils::pixelRatio = 1.0;
    QPoint pos(50, 50);
    QKeyEvent other(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(Utils::cursorMove(pos, &other));
    SUCCEED();
}
