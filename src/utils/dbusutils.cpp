// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusutils.h"
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusObjectPath>

DBusUtils::DBusUtils()
{
    qCDebug(dsrApp) << "DBusUtils constructor called.";
}

DBusUtils::~DBusUtils()
{
    qCDebug(dsrApp) << "DBusUtils destructor called.";
}

QVariant DBusUtils::redDBusProperty(const QString &service, const QString &path, const QString &interface, const char *propert)
{
    qCDebug(dsrApp) << "Reading DBus property. Service: " << service << ", Path: " << path << ", Interface: " << interface << ", Property: " << propert;
    // 创建QDBusInterface接口
    QDBusInterface ainterface(service, path,
                              interface,
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
        qCDebug(dsrApp) << "DBus interface is not valid. Error: " << QDBusConnection::sessionBus().lastError().message();
        QVariant v(0) ;
        return  v;
    }
    //调用远程的value方法
    QVariant v = ainterface.property(propert);
    qCDebug(dsrApp) << "Returning DBus property value: " << v;
    return  v;
}
QVariant DBusUtils::redDBusMethod(const QString &service, const QString &path, const QString &interface, const char *method)
{
    qCDebug(dsrApp) << "Calling DBus method. Service: " << service << ", Path: " << path << ", Interface: " << interface << ", Method: " << method;
    // 创建QDBusInterface接口
    QDBusInterface ainterface(service, path,
                              interface,
                              QDBusConnection::sessionBus());
    if (!ainterface.isValid()) {
        qCDebug(dsrApp) << "DBus interface is not valid. Error: " << QDBusConnection::sessionBus().lastError().message();
        QVariant v(0) ;
        return  v;
    }
    //调用远程的value方法
    QDBusReply<QDBusVariant> reply = ainterface.call(method);
    if (reply.isValid()) {
        qCDebug(dsrApp) << "DBus method call successful. Returning value.";
//        return reply.value();
        QVariant v(0) ;
        return  v;
    } else {
        qCDebug(dsrApp) << "DBus method call failed. Error: " << QDBusConnection::sessionBus().lastError().message();
        QVariant v(0) ;
        return  v;
    }
}
