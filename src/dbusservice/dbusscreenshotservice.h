/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBUSSCREENSHOTSERVICE_H
#define DBUSSCREENSHOTSERVICE_H

#include "../screenshot.h"

#include <QDBusAbstractAdaptor>
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
class DBusScreenshotService: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.Screenshot")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"com.deepin.Screenshot\">\n"
                "    <method name=\"StartScreenshot\"/>\n"
                "    <method name=\"DelayScreenshot\">\n"
                "      <arg direction=\"in\" type=\"x\"/>\n"
                "   </method>\n"
                "    <method name=\"NoNotifyScreenshot\"/>\n"
                "    <method name=\"TopWindowScreenshot\"/>\n"
                "    <method name=\"FullscreenScreenshot\"/>\n"
                "    <method name=\"OcrScreenshot\"/>\n"
                "    <method name=\"ScrollScreenshot\"/>\n"
                "    <method name=\"SavePathScreenshot\">\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "    </method>\n"
                "    <method name=\"StartScreenshotFor3rd\">\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "    </method>\n"
                "    <signal name=\"Done\">\n"
                "      <arg type=\"s\"/>\n"
                "    </signal>\n"
                "  </interface>\n"
                "")
public:
    explicit DBusScreenshotService(Screenshot *parent);
    ~DBusScreenshotService();

    void setSingleInstance(bool instance);

    inline Screenshot *parent() const
    {
        return static_cast<Screenshot *>(QObject::parent());
    }

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void StartScreenshot();
    void DelayScreenshot(qlonglong in0);
    void NoNotifyScreenshot();
    void OcrScreenshot();
    void ScrollScreenshot();
    void TopWindowScreenshot();
    void FullscreenScreenshot();
    void SavePathScreenshot(const QString &in0);
    void StartScreenshotFor3rd(const QString &in0);
Q_SIGNALS: // SIGNALS
    void Done(const QString &in0);
private:
    bool m_singleInstance;
};

#endif
