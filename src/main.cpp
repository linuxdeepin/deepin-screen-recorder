// SPDX-FileCopyrightText: 2011 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "main_window.h"
#include "screenshot.h"
#include "utils.h"
#include "widgets/toolbutton.h"
#include "dbusservice/dbusscreenshotservice.h"
#include "accessibility/acObjectList.h"
#include "utils/eventlogutils.h"
#include "utils/x_multi_screen_info.h"
#include "utils/log.h"

#include <DWidget>
#include <DLog>
#include <DWindowManagerHelper>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DPlatformTheme>

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
    qCDebug(dsrApp) << "isWaylandProtocol called.";
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    qCDebug(dsrApp) << "System environment retrieved.";

    // check is treeland environment.
    if (e.value(QStringLiteral("DDE_CURRENT_COMPOSITOR")) == QStringLiteral("TreeLand")) {
        qCDebug(dsrApp) << "DDE_CURRENT_COMPOSITOR is TreeLand, returning false.";
        return false;
    }

    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    bool isWayland = XDG_SESSION_TYPE == QLatin1String("wayland") ||
           WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
    qCDebug(dsrApp) << "XDG_SESSION_TYPE:" << XDG_SESSION_TYPE << ", WAYLAND_DISPLAY:" << WAYLAND_DISPLAY << ", isWayland:" << isWayland;
    return isWayland;
}

static bool CheckFFmpegEnv()
{
    qCDebug(dsrApp) << "CheckFFmpegEnv called.";
    bool flag = false;
    QDir dir;
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qDebug() << "QLibraryInfo::LibrariesPath: " << path;
    qCDebug(dsrApp) << "Libraries path:" << path;
    dir.setPath(path);
    qDebug() << "where is libs? where is " << dir;
    QStringList list = dir.entryList(QStringList() << (QString("libavcodec") + "*"), QDir::NoDotAndDotDot | QDir::Files);
    qDebug() << "Is libavcodec in there?  there is :" << list;
    qCDebug(dsrApp) << "Found libavcodec files:" << list;

    if (list.contains("libavcodec.so.58")) {
        qInfo() << "list contains libavcodec.so.58";
        qCDebug(dsrApp) << "libavcodec.so.58 found.";
        flag = true;
    }

    // x11下需要检测ffmpeg应用是否存在
    if (!isWaylandProtocol()) {
        qCDebug(dsrApp) << "Not Wayland protocol, checking for ffmpeg executable.";
        flag = !QStandardPaths::findExecutable("ffmpeg").isEmpty();
        qInfo() << "Is exists ffmpeg in PATH(" << qgetenv("PATH") << "):" << flag;
        qCDebug(dsrApp) << "FFmpeg executable existence check:" << flag;
    }
    qCDebug(dsrApp) << "CheckFFmpegEnv finished. Returning:" << flag;
    return flag;
}

void resetScaleFactor()
{
    qCDebug(dsrApp) << "resetScaleFactor called.";
    // Support V23 or later.
    // FIXME: Reconstruct the toolbar position calculation based on scaleFactor,
    // or fix the issue of the dbus-send call
    QProcess proc;
    proc.start("dbus-send",
               {"--print-reply=literal",
                "--dest=org.deepin.dde.Display1",
                "/org/deepin/dde/XSettings1",
                "org.deepin.dde.XSettings1.GetScaleFactor"});
    proc.waitForFinished();
    QByteArray data = proc.readAllStandardOutput().simplified();
    if (!data.isEmpty()) {
        double factor = data.split(' ').last().toDouble();
        if (factor > 1) {
            qDebug() << "scaleFactor available value: " << factor;
            qCDebug(dsrApp) << "Scale factor is greater than 1:" << factor << ", setting QT_SCALE_FACTOR.";
            qputenv("QT_SCALE_FACTOR", QString::number(1 / factor, 'g', 2).toLatin1());
        }
    }
    qCDebug(dsrApp) << "resetScaleFactor finished.";
}

int main(int argc, char *argv[])
{
    qCDebug(dsrApp) << "main function started.";
    // wayland调试输出
    // qputenv("WAYLAND_DEBUG", "1");
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
        qCDebug(dsrApp) << "XDG_CURRENT_DESKTOP not Deepin, setting to Deepin.";
    }

#if (QT_VERSION_MAJOR == 5)
    DGuiApplicationHelper::setUseInactiveColorGroup(false);
    qCDebug(dsrApp) << "Qt5: Inactive color group disabled.";
#endif

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    qCDebug(dsrApp) << "High DPI scaling enabled.";

    // 平板模式
    // Utils::isTabletEnvironment = DGuiApplicationHelper::isTabletEnvironment();
    // wayland 协议
    Utils::isWaylandMode = isWaylandProtocol();
    Utils::isRootUser = (getuid() == 0);
    qCDebug(dsrApp) << "isWaylandMode:" << Utils::isWaylandMode << ", isRootUser:" << Utils::isRootUser;

    if (Utils::isWaylandMode) {
        qCDebug(dsrApp) << "Wayland mode active.";
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        QSurfaceFormat format;
        format.setAlphaBufferSize(8);
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
        qCDebug(dsrApp) << "Wayland shell integration and surface format set.";
    } else {
        qCDebug(dsrApp) << "X11 mode active.";
        // check if need reset scale in multi-screen.
        Utils::forceResetScale = XMultiScreenInfo::screenNeedResetScale();
        if (Utils::forceResetScale) {
            qCDebug(dsrApp) << "Screen needs reset scale, calling resetScaleFactor.";
            resetScaleFactor();
        }
    }

    Utils::isFFmpegEnv = CheckFFmpegEnv();
    qDebug() << "Is FFmpeg Environment:" << Utils::isFFmpegEnv;
    qCDebug(dsrApp) << "FFmpeg environment check complete:" << Utils::isFFmpegEnv;

    // 适配deepin-turbo 启动加速
#if (DTK_VERSION < DTK_VERSION_CHECK(5, 4, 0, 0))
    DApplication::loadDXcbPlugin();
    QScopedPointer<DApplication> app(new DApplication(argc, argv));
    qCDebug(dsrApp) << "DTK version less than 5.4.0, loading DXcbPlugin and creating new DApplication.";
#else
    QScopedPointer<DApplication> app(DApplication::globalApplication(argc, argv));
    qCDebug(dsrApp) << "DTK version 5.4.0 or greater, getting global DApplication.";
#endif

    // treeland环境开启，方便可以用命令行参数启动treeland——demo调试
    if (Utils::isWaylandMode || QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive)) {
        Utils::isTreelandMode = true;
        qCDebug(dsrApp) << "Wayland or platform name starts with wayland, setting isTreelandMode to true.";
    }

    if (Utils::isWaylandMode) {
#ifdef __mips__
        app->setAttribute(Qt::AA_ForceRasterWidgets, false);
        qCDebug(dsrApp) << "MIPS architecture and Wayland mode, AA_ForceRasterWidgets set to false.";
#endif
    }
    // 集成测试标签
#ifdef ENABLE_ACCESSIBILITY
    QAccessible::installFactory(accessibleFactory);
    qCDebug(dsrApp) << "Accessibility factory installed.";
#endif

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService("com.deepin.ScreenRecorder")) {
        qCDebug(dsrApp) << "DBus service com.deepin.ScreenRecorder registered successfully.";
        app->setOrganizationName("deepin");
        app->setApplicationName("deepin-screen-recorder");
        app->setApplicationVersion("1.0");
        qCDebug(dsrApp) << "Application organization, name, and version set.";

#if (QT_VERSION_MAJOR == 5)
        app->setAttribute(Qt::AA_UseHighDpiPixmaps);
        qCDebug(dsrApp) << "Qt5: AA_UseHighDpiPixmaps enabled.";
#endif

        // Version Time
        app->setApplicationVersion(DApplication::buildVersion(APP_VERSION));
        qCDebug(dsrApp) << "Application build version set.";

        using namespace Dtk::Core;
        Dtk::Core::DLogManager::registerConsoleAppender();
        Dtk::Core::DLogManager::registerFileAppender();
        qCDebug(dsrApp) << "Dtk LogManager console and file appenders registered.";

        QCommandLineOption delayOption(QStringList() << "d" << "delay", "Take a screenshot after NUM seconds.", "NUM");
        QCommandLineOption fullscreenOption(QStringList() << "f" << "fullscreen", "Take a screenshot the whole screen.");
        QCommandLineOption topWindowOption(QStringList() << "w" << "top-window", "Take a screenshot of the most top window.");
        QCommandLineOption savePathOption(QStringList() << "s" << "save-path", "Specify a path to save the screenshot.", "PATH");
        QCommandLineOption prohibitNotifyOption(QStringList() << "n" << "no-notification", "Don't send notifications.");
        QCommandLineOption useGStreamer(QStringList() << "g" << "gstreamer", "Use GStreamer.");
        QCommandLineOption dbusOption(QStringList() << "u" << "dbus", "Start  from dbus.");
        QCommandLineOption screenRecordFullScreenOption(
            QStringList() << "rf" << "recordFullScreen", "Record full screen", "FileNAME", "");
        screenRecordFullScreenOption.setFlags(QCommandLineOption::Flag::HiddenFromHelp);
        QCommandLineOption screenRecordOption(QStringList() << "record" << "screenRecord" << "start screen record");
        QCommandLineOption screenShotOption(QStringList() << "shot" << "screenShot" << "start screen shot");
        QCommandLineOption screenOcrOption(QStringList() << "ocr" << "screenOcr" << "start screen ocr");
        QCommandLineOption screenScrollOption(QStringList() << "scroll" << "screenScroll" << "start screen scroll");
        qCDebug(dsrApp) << "Command line options defined.";

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
        qCDebug(dsrApp) << "Command line parser initialized and processed application arguments.";

        // Load translator.
        app->loadTranslator();
        Utils::appName = MainWindow::tr("deepin-screen-recorder");
        qInfo() << "Original Application Name is: " << QCoreApplication::applicationName()
                << ". International Application Name is: " << Utils::appName;
        qCDebug(dsrApp) << "Translator loaded and application name set.";

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Use system theme type instead of app cached theme type
        if (auto systemTheme = DGuiApplicationHelper::instance()->systemTheme()) {
            DPalette sysPalette = DGuiApplicationHelper::fetchPalette(systemTheme);
            DGuiApplicationHelper::instance()->setApplicationPalette(sysPalette);
            Utils::themeType = DGuiApplicationHelper::toColorType(sysPalette);
            qCDebug(dsrApp) << "Qt6: System theme applied.";
        }
#else
        DGuiApplicationHelper::ColorType t_type = DGuiApplicationHelper::instance()->themeType();
        DGuiApplicationHelper::instance()->setPaletteType(t_type);
        Utils::themeType = t_type;
        qCDebug(dsrApp) << "Qt5: Application palette and theme type set.";
#endif

        // 显示系统信息
        Utils::showCurrentSys();
        qCDebug(dsrApp) << "Current system information displayed.";

        qInfo() << "截图录屏版本: " << DApplication::buildVersion(APP_VERSION);
#ifdef KF5_WAYLAND_FLAGE_ON
        qInfo() << "KF5_WAYLAND_FLAGE_ON is open!!";
        qCDebug(dsrApp) << "KF5_WAYLAND_FLAGE_ON is defined.";
#else
        qInfo() << "KF5_WAYLAND_FLAGE_ON is close!!";
        qCDebug(dsrApp) << "KF5_WAYLAND_FLAGE_ON is not defined.";
#endif
        qInfo() << "Is Table:" << Utils::isTabletEnvironment;
        qInfo() << "Is Root User:" << Utils::isRootUser;
        qInfo() << "Is Exists FFmpeg Lib:" << Utils::isFFmpegEnv;
        qCDebug(dsrApp) << "Tablet environment:" << Utils::isTabletEnvironment << ", Root user:" << Utils::isRootUser << ", FFmpeg Lib exists:" << Utils::isFFmpegEnv;
        if (cmdParser.isSet(useGStreamer)) {
            Utils::isFFmpegEnv = false;
            qCDebug(dsrApp) << "useGStreamer option set, isFFmpegEnv set to false.";
        }

        QString t_launchMode = "screenShot";
        if (cmdParser.isSet(screenRecordOption)) {
            t_launchMode = "screenRecord";
            qCDebug(dsrApp) << "Launch mode set to screenRecord.";
        } else if (cmdParser.isSet(screenOcrOption)) {
            t_launchMode = "screenOcr";
            qCDebug(dsrApp) << "Launch mode set to screenOcr.";
        } else if (cmdParser.isSet(screenScrollOption)) {
            t_launchMode = "screenScroll";
            qCDebug(dsrApp) << "Launch mode set to screenScroll.";
        }
        qCDebug(dsrApp) << "Final launch mode:" << t_launchMode;
        Screenshot window;
        window.initLaunchMode(t_launchMode);
        qCDebug(dsrApp) << "Screenshot window initialized with launch mode.";
        DBusScreenshotService dbusService(&window);
        // Register debus service.
        dbus.registerObject("/com/deepin/ScreenRecorder",
                            &window,
                            QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportScriptableSlots);
        qCDebug(dsrApp) << "DBus object /com/deepin/ScreenRecorder registered.";
        QDBusConnection conn = QDBusConnection::sessionBus();

        if (!conn.registerService("com.deepin.Screenshot") || !conn.registerObject("/com/deepin/Screenshot", &window)) {
            qDebug() << "deepin-screenshot is running!";
            qCDebug(dsrApp) << "deepin-screenshot service already running. Quitting app.";
            qApp->quit();
            if (Utils::isWaylandMode) {
                _Exit(0);
                qCDebug(dsrApp) << "Exiting process for Wayland mode.";
            }
            return 0;
        }
        qCDebug(dsrApp) << "deepin-screenshot service registered successfully.";

        if (cmdParser.isSet(dbusOption)) {
            qDebug() << "dbus register waiting!";
            qCDebug(dsrApp) << "DBus option set, waiting for DBus registration.";
            return app->exec();

        } else {
            qCDebug(dsrApp) << "DBus option not set, processing screenshot command line options.";
            QJsonObject obj{{"tid", EventLogUtils::Start},
                            {"version", QCoreApplication::applicationVersion()},
                            {"mode", 1},
                            {"startup_mode", "A"}};
            if (!cmdParser.isSet(screenRecordOption)) {
                EventLogUtils::get().writeLogs(obj);
                qCDebug(dsrApp) << "Screen record option not set, writing start logs.";
            }
            dbusService.setSingleInstance(true);
            qCDebug(dsrApp) << "DBus service set to single instance.";
            if (cmdParser.isSet(delayOption)) {
                qDebug() << "cmd delay screenshot";
                window.delayScreenshot(cmdParser.value(delayOption).toInt());
                qCDebug(dsrApp) << "Delay screenshot initiated with delay:" << cmdParser.value(delayOption).toInt();
            } else if (cmdParser.isSet(fullscreenOption)) {
                window.fullscreenScreenshot();
                qCDebug(dsrApp) << "Fullscreen screenshot initiated.";
            } else if (cmdParser.isSet(topWindowOption)) {
                qDebug() << "cmd topWindow screenshot";
                window.topWindowScreenshot();
                qCDebug(dsrApp) << "Top window screenshot initiated.";
            } else if (cmdParser.isSet(savePathOption)) {
                qDebug() << "cmd savepath screenshot";
                window.savePathScreenshot(cmdParser.value(savePathOption));
                qCDebug(dsrApp) << "Save path screenshot initiated with path:" << cmdParser.value(savePathOption);
            } else if (cmdParser.isSet(prohibitNotifyOption)) {
                qDebug() << "screenshot no notify!";
                window.noNotifyScreenshot();
                qCDebug(dsrApp) << "No notify screenshot initiated.";
            } else if (cmdParser.isSet(screenRecordFullScreenOption)) {
                qDebug() << "screenRecordFullScreenOption!!!!" << cmdParser.value(screenRecordFullScreenOption);
                window.fullScreenRecord(cmdParser.value(screenRecordFullScreenOption));
                qCDebug(dsrApp) << "Full screen record initiated with file:" << cmdParser.value(screenRecordFullScreenOption);
            } else {
                window.startScreenshot();
                qCDebug(dsrApp) << "Default screenshot initiated.";
            }
        }
        qCDebug(dsrApp) << "Exiting application event loop.";
        return app->exec();
    } else {
        qCDebug(dsrApp) << "DBus service com.deepin.ScreenRecorder already registered by another instance.";
        // Send DBus message to stop screen-recorder if found other screen-recorder DBus service has started.
        QDBusInterface notification("com.deepin.ScreenRecorder",
                                    "/com/deepin/ScreenRecorder",
                                    "com.deepin.ScreenRecorder",
                                    QDBusConnection::sessionBus());
        qCDebug(dsrApp) << "DBus interface for existing screen recorder created.";

        QList<QVariant> arg;
        // Use stopApp() instead of stopRecord(), actually stop previous process.
        notification.callWithArgumentList(QDBus::AutoDetect, "stopApp", arg);
        qCDebug(dsrApp) << "Calling stopApp on existing screen recorder instance.";
    }
    qCDebug(dsrApp) << "main function finished. Returning 0.";
    return 0;
}
