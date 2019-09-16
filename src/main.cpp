/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
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

#include <DWidget>
#include <DLog>
#include <QDBusConnection>
#include <DApplication>
#include <QCoreApplication>
#include <QDBusInterface>
#include <DWindowManagerHelper>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QDir>
#include "main_window.h"
#include "utils.h"
#include "widgets/toolbutton.h"


DWIDGET_USE_NAMESPACE

static QString g_appPath;//全局路径

//获取配置文件主题类型，并重新设置
DGuiApplicationHelper::ColorType getThemeTypeSetting()
{
    //需要找到自己程序的配置文件路径，并读取配置，这里只是用home路径下themeType.cfg文件举例,具体配置文件根据自身项目情况
    QString t_appDir = g_appPath + QDir::separator() + "themetype.cfg";
    QFile t_configFile(t_appDir);

    t_configFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray t_readBuf = t_configFile.readAll();
    int t_readType = QString(t_readBuf).toInt();

    //获取读到的主题类型，并返回设置
    switch (t_readType) {
    case 0:
        // 跟随系统主题
        return DGuiApplicationHelper::UnknownType;
    case 1:
//        浅色主题
        return DGuiApplicationHelper::LightType;

    case 2:
//        深色主题
        return DGuiApplicationHelper::DarkType;
    default:
        // 跟随系统主题
        return DGuiApplicationHelper::UnknownType;
    }

}

//保存当前主题类型配置文件
void saveThemeTypeSetting(int type)
{
    //需要找到自己程序的配置文件路径，并写入配置，这里只是用home路径下themeType.cfg文件举例,具体配置文件根据自身项目情况
    QString t_appDir = g_appPath + QDir::separator() + "themetype.cfg";
    QFile t_configFile(t_appDir);

    t_configFile.open(QIODevice::WriteOnly | QIODevice::Text);
    //直接将主题类型保存到配置文件，具体配置key-value组合根据自身项目情况
    QString t_typeStr = QString::number(type);
    t_configFile.write(t_typeStr.toUtf8());
    t_configFile.close();
}


int main(int argc, char *argv[])
{

    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Construct a QGuiApplication before accessing a platform function.
    DGuiApplicationHelper::setUseInactiveColorGroup(false);

    DApplication::loadDXcbPlugin();
    DApplication app(argc, argv);
    QString t_launchMode;

    if (argc >= 2) {
        t_launchMode = QString(argv[1]);
//        qDebug() << t_launchMode;
    }

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService("com.deepin.ScreenRecorder")) {
        // Poup up warning dialog if window manager not support composite.
        if (!DWindowManagerHelper::instance()->hasComposite()) {
            Utils::warnNoComposite();
            return 0;
        }
        // Start screen-recorder process if not other screen-recorder DBus service started.
        else {
            // Init application attributes.

            g_appPath = QDir::homePath() + QDir::separator() + "." + qApp->applicationName();
            QDir t_appDir;
            t_appDir.mkpath(g_appPath);

            app.setOrganizationName("deepin");
            app.setApplicationName("deepin-screen-recorder");
            app.setApplicationVersion("1.0");

            static const QDate buildDate = QLocale( QLocale::English ).
                                           toDate( QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
            QString t_date = buildDate.toString("MMdd");

            // Version Time
            app.setApplicationVersion(DApplication::buildVersion(t_date));

            using namespace Dtk::Core;
            Dtk::Core::DLogManager::registerConsoleAppender();
            Dtk::Core::DLogManager::registerFileAppender();

            QCommandLineOption  delayOption(QStringList() << "d" << "delay",
                                            "Take a screenshot after NUM seconds.", "NUM");
            QCommandLineOption fullscreenOption(QStringList() << "f" << "fullscreen",
                                                "Take a screenshot the whole screen.");
            QCommandLineOption topWindowOption(QStringList() << "w" << "top-window",
                                               "Take a screenshot of the most top window.");
            QCommandLineOption savePathOption(QStringList() << "s" << "save-path",
                                              "Specify a path to save the screenshot.", "PATH");
            QCommandLineOption prohibitNotifyOption(QStringList() << "n" << "no-notification",
                                                    "Don't send notifications.");
            QCommandLineOption iconOption(QStringList() << "i" << "icon",
                                          "Indicate that this program's started by clicking.");
            QCommandLineOption dbusOption(QStringList() << "u" << "dbus",
                                          "Start  from dbus.");
            QCommandLineParser cmdParser;
            cmdParser.setApplicationDescription("deepin-screenshot");
            cmdParser.addHelpOption();
            cmdParser.addVersionOption();
            cmdParser.addOption(delayOption);
            cmdParser.addOption(fullscreenOption);
            cmdParser.addOption(topWindowOption);
            cmdParser.addOption(savePathOption);
            cmdParser.addOption(prohibitNotifyOption);
            cmdParser.addOption(iconOption);
            cmdParser.addOption(dbusOption);
            cmdParser.process(app);

            // Load translator.
            app.loadTranslator();

            // 应用已保存的主题设置
            DGuiApplicationHelper::instance()->setPaletteType(getThemeTypeSetting());

            //监听当前应用主题切换事件
            QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
            [] (DGuiApplicationHelper::ColorType type) {
                qDebug() << type;
                // 保存程序的主题设置  type : 0,系统主题， 1,浅色主题， 2,深色主题
                saveThemeTypeSetting(type);
            });


            // Show window.
            MainWindow window;

            window.initLaunchMode(t_launchMode);

            window.showFullScreen();
            // window.show();
            window.initResource();

            // Register debus service.
            dbus.registerObject("/com/deepin/ScreenRecorder", &window, QDBusConnection::ExportScriptableSlots);

            QDBusConnection conn = QDBusConnection::sessionBus();
            if (!conn.registerService("com.deepin.Screenshot") ||
                    !conn.registerObject("/com/deepin/Screenshot", &window)) {
                qDebug() << "deepin-screenshot is running!";

                qApp->quit();
                return 0;
            }

            return app.exec();
        }
    } else {
        // Send DBus message to stop screen-recorder if found other screen-recorder DBus service has started.
        QDBusInterface notification("com.deepin.ScreenRecorder",
                                    "/com/deepin/ScreenRecorder",
                                    "com.deepin.ScreenRecorder",
                                    QDBusConnection::sessionBus());

        QList<QVariant> arg;
        notification.callWithArgumentList(QDBus::AutoDetect, "stopRecord", arg);
    }

    return 0;
}
