/*
 * Copyright © 2016-2018 Red Hat, Inc
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

#ifndef XDG_DESKTOP_PORTAL_KDE_APPCHOOSER_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_APPCHOOSER_DIALOG_H

#include <QAbstractListModel>
#include <QDialog>

class AppChooserDialogItem;
class QGridLayout;

class AppChooserDialog : public QDialog
{
    Q_OBJECT
public:
    AppChooserDialog(const QStringList &choices, const QString &defaultApp, const QString &fileName, QDialog *parent = nullptr, Qt::WindowFlags flags = {});
    ~AppChooserDialog();

    void updateChoices(const QStringList &choices);

    QString selectedApplication() const;

private Q_SLOTS:
    void addDialogItems();

private:

    QStringList m_choices;
    QString m_defaultApp;
    QString m_selectedApplication;
    QGridLayout *m_gridLayout;
};

#endif // XDG_DESKTOP_PORTAL_KDE_APPCHOOSER_DIALOG_H


