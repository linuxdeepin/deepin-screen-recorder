// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHOTADAPTOR_H
#define SCREENSHOTADAPTOR_H

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
 * Adaptor class for interface com.deepin.Screenshot
 */
class ScreenshotAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.Screenshot")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.Screenshot\">\n"
"    <method name=\"StartScreenshot\"/>\n"
"    <method name=\"DelayScreenshot\">\n"
"      <arg direction=\"in\" type=\"x\"/>\n"
"    </method>\n"
"    <method name=\"NoNotifyScreenshot\"/>\n"
"    <method name=\"TopWindowScreenshot\"/>\n"
"    <method name=\"FullscreenScreenshot\"/>\n"
"    <method name=\"SavePathScreenshot\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    ScreenshotAdaptor(QObject *parent);
    virtual ~ScreenshotAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void DelayScreenshot(qlonglong in0);
    void FullscreenScreenshot();
    void NoNotifyScreenshot();
    void SavePathScreenshot(const QString &in0);
    void StartScreenshot();
    void TopWindowScreenshot();
Q_SIGNALS: // SIGNALS
};

#endif
