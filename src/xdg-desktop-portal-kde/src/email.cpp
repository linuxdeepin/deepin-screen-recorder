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

#include "email.h"

#include <QDesktopServices>
#include <QLoggingCategory>
#include <QUrl>

Q_LOGGING_CATEGORY(XdgDesktopPortalKdeEmail, "xdp-kde-email")

EmailPortal::EmailPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

EmailPortal::~EmailPortal()
{
}

uint EmailPortal::ComposeEmail(const QDBusObjectPath &handle, const QString &app_id, const QString &window, const QVariantMap &options, QVariantMap &results)
{
    Q_UNUSED(results)

    qCDebug(XdgDesktopPortalKdeEmail) << "ComposeEmail called with parameters:";
    qCDebug(XdgDesktopPortalKdeEmail) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeEmail) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeEmail) << "    window: " << window;
    qCDebug(XdgDesktopPortalKdeEmail) << "    options: " << options;

    QString attachmentString;
    const QStringList attachments = options.value(QStringLiteral("attachments")).toStringList();
    for (const QString &attachment : attachments) {
        attachmentString += QStringLiteral("&attachment=%1").arg(attachment);
    }

    const QString mailtoUrl = QStringLiteral("mailto:%1?subject=%2&body=%3%4").arg(options.value(QStringLiteral("address")).toString())
                                                                              .arg(options.value(QStringLiteral("subject")).toString())
                                                                              .arg(options.value(QStringLiteral("body")).toString())
                                                                              .arg(attachmentString);
    return QDesktopServices::openUrl(QUrl(mailtoUrl));
}

