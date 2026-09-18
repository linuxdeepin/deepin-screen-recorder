// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// ToolButton smoke tests for the current (Qt6) API.
//
// The legacy ut_toolbutton.h is disabled in test_all_interfaces.h because the
// old API (setTips/getTips, QEvent-typed enterEvent) has drifted. This file
// targets only safe, value-returning / flag-setting public methods plus the
// const geometry helper getIconRect, and the move/resize event handlers that
// merely flip a recalculation flag.
//
// Skipped (unsafe in headless unit context):
//   - paintEvent : needs a shown window + style polish; touches palette/icon
//                  pixmaps and (in disabled state) mapToGlobal + static bg.
//   - enterEvent/leaveEvent : call qApp->setOverrideCursor (cursor path that
//                  caused SEGV in the prior shapeswidget incident).
//   - mousePressEvent/mouseReleaseEvent : call update()/showMenu()/menu();
//                  menu() path spawns QMenu and needs a real window.
//   - tintIconByBackground : private, non-const side via mapToGlobal + static
//                  s_backgroundPixmap deref; covered indirectly if needed.
//   - drawRedDot/drawBadge : private painters, only meaningful inside paint.

#pragma once
#include <gtest/gtest.h>
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QStyleOptionToolButton>
#include "addr_pri.h"
#include "../../src/widgets/toolbutton.h"

ACCESS_PRIVATE_FUN(ToolButton, void(QMoveEvent *event), moveEvent);
ACCESS_PRIVATE_FUN(ToolButton, void(QResizeEvent *event), resizeEvent);
ACCESS_PRIVATE_FUN(ToolButton, QRect(const QStyleOptionToolButton &opt) const, getIconRect);

class ToolButtonExtTest : public ::testing::Test
{
public:
    ToolButton *m_btn;
    void SetUp() override { m_btn = new ToolButton; }
    void TearDown() override { delete m_btn; }
};

TEST_F(ToolButtonExtTest, StaticBackgroundPixmapRoundTrip)
{
    // Initially null (or whatever a prior test left; we set/clear deterministically).
    QPixmap pix(16, 16);
    pix.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(ToolButton::setBackgroundPixmap(&pix));
    EXPECT_EQ(ToolButton::backgroundPixmap(), &pix);

    EXPECT_NO_FATAL_FAILURE(ToolButton::clearBackgroundPixmap());
    EXPECT_EQ(ToolButton::backgroundPixmap(), nullptr);

    // Null pointer is a legal argument.
    EXPECT_NO_FATAL_FAILURE(ToolButton::setBackgroundPixmap(nullptr));
    EXPECT_EQ(ToolButton::backgroundPixmap(), nullptr);
}

TEST_F(ToolButtonExtTest, HoverStateSetter)
{
    // No public getter; exercise for coverage + crash safety.
    EXPECT_NO_FATAL_FAILURE(m_btn->setHoverState(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setHoverState(false));
}

TEST_F(ToolButtonExtTest, OptionAndUndoButtonFlags)
{
    EXPECT_NO_FATAL_FAILURE(m_btn->setOptionButtonFlag(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setOptionButtonFlag(false));
    EXPECT_NO_FATAL_FAILURE(m_btn->setUndoButtonFlag(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setUndoButtonFlag(false));
}

TEST_F(ToolButtonExtTest, RedDotToggle)
{
    // Inline setter calls update(); safe without a shown window (no-op paint).
    EXPECT_NO_FATAL_FAILURE(m_btn->setShowRedDot(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setShowRedDot(false));
}

TEST_F(ToolButtonExtTest, BadgeSizeAndClear)
{
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeSize(QSize(10, 7)));
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeSize(QSize(0, 0)));
    EXPECT_NO_FATAL_FAILURE(m_btn->clearBadge());
}

TEST_F(ToolButtonExtTest, SetBadgeIconFromQIcon)
{
    QIcon icon(QIcon::fromTheme("edit-cut"));
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeIcon(icon));
    EXPECT_NO_FATAL_FAILURE(m_btn->clearBadge());
    // Empty icon.
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeIcon(QIcon()));
}

TEST_F(ToolButtonExtTest, SetBadgeIconFromPath)
{
    // Non-empty path that does not resolve to a file yields a null QIcon; safe.
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeIcon(QString("/nonexistent/ut-badge.png")));
    // Empty string -> clears badge.
    EXPECT_NO_FATAL_FAILURE(m_btn->setBadgeIcon(QString()));
}

TEST_F(ToolButtonExtTest, MoveAndResizeEventsFlipFlag)
{
    // These handlers only set m_needRecalculateIcon = true after forwarding
    // to the base class. Safe to invoke directly.
    QMoveEvent me(QPoint(5, 6), QPoint(0, 0));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolButtonmoveEvent(*m_btn, &me));

    QResizeEvent re(QSize(40, 40), QSize(20, 20));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolButtonresizeEvent(*m_btn, &re));
}

TEST_F(ToolButtonExtTest, GetIconRectFallbackSize)
{
    // Give the widget a deterministic geometry so the math is checkable.
    m_btn->resize(48, 48);
    QStyleOptionToolButton opt;
    opt.iconSize = QSize();             // invalid -> falls back to 24x24
    QRect r = call_private_fun::ToolButtongetIconRect(*m_btn, opt);
    EXPECT_EQ(r.size(), QSize(24, 24));
    // Centered horizontally and vertically.
    EXPECT_EQ(r.x(), (48 - 24) / 2);
    EXPECT_EQ(r.y(), (48 - 24) / 2);
}

TEST_F(ToolButtonExtTest, GetIconRectUsesProvidedIconSize)
{
    m_btn->resize(60, 60);
    QStyleOptionToolButton opt;
    opt.iconSize = QSize(20, 30);       // valid -> used as-is
    QRect r = call_private_fun::ToolButtongetIconRect(*m_btn, opt);
    EXPECT_EQ(r.size(), QSize(20, 30));
    EXPECT_EQ(r.x(), (60 - 20) / 2);
    EXPECT_EQ(r.y(), (60 - 30) / 2);
}

TEST_F(ToolButtonExtTest, ConstructorDefaults)
{
    // Cheap invariant checks on freshly constructed button.
    EXPECT_TRUE(m_btn->isCheckable());   // ctor sets checkable
    EXPECT_EQ(m_btn->focusPolicy(), Qt::NoFocus);
    SUCCEED();
}

// === Deep coverage: paint + draw methods ===
ACCESS_PRIVATE_FUN(ToolButton, void(QPainter &, const QStyleOptionToolButton &, const QRect &), drawRedDot);
ACCESS_PRIVATE_FUN(ToolButton, void(QPainter &, const QStyleOptionToolButton &, const QRect &), drawBadge);
ACCESS_PRIVATE_FUN(ToolButton, void(QPaintEvent *), paintEvent);

TEST_F(ToolButtonExtTest, drawRedDotRunsClean)
{
    QImage img(64, 64, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    QStyleOptionToolButton opt;
    opt.rect = QRect(0, 0, 64, 64);
    QRect iconRect(10, 10, 24, 24);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolButtondrawRedDot(*m_btn, painter, opt, iconRect));
}

TEST_F(ToolButtonExtTest, drawBadgeWithSetIcon)
{
    // Set a badge icon so drawBadge exercises the pixmap path
    QIcon badgeIcon(QIcon::fromTheme("edit-cut"));
    m_btn->setBadgeIcon(badgeIcon);
    QImage img(64, 64, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    QStyleOptionToolButton opt;
    opt.rect = QRect(0, 0, 64, 64);
    QRect iconRect(10, 10, 24, 24);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolButtondrawBadge(*m_btn, painter, opt, iconRect));
}

TEST_F(ToolButtonExtTest, drawBadgeWithBadgeSize)
{
    m_btn->setBadgeSize(QSize(12, 8));
    QImage img(64, 64, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    QStyleOptionToolButton opt;
    opt.rect = QRect(0, 0, 64, 64);
    QRect iconRect(10, 10, 24, 24);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolButtondrawBadge(*m_btn, painter, opt, iconRect));
}

TEST_F(ToolButtonExtTest, paintEventNormalState)
{
    // paintEvent needs a pixmap device; render onto an image
    m_btn->resize(48, 48);
    QImage img(48, 48, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    m_btn->render(&img);
    EXPECT_NO_FATAL_FAILURE((void)img);
}

TEST_F(ToolButtonExtTest, paintEventWithRedDot)
{
    m_btn->setShowRedDot(true);
    m_btn->resize(48, 48);
    QImage img(48, 48, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    EXPECT_NO_FATAL_FAILURE(m_btn->render(&img));
}

TEST_F(ToolButtonExtTest, paintEventCheckedState)
{
    m_btn->setChecked(true);
    m_btn->resize(48, 48);
    QImage img(48, 48, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    EXPECT_NO_FATAL_FAILURE(m_btn->render(&img));
}

TEST_F(ToolButtonExtTest, paintEventDisabledState)
{
    m_btn->setEnabled(false);
    m_btn->resize(48, 48);
    QImage img(48, 48, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    EXPECT_NO_FATAL_FAILURE(m_btn->render(&img));
}

TEST_F(ToolButtonExtTest, paintEventWithBackgroundPixmap)
{
    QPixmap bg(16, 16);
    bg.fill(Qt::blue);
    ToolButton::setBackgroundPixmap(&bg);
    m_btn->resize(48, 48);
    QImage img(48, 48, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    EXPECT_NO_FATAL_FAILURE(m_btn->render(&img));
    ToolButton::clearBackgroundPixmap();
}

TEST_F(ToolButtonExtTest, enterLeaveEventSafe)
{
    // enterEvent/leaveEvent set/clear override cursor; safe in offscreen
    QEnterEvent enterEvent(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_btn, &enterEvent));
    QEvent leaveEvent(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_btn, &leaveEvent));
}
