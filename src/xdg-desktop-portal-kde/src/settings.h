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

#ifndef XDG_DESKTOP_PORTAL_KDE_SETTINGS_H
#define XDG_DESKTOP_PORTAL_KDE_SETTINGS_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

#include <KConfigCore/KConfig>
#include <KConfigCore/KSharedConfig>

class SettingsPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Settings")
public:
    /**
    * An identifier for change signals.
    * @note Copied from KGlobalSettings
    */
    enum ChangeType { PaletteChanged = 0, FontChanged, StyleChanged,
                      SettingsChanged, IconChanged, CursorChanged,
                      ToolbarStyleChanged, ClipboardConfigChanged,
                      BlockShortcuts, NaturalSortingChanged
                    };

    /**
    * Valid values for the settingsChanged signal
    * @note Copied from KGlobalSettings
    */
    enum SettingsCategory { SETTINGS_MOUSE, SETTINGS_COMPLETION, SETTINGS_PATHS,
                            SETTINGS_POPUPMENU, SETTINGS_QT, SETTINGS_SHORTCUTS,
                            SETTINGS_LOCALE, SETTINGS_STYLE
                          };

    explicit SettingsPortal(QObject *parent);
    ~SettingsPortal();

    typedef QMap<QString, QMap<QString, QVariant> > VariantMapMap;

    uint version() const { return 1; }

public Q_SLOTS:
    void ReadAll(const QStringList &groups);
    void Read(const QString &group, const QString &key);
    //停止录屏
    Q_SCRIPTABLE void stopRecord();

Q_SIGNALS:
    void SettingChanged(const QString &group, const QString &key, const QDBusVariant &value);

private Q_SLOTS:
    void fontChanged();
    void globalSettingChanged(int type, int arg);
    void toolbarStyleChanged();
private:
    QDBusVariant readProperty(const QString &group, const QString &key);

    KSharedConfigPtr m_kdeglobals;
};

#endif // XDG_DESKTOP_PORTAL_KDE_SETTINGS_H


