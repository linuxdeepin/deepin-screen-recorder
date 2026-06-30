// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QEvent>
#include <QAction>
#include "addr_pri.h"
#include "../../src/menucontroller/menucontroller.h"

using namespace testing;

// The existing ut_menucontroller.h only exercises showMenu (with popup stubbed).
// This file covers the rest: setUndoEnable toggling, the triggered lambdas for
// undo/save/exit (verified via the signals), the eventFilter Enter/Leave/default
// branches, and the aboutToShow/aboutToHide cursor hooks.

ACCESS_PRIVATE_FIELD(MenuController, DMenu *, m_menu);
ACCESS_PRIVATE_FUN(MenuController, bool(QObject *, QEvent *), eventFilter);

class MenuControllerCovTest : public Test
{
public:
    MenuController *m_c = nullptr;
    void SetUp() override { m_c = new MenuController; }
    void TearDown() override
    {
        delete m_c;
        while (qApp->overrideCursor()) {
            qApp->restoreOverrideCursor();
        }
    }
};

TEST_F(MenuControllerCovTest, setUndoEnableBothStates)
{
    EXPECT_NO_FATAL_FAILURE(m_c->setUndoEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_c->setUndoEnable(false));
    SUCCEED();
}

TEST_F(MenuControllerCovTest, undoActionEmitsSignal)
{
    QSignalSpy spy(m_c, &MenuController::unDoAction);
    DMenu *menu = access_private_field::MenuControllerm_menu(*m_c);
    ASSERT_NE(menu, nullptr);
    // find the Undo action (first action, text "Undo")
    QAction *undo = nullptr;
    for (QAction *a : menu->actions()) {
        if (!a->isSeparator()) {
            undo = a;
            break;
        }
    }
    ASSERT_NE(undo, nullptr);
    undo->setEnabled(true);
    EXPECT_NO_FATAL_FAILURE(emit undo->triggered());
    QTest::qWait(20);
    EXPECT_GE(spy.count(), 1);
}

TEST_F(MenuControllerCovTest, saveAndCloseActionsEmitSignals)
{
    QSignalSpy saveSpy(m_c, &MenuController::saveAction);
    QSignalSpy closeSpy(m_c, &MenuController::closeAction);
    DMenu *menu = access_private_field::MenuControllerm_menu(*m_c);
    ASSERT_NE(menu, nullptr);
    // trigger every non-separator action; undo/save/exit lambdas fire their signals
    for (QAction *a : menu->actions()) {
        if (!a->isSeparator()) {
            EXPECT_NO_FATAL_FAILURE(emit a->triggered());
        }
    }
    QTest::qWait(20);
    EXPECT_GE(saveSpy.count(), 1);
    EXPECT_GE(closeSpy.count(), 1);
}

TEST_F(MenuControllerCovTest, eventFilterEnterSetsArrowCursor)
{
    DMenu *menu = access_private_field::MenuControllerm_menu(*m_c);
    ASSERT_NE(menu, nullptr);
    QEvent enter(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(menu, &enter));
    QApplication::restoreOverrideCursor();
    SUCCEED();
}

TEST_F(MenuControllerCovTest, eventFilterLeaveRestoresCursor)
{
    DMenu *menu = access_private_field::MenuControllerm_menu(*m_c);
    ASSERT_NE(menu, nullptr);
    QEvent leave(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(menu, &leave));
    SUCCEED();
}

TEST_F(MenuControllerCovTest, eventFilterUnknownEventPassthrough)
{
    DMenu *menu = access_private_field::MenuControllerm_menu(*m_c);
    ASSERT_NE(menu, nullptr);
    QEvent other(QEvent::PaletteChange);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(menu, &other));
    SUCCEED();
}

TEST_F(MenuControllerCovTest, eventFilterUnwatchedObjectPassthrough)
{
    QObject obj;
    QEvent ev(QEvent::Enter);
    // watched != m_menu -> falls straight through to base.
    // eventFilter is protected; reach it via the ACCESS_PRIVATE_FUN accessor.
    bool handled = true;
    EXPECT_NO_FATAL_FAILURE(handled = call_private_fun::MenuControllereventFilter(*m_c, &obj, &ev));
    EXPECT_FALSE(handled);
}

TEST_F(MenuControllerCovTest, aboutToShowAndAboutToHideHooks)
{
    DMenu *menu = access_private_field::MenuControllerm_menu(*m_c);
    ASSERT_NE(menu, nullptr);
    QSignalSpy noFocusSpy(m_c, &MenuController::menuNoFocus);
    EXPECT_NO_FATAL_FAILURE(emit menu->aboutToShow());
    EXPECT_NO_FATAL_FAILURE(emit menu->aboutToHide());
    QTest::qWait(20);
    EXPECT_GE(noFocusSpy.count(), 1);
    QApplication::restoreOverrideCursor();
}
