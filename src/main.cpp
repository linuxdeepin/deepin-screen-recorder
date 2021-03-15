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

#include "main_window.h"
#include "screenshot.h"
#include "utils.h"
#include "widgets/toolbutton.h"
#include "dbusservice/dbusscreenshotservice.h"
#include "accessibility/acObjectList.h"

#include <DWidget>
#include <DLog>
#include <DWindowManagerHelper>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <DApplication>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QDir>
//#include <sanitizer/asan_interface.h>


DWIDGET_USE_NAMESPACE
/*
 * 截图录屏不能从界面设置主题，不存在主题设置，保存设置的主题过程
 * 初始化利用系统主题来设置
static QString g_appPath;//全局路径
static int g_themeType = 0;

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
        g_themeType = 0;
        return DGuiApplicationHelper::UnknownType;
    case 1:
//        浅色主题
        g_themeType = 1;
        return DGuiApplicationHelper::LightType;

    case 2:
//        深色主题
        g_themeType = 2;
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
    g_themeType = type;

    t_configFile.open(QIODevice::WriteOnly | QIODevice::Text);
    //直接将主题类型保存到配置文件，具体配置key-value组合根据自身项目情况
    QString t_typeStr = QString::number(type);
    t_configFile.write(t_typeStr.toUtf8());
    t_configFile.close();
}

*/
int main(int argc, char *argv[])
{

//    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Construct a QGuiApplication before accessing a platform function.
    DGuiApplicationHelper::setUseInactiveColorGroup(false);
    Utils::isTabletEnvironment = DGuiApplicationHelper::isTabletEnvironment();
    //Utils::isTabletEnvironment = false;
    qDebug() << Utils::isTabletEnvironment;

    // 内存检测标签
//#ifdef ENABLE_TSAN_TOOL
//    __sanitizer_set_report_path("./asan.log");
//#endif

// 适配deepin-turbo 启动加速
#if(DTK_VERSION < DTK_VERSION_CHECK(5,4,0,0))
    DApplication::loadDXcbPlugin();
    QScopedPointer<DApplication> app(new DApplication(argc, argv));
#else
    QScopedPointer<DApplication> app(DApplication::globalApplication(argc,argv));
#endif

    // 集成测试标签
#ifdef ENABLE_ACCESSIBILITY
        QAccessible::installFactory(accessibleFactory);
#endif
    /*
    QString t_launchMode;

    if (argc >= 2) {
        t_launchMode = QString(argv[1]);
//        qDebug() << t_launchMode;
    }
    */

    QDBusConnection dbus = QDBusConnection::sessionBus();
//    dbus.registerService("com.deepin.ScreenRecorder");
    if (dbus.registerService("com.deepin.ScreenRecorder")) {
        // Poup up warning dialog if window manager not support composite.
        //if (!DWindowManagerHelper::instance()->hasComposite() && t_launchMode == "screenRecord") {
            //Utils::warnNoComposite();
            //return 0;
        //}
        // Start screen-recorder process if not other screen-recorder DBus service started.
        //else {
            // Init application attributes.

            //g_appPath = QDir::homePath() + QDir::separator() + "." + qApp->applicationName();
            //QDir t_appDir;
            //t_appDir.mkpath(g_appPath);

            app->setOrganizationName("deepin");
            app->setApplicationName("deepin-screen-recorder");
            app->setApplicationVersion("1.0");
            app->setAttribute(Qt::AA_UseHighDpiPixmaps);

            static const QDate buildDate = QLocale( QLocale::English ).
                                           toDate( QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
            QString t_date = buildDate.toString("MMdd");

            // Version Time
            app->setApplicationVersion(DApplication::buildVersion(t_date));

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
            QCommandLineOption screenRecordOption(QStringList() << "record" << "screenRecord" << "start screen record");
            QCommandLineOption screenShotOption(QStringList() << "shot" << "screenShot" << "start screen shot");
            //for test
//            QCommandLineOption testOption(QStringList() << "t" << "test",
//                                          "Start  from test.");
            //for test
            QCommandLineParser cmdParser;
            cmdParser.setApplicationDescription("deepin-screen-recorder");
            cmdParser.addHelpOption();
            cmdParser.addVersionOption();
            cmdParser.addOption(delayOption);
            cmdParser.addOption(fullscreenOption);
            cmdParser.addOption(topWindowOption);
            cmdParser.addOption(savePathOption);
            cmdParser.addOption(prohibitNotifyOption);
            cmdParser.addOption(iconOption);
            cmdParser.addOption(dbusOption);
            cmdParser.addOption(screenRecordOption);
            cmdParser.addOption(screenShotOption);
            cmdParser.process(*app);

            // Load translator.
            app->loadTranslator();
            // Show window.
//            MainWindow window;
            Screenshot window;

            // 应用已保存的主题设置
            DGuiApplicationHelper::ColorType t_type = DGuiApplicationHelper::instance()->themeType();
            //saveThemeTypeSetting(t_type);

            window.setConfigThemeType(t_type);

            DGuiApplicationHelper::instance()->setPaletteType(t_type);

            //监听当前应用主题切换事件
            /* 截图过程中不存在切换主题的操作
            QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
            [] (DGuiApplicationHelper::ColorType type) {
                qDebug() << type;
                // 保存程序的主题设置  type : 0,系统主题， 1,浅色主题， 2,深色主题
                saveThemeTypeSetting(type);
                DGuiApplicationHelper::instance()->setPaletteType(type);
            });
            */
            const QString& t_launchMode = cmdParser.isSet(screenRecordOption) ? "screenRecord" : "screenShot";
            window.initLaunchMode(t_launchMode);
            DBusScreenshotService dbusService (&window);
            /*
            if (!DWindowManagerHelper::instance()->hasComposite() && t_launchMode == "screenRecord") {
                Utils::warnNoComposite();
                qApp->quit();
                return 0;
            }
            */
            // Register debus service.
            dbus.registerObject("/com/deepin/ScreenRecorder", &window, QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportScriptableSlots);
            QDBusConnection conn = QDBusConnection::sessionBus();
//            conn.unregisterObject("com.deepin.Screenshot");
            if (!conn.registerService("com.deepin.Screenshot") ||
                    !conn.registerObject("/com/deepin/Screenshot", &window)) {
                qDebug() << "deepin-screenshot is running!";

                qApp->quit();
                return 0;
            }


            if (cmdParser.isSet(dbusOption)) {
                qDebug() << "dbus register waiting!";
                return app->exec();

            } else {
                dbusService.setSingleInstance(true);
                if (cmdParser.isSet(delayOption)) {
                    qDebug() << "cmd delay screenshot";
                    window.delayScreenshot(cmdParser.value(delayOption).toInt());
                } else if (cmdParser.isSet(fullscreenOption)) {
                    window.fullscreenScreenshot();
                } else if (cmdParser.isSet(topWindowOption)) {
                    qDebug() << "cmd topWindow screenshot";
                    window.topWindowScreenshot();
                } else if (cmdParser.isSet(savePathOption)) {
                    qDebug() << "cmd savepath screenshot";
                    window.savePathScreenshot(cmdParser.value(savePathOption));
                } else if (cmdParser.isSet(prohibitNotifyOption)) {
                    qDebug() << "screenshot no notify!";
                    window.noNotifyScreenshot();
                } else if (cmdParser.isSet(iconOption)) {
                    window.delayScreenshot(0.2);
                } /*else if (cmdParser.isSet(testOption)) {
                    window.testScreenshot();
                }*/ else {
                    window.startScreenshot();
                }
            }

            return app->exec();
        //}
    } else {
//        Send DBus message to stop screen - recorder if found other screen - recorder DBus service has started.
        QDBusInterface notification("com.deepin.ScreenRecorder",
                                    "/com/deepin/ScreenRecorder",
                                    "com.deepin.ScreenRecorder",
                                    QDBusConnection::sessionBus());

        QList<QVariant> arg;
        notification.callWithArgumentList(QDBus::AutoDetect, "stopRecord", arg);
    }

    return 0;
}
