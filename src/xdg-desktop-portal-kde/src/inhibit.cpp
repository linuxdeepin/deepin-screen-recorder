/*
 * Copyright © 2017 Red Hat, Inc
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

#include "inhibit.h"
#include "request.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopPortalKdeInhibit, "xdp-kde-inhibit")

InhibitPortal::InhibitPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

InhibitPortal::~InhibitPortal()
{
}

void InhibitPortal::Inhibit(const QDBusObjectPath &handle, const QString &app_id, const QString &window, uint flags, const QVariantMap &options)
{
    qCDebug(XdgDesktopPortalKdeInhibit) << "Inhibit called with parameters:";
    qCDebug(XdgDesktopPortalKdeInhibit) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeInhibit) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeInhibit) << "    window: " << window;
    qCDebug(XdgDesktopPortalKdeInhibit) << "    flags: " << flags;
    qCDebug(XdgDesktopPortalKdeInhibit) << "    options: " << options;

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.Solid.PowerManagement"),
                                                          QStringLiteral("/org/kde/Solid/PowerManagement/PolicyAgent"),
                                                          QStringLiteral("org.kde.Solid.PowerManagement.PolicyAgent"),
                                                          QStringLiteral("AddInhibition"));
    //         interrupt session (1)
    message << (uint)1 << app_id << options.value(QStringLiteral("reason")).toString();

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, [handle, this] (QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<uint> reply = *watcher;
        if (reply.isError()) {
            qCDebug(XdgDesktopPortalKdeInhibit) << "Inhibition error: " << reply.error().message();
        } else {
            QDBusConnection sessionBus = QDBusConnection::sessionBus();
            Request *request = new Request(this, QStringLiteral("org.freedesktop.impl.portal.Inhibit"), QVariant(reply.value()));
            if (sessionBus.registerVirtualObject(handle.path(), request, QDBusConnection::VirtualObjectRegisterOption::SubPath)) {
                connect(request, &Request::closeRequested, [request, handle] () {
                    QDBusConnection::sessionBus().unregisterObject(handle.path());
                    request->deleteLater();
                });
            } else {
                qCDebug(XdgDesktopPortalKdeInhibit) << sessionBus.lastError().message();
                qCDebug(XdgDesktopPortalKdeInhibit) << "Failed to register request object with inhibition";
                request->deleteLater();
            }
        }
    });
}
