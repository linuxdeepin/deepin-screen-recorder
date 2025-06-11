// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_NAME_H
#define DBUS_NAME_H

enum DBusNameId {
    DBUS_NOTIFICATION = 0,
    DBUS_AUDIO,
    DBUS_DISPLAY,
    DBUS_CONTROL_CENTER,
    DBUS_ZONE,
    DBUS_SOUND_EFFECT,
    DBUS_KEYBINDING,
    DBUS_AUDIO_SOURCE,
    DBUS_AUDIO_SINK,
    DBUS_LOCK_FRONT,
    DBUS_DOCK,
    DBUS_INPUT_DEVICES,
    DBUS_SYSTEM_INFO,
    DBUS_CLIPBOARD,
    DBUS_SESSION_MANAGER,
    DBUS_COUNT,
};

const char* dbus_name_get_name(DBusNameId id);
const char* dbus_name_get_path(DBusNameId id);
const char* dbus_name_get_interface(DBusNameId id);

#define NOTIFICATION_NAME dbus_name_get_name(DBUS_NOTIFICATION)
#define NOTIFICATION_PATH dbus_name_get_path(DBUS_NOTIFICATION)
#define NOTIFICATION_INTERFACE dbus_name_get_interface(DBUS_NOTIFICATION)

#define AUDIO_NAME dbus_name_get_name(DBUS_AUDIO)
#define AUDIO_PATH dbus_name_get_path(DBUS_AUDIO)
#define AUDIO_INTERFACE dbus_name_get_interface(DBUS_AUDIO)

#define DISPLAY_NAME dbus_name_get_name(DBUS_DISPLAY)
#define DISPLAY_PATH dbus_name_get_path(DBUS_DISPLAY)
#define DISPLAY_INTERFACE dbus_name_get_interface(DBUS_DISPLAY)

#define CONTROL_CENTER_NAME dbus_name_get_name(DBUS_CONTROL_CENTER)
#define CONTROL_CENTER_PATH dbus_name_get_path(DBUS_CONTROL_CENTER)
#define CONTROL_CENTER_INTERFACE dbus_name_get_interface(DBUS_CONTROL_CENTER)

#define ZONE_NAME dbus_name_get_name(DBUS_ZONE)
#define ZONE_PATH dbus_name_get_path(DBUS_ZONE)
#define ZONE_INTERFACE dbus_name_get_interface(DBUS_ZONE)

#define SOUND_EFFECT_NAME dbus_name_get_name(DBUS_SOUND_EFFECT)
#define SOUND_EFFECT_PATH dbus_name_get_path(DBUS_SOUND_EFFECT)
#define SOUND_EFFECT_INTERFACE dbus_name_get_interface(DBUS_SOUND_EFFECT)

#define KEYBINDING_NAME dbus_name_get_name(DBUS_KEYBINDING)
#define KEYBINDING_PATH dbus_name_get_path(DBUS_KEYBINDING)
#define KEYBINDING_INTERFACE dbus_name_get_interface(DBUS_KEYBINDING)

#define AUDIO_SOURCE_NAME dbus_name_get_name(DBUS_AUDIO_SOURCE)
#define AUDIO_SOURCE_PATH dbus_name_get_path(DBUS_AUDIO_SOURCE)
#define AUDIO_SOURCE_INTERFACE dbus_name_get_interface(DBUS_AUDIO_SOURCE)

#define AUDIO_SINK_NAME dbus_name_get_name(DBUS_AUDIO_SINK)
#define AUDIO_SINK_PATH dbus_name_get_path(DBUS_AUDIO_SINK)
#define AUDIO_SINK_INTERFACE dbus_name_get_interface(DBUS_AUDIO_SINK)

#define LOCK_FRONT_NAME dbus_name_get_name(DBUS_LOCK_FRONT)
#define LOCK_FRONT_PATH dbus_name_get_path(DBUS_LOCK_FRONT)
#define LOCK_FRONT_INTERFACE dbus_name_get_interface(DBUS_LOCK_FRONT)

#define DOCK_NAME dbus_name_get_name(DBUS_DOCK)
#define DOCK_PATH dbus_name_get_path(DBUS_DOCK)
#define DOCK_INTERFACE dbus_name_get_interface(DBUS_DOCK)

#define INPUT_DEVICES_NAME dbus_name_get_name(DBUS_INPUT_DEVICES)
#define INPUT_DEVICES_PATH dbus_name_get_path(DBUS_INPUT_DEVICES)
#define INPUT_DEVICES_INTERFACE dbus_name_get_interface(DBUS_INPUT_DEVICES)

#define SYSTEM_INFO_NAME dbus_name_get_name(DBUS_SYSTEM_INFO)
#define SYSTEM_INFO_PATH dbus_name_get_path(DBUS_SYSTEM_INFO)
#define SYSTEM_INFO_INTERFACE dbus_name_get_interface(DBUS_SYSTEM_INFO)

#define CLIPBOARD_NAME dbus_name_get_name(DBUS_CLIPBOARD)
#define CLIPBOARD_PATH dbus_name_get_path(DBUS_CLIPBOARD)
#define CLIPBOARD_INTERFACE dbus_name_get_interface(DBUS_CLIPBOARD)

#define SESSION_MANAGER_NAME dbus_name_get_name(DBUS_SESSION_MANAGER)
#define SESSION_MANAGER_PATH dbus_name_get_path(DBUS_SESSION_MANAGER)
#define SESSION_MANAGER_INTERFACE dbus_name_get_interface(DBUS_SESSION_MANAGER)

#endif // DBUS_NAME_H
