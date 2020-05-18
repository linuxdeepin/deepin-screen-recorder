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

#include <QDialog>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopPortalKdeDesktopPortal, "xdp-kde-desktop-portal")

DesktopPortal::DesktopPortal(int &argc, char **argv,QObject *parent)
    : QObject(parent)
    , m_access(new AccessPortal(this))
    , m_appChooser(new AppChooserPortal(this))
    , m_email(new EmailPortal(this))
    , m_fileChooser(new FileChooserPortal(this))
    , m_inhibit(new InhibitPortal(this))
    , m_notification(new NotificationPortal(this))
    , m_print(new PrintPortal(this))
#if SCREENCAST_ENABLED
    , m_screenCast(new ScreenCastPortal(this))
    , m_remoteDesktop(new RemoteDesktopPortal(this))
#endif
    , m_screenshot(new ScreenshotPortal(this))
    , m_settings(new SettingsPortal(this))
{
#if SCREENCAST_ENABLED
    WaylandIntegration::init(argc,argv);
#endif
}

DesktopPortal::~DesktopPortal()
{
}
