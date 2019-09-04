/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
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

#include <QWidget>
#include <QDBusConnection>
#include <DApplication>
#include <QDBusInterface>
#include <DWindowManagerHelper>
#include "main_window.h"
#include "utils.h"
#include "widgets/toolbutton.h"
#include <QDateTime>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // Construct a QGuiApplication before accessing a platform function.
    DApplication::loadDXcbPlugin();
    DApplication app(argc, argv);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService("com.deepin.ScreenRecorder")) {
        // Poup up warning dialog if window manager not support composite.
        if (!DWindowManagerHelper::instance()->hasComposite()) {
            Utils::warnNoComposite();
            return 0;
        }
        // Start screen-recorder process if not other screen-recorder DBus service started.
        else {
            // Init application attributes.
            app.setOrganizationName("deepin");
            app.setApplicationName("deepin-screen-recorder");
            app.setApplicationVersion("1.0");

            // Load translator.
            app.loadTranslator();
//            app.setStyle("chameleon");
//            app.setAttribute(Qt::AA_UseHighDpiPixmaps);

            static const QDate buildDate = QLocale( QLocale::English ).
                                           toDate( QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
            QString t_date = buildDate.toString("MMdd");

            // Version Time
            app.setApplicationVersion(DApplication::buildVersion(t_date));

            // Show window.
            MainWindow window;

            window.showFullScreen();
            // window.show();
            window.initResource();

            // Register debus service.
            dbus.registerObject("/com/deepin/ScreenRecorder", &window, QDBusConnection::ExportScriptableSlots);

            return app.exec();
        }
    } else {
        // Send DBus message to stop screen-recorder if found other screen-recorder DBus service has started.
        QDBusInterface notification("com.deepin.ScreenRecorder",
                                    "/com/deepin/ScreenRecorder",
                                    "com.deepin.ScreenRecorder",
                                    QDBusConnection::sessionBus());

        QList<QVariant> arg;
        notification.callWithArgumentList(QDBus::AutoDetect, "stopRecord", arg);
    }

    return 0;
}
