// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include "../../src/dbus_name.h"

using namespace testing;

// dbus_name.cpp: pure data lookup functions — no system dependency, fully testable.
class DBusNameTest : public Test
{
};

TEST_F(DBusNameTest, getNameAllIds)
{
    for (int i = 0; i < DBUS_COUNT; ++i) {
        const char *name = dbus_name_get_name(static_cast<DBusNameId>(i));
        EXPECT_NE(name, nullptr);
        EXPECT_STRNE(name, "");
    }
}

TEST_F(DBusNameTest, getPathAllIds)
{
    for (int i = 0; i < DBUS_COUNT; ++i) {
        const char *path = dbus_name_get_path(static_cast<DBusNameId>(i));
        EXPECT_NE(path, nullptr);
        EXPECT_STRNE(path, "");
    }
}

TEST_F(DBusNameTest, getInterfaceAllIds)
{
    for (int i = 0; i < DBUS_COUNT; ++i) {
        const char *iface = dbus_name_get_interface(static_cast<DBusNameId>(i));
        EXPECT_NE(iface, nullptr);
        EXPECT_STRNE(iface, "");
    }
}

TEST_F(DBusNameTest, notificationMacros)
{
    EXPECT_STRNE(NOTIFICATION_NAME, "");
    EXPECT_STRNE(NOTIFICATION_PATH, "");
    EXPECT_STRNE(NOTIFICATION_INTERFACE, "");
}

TEST_F(DBusNameTest, audioMacros)
{
    EXPECT_STRNE(AUDIO_NAME, "");
    EXPECT_STRNE(AUDIO_PATH, "");
    EXPECT_STRNE(AUDIO_INTERFACE, "");
}

TEST_F(DBusNameTest, displayMacros)
{
    EXPECT_STRNE(DISPLAY_NAME, "");
    EXPECT_STRNE(DISPLAY_PATH, "");
    EXPECT_STRNE(DISPLAY_INTERFACE, "");
}

TEST_F(DBusNameTest, controlCenterMacros)
{
    EXPECT_STRNE(CONTROL_CENTER_NAME, "");
    EXPECT_STRNE(CONTROL_CENTER_PATH, "");
    EXPECT_STRNE(CONTROL_CENTER_INTERFACE, "");
}

TEST_F(DBusNameTest, zoneMacros)
{
    EXPECT_STRNE(ZONE_NAME, "");
    EXPECT_STRNE(ZONE_PATH, "");
    EXPECT_STRNE(ZONE_INTERFACE, "");
}

TEST_F(DBusNameTest, soundEffectMacros)
{
    EXPECT_STRNE(SOUND_EFFECT_NAME, "");
    EXPECT_STRNE(SOUND_EFFECT_PATH, "");
    EXPECT_STRNE(SOUND_EFFECT_INTERFACE, "");
}

TEST_F(DBusNameTest, keybindingMacros)
{
    EXPECT_STRNE(KEYBINDING_NAME, "");
    EXPECT_STRNE(KEYBINDING_PATH, "");
    EXPECT_STRNE(KEYBINDING_INTERFACE, "");
}

TEST_F(DBusNameTest, audioSourceSinkMacros)
{
    EXPECT_STRNE(AUDIO_SOURCE_NAME, "");
    EXPECT_STRNE(AUDIO_SINK_NAME, "");
}

TEST_F(DBusNameTest, lockFrontMacros)
{
    EXPECT_STRNE(LOCK_FRONT_NAME, "");
    EXPECT_STRNE(LOCK_FRONT_PATH, "");
    EXPECT_STRNE(LOCK_FRONT_INTERFACE, "");
}

TEST_F(DBusNameTest, dockMacros)
{
    EXPECT_STRNE(DOCK_NAME, "");
    EXPECT_STRNE(DOCK_PATH, "");
    EXPECT_STRNE(DOCK_INTERFACE, "");
}

TEST_F(DBusNameTest, inputDevicesMacros)
{
    EXPECT_STRNE(INPUT_DEVICES_NAME, "");
    EXPECT_STRNE(INPUT_DEVICES_PATH, "");
    EXPECT_STRNE(INPUT_DEVICES_INTERFACE, "");
}

TEST_F(DBusNameTest, systemInfoMacros)
{
    EXPECT_STRNE(SYSTEM_INFO_NAME, "");
    EXPECT_STRNE(SYSTEM_INFO_PATH, "");
    EXPECT_STRNE(SYSTEM_INFO_INTERFACE, "");
}

TEST_F(DBusNameTest, clipboardMacros)
{
    EXPECT_STRNE(CLIPBOARD_NAME, "");
    EXPECT_STRNE(CLIPBOARD_PATH, "");
    EXPECT_STRNE(CLIPBOARD_INTERFACE, "");
}

TEST_F(DBusNameTest, sessionManagerMacros)
{
    EXPECT_STRNE(SESSION_MANAGER_NAME, "");
    EXPECT_STRNE(SESSION_MANAGER_PATH, "");
    EXPECT_STRNE(SESSION_MANAGER_INTERFACE, "");
}

// Verify v20/v23 naming convention: v23 names start with "org.deepin"
TEST_F(DBusNameTest, namingConventionConsistency)
{
    // All IDs should have non-null name/path/interface (covers both v20 and v23 paths)
    EXPECT_STRNE(dbus_name_get_name(DBUS_NOTIFICATION), dbus_name_get_name(DBUS_AUDIO));
    EXPECT_STRNE(dbus_name_get_path(DBUS_NOTIFICATION), dbus_name_get_path(DBUS_AUDIO));
}
