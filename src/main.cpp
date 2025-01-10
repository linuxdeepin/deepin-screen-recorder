// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "main_window.h"
#include "screenshot.h"
#include "utils.h"
#include "widgets/toolbutton.h"
#include "dbusservice/dbusscreenshotservice.h"
#include "accessibility/acObjectList.h"
#include "utils/eventlogutils.h"

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
#include <QStandardPaths>

DWIDGET_USE_NAMESPACE

static bool isWaylandProtocol()
{
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();

    // check is treeland environment.
    if (e.value(QStringLiteral("DDE_CURRENT_COMPOSITOR")) == QStringLiteral("TreeLand")) {
        return false;
    }

    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||  WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive) ;
}

static bool CheckFFmpegEnv()
{
    bool flag = false;
    QDir dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qDebug() << "QLibraryInfo::LibrariesPath: " << path;
    dir.setPath(path);
    qDebug() <<  "where is libs? where is " << dir ;
    QStringList list = dir.entryList(QStringList() << (QString("libavcodec") + "*"), QDir::NoDotAndDotDot | QDir::Files);
    qDebug()  << "Is libavcodec in there?  there is :" << list ;

    if (list.contains("libavcodec.so.58")) {
        qInfo()  << "list contains libavcodec.so.58" ;
        flag = true;
    }

    //x11下需要检测ffmpeg应用是否存在
    if (!isWaylandProtocol()) {
        flag = !QStandardPaths::findExecutable("ffmpeg").isEmpty();
        qInfo() << "Is exists ffmpeg in PATH(" << qgetenv("PATH") << "):" << flag;
    }

    return flag;
}

int main(int argc, char *argv[])
{
    //wayland调试输出
    //qputenv("WAYLAND_DEBUG", "1");
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

#if (QT_VERSION_MAJOR == 5)
    DGuiApplicationHelper::setUseInactiveColorGroup(false);
#endif

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Support V23 or later.
    // FIXME: Reconstruct the toolbar position calculation based on scaleFactor, 
    // or fix the issue of the dbus-send call
    QProcess proc;
    proc.start("dbus-send", {"--print-reply=literal", "--dest=org.deepin.dde.Display1", "/org/deepin/dde/XSettings1",  "org.deepin.dde.XSettings1.GetScaleFactor"});
    proc.waitForFinished();
    QByteArray data = proc.readAllStandardOutput().simplified();
    if (!data.isEmpty()) {
        double factor = data.split(' ').last().toDouble();
        if (factor > 1) {
            qDebug() << "scaleFactor available value: " << factor;
            qputenv("QT_SCALE_FACTOR", QString::number(1 / factor, 'g', 2).toLatin1());
        }
    }

    // 平板模式
    Utils::isTabletEnvironment = false; // DGuiApplicationHelper::isTabletEnvironment();
    //qInfo() << "Is Table:" << Utils::isTabletEnvironment;

    // wayland 协议
    Utils::isWaylandMode = isWaylandProtocol();

    Utils::isRootUser = (getuid() == 0);
    //qInfo() << "Is Root User:" << Utils::isRootUser;

    if (Utils::isWaylandMode) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        QSurfaceFormat format;
        format.setAlphaBufferSize(8);
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
    }

    Utils::isFFmpegEnv = CheckFFmpegEnv();
    qDebug() << "Is FFmpeg Environment:" << Utils::isFFmpegEnv;

    // 适配deepin-turbo 启动加速
#if(DTK_VERSION < DTK_VERSION_CHECK(5,4,0,0))
    DApplication::loadDXcbPlugin();
    QScopedPointer<DApplication> app(new DApplication(argc, argv));
#else
    QScopedPointer<DApplication> app(DApplication::globalApplication(argc, argv));
#endif


    //treeland环境开启，方便可以用命令行参数启动treeland——demo调试
    if (Utils::isWaylandMode || QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive))
        Utils::isTreelandMode = true;


    if (Utils::isWaylandMode) {
#ifdef __mips__
        app->setAttribute(Qt::AA_ForceRasterWidgets, false);
#endif
    }
    // 集成测试标签
#ifdef ENABLE_ACCESSIBILITY
    QAccessible::installFactory(accessibleFactory);
#endif    

    QDBusConnection dbus = QDBusConnection::sessionBus();
    //    dbus.registerService("com.deepin.ScreenRecorder");
    if (dbus.registerService("com.deepin.ScreenRecorder")) {
        app->setOrganizationName("deepin");
        app->setApplicationName("deepin-screen-recorder");
        app->setApplicationVersion("1.0");

#if (QT_VERSION_MAJOR == 5)
    app->setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
        //        static const QDate buildDate = QLocale(QLocale::English).
        //                                       toDate(QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
        //        QString t_date = buildDate.toString("MMdd");

        // Version Time
        app->setApplicationVersion(DApplication::buildVersion(APP_VERSION));

        using namespace Dtk::Core;
        Dtk::Core::DLogManager::registerConsoleAppender();
        Dtk::Core::DLogManager::registerFileAppender();

        QCommandLineOption  delayOption(QStringList() << "d" << "delay", "Take a screenshot after NUM seconds.", "NUM");
        QCommandLineOption fullscreenOption(QStringList() << "f" << "fullscreen", "Take a screenshot the whole screen.");
        QCommandLineOption topWindowOption(QStringList() << "w" << "top-window", "Take a screenshot of the most top window.");
        QCommandLineOption savePathOption(QStringList() << "s" << "save-path", "Specify a path to save the screenshot.", "PATH");
        QCommandLineOption prohibitNotifyOption(QStringList() << "n" << "no-notification", "Don't send notifications.");
        QCommandLineOption useGStreamer(QStringList() << "g" << "gstreamer", "Use GStreamer.");
        QCommandLineOption dbusOption(QStringList() << "u" << "dbus", "Start  from dbus.");
        QCommandLineOption screenRecordFullScreenOption(QStringList() << "rf" << "recordFullScreen","Record full screen","FileNAME","");
        screenRecordFullScreenOption.setFlags(QCommandLineOption::Flag::HiddenFromHelp);
        QCommandLineOption screenRecordOption(QStringList() << "record" << "screenRecord" << "start screen record");
        QCommandLineOption screenShotOption(QStringList() << "shot" << "screenShot" << "start screen shot");
        QCommandLineOption screenOcrOption(QStringList() << "ocr" << "screenOcr" << "start screen ocr");
        QCommandLineOption screenScrollOption(QStringList() << "scroll" << "screenScroll" << "start screen scroll");


        QCommandLineParser cmdParser;
        cmdParser.setApplicationDescription("deepin-screen-recorder");
        cmdParser.addHelpOption();
        cmdParser.addVersionOption();
        cmdParser.addOption(delayOption);
        cmdParser.addOption(fullscreenOption);
        cmdParser.addOption(topWindowOption);
        cmdParser.addOption(savePathOption);
        cmdParser.addOption(prohibitNotifyOption);
        cmdParser.addOption(useGStreamer);
        cmdParser.addOption(dbusOption);
        cmdParser.addOption(screenRecordFullScreenOption);
        cmdParser.addOption(screenRecordOption);
        cmdParser.addOption(screenShotOption);
        cmdParser.addOption(screenOcrOption);
        cmdParser.addOption(screenScrollOption);
        cmdParser.process(*app);

        // Load translator.
        app->loadTranslator();
        Utils::appName = MainWindow::tr("deepin-screen-recorder");
        qInfo() << "Original Application Name is: " << QCoreApplication::applicationName() << ". International Application Name is: " << Utils::appName;

        // 主题设置
        DGuiApplicationHelper::ColorType t_type = DGuiApplicationHelper::instance()->themeType();
        DGuiApplicationHelper::instance()->setPaletteType(t_type);
        Utils::themeType = t_type;

        //qDebug() << "截图录屏日志路径: " << Dtk::Core::DLogManager::getlogFilePath();
        qDebug() << "截图录屏版本: " << DApplication::buildVersion(APP_VERSION);

        //显示系统信息
        Utils::showCurrentSys();

        //qInfo() << "截图录屏日志路径: " << Dtk::Core::DLogManager::getlogFilePath().right(Dtk::Core::DLogManager::getlogFilePath().length() - Dtk::Core::DLogManager::getlogFilePath().indexOf(".cache"));
        qInfo() << "截图录屏版本: " << DApplication::buildVersion(APP_VERSION);
#ifdef KF5_WAYLAND_FLAGE_ON
        qInfo() << "KF5_WAYLAND_FLAGE_ON is open!!";
#else
        qInfo() << "KF5_WAYLAND_FLAGE_ON is close!!";
#endif
        qInfo() << "Is Table:" << Utils::isTabletEnvironment;
        qInfo() << "Is Root User:" << Utils::isRootUser;
        qInfo() << "Is Exists FFmpeg Lib:" << Utils::isFFmpegEnv;
        if (cmdParser.isSet(useGStreamer)) {
            Utils::isFFmpegEnv = false;
        }

        QString t_launchMode = "screenShot";
        if (cmdParser.isSet(screenRecordOption)) {
            t_launchMode = "screenRecord";
        } else if (cmdParser.isSet(screenOcrOption)) {
            t_launchMode = "screenOcr";
        } else if (cmdParser.isSet(screenScrollOption)) {
            t_launchMode = "screenScroll";
        }
        Screenshot window;
        window.initLaunchMode(t_launchMode);
        DBusScreenshotService dbusService(&window);
        // Register debus service.
        dbus.registerObject("/com/deepin/ScreenRecorder", &window, QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportScriptableSlots);
        QDBusConnection conn = QDBusConnection::sessionBus();

        if (!conn.registerService("com.deepin.Screenshot") || !conn.registerObject("/com/deepin/Screenshot", &window)) {
            qDebug() << "deepin-screenshot is running!";
            qApp->quit();
            if (Utils::isWaylandMode) {
                _Exit(0);
            }
            return 0;
        }

        if (cmdParser.isSet(dbusOption)) {
            qDebug() << "dbus register waiting!";
            return app->exec();

        } else {
            QJsonObject obj{
                {"tid", EventLogUtils::Start},
                {"version", QCoreApplication::applicationVersion()},
                {"mode", 1},
                {"startup_mode", "A"}
            };
            if (!cmdParser.isSet(screenRecordOption))
                EventLogUtils::get().writeLogs(obj);
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
            } else if (cmdParser.isSet(screenRecordFullScreenOption)) {
                qDebug() << "screenRecordFullScreenOption!!!!"<< cmdParser.value(screenRecordFullScreenOption);
                window.fullScreenRecord(cmdParser.value(screenRecordFullScreenOption));
            } else {
                window.startScreenshot();
            }
        }

        return app->exec();
    } else {
        //      Send DBus message to stop screen - recorder if found other screen - recorder DBus service has started.
        QDBusInterface notification("com.deepin.ScreenRecorder",
                                    "/com/deepin/ScreenRecorder",
                                    "com.deepin.ScreenRecorder",
                                    QDBusConnection::sessionBus());

        QList<QVariant> arg;
        notification.callWithArgumentList(QDBus::AutoDetect, "stopRecord", arg);
    }

    return 0;
}
