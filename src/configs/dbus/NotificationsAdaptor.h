// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONSADAPTOR_H
#define NOTIFICATIONSADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.freedesktop.Notifications
 */
class NotificationsAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.freedesktop.Notifications\">\n"
"    <method name=\"CloseNotification\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"GetCapbilities\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"GetServerInformation\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"Notify\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"as\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In6\"/>\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"      <arg direction=\"out\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"GetAllRecords\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"RemoveRecord\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"ClearRecords\"/>\n"
"    <signal name=\"NotificationClosed\">\n"
"      <arg type=\"u\"/>\n"
"      <arg type=\"u\"/>\n"
"    </signal>\n"
"    <signal name=\"ActionInvoked\">\n"
"      <arg type=\"u\"/>\n"
"      <arg type=\"s\"/>\n"
"    </signal>\n"
"    <signal name=\"RecordAdded\">\n"
"      <arg type=\"s\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    NotificationsAdaptor(QObject *parent);
    virtual ~NotificationsAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void ClearRecords();
    void CloseNotification(uint in0);
    QString GetAllRecords();
    QStringList GetCapbilities();
    QString GetServerInformation(QString &out1, QString &out2, QString &out3);
    uint Notify(const QString &in0, uint in1, const QString &in2, const QString &in3, const QString &in4, const QStringList &in5, const QVariantMap &in6, int in7);
    void RemoveRecord(const QString &in0);
Q_SIGNALS: // SIGNALS
    void ActionInvoked(uint in0, const QString &in1);
    void NotificationClosed(uint in0, uint in1);
    void RecordAdded(const QString &in0);
};

#endif
