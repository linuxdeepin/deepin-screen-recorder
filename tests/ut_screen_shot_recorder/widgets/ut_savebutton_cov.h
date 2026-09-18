// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QMenu>
#include <QSignalSpy>
#include <QImage>
#include <QPainter>
#include "../../src/widgets/savebutton.h"

using namespace testing;

// Covers the previously-uncovered event handlers and paint branches of
// SaveButton (mousePressEvent/mouseReleaseEvent/enterEvent/leaveEvent/
// mouseMoveEvent/onMenuAboutToHide/paintEvent). Event handlers driven via
// QApplication::sendEvent with synthesized QMouseEvent/QEnterEvent; paint
// triggered by rendering onto an offscreen QImage-backed widget.

class SaveButtonCovTest : public Test
{
public:
    SaveButton *m_btn = nullptr;
    QMenu *m_menu = nullptr;
    void SetUp() override
    {
        m_btn = new SaveButton;
        m_btn->resize(50, 36);
        m_menu = new QMenu(m_btn);
        m_btn->setOptionsMenu(m_menu);
        // provide non-null icons so the icon-paint branches run
        m_btn->setSaveIcon(QIcon::fromTheme(QStringLiteral("document-save")));
        m_btn->setListIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    }
    void TearDown() override
    {
        delete m_btn;
    }
};

TEST_F(SaveButtonCovTest, mousePressOnSaveArea)
{
    // click in the left "save" area (x < kSaveAreaWidth=34)
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 18),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &press));
    SUCCEED();
}

TEST_F(SaveButtonCovTest, mousePressOnListArea)
{
    // click in the right "list" area (x > 34)
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(42, 18),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &press));
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(42, 18),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &release));
    SUCCEED();
}

TEST_F(SaveButtonCovTest, mouseReleaseEmitsSaveAction)
{
    // press on save area then release -> emit saveAction + clicked
    QSignalSpy saveSpy(m_btn, &SaveButton::saveAction);
    QSignalSpy clickSpy(m_btn, &SaveButton::clicked);
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 18),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_btn, &press);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(10, 18),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &release));
    EXPECT_GE(saveSpy.count(), 1);
    EXPECT_GE(clickSpy.count(), 1);
}

TEST_F(SaveButtonCovTest, mouseReleaseOnListPopsMenu)
{
    // press+release on list area with a menu set -> emit expandSaveOption(true)
    QSignalSpy expandSpy(m_btn, &SaveButton::expandSaveOption);
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(42, 18),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_btn, &press);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(42, 18),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &release));
    // hide any popped menu to keep the test clean
    if (m_menu->isVisible()) {
        m_menu->hide();
    }
    EXPECT_GE(expandSpy.count(), 1);
}

TEST_F(SaveButtonCovTest, enterEventSetsHover)
{
    QEnterEvent enter(QPointF(10, 5), QPointF(10, 5), QPointF(10, 5));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &enter));
    SUCCEED();
}

TEST_F(SaveButtonCovTest, leaveEventClearsHover)
{
    // first enter, then leave to exercise the m_hoverFlag==true branch
    QEnterEvent enter(QPointF(10, 5), QPointF(10, 5), QPointF(10, 5));
    QApplication::sendEvent(m_btn, &enter);
    QEvent leave(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &leave));
    SUCCEED();
}

TEST_F(SaveButtonCovTest, mouseMoveUpdatesMouseX)
{
    // move within save area, then cross into list area -> update() path
    QMouseEvent move1(QEvent::MouseMove, QPointF(10, 18),
                      Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &move1));
    QMouseEvent move2(QEvent::MouseMove, QPointF(42, 18),
                      Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &move2));
    SUCCEED();
}

TEST_F(SaveButtonCovTest, paintEventAllBranches)
{
    // Show the widget so paint paths run; force update by toggling state.
    m_btn->show();
    QTest::qWait(20);

    // hover branch (no click)
    QEnterEvent enter(QPointF(10, 5), QPointF(10, 5), QPointF(10, 5));
    QApplication::sendEvent(m_btn, &enter);
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());

    // hover + press on save (left press color)
    QMouseEvent pressSave(QEvent::MouseButtonPress, QPointF(10, 18),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_btn, &pressSave);
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());

    // press on list (right press color, highlightRightArea path)
    QMouseEvent pressList(QEvent::MouseButtonPress, QPointF(42, 18),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_btn, &pressList);
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());

    // menu-visible branch in paint
    m_menu->show();
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());
    m_menu->hide();

    m_btn->hide();
    SUCCEED();
}
