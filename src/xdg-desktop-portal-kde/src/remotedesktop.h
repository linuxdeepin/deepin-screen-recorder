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

#ifndef XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_H
#define XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class ScreenCastCommon;

class RemoteDesktopPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.RemoteDesktop")
public:
    explicit RemoteDesktopPortal(QObject *parent);
    ~RemoteDesktopPortal();

    enum DeviceType {
        None = 0x0,
        Keyboard = 0x1,
        Pointer = 0x2,
        TouchScreen = 0x4,
        All = (Keyboard | Pointer | TouchScreen)
    };
    Q_DECLARE_FLAGS(DeviceTypes, DeviceType)

    uint version() const { return 1; }
    uint AvailableDeviceTypes() const { return All; };

public Q_SLOTS:
    uint CreateSession(const QDBusObjectPath &handle,
                       const QDBusObjectPath &session_handle,
                       const QString &app_id,
                       const QVariantMap &options,
                       QVariantMap &results);

    uint SelectDevices(const QDBusObjectPath &handle,
                       const QDBusObjectPath &session_handle,
                       const QString &app_id,
                       const QVariantMap &options,
                       QVariantMap &results);

    uint Start(const QDBusObjectPath &handle,
               const QDBusObjectPath &session_handle,
               const QString &app_id,
               const QString &parent_window,
               const QVariantMap &options,
               QVariantMap &results);

    void NotifyPointerMotion(const QDBusObjectPath &session_handle,
                             const QVariantMap &options,
                             double dx,
                             double dy);

    void NotifyPointerMotionAbsolute(const QDBusObjectPath &session_handle,
                             const QVariantMap &options,
                             uint stream,
                             double dx,
                             double dy);

    void NotifyPointerButton(const QDBusObjectPath &session_handle,
                             const QVariantMap &options,
                             int button,
                             uint state);

    void NotifyPointerAxis(const QDBusObjectPath &session_handle,
                           const QVariantMap &options,
                           double dx,
                           double dy);

    void NotifyPointerAxisDiscrete(const QDBusObjectPath &session_handle,
                                   const QVariantMap &options,
                                   uint axis,
                                   int steps);

    void NotifyKeyboardKeycode(const QDBusObjectPath &session_handle,
                               const QVariantMap &options,
                               int keycode,
                               uint state);

    void NotifyKeyboardKeysym(const QDBusObjectPath &session_handle,
                              const QVariantMap &options,
                              int keysym,
                              uint state);

    void NotifyTouchDown(const QDBusObjectPath &session_handle,
                         const QVariantMap &options,
                         uint stream,
                         uint slot,
                         int x,
                         int y);

    void NotifyTouchMotion(const QDBusObjectPath &session_handle,
                           const QVariantMap &options,
                           uint stream,
                           uint slot,
                           int x,
                           int y);

    void NotifyTouchUp(const QDBusObjectPath &session_handle,
                       const QVariantMap &options,
                       uint slot);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(RemoteDesktopPortal::DeviceTypes)

#endif // XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_H

