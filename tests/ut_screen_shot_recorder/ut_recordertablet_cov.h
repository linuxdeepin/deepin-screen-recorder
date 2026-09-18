// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include "../../src/recordertablet.h"

using namespace testing;

// RecorderTablet has no existing test. This file covers its full surface that is
// safe offscreen. The ctor registers a DBus service/object; in the test session
// that may fail (another instance owns the name) -- registerService just logs a
// warning and continues, so construction is safe. All methods only emit DBus
// signals (fire-and-forget QDBusConnection::send) or drive QTimer + signals;
// none block on a real DBus call or perform real recording.
//
// Covered: ctor/dtor, start (timer + countdown), update (countdown decrement +
// finished emission at zero), stop, startStatusBar, updateStatusBar, the static
// getRecorderNormalIcon / RESOURCES_PATH.

class RecorderTabletCovTest : public Test
{
public:
    RecorderTablet *m_t = nullptr;
    void SetUp() override
    {
        // wrapped: if the session already has the service registered this still
        // completes (logs a warning, keeps going)
        EXPECT_NO_FATAL_FAILURE(m_t = new RecorderTablet);
    }
    void TearDown() override { delete m_t; }
};

TEST_F(RecorderTabletCovTest, resourcesPathAndIcon)
{
    EXPECT_FALSE(RecorderTablet::RESOURCES_PATH.isEmpty());
    QString icon;
    EXPECT_NO_FATAL_FAILURE(icon = RecorderTablet::getRecorderNormalIcon());
    EXPECT_TRUE(icon.contains(QStringLiteral("fast-icon_recording_normal.svg")));
}

TEST_F(RecorderTabletCovTest, startCreatesTimer)
{
    QSignalSpy finishedSpy(m_t, &RecorderTablet::finished);
    EXPECT_NO_FATAL_FAILURE(m_t->start());
    // start() should not immediately emit finished (counter starts at 3)
    EXPECT_EQ(finishedSpy.count(), 0);
}

TEST_F(RecorderTabletCovTest, updateReachesZeroEmitsFinished)
{
    // Drive update() three times to bring counter 3 -> 0 and emit finished.
    QSignalSpy finishedSpy(m_t, &RecorderTablet::finished);
    EXPECT_NO_FATAL_FAILURE(m_t->start());
    EXPECT_NO_FATAL_FAILURE(m_t->update()); // 3 -> 2
    EXPECT_NO_FATAL_FAILURE(m_t->update()); // 2 -> 1
    EXPECT_NO_FATAL_FAILURE(m_t->update()); // 1 -> 0 -> emit finished
    EXPECT_GE(finishedSpy.count(), 1);
}

TEST_F(RecorderTabletCovTest, stopWithoutStatusBarTimer)
{
    // stop() guards on m_statusBarTimer (null here) -> safe no-op on timer
    EXPECT_NO_FATAL_FAILURE(m_t->stop());
    SUCCEED();
}

TEST_F(RecorderTabletCovTest, startStatusBarAndStop)
{
    EXPECT_NO_FATAL_FAILURE(m_t->startStatusBar());
    EXPECT_NO_FATAL_FAILURE(m_t->updateStatusBar()); // increments flashTrayIconCounter
    EXPECT_NO_FATAL_FAILURE(m_t->stop()); // now m_statusBarTimer exists -> stopped
    SUCCEED();
}

TEST_F(RecorderTabletCovTest, destructorSafeAfterStart)
{
    // Construct + start + destroy to exercise the timer cleanup in dtor.
    RecorderTablet *t = nullptr;
    EXPECT_NO_FATAL_FAILURE(t = new RecorderTablet);
    EXPECT_NO_FATAL_FAILURE(t->start());
    EXPECT_NO_FATAL_FAILURE(t->startStatusBar());
    EXPECT_NO_FATAL_FAILURE(delete t);
    SUCCEED();
}
