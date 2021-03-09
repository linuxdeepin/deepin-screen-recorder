/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbusnotify.h"

/*
 * Implementation of interface class DBusNotify
 */

const QString NOTIFY_DBUS_NAME = "org.freedesktop.Notifications";
const QString NOTIFY_DBUS_PATH = "/org/freedesktop/Notifications";

DBusNotify::DBusNotify(QObject *parent)
    : QDBusAbstractInterface(NOTIFY_DBUS_NAME, NOTIFY_DBUS_PATH,
                             staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    QDBusConnection::sessionBus().connect(this->service(), this->path(),
    "org.freedesktop.DBus.Properties", "PropertiesChanged","sa{sv}as",
                                          this, SLOT(__propertyChanged__(QDBusMessage)));
}

DBusNotify::~DBusNotify()
{
    QDBusConnection::sessionBus().disconnect(service(), path(),
    "org.freedesktop.DBus.Properties", "PropertiesChanged", "sa{sv}as",
                                             this, SLOT(propertyChanged(QDBusMessage)));
}

