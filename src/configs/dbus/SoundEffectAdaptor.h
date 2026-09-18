// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SOUNDEFFECTADAPTOR_H
#define SOUNDEFFECTADAPTOR_H

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
 * Adaptor class for interface com.deepin.daemon.SoundEffect
 */
class SoundEffectAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.SoundEffect")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.daemon.SoundEffect\">\n"
"    <method name=\"PlaySystemSound\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <property access=\"readwrite\" type=\"b\" name=\"Enabled\"/>\n"
"  </interface>\n"
        "")
public:
    SoundEffectAdaptor(QObject *parent);
    virtual ~SoundEffectAdaptor();

public: // PROPERTIES
    Q_PROPERTY(bool Enabled READ enabled WRITE setEnabled)
    bool enabled() const;
    void setEnabled(bool value);

public Q_SLOTS: // METHODS
    void PlaySystemSound(const QString &in0);
Q_SIGNALS: // SIGNALS
};

#endif
