// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/utils/delaytime.h"

using namespace testing;

// DelayTime is a QThread that counts down and emits progress/doWork signals.
// In ENABLE_UNIT_TEST mode, forceToExitApp() emits doWork() instead of _Exit(0).

ACCESS_PRIVATE_FIELD(DelayTime, bool, m_stopFlag);
ACCESS_PRIVATE_FIELD(DelayTime, bool, m_isForceToExitApp);
ACCESS_PRIVATE_FIELD(DelayTime, int, m_delayTime);

// Stub QThread::msleep to avoid real delays — uses ADDR on QThread base class
static void delaytime_msleep_stub(void *obj, unsigned long secs)
{
    Q_UNUSED(obj); Q_UNUSED(secs);
}

class DelayTimeTest : public Test
{
public:
    DelayTime *m_dt = nullptr;
    Stub stub;

    void SetUp() override
    {
        stub.set(ADDR(QThread, msleep), delaytime_msleep_stub);
        m_dt = new DelayTime(10); // 10ms countdown (very short for fast test)
    }
    void TearDown() override
    {
        stub.reset(ADDR(QThread, msleep));
        if (m_dt) {
            m_dt->stop();
            delete m_dt;
            m_dt = nullptr;
        }
    }
};

TEST_F(DelayTimeTest, constructorSetsDefaults)
{
    EXPECT_EQ(access_private_field::DelayTimem_delayTime(*m_dt), 10);
    EXPECT_FALSE(access_private_field::DelayTimem_stopFlag(*m_dt));
    EXPECT_FALSE(access_private_field::DelayTimem_isForceToExitApp(*m_dt));
}

TEST_F(DelayTimeTest, setForceToExitApp)
{
    m_dt->setForceToExitApp(true);
    EXPECT_TRUE(access_private_field::DelayTimem_isForceToExitApp(*m_dt));
    m_dt->setForceToExitApp(false);
    EXPECT_FALSE(access_private_field::DelayTimem_isForceToExitApp(*m_dt));
}

TEST_F(DelayTimeTest, stopSetsFlagAndQuits)
{
    EXPECT_NO_FATAL_FAILURE(m_dt->stop());
    EXPECT_TRUE(access_private_field::DelayTimem_stopFlag(*m_dt));
}

TEST_F(DelayTimeTest, runEmitsProgressAndDoWork)
{
    QSignalSpy progressSpy(m_dt, &DelayTime::progress);
    QSignalSpy doWorkSpy(m_dt, &DelayTime::doWork);
    m_dt->start();
    m_dt->wait(2000);
    EXPECT_GT(progressSpy.count(), 0);
    EXPECT_EQ(doWorkSpy.count(), 1);
}

TEST_F(DelayTimeTest, stopBeforeCompletionNoDoWork)
{
    QSignalSpy doWorkSpy(m_dt, &DelayTime::doWork);
    m_dt->start();
    // Immediately stop before run() can finish countdown
    m_dt->stop();
    m_dt->wait(2000);
    // Stopped early — doWork should NOT be emitted
    EXPECT_EQ(doWorkSpy.count(), 0);
}

TEST_F(DelayTimeTest, forceExitAppEmitsDoWorkInsteadOfExit)
{
    // ENABLE_UNIT_TEST mode: forceToExitApp() emits doWork instead of _Exit
    m_dt->setForceToExitApp(true);
    QSignalSpy doWorkSpy(m_dt, &DelayTime::doWork);
    m_dt->start();
    m_dt->wait(2000);
    // Both run() completion and forceToExitApp() emit doWork
    EXPECT_GE(doWorkSpy.count(), 1);
}

TEST_F(DelayTimeTest, constructorWithDifferentMsec)
{
    DelayTime *dt50 = new DelayTime(50);
    EXPECT_EQ(access_private_field::DelayTimem_delayTime(*dt50), 50);
    dt50->stop();
    delete dt50;
}
