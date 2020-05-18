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

#include "screenshot.h"
#include "screenshotdialog.h"
#include "utils.h"

#include <QColorDialog>
#include <QDateTime>
#include <QtDBus>
#include <QDBusArgument>
#include <QDBusReply>
#include <QLoggingCategory>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QPointer>

Q_LOGGING_CATEGORY(XdgDesktopPortalKdeScreenshot, "xdp-kde-screenshot")

// Keep in sync with qflatpakcolordialog from Qt flatpak platform theme
Q_DECLARE_METATYPE(ScreenshotPortal::ColorRGB)

QDBusArgument &operator <<(QDBusArgument &arg, const ScreenshotPortal::ColorRGB &color)
{
    arg.beginStructure();
    arg << color.red << color.green << color.blue;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >>(const QDBusArgument &arg, ScreenshotPortal::ColorRGB &color)
{
    double red, green, blue;
    arg.beginStructure();
    arg >> red >> green >> blue;
    color.red = red;
    color.green = green;
    color.blue = blue;
    arg.endStructure();

    return arg;
}

QDBusArgument &operator<< (QDBusArgument &argument, const QColor &color)
{
    argument.beginStructure();
    argument << color.rgba();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QColor &color)
{
    argument.beginStructure();
    QRgb rgba;
    argument >> rgba;
    argument.endStructure();
    color = QColor::fromRgba(rgba);
    return argument;
}

ScreenshotPortal::ScreenshotPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDBusRegisterMetaType<QColor>();
    qDBusRegisterMetaType<ColorRGB>();
}

ScreenshotPortal::~ScreenshotPortal()
{
}

uint ScreenshotPortal::Screenshot(const QDBusObjectPath &handle,
                                  const QString &app_id,
                                  const QString &parent_window,
                                  const QVariantMap &options,
                                  QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeScreenshot) << "Screenshot called with parameters:";
    qCDebug(XdgDesktopPortalKdeScreenshot) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeScreenshot) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeScreenshot) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdeScreenshot) << "    options: " << options;

    QPointer<ScreenshotDialog> screenshotDialog = new ScreenshotDialog;
    Utils::setParentWindow(screenshotDialog, parent_window);

    const bool modal = options.value(QStringLiteral("modal"), false).toBool();
    screenshotDialog->setModal(modal);

    const bool interactive = options.value(QStringLiteral("interactive"), false).toBool();
    if (!interactive) {
        screenshotDialog->takeScreenshot();
    }

    QImage screenshot = screenshotDialog->exec() ? screenshotDialog->image() : QImage();

    if (screenshotDialog) {
        screenshotDialog->deleteLater();
    }

    if (screenshot.isNull()) {
        return 1;
    }

    const QString filename = QStringLiteral("%1/Screenshot_%2.png").arg(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                                                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")));

    if (!screenshot.save(filename, "PNG")) {
        return 1;
    }

    const QString resultFileName = QStringLiteral("file://") + filename;
    results.insert(QStringLiteral("uri"), resultFileName);

    return 0;
}

uint ScreenshotPortal::PickColor(const QDBusObjectPath &handle,
                                 const QString &app_id,
                                 const QString &parent_window,
                                 const QVariantMap &options,
                                 QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdeScreenshot) << "PickColor called with parameters:";
    qCDebug(XdgDesktopPortalKdeScreenshot) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeScreenshot) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdeScreenshot) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdeScreenshot) << "    options: " << options;

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/ColorPicker"),
                                                      QStringLiteral("org.kde.kwin.ColorPicker"),
                                                      QStringLiteral("pick"));
    QDBusReply<QColor> reply = QDBusConnection::sessionBus().call(msg);
    if (reply.isValid() && !reply.error().isValid()) {
        QColor selectedColor = reply.value();
        ColorRGB color;
        color.red = selectedColor.redF();
        color.green = selectedColor.greenF();
        color.blue = selectedColor.blueF();

        results.insert(QStringLiteral("color"), QVariant::fromValue<ScreenshotPortal::ColorRGB>(color));
        return 0;
    }

    return 1;
}
