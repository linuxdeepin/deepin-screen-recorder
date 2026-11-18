// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusutils.h"
#include "log.h"
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusConnectionInterface>
#include <QByteArray>
#include <QCoreApplication>

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


// QDBusConnection::sessionBus() caches a single process-wide connection.
// In the persistent --dbus process this cache is never refreshed after an
// account switch, so the stale connection still points at the old session
// bus where com.deepin.copilot is absent.  This helper builds a brand-new
// connection from the *current* DBUS_SESSION_BUS_ADDRESS every time it is
// called, ensuring the AI assistant check and D-Bus calls hit the right bus.
QDBusConnection DBusUtils::freshSessionBusConnection()
{
    QByteArray addr = qgetenv("DBUS_SESSION_BUS_ADDRESS");
    QString connName = QStringLiteral("ai_check_%1").arg(QCoreApplication::applicationPid());
    QDBusConnection::disconnectFromBus(connName);
    if (addr.isEmpty()) {
        return QDBusConnection(connName);
    }
    return QDBusConnection::connectToBus(QString::fromUtf8(addr), connName);
}

bool DBusUtils::isAiAssistantAvailable()
{
    QDBusConnection bus = DBusUtils::freshSessionBusConnection();
    if (!bus.isConnected()) return false;


    QDBusInterface copilot("com.deepin.copilot",
                               "/com/deepin/copilot",
                               "com.deepin.copilot",
                               bus);
    copilot.call("version");


    auto hasMethods = [&](const QString &service) -> bool {
        QDBusInterface intros(service,
                              "/com/deepin/copilot",
                              "org.freedesktop.DBus.Introspectable",
                              bus);
        if (!intros.isValid()) return false;
        QDBusReply<QString> xml = intros.call("Introspect");
        if (!xml.isValid()) return false;
        const QString xmlStr = xml.value();
        return xmlStr.contains("launchAiQuickOCR") && xmlStr.contains("launchChatUploadImage");
    };

    // 优先检测 deepin.copilot，再回退 iflytek
    if (hasMethods("com.deepin.copilot")) return true;
    if (hasMethods("com.iflytek.aiassistant")) return true;

    return false;
}
