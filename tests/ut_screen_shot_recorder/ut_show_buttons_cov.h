// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include "../../src/show_buttons.h"
#include "../../src/keydefine.h"

using namespace testing;

// Covers the ShowButtons branches the existing ut_show_buttons.h misses:
//   - showContentButtons(int) / releaseContentButtons(int) Wayland overloads
//     (existing only tests the unsigned-char X11 overloads).
//   - showContentButtons(int) with an invalid key -> empty-keycode branch.
//   - releaseContentButtons(unsigned char) for a key that was never pressed
//     (not-in-vector branch).
//   - getKeyCodeFromEvent / getKeyCodeFromEventWayland for an unknown code.

class ShowButtonsCovTest : public Test
{
public:
    ShowButtons m_sb;
};

TEST_F(ShowButtonsCovTest, waylandShowAndReleaseRoundTrip)
{
    QSignalSpy spy(&m_sb, &ShowButtons::keyShowSignal);
    EXPECT_NO_FATAL_FAILURE(m_sb.showContentButtons(static_cast<int>(KEY_A)));
    EXPECT_NO_FATAL_FAILURE(m_sb.showContentButtons(static_cast<int>(KEY_A))); // duplicate -> skip
    EXPECT_NO_FATAL_FAILURE(m_sb.releaseContentButtons(static_cast<int>(KEY_A)));
    EXPECT_NO_FATAL_FAILURE(m_sb.releaseContentButtons(static_cast<int>(KEY_A))); // not-in-vector branch
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ShowButtonsCovTest, waylandShowInvalidKey)
{
    EXPECT_NO_FATAL_FAILURE(m_sb.showContentButtons(999999)); // -> empty keycode branch
    EXPECT_NO_FATAL_FAILURE(m_sb.releaseContentButtons(999999));
    SUCCEED();
}

TEST_F(ShowButtonsCovTest, x11ReleaseKeyNotPressed)
{
    // releasing a key never pressed -> not-in-vector branch
    EXPECT_NO_FATAL_FAILURE(m_sb.releaseContentButtons(static_cast<unsigned char>(KEY_B)));
    SUCCEED();
}

TEST_F(ShowButtonsCovTest, x11ShowInvalidKeyCode)
{
    EXPECT_NO_FATAL_FAILURE(m_sb.showContentButtons(static_cast<unsigned char>(0xFE))); // unlikely mapped
    EXPECT_NO_FATAL_FAILURE(m_sb.releaseContentButtons(static_cast<unsigned char>(0xFE)));
    SUCCEED();
}

TEST_F(ShowButtonsCovTest, getKeyCodeUnknownReturnsEmpty)
{
    QString r1, r2;
    EXPECT_NO_FATAL_FAILURE(r1 = m_sb.getKeyCodeFromEvent(static_cast<unsigned char>(0xFD)));
    EXPECT_TRUE(r1.isEmpty());
    EXPECT_NO_FATAL_FAILURE(r2 = m_sb.getKeyCodeFromEventWayland(888888));
    EXPECT_TRUE(r2.isEmpty());
}

TEST_F(ShowButtonsCovTest, getKeyCodeKnownEntries)
{
    // spot-check a few mappings beyond what the existing tests cover
    EXPECT_EQ(m_sb.getKeyCodeFromEvent(KEY_SPACE), QStringLiteral("SPACE"));
    EXPECT_EQ(m_sb.getKeyCodeFromEventWayland(KEY_ENTER), QStringLiteral("ENTER"));
    EXPECT_EQ(m_sb.getKeyCodeFromEventWayland(KEY_TAB), QStringLiteral("TAB"));
}
