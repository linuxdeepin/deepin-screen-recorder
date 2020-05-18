/*
 * Copyright © 2018 Red Hat, Inc
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

#ifndef XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_DIALOG_H

#include <QAbstractListModel>
#include <QDialog>

#include "remotedesktop.h"

namespace Ui
{
class RemoteDesktopDialog;
}

class RemoteDesktopDialog : public QDialog
{
    Q_OBJECT
public:
    RemoteDesktopDialog(const QString &appName, RemoteDesktopPortal::DeviceTypes deviceTypes, bool screenSharingEnabled = false,
                        bool multiple = false, QDialog *parent = nullptr, Qt::WindowFlags flags = {});
    ~RemoteDesktopDialog();

    QList<quint32> selectedScreens() const;
    RemoteDesktopPortal::DeviceTypes deviceTypes() const;

private:
    Ui::RemoteDesktopDialog * m_dialog;
};

#endif // XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_DIALOG_H
