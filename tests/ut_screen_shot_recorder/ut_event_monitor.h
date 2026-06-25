// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "../../src/event_monitor.h"

using namespace testing;

// EventMonitor 的 X11 run()/handleEvent/getCursorImage 涉及真实 XRecord，风险高不测；
// 仅覆盖构造 + wayland DBus 槽 + getCursorImageWayland（DBus 已被全局 stub）。
class EventMonitorTest : public Test
{
public:
    EventMonitor *m_mon;
    void SetUp() override { m_mon = new EventMonitor; }
    void TearDown() override { delete m_mon; }
};

TEST_F(EventMonitorTest, construct)
{
    SUCCEED(); // 构造在 SetUp 完成，isWaylandMode=false 跳过 wayland 初始化
}

TEST_F(EventMonitorTest, waylandSlots)
{
    QSignalSpy pressSpy(m_mon, &EventMonitor::mousePress);
    QSignalSpy releaseSpy(m_mon, &EventMonitor::mouseRelease);
    QSignalSpy moveSpy(m_mon, &EventMonitor::mouseMove);

    // Button1/Button3 触发 press/release；其他按钮不触发
    m_mon->ButtonPressEvent(Button1, 10, 20, QString());
    m_mon->ButtonPressEvent(999, 1, 2, QString());
    m_mon->ButtonReleaseEvent(Button3, 30, 40, QString());
    m_mon->CursorMoveEvent(50, 60, QString());

    EXPECT_EQ(pressSpy.count(), 1);
    EXPECT_EQ(releaseSpy.count(), 1);
    EXPECT_EQ(moveSpy.count(), 1);
}

TEST_F(EventMonitorTest, getCursorImageWayland)
{
    // DBus callWithArgumentList 被全局 stub 成空 -> 返回空 QImage，不崩
    QImage img = m_mon->getCursorImageWayland();
    EXPECT_TRUE(img.isNull());
}

TEST_F(EventMonitorTest, releaseResSafeWhenNoDisplay)
{
    // 未 run() 时 m_display 为空，releaseRes 应安全无操作
    EXPECT_NO_FATAL_FAILURE(m_mon->releaseRes());
}

// --- Extended coverage: wayland slot branches and signal emissions ---

TEST_F(EventMonitorTest, buttonPressEventNonTriggeringButtons)
{
    QSignalSpy pressSpy(m_mon, &EventMonitor::mousePress);
    // Button2 (middle), Button4-7 should NOT trigger mousePress
    m_mon->ButtonPressEvent(Button2, 10, 20, QString());
    m_mon->ButtonPressEvent(4, 10, 20, QString());
    m_mon->ButtonPressEvent(999, 1, 2, QString());
    EXPECT_EQ(pressSpy.count(), 0);
}

TEST_F(EventMonitorTest, buttonReleaseEventButton1And3)
{
    QSignalSpy releaseSpy(m_mon, &EventMonitor::mouseRelease);
    m_mon->ButtonReleaseEvent(Button1, 10, 20, QString());
    m_mon->ButtonReleaseEvent(Button3, 30, 40, QString());
    EXPECT_EQ(releaseSpy.count(), 2);
}

TEST_F(EventMonitorTest, buttonReleaseEventNonTriggeringButtons)
{
    QSignalSpy releaseSpy(m_mon, &EventMonitor::mouseRelease);
    m_mon->ButtonReleaseEvent(Button2, 10, 20, QString());
    m_mon->ButtonReleaseEvent(999, 1, 2, QString());
    EXPECT_EQ(releaseSpy.count(), 0);
}

TEST_F(EventMonitorTest, cursorMoveEventEmitsMouseMove)
{
    QSignalSpy moveSpy(m_mon, &EventMonitor::mouseMove);
    m_mon->CursorMoveEvent(100, 200, QString());
    EXPECT_EQ(moveSpy.count(), 1);
    QList<QVariant> args = moveSpy.takeFirst();
    EXPECT_EQ(args.at(0).toInt(), 100);
    EXPECT_EQ(args.at(1).toInt(), 200);
}

TEST_F(EventMonitorTest, destructorCallsReleaseRes)
{
    // Destructor should not crash even without prior run()
    EventMonitor *mon = new EventMonitor();
    EXPECT_NO_FATAL_FAILURE(delete mon);
}

TEST_F(EventMonitorTest, initWaylandEventMonitorWhenWaylandOff)
{
    // When Utils::isWaylandMode is false, initWaylandEventMonitor is not called in ctor
    // but we can call it directly; it will fail on DBus but should not crash
    EXPECT_NO_FATAL_FAILURE(m_mon->initWaylandEventMonitor());
}
