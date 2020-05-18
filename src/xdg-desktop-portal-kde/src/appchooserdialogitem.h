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

#ifndef XDG_DESKTOP_PORTAL_KDE_APPCHOOSER_DIALOG_ITEM_H
#define XDG_DESKTOP_PORTAL_KDE_APPCHOOSER_DIALOG_ITEM_H

class QMouseEvent;

#include <QToolButton>

class AppChooserDialogItem : public QToolButton
{
    Q_OBJECT
public:
    explicit AppChooserDialogItem(const QString &applicationName, const QString &icon, const QString &applicationExec, QWidget *parent = nullptr);
    ~AppChooserDialogItem() override;

    QString applicationName() const;

    void mousePressEvent(QMouseEvent * event) override;
Q_SIGNALS:
    void clicked(const QString &applicationName);

private:
    QString m_applicationName;
};

#endif // XDG_DESKTOP_PORTAL_KDE_APPCHOOSER_DIALOG_ITEM_H



