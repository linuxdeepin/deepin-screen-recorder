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

#ifndef XDG_DESKTOP_PORTAL_KDE_PRINT_H
#define XDG_DESKTOP_PORTAL_KDE_PRINT_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QPrinter>
#include <QPageSize>

class PrintPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Print")
public:
    explicit PrintPortal(QObject *parent);
    ~PrintPortal();

public Q_SLOTS:
    uint Print(const QDBusObjectPath &handle,
               const QString &app_id,
               const QString &parent_window,
               const QString &title,
               const QDBusUnixFileDescriptor &fd,
               const QVariantMap &options,
               QVariantMap &results);

    uint PreparePrint(const QDBusObjectPath &handle,
                      const QString &app_id,
                      const QString &parent_window,
                      const QString &title,
                      const QVariantMap &settings,
                      const QVariantMap &page_setup,
                      const QVariantMap &options,
                      QVariantMap &results);
private:
    QMap<uint, QPrinter*> m_printers;

    bool cupsAvailable();
    QStringList printArguments(const QPrinter *printer, bool useCupsOptions, const QString &version,
                               QPrinter::Orientation documentOrientation);
    QStringList destination(const QPrinter *printer, const QString &version);
    QStringList copies(const QPrinter *printer, const QString &version);
    QStringList jobname(const QPrinter *printer, const QString &version);
    QStringList cupsOptions(const QPrinter *printer, QPrinter::Orientation documentOrientation);
    QStringList pages(const QPrinter *printer, bool useCupsOptions, const QString &version);
    QStringList optionMedia(const QPrinter *printer);
    QString mediaPaperSource(const QPrinter *printer);
    QStringList optionOrientation(const QPrinter *printer, QPrinter::Orientation documentOrientation);
    QStringList optionDoubleSidedPrinting(const QPrinter *printer);
    QStringList optionPageOrder(const QPrinter *printer);
    QStringList optionCollateCopies(const QPrinter *printer);
    QStringList optionPageMargins(const QPrinter *printer);
    QStringList optionCupsProperties(const QPrinter *printer);
};

#endif // XDG_DESKTOP_PORTAL_KDE_PRINT_H

