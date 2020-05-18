/*
 * Copyright © 2016 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 */

#include "desktopportal.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgRequestKdeRequest, "xdp-kde-request")

Request::Request(QObject *parent, const QString &portalName, const QVariant &data)
    : QDBusVirtualObject(parent)
    , m_data(data)
    , m_portalName(portalName)
{
}

Request::~Request()
{
}

bool Request::handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
{
    Q_UNUSED(connection);

    /* Check to make sure we're getting properties on our interface */
    if (message.type() != QDBusMessage::MessageType::MethodCallMessage) {
        return false;
    }

    qCDebug(XdgRequestKdeRequest) << message.interface();
    qCDebug(XdgRequestKdeRequest) << message.member();
    qCDebug(XdgRequestKdeRequest) << message.path();

    QList<QVariant> arguments;
    if (message.interface() == QLatin1String("org.freedesktop.impl.portal.Request")) {
        if (message.member() == QLatin1String("Close")) {
            if (m_portalName == QLatin1String("org.freedesktop.impl.portal.Inhibit")) {
                QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.Solid.PowerManagement"),
                                                                      QStringLiteral("/org/kde/Solid/PowerManagement/PolicyAgent"),
                                                                      QStringLiteral("org.kde.Solid.PowerManagement.PolicyAgent"),
                                                                      QStringLiteral("ReleaseInhibition"));

                message << m_data.toUInt();

                QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
                QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
                connect(watcher, &QDBusPendingCallWatcher::finished, [this] (QDBusPendingCallWatcher *watcher) {
                    QDBusPendingReply<> reply = *watcher;
                    if (reply.isError()) {
                        qCDebug(XdgRequestKdeRequest) << "Uninhibit error: " << reply.error().message();
                    } else {
                        Q_EMIT closeRequested();
                    }
                });
            }
        }
    }

    return true;
}

QString Request::introspect(const QString &path) const
{
    QString nodes;

    if (path.startsWith(QLatin1String("/org/freedesktop/portal/desktop/request/"))) {
        nodes = QStringLiteral(
            "<interface name=\"org.freedesktop.impl.portal.Request\">"
            "    <method name=\"Close\">"
            "    </method>"
            "</interface>");
    }

    qCDebug(XdgRequestKdeRequest) << nodes;

    return nodes;
}

