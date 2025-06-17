// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbus_name.h"

#include <DSysInfo>


struct DBusNameItem {
    const char* name;
    const char* path;
    const char* interface;
};

DBusNameItem *dbus_name_get_name_list_v20()
{
    static DBusNameItem list[DBUS_COUNT] = {
        {"com.deepin.dde.Notification", "/com/deepin/dde/Notification", "com.deepin.dde.Notification"},
        {"com.deepin.daemon.Audio", "/com/deepin/daemon/Audio", "com.deepin.daemon.Audio"},
        {"com.deepin.daemon.Display", "/com/deepin/daemon/Display", "com.deepin.daemon.Display"},
        {"com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter", "com.deepin.dde.ControlCenter"},
        {"com.deepin.daemon.Zone", "/com/deepin/daemon/Zone", "com.deepin.daemon.Zone"},
        {"com.deepin.daemon.SoundEffect", "/com/deepin/daemon/SoundEffect", "com.deepin.daemon.SoundEffect"},
        {"com.deepin.daemon.Keybinding", "/com/deepin/daemon/Keybinding", "com.deepin.daemon.Keybinding"},
        {"com.deepin.daemon.Audio.Source", "/com/deepin/daemon/Audio/Source", "com.deepin.daemon.Audio.Source"},
        {"com.deepin.daemon.Audio.Sink", "/com/deepin/daemon/Audio/Sink", "com.deepin.daemon.Audio.Sink"},
        {"com.deepin.dde.lockFront", "/com/deepin/dde/lockFront", "com.deepin.dde.lockFront"},
        {"com.deepin.dde.Dock", "/com/deepin/dde/Dock", "com.deepin.dde.Dock"},
        {"com.deepin.daemon.InputDevices", "/com/deepin/api/XEventMonitor", "com.deepin.api.XEventMonitor"},
        {"com.deepin.daemon.SystemInfo", "/com/deepin/daemon/SystemInfo", "com.deepin.daemon.SystemInfo"},
        {"com.deepin.daemon.Clipboard", "/com/deepin/dde/ClipboardLoader", "com.deepin.dde.ClipboardLoader"},
        {"com.deepin.SessionManager", "/com/deepin/SessionManager", "com.deepin.SessionManager"}
    };
    return list;
}

DBusNameItem *dbus_name_get_name_list_v23()
{
    static DBusNameItem list[DBUS_COUNT] = {
        {"org.deepin.dde.Notification1", "/org/deepin/dde/Notification1", "org.deepin.dde.Notification1"},
        {"org.deepin.dde.Audio1", "/org/deepin/dde/Audio1", "org.deepin.dde.Audio1"},
        {"org.deepin.dde.Display1", "/org/deepin/dde/Display1", "org.deepin.dde.Display1"},
        {"org.deepin.dde.ControlCenter1", "/org/deepin/dde/ControlCenter1", "org.deepin.dde.ControlCenter1"},
        {"org.deepin.dde.Zone1", "/org/deepin/dde/Zone1", "org.deepin.dde.Zone1"},
        {"org.deepin.dde.SoundEffect1", "/org/deepin/dde/SoundEffect1", "org.deepin.dde.SoundEffect1"},
        {"org.deepin.dde.Keybinding1", "/org/deepin/dde/Keybinding1", "org.deepin.dde.Keybinding1"},
        {"org.deepin.dde.Audio1.Source", "/org/deepin/dde/Audio1/Source", "org.deepin.dde.Audio1.Source"},
        {"org.deepin.dde.Audio1.Sink", "/org/deepin/dde/Audio1/Sink", "org.deepin.dde.Audio1.Sink"},
        {"org.deepin.dde.LockFront1", "/org/deepin/dde/LockFront1", "org.deepin.dde.LockFront1"},
        {"org.deepin.dde.Dock1", "/org/deepin/dde/Dock1", "org.deepin.dde.Dock1"},
        {"org.deepin.dde.InputDevices1", "/org/deepin/dde/XEventMonitor1", "org.deepin.dde.XEventMonitor1"},
        {"org.deepin.dde.SystemInfo1", "/org/deepin/dde/SystemInfo1", "org.deepin.dde.SystemInfo1"},
        {"org.deepin.dde.ClipboardLoader1", "/org/deepin/dde/ClipboardLoader1", "org.deepin.dde.ClipboardLoader1"},
        {"org.deepin.dde.SessionManager1", "/org/deepin/dde/SessionManager1", "org.deepin.dde.SessionManager1"}
    };
    return list;
}

DBusNameItem *dbus_name_get_name_list()
{
    static DBusNameItem *list = []() {
        if (Dtk::Core::DSysInfo::majorVersion() == "20") {
            return dbus_name_get_name_list_v20();
        } else {
            return dbus_name_get_name_list_v23();
        }
    }();
    return list;
}

const char* dbus_name_get_name(DBusNameId id)
{
    return dbus_name_get_name_list()[id].name;
}

const char* dbus_name_get_path(DBusNameId id)
{
    return dbus_name_get_name_list()[id].path;
}

const char* dbus_name_get_interface(DBusNameId id)
{
    return dbus_name_get_name_list()[id].interface;
}


