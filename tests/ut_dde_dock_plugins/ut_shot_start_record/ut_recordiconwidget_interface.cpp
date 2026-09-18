// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDBusConnection>
#include <QDBusPendingReply>

#include "../../../src/dde-dock-plugins/shotstartrecord/recordiconwidget_interface.h"

namespace {
class RecordIconwidgetInterfaceTest : public testing::Test
{
public:
    void SetUp() override
    {
        m_iface = new recordiconwidget_interface(
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
    recordiconwidget_interface *m_iface = nullptr;
};
} // namespace

// ---- property getters ----
TEST_F(RecordIconwidgetInterfaceTest, DisplayMode_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->displayMode());
}

TEST_F(RecordIconwidgetInterfaceTest, DockedApps_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->dockedApps());
}

TEST_F(RecordIconwidgetInterfaceTest, Entries_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->entries());
}

TEST_F(RecordIconwidgetInterfaceTest, FrontendWindowRect_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->frontendWindowRect());
}

TEST_F(RecordIconwidgetInterfaceTest, HideMode_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->hideMode());
}

TEST_F(RecordIconwidgetInterfaceTest, HideState_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->hideState());
}

TEST_F(RecordIconwidgetInterfaceTest, HideTimeout_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->hideTimeout());
}

TEST_F(RecordIconwidgetInterfaceTest, IconSize_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->iconSize());
}

TEST_F(RecordIconwidgetInterfaceTest, Opacity_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->opacity());
}

TEST_F(RecordIconwidgetInterfaceTest, ShowTimeout_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->showTimeout());
}

TEST_F(RecordIconwidgetInterfaceTest, WindowSize_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->windowSize());
}

TEST_F(RecordIconwidgetInterfaceTest, WindowSizeEfficient_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->windowSizeEfficient());
}

TEST_F(RecordIconwidgetInterfaceTest, WindowSizeFashion_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->windowSizeFashion());
}

// ---- property setters ----
TEST_F(RecordIconwidgetInterfaceTest, DisplayMode_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setDisplayMode(0));
}

TEST_F(RecordIconwidgetInterfaceTest, HideMode_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setHideMode(0));
}

TEST_F(RecordIconwidgetInterfaceTest, HideTimeout_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setHideTimeout(30u));
}

TEST_F(RecordIconwidgetInterfaceTest, IconSize_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setIconSize(48u));
}

TEST_F(RecordIconwidgetInterfaceTest, Opacity_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setOpacity(0.5));
}

TEST_F(RecordIconwidgetInterfaceTest, Position_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setPosition(1));
}

TEST_F(RecordIconwidgetInterfaceTest, ShowTimeout_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setShowTimeout(20u));
}

TEST_F(RecordIconwidgetInterfaceTest, WindowSize_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setWindowSize(100u));
}

TEST_F(RecordIconwidgetInterfaceTest, WindowSizeEfficient_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setWindowSizeEfficient(100u));
}

TEST_F(RecordIconwidgetInterfaceTest, WindowSizeFashion_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setWindowSizeFashion(100u));
}

// ---- D-Bus methods ----
TEST_F(RecordIconwidgetInterfaceTest, ActivateWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->ActivateWindow(1u));
}

TEST_F(RecordIconwidgetInterfaceTest, CancelPreviewWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->CancelPreviewWindow());
}

TEST_F(RecordIconwidgetInterfaceTest, CloseWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->CloseWindow(1u));
}

TEST_F(RecordIconwidgetInterfaceTest, GetDockedAppsDesktopFiles_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->GetDockedAppsDesktopFiles());
}

TEST_F(RecordIconwidgetInterfaceTest, GetEntryIDs_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->GetEntryIDs());
}

TEST_F(RecordIconwidgetInterfaceTest, GetPluginSettings_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->GetPluginSettings());
}

TEST_F(RecordIconwidgetInterfaceTest, IsDocked_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->IsDocked(QStringLiteral("test.desktop")));
}

TEST_F(RecordIconwidgetInterfaceTest, IsOnDock_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->IsOnDock(QStringLiteral("test.desktop")));
}

TEST_F(RecordIconwidgetInterfaceTest, MakeWindowAbove_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MakeWindowAbove(1u));
}

TEST_F(RecordIconwidgetInterfaceTest, MaximizeWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MaximizeWindow(1u));
}

TEST_F(RecordIconwidgetInterfaceTest, MergePluginSettings_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MergePluginSettings(QStringLiteral("cfg")));
}

TEST_F(RecordIconwidgetInterfaceTest, MinimizeWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MinimizeWindow(1u));
}

TEST_F(RecordIconwidgetInterfaceTest, MoveEntry_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MoveEntry(0, 1));
}

TEST_F(RecordIconwidgetInterfaceTest, MoveWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MoveWindow(1u));
}

TEST_F(RecordIconwidgetInterfaceTest, PreviewWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->PreviewWindow(1u));
}

TEST_F(RecordIconwidgetInterfaceTest, QueryWindowIdentifyMethod_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->QueryWindowIdentifyMethod(1u));
}

TEST_F(RecordIconwidgetInterfaceTest, RemovePluginSettings_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->RemovePluginSettings(QStringLiteral("key"), QStringList{}));
}

TEST_F(RecordIconwidgetInterfaceTest, RequestDock_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->RequestDock(QStringLiteral("test.desktop"), 0));
}

TEST_F(RecordIconwidgetInterfaceTest, RequestUndock_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->RequestUndock(QStringLiteral("test.desktop")));
}

TEST_F(RecordIconwidgetInterfaceTest, SetFrontendWindowRect_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->SetFrontendWindowRect(0, 0, 100u, 100u));
}

TEST_F(RecordIconwidgetInterfaceTest, SetPluginSettings_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->SetPluginSettings(QStringLiteral("cfg")));
}

// ---- staticInterfaceName ----
TEST_F(RecordIconwidgetInterfaceTest, StaticInterfaceName_ReturnsDockInterfaceName)
{
    EXPECT_STRNE(nullptr, recordiconwidget_interface::staticInterfaceName());
}
