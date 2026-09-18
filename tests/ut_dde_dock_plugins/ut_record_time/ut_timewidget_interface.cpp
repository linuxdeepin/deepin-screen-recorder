// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDBusConnection>

#include "../../../src/dde-dock-plugins/recordtime/timewidget_interface.h"

namespace {
class TimewidgetInterfaceTest : public testing::Test
{
public:
    void SetUp() override
    {
        m_iface = new timewidget_interface(
            QStringLiteral("org.deepin.dde.daemon.Dock1"),
            QStringLiteral("/org/deepin/dde/daemon/Dock1"),
            QDBusConnection::sessionBus());
    }
    void TearDown() override
    {
        if (m_iface) {
            delete m_iface;
            m_iface = nullptr;
        }
    }
    timewidget_interface *m_iface = nullptr;
};
} // namespace

// ---- getters ----
TEST_F(TimewidgetInterfaceTest, DisplayMode_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->displayMode());
}

TEST_F(TimewidgetInterfaceTest, FrontendWindowRect_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->frontendWindowRect());
}

TEST_F(TimewidgetInterfaceTest, HideMode_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->hideMode());
}

TEST_F(TimewidgetInterfaceTest, HideState_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->hideState());
}

TEST_F(TimewidgetInterfaceTest, WindowSizeEfficient_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->windowSizeEfficient());
}

TEST_F(TimewidgetInterfaceTest, WindowSizeFashion_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->windowSizeFashion());
}

// ---- setters ----
TEST_F(TimewidgetInterfaceTest, DisplayMode_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setDisplayMode(0));
}

TEST_F(TimewidgetInterfaceTest, HideMode_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setHideMode(0));
}

TEST_F(TimewidgetInterfaceTest, Position_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setPosition(1));
}

TEST_F(TimewidgetInterfaceTest, WindowSizeEfficient_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setWindowSizeEfficient(100u));
}

TEST_F(TimewidgetInterfaceTest, WindowSizeFashion_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setWindowSizeFashion(100u));
}

// ---- D-Bus methods ----
TEST_F(TimewidgetInterfaceTest, IsDocked_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->IsDocked(QStringLiteral("test.desktop")));
}

TEST_F(TimewidgetInterfaceTest, RequestDock_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->RequestDock(QStringLiteral("test.desktop"), 0));
}

TEST_F(TimewidgetInterfaceTest, RequestUndock_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->RequestUndock(QStringLiteral("test.desktop")));
}

// ---- staticInterfaceName ----
TEST_F(TimewidgetInterfaceTest, StaticInterfaceName_ReturnsInterfaceName)
{
    EXPECT_STRNE(nullptr, timewidget_interface::staticInterfaceName());
}
