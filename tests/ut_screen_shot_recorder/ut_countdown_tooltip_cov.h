// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include <QPainter>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/countdown_tooltip.h"
#include "../../src/utils.h"

using namespace testing;

// Covers the remainder of CountdownTooltip not exercised by ut_countdown_tooltip.h:
//   - startAtOnce (emits finished immediately)
//   - start() second-call branch (timer already exists -> skips creation)
//   - paintEvent for counters 2 and 3 (existing test only runs counter 1)
//   - paintEvent with counter <= 0 (early-return no-draw branch)
//   - tablet-environment text branch in the constructor

// NOTE: ACCESS_PRIVATE_FUN(paintEvent), ACCESS_PRIVATE_FIELD(showCountdownCounter),
// and ACCESS_PRIVATE_FIELD(showCountdownTimer) are already declared by
// ut_countdown_tooltip.h (included earlier in the same TU). Reused here without
// redeclaration to avoid redefinition errors.

static QPixmap nullFullscreenPixmap_stub()
{
    return QPixmap(); // null -> the blur-pixmap branches are skipped
}

class CountdownTooltipCovTest : public Test
{
public:
    CountdownTooltip *m_t = nullptr;
    Stub stub;
    void SetUp() override
    {
        stub.set(ADDR(TempFile, getFullscreenPixmap), nullFullscreenPixmap_stub);
        m_t = new CountdownTooltip;
        m_t->resize(200, 200);
    }
    void TearDown() override
    {
        stub.reset(ADDR(TempFile, getFullscreenPixmap));
        delete m_t;
    }
};

TEST_F(CountdownTooltipCovTest, startAtOnceEmitsFinished)
{
    QSignalSpy spy(m_t, &CountdownTooltip::finished);
    EXPECT_NO_FATAL_FAILURE(m_t->startAtOnce());
    EXPECT_GE(spy.count(), 1);
}

TEST_F(CountdownTooltipCovTest, startCreatesTimer)
{
    EXPECT_NO_FATAL_FAILURE(m_t->start());
    // second call: timer already exists branch
    EXPECT_NO_FATAL_FAILURE(m_t->start());
    SUCCEED();
}

TEST_F(CountdownTooltipCovTest, updateCountdownReachesZeroEmitsFinished)
{
    QSignalSpy spy(m_t, &CountdownTooltip::finished);
    access_private_field::CountdownTooltipshowCountdownCounter(*m_t) = 1;
    access_private_field::CountdownTooltipshowCountdownTimer(*m_t) = new QTimer(m_t);
    EXPECT_NO_FATAL_FAILURE(m_t->update());
    QTest::qWait(10);
    EXPECT_GE(spy.count(), 1);
}

TEST_F(CountdownTooltipCovTest, paintEventCounterTwo)
{
    access_private_field::CountdownTooltipshowCountdownCounter(*m_t) = 2;
    QPaintEvent pe(QRect(0, 0, 50, 50));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::CountdownTooltippaintEvent(*m_t, &pe));
}

TEST_F(CountdownTooltipCovTest, paintEventCounterThree)
{
    access_private_field::CountdownTooltipshowCountdownCounter(*m_t) = 3;
    QPaintEvent pe(QRect(0, 0, 50, 50));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::CountdownTooltippaintEvent(*m_t, &pe));
}

TEST_F(CountdownTooltipCovTest, paintEventCounterZeroNoDraw)
{
    access_private_field::CountdownTooltipshowCountdownCounter(*m_t) = 0;
    QPaintEvent pe(QRect(0, 0, 50, 50));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::CountdownTooltippaintEvent(*m_t, &pe));
    SUCCEED();
}

TEST_F(CountdownTooltipCovTest, constructInTabletEnvironment)
{
    bool saved = Utils::isTabletEnvironment;
    Utils::isTabletEnvironment = true;
    CountdownTooltip *t = nullptr;
    EXPECT_NO_FATAL_FAILURE(t = new CountdownTooltip);
    delete t;
    Utils::isTabletEnvironment = saved;
    SUCCEED();
}

TEST_F(CountdownTooltipCovTest, constructLightTheme)
{
    int saved = Utils::themeType;
    Utils::themeType = 1;
    CountdownTooltip *t = nullptr;
    EXPECT_NO_FATAL_FAILURE(t = new CountdownTooltip);
    delete t;
    Utils::themeType = saved;
    SUCCEED();
}
