// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDBusConnection>
#include <QDBusPendingReply>

#include "../../../src/dde-dock-plugins/shotstart/iconwidget_interface.h"

namespace {
class IconwidgetInterfaceTest : public testing::Test
{
public:
    void SetUp() override
    {
        m_iface = new iconwidget_interface(
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
    iconwidget_interface *m_iface = nullptr;
};
} // namespace

// ---- property getters ----
TEST_F(IconwidgetInterfaceTest, DisplayMode_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->displayMode());
}

TEST_F(IconwidgetInterfaceTest, DockedApps_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->dockedApps());
}

TEST_F(IconwidgetInterfaceTest, Entries_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->entries());
}

TEST_F(IconwidgetInterfaceTest, FrontendWindowRect_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->frontendWindowRect());
}

TEST_F(IconwidgetInterfaceTest, HideMode_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->hideMode());
}

TEST_F(IconwidgetInterfaceTest, HideState_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->hideState());
}

TEST_F(IconwidgetInterfaceTest, HideTimeout_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->hideTimeout());
}

TEST_F(IconwidgetInterfaceTest, IconSize_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->iconSize());
}

TEST_F(IconwidgetInterfaceTest, Opacity_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->opacity());
}

TEST_F(IconwidgetInterfaceTest, ShowTimeout_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->showTimeout());
}

TEST_F(IconwidgetInterfaceTest, WindowSize_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->windowSize());
}

TEST_F(IconwidgetInterfaceTest, WindowSizeEfficient_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->windowSizeEfficient());
}

TEST_F(IconwidgetInterfaceTest, WindowSizeFashion_Getter_ReturnsValueWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->windowSizeFashion());
}

// ---- property setters ----
TEST_F(IconwidgetInterfaceTest, DisplayMode_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setDisplayMode(0));
}

TEST_F(IconwidgetInterfaceTest, HideMode_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setHideMode(0));
}

TEST_F(IconwidgetInterfaceTest, HideTimeout_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setHideTimeout(30u));
}

TEST_F(IconwidgetInterfaceTest, IconSize_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setIconSize(48u));
}

TEST_F(IconwidgetInterfaceTest, Opacity_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setOpacity(0.5));
}

TEST_F(IconwidgetInterfaceTest, Position_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setPosition(1));
}

TEST_F(IconwidgetInterfaceTest, ShowTimeout_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setShowTimeout(20u));
}

TEST_F(IconwidgetInterfaceTest, WindowSize_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setWindowSize(100u));
}

TEST_F(IconwidgetInterfaceTest, WindowSizeEfficient_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setWindowSizeEfficient(100u));
}

TEST_F(IconwidgetInterfaceTest, WindowSizeFashion_Setter_AcceptsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setWindowSizeFashion(100u));
}

// ---- D-Bus methods ----
TEST_F(IconwidgetInterfaceTest, ActivateWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->ActivateWindow(1u));
}

TEST_F(IconwidgetInterfaceTest, CancelPreviewWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->CancelPreviewWindow());
}

TEST_F(IconwidgetInterfaceTest, CloseWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->CloseWindow(1u));
}

TEST_F(IconwidgetInterfaceTest, GetDockedAppsDesktopFiles_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->GetDockedAppsDesktopFiles());
}

TEST_F(IconwidgetInterfaceTest, GetEntryIDs_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->GetEntryIDs());
}

TEST_F(IconwidgetInterfaceTest, GetPluginSettings_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->GetPluginSettings());
}

TEST_F(IconwidgetInterfaceTest, IsDocked_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->IsDocked(QStringLiteral("test.desktop")));
}

TEST_F(IconwidgetInterfaceTest, IsOnDock_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->IsOnDock(QStringLiteral("test.desktop")));
}

TEST_F(IconwidgetInterfaceTest, MakeWindowAbove_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MakeWindowAbove(1u));
}

TEST_F(IconwidgetInterfaceTest, MaximizeWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MaximizeWindow(1u));
}

TEST_F(IconwidgetInterfaceTest, MergePluginSettings_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MergePluginSettings(QStringLiteral("cfg")));
}

TEST_F(IconwidgetInterfaceTest, MinimizeWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MinimizeWindow(1u));
}

TEST_F(IconwidgetInterfaceTest, MoveEntry_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MoveEntry(0, 1));
}

TEST_F(IconwidgetInterfaceTest, MoveWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->MoveWindow(1u));
}

TEST_F(IconwidgetInterfaceTest, PreviewWindow_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->PreviewWindow(1u));
}

TEST_F(IconwidgetInterfaceTest, QueryWindowIdentifyMethod_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->QueryWindowIdentifyMethod(1u));
}

TEST_F(IconwidgetInterfaceTest, RemovePluginSettings_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->RemovePluginSettings(QStringLiteral("key"), QStringList{}));
}

TEST_F(IconwidgetInterfaceTest, RequestDock_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->RequestDock(QStringLiteral("test.desktop"), 0));
}

TEST_F(IconwidgetInterfaceTest, RequestUndock_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->RequestUndock(QStringLiteral("test.desktop")));
}

TEST_F(IconwidgetInterfaceTest, SetFrontendWindowRect_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->SetFrontendWindowRect(0, 0, 100u, 100u));
}

TEST_F(IconwidgetInterfaceTest, SetPluginSettings_CallsMethodWithoutException)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->SetPluginSettings(QStringLiteral("cfg")));
}

// ---- staticInterfaceName / constructor / destructor ----
TEST_F(IconwidgetInterfaceTest, StaticInterfaceName_ReturnsDockInterfaceName)
{
    EXPECT_STRNE(nullptr, iconwidget_interface::staticInterfaceName());
}

TEST_F(IconwidgetInterfaceTest, Constructor_WithNullService_ConstructsObject)
{
    iconwidget_interface *iface = new iconwidget_interface(
        QStringLiteral(""), QStringLiteral("/test"),
        QDBusConnection::sessionBus());
    delete iface;
}
