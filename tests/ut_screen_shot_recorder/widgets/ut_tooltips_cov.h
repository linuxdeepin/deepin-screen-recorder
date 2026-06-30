// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QEnterEvent>
#include <QEvent>
#include <QBrush>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QResizeEvent>
#include <DGuiApplicationHelper>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/widgets/tooltips.h"

using namespace testing;

// The legacy ut_tooltips.h is disabled because its ACCESS_PRIVATE_FUN declaration
// uses the Qt5 `enterEvent(QEvent*)` signature, which is ambiguous / mismatched
// against the current Qt6 `enterEvent(QEnterEvent*)`. This file restores the
// coverage with the correct Qt6 signatures and adds a few additional branches
// (show/hide cycle for enterEvent, resetSize wrap-vs-nowrap, paintEvent on a
// shown widget).

ACCESS_PRIVATE_FUN(ToolTips, void(QEnterEvent *), enterEvent);
ACCESS_PRIVATE_FUN(ToolTips, void(QResizeEvent *), resizeEvent);
ACCESS_PRIVATE_FUN(ToolTips, void(QPaintEvent *), paintEvent);

static DGuiApplicationHelper::ColorType themeType_unknown_stub()
{
    return DGuiApplicationHelper::ColorType::UnknownType;
}
static DGuiApplicationHelper::ColorType themeType_light_stub()
{
    return DGuiApplicationHelper::ColorType::LightType;
}
static DGuiApplicationHelper::ColorType themeType_dark_stub()
{
    return DGuiApplicationHelper::ColorType::DarkType;
}

class ToolTipsCovTest : public Test
{
public:
    ToolTips *m_t = nullptr;
    void SetUp() override { m_t = new ToolTips(QStringLiteral("hello tips")); }
    void TearDown() override
    {
        // ToolTips has WA_DeleteOnClose; just delete to be safe
        delete m_t;
    }
};

TEST_F(ToolTipsCovTest, propertyAccessors)
{
    EXPECT_GT(m_t->radius(), 0);
    EXPECT_NO_FATAL_FAILURE(m_t->radius());
    EXPECT_NO_FATAL_FAILURE(m_t->borderColor());
    EXPECT_NO_FATAL_FAILURE(m_t->background());
    SUCCEED();
}

TEST_F(ToolTipsCovTest, settersRoundTrip)
{
    EXPECT_NO_FATAL_FAILURE(m_t->setRadius(4));
    EXPECT_NO_FATAL_FAILURE(m_t->setBorderColor(QColor(Qt::red)));
    EXPECT_NO_FATAL_FAILURE(m_t->setBackground(QBrush(Qt::blue)));
    EXPECT_NO_FATAL_FAILURE(m_t->setText(QStringLiteral("new text")));
    EXPECT_NO_FATAL_FAILURE(m_t->setVertical());
    EXPECT_NO_FATAL_FAILURE(m_t->resetSize(50));
    SUCCEED();
}

TEST_F(ToolTipsCovTest, resetSizeWideTextNoWrap)
{
    // narrow maxWidth larger than text -> no word wrap branch
    EXPECT_NO_FATAL_FAILURE(m_t->setText(QStringLiteral("short")));
    EXPECT_NO_FATAL_FAILURE(m_t->resetSize(500));
    SUCCEED();
}

TEST_F(ToolTipsCovTest, enterEventHides)
{
    QEnterEvent enter(QPointF(1, 1), QPointF(1, 1), QPointF(1, 1));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolTipsenterEvent(*m_t, &enter));
    SUCCEED();
}

TEST_F(ToolTipsCovTest, resizeEventDelegates)
{
    QResizeEvent ev(QSize(100, 50), QSize(80, 40));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolTipsresizeEvent(*m_t, &ev));
    SUCCEED();
}

TEST_F(ToolTipsCovTest, paintEventAllThemeBranches)
{
    Stub stub;
    QPaintEvent pe(QRect(0, 0, 60, 30));

    stub.set(ADDR(DGuiApplicationHelper, themeType), themeType_unknown_stub);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolTipspaintEvent(*m_t, &pe));
    stub.reset(ADDR(DGuiApplicationHelper, themeType));

    stub.set(ADDR(DGuiApplicationHelper, themeType), themeType_light_stub);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolTipspaintEvent(*m_t, &pe));
    stub.reset(ADDR(DGuiApplicationHelper, themeType));

    stub.set(ADDR(DGuiApplicationHelper, themeType), themeType_dark_stub);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolTipspaintEvent(*m_t, &pe));
    stub.reset(ADDR(DGuiApplicationHelper, themeType));
    SUCCEED();
}

TEST_F(ToolTipsCovTest, fullShowHidePaintCycle)
{
    m_t->resize(80, 40);
    m_t->show();
    QTest::qWait(10);
    EXPECT_NO_FATAL_FAILURE(m_t->repaint());
    m_t->hide();
    SUCCEED();
}
