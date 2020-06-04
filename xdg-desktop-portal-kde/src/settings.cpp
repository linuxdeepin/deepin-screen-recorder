/*
 * Copyright © 2018-2019 Red Hat, Inc
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

#include "settings.h"

#include <QDBusMetaType>
#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QApplication>

#include <QLoggingCategory>

#include <KConfigCore/KConfigGroup>
#include "desktopportal.h"
#include "waylandintegration.h"
Q_LOGGING_CATEGORY(XdgDesktopPortalKdeSettings, "xdp-kde-settings")

Q_DECLARE_METATYPE(SettingsPortal::VariantMapMap)

QDBusArgument &operator<<(QDBusArgument &argument, const SettingsPortal::VariantMapMap &mymap)
{
    argument.beginMap(QVariant::String, QVariant::Map);

    QMapIterator<QString, QVariantMap> i(mymap);
    while (i.hasNext()) {
        i.next();
        argument.beginMapEntry();
        argument << i.key() << i.value();
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, SettingsPortal::VariantMapMap &mymap)
{
    argument.beginMap();
    mymap.clear();

    while (!argument.atEnd()) {
        QString key;
        QVariantMap value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        mymap.insert(key, value);
    }

    argument.endMap();
    return argument;
}

static bool groupMatches(const QString &group, const QStringList &patterns)
{
    for (const QString &pattern : patterns) {
        if (pattern.isEmpty()) {
            return true;
        }

        if (pattern == group) {
            return true;
        }

        if (pattern.endsWith(QLatin1Char('*')) && group.startsWith(pattern.left(pattern.length() - 1))) {
            return true;
        }
    }

    return false;
}

SettingsPortal::SettingsPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDBusRegisterMetaType<VariantMapMap>();

    m_kdeglobals = KSharedConfig::openConfig();

    QDBusConnection::sessionBus().connect(QString(), QStringLiteral("/KDEPlatformTheme"), QStringLiteral("org.kde.KDEPlatformTheme"),
                                          QStringLiteral("refreshFonts"), this, SLOT(fontChanged()));
    QDBusConnection::sessionBus().connect(QString(), QStringLiteral("/KGlobalSettings"), QStringLiteral("org.kde.KGlobalSettings"),
                                          QStringLiteral("notifyChange"), this, SLOT(globalSettingChanged(int,int)));
    QDBusConnection::sessionBus().connect(QString(), QStringLiteral("/KToolBar"), QStringLiteral("org.kde.KToolBar"),
                                          QStringLiteral("styleChanged"), this, SLOT(toolbarStyleChanged()));
}

SettingsPortal::~SettingsPortal()
{
}

void SettingsPortal::ReadAll(const QStringList &groups)
{
    qCDebug(XdgDesktopPortalKdeSettings) << "ReadAll called with parameters:";
    qCDebug(XdgDesktopPortalKdeSettings) << "    groups: " << groups;

    //FIXME this is super ugly, but I was unable to make it properly return VariantMapMap
    QObject *obj = QObject::parent();

    if (!obj) {
        qCWarning(XdgDesktopPortalKdeSettings) << "Failed to get dbus context";
        return;
    }

    void *ptr = obj->qt_metacast("QDBusContext");
    QDBusContext *q_ptr = reinterpret_cast<QDBusContext *>(ptr);

    if (!q_ptr) {
        qCWarning(XdgDesktopPortalKdeSettings) << "Failed to get dbus context";
        return;
    }

    VariantMapMap result;
    for (const QString &settingGroupName : m_kdeglobals->groupList()) {
        //NOTE: use org.kde.kdeglobals prefix

        QString uniqueGroupName = QStringLiteral("org.kde.kdeglobals.") + settingGroupName;

        if (!groupMatches(uniqueGroupName, groups)) {
            continue;
        }

        QVariantMap map;
        KConfigGroup configGroup(m_kdeglobals, settingGroupName);

        for (const QString &key : configGroup.keyList()) {
            map.insert(key, configGroup.readEntry(key));
        }

        result.insert(uniqueGroupName, map);
    }

    QDBusMessage message = q_ptr->message();
    QDBusMessage reply = message.createReply(QVariant::fromValue(result));
    QDBusConnection::sessionBus().send(reply);
}

void SettingsPortal::Read(const QString &group, const QString &key)
{
    qCDebug(XdgDesktopPortalKdeSettings) << "Read called with parameters:";
    qCDebug(XdgDesktopPortalKdeSettings) << "    group: " << group;
    qCDebug(XdgDesktopPortalKdeSettings) << "    key: " << key;

    //FIXME this is super ugly, but I was unable to make it properly return VariantMapMap
    QObject *obj = QObject::parent();

    if (!obj) {
        qCWarning(XdgDesktopPortalKdeSettings) << "Failed to get dbus context";
        return;
    }

    void *ptr = obj->qt_metacast("QDBusContext");
    QDBusContext *q_ptr = reinterpret_cast<QDBusContext *>(ptr);

    if (!q_ptr) {
        qCWarning(XdgDesktopPortalKdeSettings) << "Failed to get dbus context";
        return;
    }

    QDBusMessage reply;
    QDBusMessage message = q_ptr->message();

    // All our namespaces start with this prefix
    if (!group.startsWith(QStringLiteral("org.kde.kdeglobals"))) {
        qCWarning(XdgDesktopPortalKdeSettings) << "Namespace " << group << " is not supported";
        reply = message.createErrorReply(QDBusError::UnknownProperty, QStringLiteral("Namespace is not supported"));
        QDBusConnection::sessionBus().send(reply);
        return;
    }

    QDBusVariant result = readProperty(group, key);
    if (result.variant().isNull()) {
        reply = message.createErrorReply(QDBusError::UnknownProperty, QStringLiteral("Property doesn't exist"));
    } else {
        reply = message.createReply(QVariant::fromValue(result));
    }

    QDBusConnection::sessionBus().send(reply);
}

void SettingsPortal::fontChanged()
{
    Q_EMIT SettingChanged(QStringLiteral("org.kde.kdeglobals.General"), QStringLiteral("font"), readProperty(QStringLiteral("org.kde.kdeglobals.General"), QStringLiteral("font")));
}

void SettingsPortal::globalSettingChanged(int type, int arg)
{
    m_kdeglobals->reparseConfiguration();

    // Mostly based on plasma-integration needs
    switch (type) {
    case PaletteChanged:
        // Plasma-integration will be loading whole palette again, there is no reason to try to identify
        // particular categories or colors
        Q_EMIT SettingChanged(QStringLiteral("org.kde.kdeglobals.General"), QStringLiteral("ColorScheme"), readProperty(QStringLiteral("org.kde.kdeglobals.General"), QStringLiteral("ColorScheme")));
        break;
    case FontChanged:
        fontChanged();
        break;
    case StyleChanged:
        Q_EMIT SettingChanged(QStringLiteral("org.kde.kdeglobals.KDE"), QStringLiteral("widgetStyle"), readProperty(QStringLiteral("org.kde.kdeglobals.KDE"), QStringLiteral("widgetStyle")));
        break;
    case SettingsChanged: {
        SettingsCategory category = static_cast<SettingsCategory>(arg);
        if (category == SETTINGS_QT || category == SETTINGS_MOUSE) {
            // TODO
        } else if (category == SETTINGS_STYLE) {
            // TODO
        }
        break;
    }
    case IconChanged:
        // we will get notified about each category, but it probably makes sense to send this signal just once
        if (arg == 0) { // KIconLoader::Desktop
            Q_EMIT SettingChanged(QStringLiteral("org.kde.kdeglobals.Icons"), QStringLiteral("Theme"), readProperty(QStringLiteral("org.kde.kdeglobals.Icons"), QStringLiteral("Theme")));
        }
        break;
    case CursorChanged:
        // TODO
        break;
    case ToolbarStyleChanged:
        toolbarStyleChanged();
        break;
    default:
        break;
    }
}

void SettingsPortal::toolbarStyleChanged()
{
    Q_EMIT SettingChanged(QStringLiteral("org.kde.kdeglobals.Toolbar style"), QStringLiteral("ToolButtonStyle"), readProperty(QStringLiteral("org.kde.kdeglobals.Toolbar style"), QStringLiteral("ToolButtonStyle")));
}

QDBusVariant SettingsPortal::readProperty(const QString &group, const QString &key)
{
    QString groupName = group.right(group.length() - QStringLiteral("org.kde.kdeglobals.").length());

    if (!m_kdeglobals->hasGroup(groupName)) {
        qCWarning(XdgDesktopPortalKdeSettings) << "Group " << group << " doesn't exist";
        return QDBusVariant();
    }

    KConfigGroup configGroup(m_kdeglobals, groupName);

    if (!configGroup.hasKey(key)) {
        qCWarning(XdgDesktopPortalKdeSettings) << "Key " << key << " doesn't exist";
        return QDBusVariant();
    }

    return QDBusVariant(configGroup.readEntry(key));
}
void SettingsPortal::stopRecord()
{
    WaylandIntegration::stopStreaming();

    //通知录屏完成
    QDBusInterface notification(QString::fromUtf8("com.deepin.ScreenRecorder"),
                                QString::fromUtf8("/com/deepin/ScreenRecorder"),
                                QString::fromUtf8("com.deepin.ScreenRecorder"),
                                QDBusConnection::sessionBus());
    QList<QVariant> arg;
    notification.callWithArgumentList(QDBus::AutoDetect,QString::fromUtf8("waylandRecordOver"),arg);
    qDebug() << Q_FUNC_INFO;
    //QApplication::quit();
}




























