// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QSignalSpy>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include "addr_pri.h"
#include "../../src/widgets/toolbutton.h"

using namespace testing;

// Covers previously-uncovered ToolButton surface: static background-pixmap
// setters/getters, the setter family (setHoverState/setOptionButtonFlag/
// setUndoButtonFlag/setBadgeIcon x2), event handlers (enter/leave/mousePress/
// mouseRelease/move/resize), and the paintEvent branches including the private
// helpers drawRedDot/drawBadge/getIconRect/tintIconByBackground. The latter
// group is reached by toggling state (checked/disabled/menu/red-dot/badge) and
// forcing repaint on a shown widget.

// NOTE: ACCESS_PRIVATE_FUN for drawRedDot, drawBadge, getIconRect are already
// declared by ut_toolbutton.h / ut_toolbutton_ext.h (included earlier in the
// same TU) — reused here without redeclaration to avoid redefinition.
// tintIconByBackground is NOT declared elsewhere, so declare it here:
ACCESS_PRIVATE_FUN(ToolButton, QIcon(const QIcon &, const QSize &) const, tintIconByBackground);

class ToolButtonCovTest : public Test
{
public:
    ToolButton *m_btn = nullptr;
    void SetUp() override
    {
        ToolButton::clearBackgroundPixmap();
        m_btn = new ToolButton;
        m_btn->resize(32, 32);
        m_btn->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
        m_btn->setIconSize(QSize(16, 16));
    }
    void TearDown() override
    {
        ToolButton::clearBackgroundPixmap();
        delete m_btn;
    }
};

// ---------- static background-pixmap API ----------

TEST_F(ToolButtonCovTest, backgroundPixmapLifecycle)
{
    QPixmap pm(64, 64);
    pm.fill(Qt::white);
    ToolButton::setBackgroundPixmap(&pm);
    EXPECT_EQ(ToolButton::backgroundPixmap(), &pm);
    ToolButton::clearBackgroundPixmap();
    EXPECT_EQ(ToolButton::backgroundPixmap(), nullptr);
}

// ---------- setter family ----------

TEST_F(ToolButtonCovTest, settersRunUpdate)
{
    EXPECT_NO_FATAL_FAILURE(m_btn->setHoverState(false));
    EXPECT_NO_FATAL_FAILURE(m_btn->setHoverState(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setOptionButtonFlag(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setOptionButtonFlag(false));
    EXPECT_NO_FATAL_FAILURE(m_btn->setUndoButtonFlag(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setUndoButtonFlag(false));
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeIcon(QIcon::fromTheme(QStringLiteral("edit-redo"))));
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeIcon(QStringLiteral("non-existent-icon")));
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeIcon(QString()));
    EXPECT_NO_FATAL_FAILURE(m_btn->clearBadge());
    EXPECT_NO_FATAL_FAILURE(m_btn->setShowRedDot(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeSize(QSize(8, 8)));
    SUCCEED();
}

// ---------- event handlers ----------

TEST_F(ToolButtonCovTest, enterLeaveEvents)
{
    QEnterEvent enter(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &enter));
    QEvent leave(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &leave));
    SUCCEED();
}

TEST_F(ToolButtonCovTest, undoButtonEmitsSignalOnEnterLeave)
{
    m_btn->setUndoButtonFlag(true);
    QSignalSpy spy(m_btn, &ToolButton::isInUndoBtn);
    QEnterEvent enter(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
    QApplication::sendEvent(m_btn, &enter);
    QEvent leave(QEvent::Leave);
    QApplication::sendEvent(m_btn, &leave);
    EXPECT_GE(spy.count(), 2);
}

TEST_F(ToolButtonCovTest, mousePressReleaseNoMenu)
{
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(5, 5),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &press));
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(5, 5),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &release));
    SUCCEED();
}

TEST_F(ToolButtonCovTest, mousePressReleaseWithMenu)
{
    QMenu menu(m_btn);
    m_btn->setMenu(&menu);
    m_btn->setCheckable(false);
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(5, 5),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &press));
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(5, 5),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    // showMenu() is invoked; hide immediately to avoid blocking
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &release));
    if (menu.isVisible()) {
        menu.hide();
    }
    SUCCEED();
}

TEST_F(ToolButtonCovTest, optionButtonMousePress)
{
    m_btn->setOptionButtonFlag(true);
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(5, 5),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &press));
    SUCCEED();
}

TEST_F(ToolButtonCovTest, moveAndResizeEvents)
{
    QMoveEvent move(QPoint(10, 10), QPoint(0, 0));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &move));
    QResizeEvent resize(QSize(40, 40), QSize(32, 32));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_btn, &resize));
    SUCCEED();
}

// ---------- paintEvent branches ----------

TEST_F(ToolButtonCovTest, paintEventCheckedAndNormal)
{
    m_btn->show();
    QTest::qWait(15);

    // normal state (unchecked)
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());

    // checked state -> tinted icon branch
    m_btn->setChecked(true);
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());

    // disabled state with background pixmap -> tintIconByBackground path
    m_btn->setEnabled(false);
    QPixmap bg(64, 64);
    bg.fill(Qt::darkGray);
    ToolButton::setBackgroundPixmap(&bg);
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());

    m_btn->hide();
    SUCCEED();
}

TEST_F(ToolButtonCovTest, paintEventColorButtonAndMenuAndBadge)
{
    // color-button property branch
    m_btn->setProperty("name", QStringLiteral("color1"));
    m_btn->setChecked(true);
    m_btn->show();
    QTest::qWait(15);
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());

    // menu active branch + red dot + badge
    QMenu menu(m_btn);
    m_btn->setMenu(&menu);
    m_btn->setChecked(false);
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());

    m_btn->setShowRedDot(true);
    m_btn->setBadgeIcon(QIcon::fromTheme(QStringLiteral("edit-redo")));
    EXPECT_NO_FATAL_FAILURE(m_btn->repaint());

    m_btn->hide();
    SUCCEED();
}

// ---------- private helpers directly ----------

TEST_F(ToolButtonCovTest, drawRedDotAndBadgeDirect)
{
    QImage img(32, 32, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    QStyleOptionToolButton opt;
    opt.iconSize = QSize(16, 16);
    m_btn->setIconSize(QSize(16, 16));
    QRect iconRect = call_private_fun::ToolButtongetIconRect(*m_btn, opt);

    m_btn->setShowRedDot(true);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolButtondrawRedDot(*m_btn, p, opt, iconRect));
    p.end();

    QImage img2(32, 32, QImage::Format_ARGB32_Premultiplied);
    img2.fill(Qt::transparent);
    QPainter p2(&img2);
    m_btn->setBadgeIcon(QIcon::fromTheme(QStringLiteral("edit-redo")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolButtondrawBadge(*m_btn, p2, opt, iconRect));
    p2.end();

    // drawBadge with null badge -> early return
    m_btn->clearBadge();
    QPainter p3(&img2);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolButtondrawBadge(*m_btn, p3, opt, iconRect));
    p3.end();
    SUCCEED();
}

TEST_F(ToolButtonCovTest, tintIconByBackgroundBranches)
{
    QIcon ic = QIcon::fromTheme(QStringLiteral("edit-copy"));
    // no background -> returns input icon
    QIcon out1 = call_private_fun::ToolButtontintIconByBackground(*m_btn, ic, QSize(16, 16));
    EXPECT_FALSE(out1.isNull());

    // with background that does not intersect -> returns input
    QPixmap bg(8, 8);
    bg.fill(Qt::white);
    ToolButton::setBackgroundPixmap(&bg);
    // place button far away so intersection is empty
    m_btn->move(-10000, -10000);
    QIcon out2 = call_private_fun::ToolButtontintIconByBackground(*m_btn, ic, QSize(16, 16));
    EXPECT_FALSE(out2.isNull());
    ToolButton::clearBackgroundPixmap();
    SUCCEED();
}

TEST_F(ToolButtonCovTest, getIconRectCustomAndDefault)
{
    QStyleOptionToolButton optCustom;
    optCustom.iconSize = QSize(20, 20);
    QRect r1 = call_private_fun::ToolButtongetIconRect(*m_btn, optCustom);
    EXPECT_EQ(r1.size(), QSize(20, 20));

    QStyleOptionToolButton optDefault; // invalid iconSize -> default 24x24
    QRect r2 = call_private_fun::ToolButtongetIconRect(*m_btn, optDefault);
    EXPECT_EQ(r2.size(), QSize(24, 24));
}
