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

#ifndef XDG_DESKTOP_PORTAL_KDE_ACCESS_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_ACCESS_DIALOG_H

#include <QDialog>

namespace Ui
{
class AccessDialog;
}

class AccessDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AccessDialog(QDialog *parent = nullptr, Qt::WindowFlags flags = {});
    ~AccessDialog();

    void setAcceptLabel(const QString &label);
    void setBody(const QString &body);
    void setIcon(const QString &icon);
    void setRejectLabel(const QString &label);
    void setTitle(const QString &title);
    void setSubtitle(const QString &subtitle);

private:
    Ui::AccessDialog * m_dialog;

};

#endif // XDG_DESKTOP_PORTAL_KDE_ACCESS_DIALOG_H


