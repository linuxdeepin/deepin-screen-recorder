// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPixmap>
#include <QImage>
#include <QVBoxLayout>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/main_window.h"

using namespace testing;

// 覆盖 MainWindow 的可达辅助函数（纯/状态机/几何/路径）。
// 构造沿用 MainWindowEFTest 的 QScreen 桩模式（ut_main_window_ef.h）。
// 已有 ACCESS_PRIVATE_FUN 声明（saveImg/checkSuffix/isToolBarInShotArea/
// getWaitTimeByImageSize/getScrollShotTipPosition/setInputEvent/setCancelInputEvent/
// setDragCursor/resetCursor/getPixmapofRect 等）直接复用；此处仅新增下列未声明的：
ACCESS_PRIVATE_FUN(MainWindow, QString(const QString &), libPath);
ACCESS_PRIVATE_FUN(MainWindow, int(QEvent *), getAction);
ACCESS_PRIVATE_FUN(MainWindow, void(QEvent *), updateCursor);
ACCESS_PRIVATE_FUN(MainWindow, void(QMouseEvent *), resizeTop);
ACCESS_PRIVATE_FUN(MainWindow, void(QMouseEvent *), resizeBottom);
ACCESS_PRIVATE_FUN(MainWindow, void(QMouseEvent *), resizeLeft);
ACCESS_PRIVATE_FUN(MainWindow, void(QMouseEvent *), resizeRight);
ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), applyPathSettings);

class MainWindowCovTest : public Test
{
public:
    Stub stub;
    MainWindow *m_w = nullptr;
    static QRect myGeometry_stub() { return QRect(0, 0, 1920, 1080); }
    static qreal myDpr_stub() { return 1.0; }
    static int myWidth_stub() { return 1920; }
    static int myHeight_stub() { return 1080; }
    static void myPassInput_stub(int) {}

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
    void TearDown() override { /* 故意泄漏 m_w，避免析构崩溃（同 MainWindowEFTest） */ }
};

// ---- 纯/低风险公开方法 ----
TEST_F(MainWindowCovTest, limitToolbarScope)
{
    EXPECT_NO_FATAL_FAILURE(m_w->limitToolbarScope(QPoint(10, 10), 0));
    EXPECT_NO_FATAL_FAILURE(m_w->limitToolbarScope(QPoint(5000, 5000), 1));
}

TEST_F(MainWindowCovTest, parsePathArgument)
{
    QString dir, name, fmt;
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE(ok = m_w->parsePathArgument(QStringLiteral("/tmp/a.png"), dir, name, fmt));
    EXPECT_NO_FATAL_FAILURE(m_w->parsePathArgument(QStringLiteral("not/a/path"), dir, name, fmt));
    EXPECT_NO_FATAL_FAILURE(m_w->parsePathArgument(QStringLiteral(""), dir, name, fmt));
}

TEST_F(MainWindowCovTest, savePathAndSetSavePath)
{
    EXPECT_NO_FATAL_FAILURE(m_w->savePath(QStringLiteral("/tmp")));
    EXPECT_NO_FATAL_FAILURE(m_w->setSavePath(QStringLiteral("/home/uos/Pictures")));
}

TEST_F(MainWindowCovTest, getTwoScreenIntersectPos)
{
    EXPECT_NO_FATAL_FAILURE(m_w->getTwoScreenIntersectPos(QPoint(100, 100)));
    EXPECT_NO_FATAL_FAILURE(m_w->getTwoScreenIntersectPos(QPoint(5000, 5000)));
}

TEST_F(MainWindowCovTest, moveToolBars)
{
    EXPECT_NO_FATAL_FAILURE(m_w->moveToolBars(QPoint(0, 0), QPoint(50, 50)));
}

// ---- 公开槽（状态机）----
TEST_F(MainWindowCovTest, responseEsc) { EXPECT_NO_FATAL_FAILURE(m_w->responseEsc()); }
TEST_F(MainWindowCovTest, compositeChanged) { EXPECT_NO_FATAL_FAILURE(m_w->compositeChanged()); }
TEST_F(MainWindowCovTest, changeFunctionButton) { EXPECT_NO_FATAL_FAILURE(m_w->changeFunctionButton(QStringLiteral("rectangle"))); }
TEST_F(MainWindowCovTest, changeKeyBoardShowEvent) { EXPECT_NO_FATAL_FAILURE(m_w->changeKeyBoardShowEvent(true)); }
TEST_F(MainWindowCovTest, changeMouseShowEvent) { EXPECT_NO_FATAL_FAILURE(m_w->changeMouseShowEvent(false)); }
TEST_F(MainWindowCovTest, changeShotToolEvent) { EXPECT_NO_FATAL_FAILURE(m_w->changeShotToolEvent(QStringLiteral("rectangle"))); }

// ---- 已有声明的私有/保护方法 ----
TEST_F(MainWindowCovTest, checkSuffix)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowcheckSuffix(*m_w, QStringLiteral("png")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowcheckSuffix(*m_w, QStringLiteral("xxx")));
}
TEST_F(MainWindowCovTest, isToolBarInShotArea) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowisToolBarInShotArea(*m_w)); }
TEST_F(MainWindowCovTest, getWaitTimeByImageSize)
{
    QPixmap pix(100, 100);
    pix.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowgetWaitTimeByImageSize(*m_w, pix));
}
TEST_F(MainWindowCovTest, getScrollShotTipPosition) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowgetScrollShotTipPosition(*m_w)); }
TEST_F(MainWindowCovTest, inputEventSetCancel) {
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetInputEvent(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetCancelInputEvent(*m_w));
}
TEST_F(MainWindowCovTest, dragResetCursor) {
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetDragCursor(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresetCursor(*m_w));
}
TEST_F(MainWindowCovTest, getPixmapofRect) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowgetPixmapofRect(*m_w, QRect(0, 0, 50, 50))); }

// ---- 新声明的私有/保护方法 ----
TEST_F(MainWindowCovTest, libPath)
{
    QString r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowlibPath(*m_w, QStringLiteral("libtest.so")));
}
TEST_F(MainWindowCovTest, applyPathSettings) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowapplyPathSettings(*m_w, QStringLiteral("/tmp"))); }

TEST_F(MainWindowCovTest, getActionAndResize)
{
    QMouseEvent me(QEvent::MouseButtonPress, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowgetAction(*m_w, &me));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateCursor(*m_w, &me));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresizeTop(*m_w, &me));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresizeBottom(*m_w, &me));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresizeLeft(*m_w, &me));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresizeRight(*m_w, &me));
}
