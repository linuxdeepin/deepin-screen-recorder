// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZONEADAPTOR_H
#define ZONEADAPTOR_H

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
 * Adaptor class for interface com.deepin.daemon.Zone
 */
class ZoneAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.Zone")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.daemon.Zone\">\n"
"    <method name=\"BottomLeftAction\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"BottomRightAction\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"EnableZoneDetected\">\n"
"      <arg direction=\"in\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"SetBottomLeft\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"SetBottomRight\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"SetTopLeft\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"SetTopRight\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"TopLeftAction\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"TopRightAction\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    ZoneAdaptor(QObject *parent);
    virtual ~ZoneAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QString BottomLeftAction();
    QString BottomRightAction();
    void EnableZoneDetected(bool in0);
    void SetBottomLeft(const QString &in0);
    void SetBottomRight(const QString &in0);
    void SetTopLeft(const QString &in0);
    void SetTopRight(const QString &in0);
    QString TopLeftAction();
    QString TopRightAction();
Q_SIGNALS: // SIGNALS
};

#endif
