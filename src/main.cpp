// SPDX-FileCopyrightText: 2011-2026 UnionTech Software Technology Co., Ltd.
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
#include "utils/logger.h"
#include "screenshot/core/screenshottypes.h"
#include "screenshot/session/screenshotsessioncontroller.h"
#include "screenshot/session/screenshotsessionfactory.h"

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
#include <QTimer>

DWIDGET_USE_NAMESPACE

static bool isWaylandProtocol()
{
    qCDebug(dsrApp) << "isWaylandProtocol called.";
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    qCDebug(dsrApp) << "System environment retrieved.";

    // TreeLand使用wayland协议，但需要特殊处理，不视为普通wayland
    if (e.value(QStringLiteral("DDE_CURRENT_COMPOSITOR")) == QStringLiteral("TreeLand")) {
        qCDebug(dsrApp) << "DDE_CURRENT_COMPOSITOR is TreeLand, returning false (TreeLand handled separately).";
        return false;
    }

    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    bool isWayland = XDG_SESSION_TYPE == QLatin1String("wayland") ||
           WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
    qCDebug(dsrApp) << "XDG_SESSION_TYPE:" << XDG_SESSION_TYPE << ", WAYLAND_DISPLAY:" << WAYLAND_DISPLAY << ", isWayland:" << isWayland;
    return isWayland;
}

static bool isTreelandEnvironment()
{
    qCDebug(dsrApp) << "isTreelandEnvironment called.";
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    bool isTreeland = e.value(QStringLiteral("DDE_CURRENT_COMPOSITOR")) == QStringLiteral("TreeLand");
    qCDebug(dsrApp) << "DDE_CURRENT_COMPOSITOR:" << e.value(QStringLiteral("DDE_CURRENT_COMPOSITOR")) << ", isTreeland:" << isTreeland;
    return isTreeland;
}

static bool initQt6XcbEnv()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool result = false;
    if (QGuiApplication::instance()) {
        const QString platformName = QGuiApplication::platformName().toLower();
        result = platformName == QLatin1String("xcb");
    } else {
        const QString platformEnv = qEnvironmentVariable("QT_QPA_PLATFORM").toLower();
        result = platformEnv.contains(QLatin1String("xcb"));
    }
    return result;
#endif
    return false;
}

static bool CheckFFmpegEnv()
{
    qCDebug(dsrApp) << "CheckFFmpegEnv called.";
    bool flag = false;
    QDir dir;
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qCDebug(dsrApp) << "QLibraryInfo::LibrariesPath: " << path;
    qCDebug(dsrApp) << "Libraries path:" << path;
    dir.setPath(path);
    qCDebug(dsrApp) << "where is libs? where is " << dir;
    QStringList list = dir.entryList(QStringList() << (QString("libavcodec") + "*"), QDir::NoDotAndDotDot | QDir::Files);
    qCDebug(dsrApp) << "Is libavcodec in there?  there is :" << list;
    qCDebug(dsrApp) << "Found libavcodec files:" << list;

    if (list.contains("libavcodec.so.58")) {
        qCInfo(dsrApp) << "list contains libavcodec.so.58";
        qCDebug(dsrApp) << "libavcodec.so.58 found.";
        flag = true;
    }

    // x11下需要检测ffmpeg应用是否存在
    if (!isWaylandProtocol()) {
        qCDebug(dsrApp) << "Not Wayland protocol, checking for ffmpeg executable.";
        flag = !QStandardPaths::findExecutable("ffmpeg").isEmpty();
        qCInfo(dsrApp) << "Is exists ffmpeg in PATH(" << qgetenv("PATH") << "):" << flag;
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
            qCDebug(dsrApp) << "scaleFactor available value: " << factor;
            qCDebug(dsrApp) << "Scale factor is greater than 1:" << factor << ", setting QT_SCALE_FACTOR.";
            qputenv("QT_SCALE_FACTOR", QString::number(1 / factor, 'g', 2).toLatin1());
        }
    }
    qCDebug(dsrApp) << "resetScaleFactor finished.";
}

int main(int argc, char *argv[])
{
    // deepin-debug-config 调试模式适配：日志格式带分类，便于按分类过滤
    Dtk::Core::DLogManager::setLogFormat(
        "%{time}{yy-MM-ddTHH:mm:ss.zzz} [%{type:-7}] [%{category}] <%{function}:%{line}> %{message}");
    // 动态日志规则监听需在任何日志打印之前初始化，且生命周期需覆盖整个 main
    MLogger logger;
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


    Utils::isQt6XcbEnv = initQt6XcbEnv();
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // 平板模式
    // Utils::isTabletEnvironment = DGuiApplicationHelper::isTabletEnvironment();
    // wayland 协议
    Utils::isWaylandMode = isWaylandProtocol();
    Utils::isTreelandMode = isTreelandEnvironment();
    Utils::isRootUser = (getuid() == 0);
    qCDebug(dsrApp) << "isWaylandMode:" << Utils::isWaylandMode << ", isTreelandMode:" << Utils::isTreelandMode << ", isRootUser:" << Utils::isRootUser;

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
    qCDebug(dsrApp) << "Is FFmpeg Environment:" << Utils::isFFmpegEnv;
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
        QCommandLineOption screenshotSessionPreviewOption(
            QStringList() << "screenshot-session-preview" << "treeland-snap-preview",
            "Use the compositor-independent screenshot session preview path.");
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
        cmdParser.addOption(screenshotSessionPreviewOption);
        cmdParser.process(*app);
        if (cmdParser.isSet(dbusOption)) {
            // The D-Bus service owns no visible window between screenshot
            // sessions. Keep the process alive so subsequent calls reuse the
            // same registered service and object instances.
            app->setQuitOnLastWindowClosed(false);
        }
        qCDebug(dsrApp) << "Command line parser initialized and processed application arguments.";

        // Load translator.
        app->loadTranslator();
        Utils::appName = MainWindow::tr("deepin-screen-recorder");
        qCInfo(dsrApp) << "Original Application Name is: " << QCoreApplication::applicationName()
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

        qCInfo(dsrApp) << "截图录屏版本: " << DApplication::buildVersion(APP_VERSION);
#ifdef KF5_WAYLAND_FLAGE_ON
        qCInfo(dsrApp) << "KF5_WAYLAND_FLAGE_ON is open!!";
        qCDebug(dsrApp) << "KF5_WAYLAND_FLAGE_ON is defined.";
#else
        qCInfo(dsrApp) << "KF5_WAYLAND_FLAGE_ON is close!!";
        qCDebug(dsrApp) << "KF5_WAYLAND_FLAGE_ON is not defined.";
#endif
        qCInfo(dsrApp) << "Is Table:" << Utils::isTabletEnvironment;
        qCInfo(dsrApp) << "Is Root User:" << Utils::isRootUser;
        qCInfo(dsrApp) << "Is Exists FFmpeg Lib:" << Utils::isFFmpegEnv;
        qCDebug(dsrApp) << "Tablet environment:" << Utils::isTabletEnvironment << ", Root user:" << Utils::isRootUser << ", FFmpeg Lib exists:" << Utils::isFFmpegEnv;

        const bool usePlatformScreenshotSession =
                Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()
                && !cmdParser.isSet(dbusOption)
                && !cmdParser.isSet(screenRecordOption)
                && !cmdParser.isSet(screenOcrOption)
                && !cmdParser.isSet(screenScrollOption)
                && !cmdParser.isSet(screenRecordFullScreenOption);
        if (cmdParser.isSet(screenshotSessionPreviewOption)
                || usePlatformScreenshotSession) {
            Dsr::Screenshot::Request request;
            if (cmdParser.isSet(fullscreenOption))
                request.mode = Dsr::Screenshot::Mode::FullScreen;
            else if (cmdParser.isSet(topWindowOption))
                request.mode = Dsr::Screenshot::Mode::TopWindow;
            request.enableWindowSnap = request.mode == Dsr::Screenshot::Mode::Region;
            request.outputPath = cmdParser.isSet(savePathOption)
                    ? cmdParser.value(savePathOption)
                    : QString();

            QString factoryError;
            auto *session = Dsr::Screenshot::ScreenshotSessionFactory::createForCurrentPlatform(
                    app.data(), &factoryError);
            if (!session) {
                qCritical() << "Unable to create screenshot session:" << factoryError;
                return 2;
            }

            DApplication *application = app.data();
            bool sessionFailed = false;
            QObject::connect(session, &Dsr::Screenshot::ScreenshotSessionController::saved,
                             application, [](const QString &path) {
                                 qInfo() << "Screenshot session result:" << path;
                             });
            QObject::connect(session, &Dsr::Screenshot::ScreenshotSessionController::failed,
                             application, [&sessionFailed](const QString &error) {
                                 qCritical() << "Screenshot session failed:" << error;
                                 sessionFailed = true;
                             });
            QObject::connect(session, &Dsr::Screenshot::ScreenshotSessionController::finished,
                             application,
                             [application, &sessionFailed]() {
                                 application->exit(sessionFailed ? 1 : 0);
                             },
                             Qt::QueuedConnection);
            if (!session->start(request)) {
                qCritical() << "Screenshot session is already active";
                return 2;
            }
            const int exitCode = app->exec();
            // Tear down the session while QApplication and its platform
            // integration are still alive.  Waiting for DApplication's member
            // destruction leaves hidden annotation widgets alive until the
            // platform teardown has already started.
            delete session;
            return exitCode;
        }
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
                            QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableProperties);
        qCDebug(dsrApp) << "DBus object /com/deepin/ScreenRecorder registered.";
        QDBusConnection conn = QDBusConnection::sessionBus();

        if (!conn.registerService("com.deepin.Screenshot") || !conn.registerObject("/com/deepin/Screenshot", &window)) {
            qCDebug(dsrApp) << "deepin-screenshot is running!";
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
            qCDebug(dsrApp) << "dbus register waiting!";
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
            
            // 检查是否有其他截图模式参数
            bool hasOtherMode = cmdParser.isSet(delayOption) || 
                                cmdParser.isSet(fullscreenOption) || 
                                cmdParser.isSet(topWindowOption) || 
                                cmdParser.isSet(prohibitNotifyOption) || 
                                cmdParser.isSet(screenRecordFullScreenOption);
            
            // 如果设置了 save-path 且有其他模式，只设置路径不启动截图
            if (cmdParser.isSet(savePathOption) && hasOtherMode) {
                window.setSavePath(cmdParser.value(savePathOption));
                qCDebug(dsrApp) << "Save path set to:" << cmdParser.value(savePathOption);
            }
            
            // 处理截图模式参数
            if (cmdParser.isSet(delayOption)) {
                qCDebug(dsrApp) << "cmd delay screenshot";
                window.delayScreenshot(cmdParser.value(delayOption).toInt());
                qCDebug(dsrApp) << "Delay screenshot initiated with delay:" << cmdParser.value(delayOption).toInt();
            } else if (cmdParser.isSet(fullscreenOption)) {
                window.fullscreenScreenshot();
                qCDebug(dsrApp) << "Fullscreen screenshot initiated.";
            } else if (cmdParser.isSet(topWindowOption)) {
                qCDebug(dsrApp) << "cmd topWindow screenshot";
                window.topWindowScreenshot();
                qCDebug(dsrApp) << "Top window screenshot initiated.";
            } else if (cmdParser.isSet(prohibitNotifyOption)) {
                qCDebug(dsrApp) << "screenshot no notify!";
                window.noNotifyScreenshot();
                qCDebug(dsrApp) << "No notify screenshot initiated.";
            } else if (cmdParser.isSet(screenRecordFullScreenOption)) {
                qCDebug(dsrApp) << "screenRecordFullScreenOption!!!!" << cmdParser.value(screenRecordFullScreenOption);
                window.fullScreenRecord(cmdParser.value(screenRecordFullScreenOption));
                qCDebug(dsrApp) << "Full screen record initiated with file:" << cmdParser.value(screenRecordFullScreenOption);
            } else if (cmdParser.isSet(savePathOption)) {
                // 只设置了 save-path，没有其他截图模式，启动保存路径截图
                qCDebug(dsrApp) << "cmd savepath screenshot";
                window.savePathScreenshot(cmdParser.value(savePathOption));
                qCDebug(dsrApp) << "Save path screenshot initiated with path:" << cmdParser.value(savePathOption);
            } else {
                // 如果没有设置任何参数，启动默认截图
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
