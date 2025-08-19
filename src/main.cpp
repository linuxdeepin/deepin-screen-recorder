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
#include "utils/screengrabber.h"
#define Bool int
#include <X11/extensions/Xinerama.h>
DWIDGET_USE_NAMESPACE

#include <xcb/xcb.h>
#include <xcb/randr.h>

static bool isWaylandProtocol()
{
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||  WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
}

static float getScreenFactor()
{
    float factor = 1;
    QFile file("/etc/lightdm/deepin/xsettingsd.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<< "/etc/lightdm/deepin/xsettingsd.conf is not available";
        return factor;
    }

    while (!file.atEnd()) {
        QString line = file.readLine();
        if (line.trimmed().contains("Xft/DPI")) {

            QStringList factorCacheList = line.split(" ", QString::SkipEmptyParts);
            if (factorCacheList.size() == 2) {
                bool ok = false;

                int dpi = factorCacheList.at(1).trimmed().toInt(&ok);
                if(!ok)
                    break;

                factor = 1.0f * dpi / 98304;
                break;
            }
        }
    }
    file.close();
    return factor;
}

static bool checkShouldScale() {
    Display *display = XOpenDisplay(nullptr);

    bool isShouldScale = false;

    int eventBase, errorBase, screensNums;

    if (display == nullptr) {
        qDebug() << "Cannot open display.";
        return isShouldScale;
    }

    if (!XineramaQueryExtension(display, &eventBase, &errorBase)) {
        qDebug() << "Xinerama extension not available.";
        XCloseDisplay(display);
        return isShouldScale;
    }

    XineramaScreenInfo *screenXi = (XineramaScreenInfo *)XineramaQueryScreens(display, &screensNums);

    if (screensNums <= 0) {
        qDebug() << "No screens found.";
        goto clean;
    }

    if (screensNums == 1) {
        qDebug() << "One screen no need scale.";
        goto clean;
    }

    if (screensNums > 2) {
        qDebug() << "Only support 2 screens, this will not need scale.";
        goto clean;
    }

    for (int i = 0; i < screensNums; i++) {
        qDebug() << "XineramaScreenInfo:"
                 << " screen_number:" << screenXi[i].screen_number
                 << " x_org:" << screenXi[i].x_org
                 << " y_org:" << screenXi[i].y_org
                 << " width:" << screenXi[i].width
                 << " height:" << screenXi[i].height;
        if (screenXi[i].x_org == 0 && screenXi[i].y_org > 0) {
            if (screenXi[i].width > screenXi[1-i].width && screenXi[i].height > screenXi[1-i].height) {
                isShouldScale = true;
                qDebug() << "Primary Screen is bellow, this will scale";
                goto clean;
            }
        }
    }

clean:
    XCloseDisplay(display);
    return isShouldScale;
}

static bool CheckFFmpegEnv()
{
    bool flag = false;
    QDir dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qDebug() << "QLibraryInfo::LibrariesPath: " << path;
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (QString("libavcodec") + "*"), QDir::NoDotAndDotDot | QDir::Files);
    qDebug() << list << " exists in the " << path;

    QString libName = "";
    QRegExp re("libavcodec.so.*"); //Sometimes libavcodec.so may not exist, so find it through regular expression.
    for (int i = 0; i < list.count(); i++) {
        if (re.exactMatch(list[i])) {
            libName = list[i];
            break;
        }
    }
    qDebug() << "目录(" << path << ")中存在编码库(" << libName << ")";
    //是否存在编码库,存在编码库需要继续判断是否存在对应的编码器
    if (!libName.isEmpty()) {
        QLibrary libavcodec;
        libavcodec.setFileName(libName);
        if (libavcodec.load()) {
            qDebug() << "编码库加载成功！";
            typedef AVCodec *(*p_avcodec_find_encoder)(enum AVCodecID id);
            p_avcodec_find_encoder m_avcodec_find_encoder = nullptr;
            m_avcodec_find_encoder = reinterpret_cast<p_avcodec_find_encoder>(libavcodec.resolve("avcodec_find_encoder"));
            AVCodec *pCodec = nullptr;
            if (m_avcodec_find_encoder) {
                pCodec = m_avcodec_find_encoder(AV_CODEC_ID_H264);
            } else {
                qWarning() << "编码库中无avcodec_find_encoder接口！";
            }
            if (pCodec) {
                qDebug() << "编码器存在 AVCodecID:" << AV_CODEC_ID_H264;
                flag = true;
                //x11下需要检测ffmpeg应用是否存在,wayland不需要检查
                if (!isWaylandProtocol()) {
                    qInfo() << "Is exists ffmpeg in path(/usr/bin/): " << QFile("/usr/bin/ffmpeg").exists();
                    if (!QFile("/usr/bin/ffmpeg").exists()) {
                        flag = false;
                    }
                }
            } else {
                qWarning() << "Can not find output video encoder! (没有找到合适的编码器！) AVCodecID:" << AV_CODEC_ID_H264;
            }
        } else {
            qWarning() << "编码库加载失败！";
        }
    } else {
        qWarning() << "目录(" << path << ")中不存在编码库(" << libName << ")";
    }

    return flag;
}

static bool checkSpecificScreenLayout()
{
    // Connect to X server using XCB
    xcb_connection_t *connection = xcb_connect(nullptr, nullptr);
    if (!connection || xcb_connection_has_error(connection)) {
        // Handle connection failure
        if (connection) {
            xcb_disconnect(connection);
        }
        return false;
    }

    // Get X server setup information
    const xcb_setup_t *setup = xcb_get_setup(connection);
    if (!setup) {
        xcb_disconnect(connection);
        return false;
    }

    xcb_screen_t *screen = xcb_setup_roots_iterator(setup).data;
    if (!screen) {
        xcb_disconnect(connection);
        return false;
    }

    // Check if RANDR extension is available (version 1.3 or higher)
    xcb_randr_query_version_cookie_t version_cookie =
        xcb_randr_query_version(connection, 1, 3);
    xcb_generic_error_t *error = nullptr;
    xcb_randr_query_version_reply_t *version_reply =
        xcb_randr_query_version_reply(connection, version_cookie, &error);

    // Check for errors
    if (error) {
        free(error);
        error = nullptr;
    }

    if (!version_reply) {
        xcb_disconnect(connection);
        return false;
    }

    free(version_reply);
    version_reply = nullptr;

    // Get screen resources (outputs, CRTCs, modes)
    xcb_randr_get_screen_resources_cookie_t resources_cookie =
        xcb_randr_get_screen_resources(connection, screen->root);

    error = nullptr; // Reset error pointer
    xcb_randr_get_screen_resources_reply_t *resources_reply =
        xcb_randr_get_screen_resources_reply(connection, resources_cookie, &error);

    if (error) {
        free(error);
        error = nullptr;
    }

    if (!resources_reply) {
        xcb_disconnect(connection);
        return false;
    }

    // Get output ports (monitors)
    xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_outputs(resources_reply);
    int num_outputs = xcb_randr_get_screen_resources_outputs_length(resources_reply);

    if (num_outputs < 2 || !outputs) {
        free(resources_reply);
        xcb_disconnect(connection);
        return false;
    }

    // Structure to store position and dimensions of active screens
    struct ScreenInfo {
        int x, y;        // Top-left corner coordinates
        int width, height; // Dimensions
    };

    // Pre-allocate memory to avoid multiple reallocations
    std::vector<ScreenInfo> screens;
    screens.reserve(num_outputs);

    // Collect information about all active screens
    for (int i = 0; i < num_outputs; i++) {
        xcb_randr_get_output_info_cookie_t output_cookie =
            xcb_randr_get_output_info(connection, outputs[i], XCB_CURRENT_TIME);

        error = nullptr; // Reset error pointer
        xcb_randr_get_output_info_reply_t *output_reply =
            xcb_randr_get_output_info_reply(connection, output_cookie, &error);

        // Handle errors or invalid replies
        if (error) {
            free(error);
            error = nullptr;
        }

        if (!output_reply) {
            continue;
        }

        // Only process connected outputs with an assigned CRTC
        if (output_reply->connection != XCB_RANDR_CONNECTION_CONNECTED ||
            output_reply->crtc == XCB_NONE) {
            free(output_reply);
            continue;
        }

        // Get CRTC information (contains position and dimensions)
        xcb_randr_get_crtc_info_cookie_t crtc_cookie =
            xcb_randr_get_crtc_info(connection, output_reply->crtc, XCB_CURRENT_TIME);

        error = nullptr; // Reset error pointer
        xcb_randr_get_crtc_info_reply_t *crtc_reply =
            xcb_randr_get_crtc_info_reply(connection, crtc_cookie, &error);

        // Handle errors or invalid replies
        if (error) {
            free(error);
            error = nullptr;
        }

        if (!crtc_reply) {
            free(output_reply);
            continue;
        }

        // Store valid screen information
        ScreenInfo info;
        info.x = crtc_reply->x;
        info.y = crtc_reply->y;
        info.width = crtc_reply->width;
        info.height = crtc_reply->height;

        screens.push_back(info);

        // Free resources
        free(crtc_reply);
        free(output_reply);
    }

    // Release resources no longer needed
    free(resources_reply);
    resources_reply = nullptr;

    // Exit if fewer than 2 active screens detected
    if (screens.size() < 2) {
        xcb_disconnect(connection);
        return false;
    }

    // 1. Find the leftmost screen
    int leftIndex = 0;
    for (size_t i = 1; i < screens.size(); ++i) {
        if (screens[i].x < screens[leftIndex].x) {
            leftIndex = i;
        }
    }

    // 2. Find the closest screen to the right of the leftmost screen
    int rightIndex = -1;
    int minDistance = INT_MAX;

    for (size_t i = 0; i < screens.size(); ++i) {
        if (i != (size_t)leftIndex && screens[i].x > screens[leftIndex].x) {
            int distance = screens[i].x - (screens[leftIndex].x + screens[leftIndex].width);
            if (distance < minDistance) {
                minDistance = distance;
                rightIndex = i;
            }
        }
    }

    // Exit if no right screen found
    if (rightIndex == -1) {
        xcb_disconnect(connection);
        return false;
    }

    // Check for specific layout condition: screens at different vertical and horizontal positions
    bool result = false;
    bool screenPositionsDifferent = (screens[leftIndex].y != screens[rightIndex].y) &&
                                   (screens[leftIndex].x != screens[rightIndex].x);

    result = screenPositionsDifferent;

    xcb_disconnect(connection);

    return result;
}

int main(int argc, char *argv[])
{

    //wayland调试输出
    //qputenv("WAYLAND_DEBUG", "1");
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    DGuiApplicationHelper::setUseInactiveColorGroup(false);

    //TODO 临时方案，后续Qt处理后会移除
    if (checkSpecificScreenLayout()) {
        qWarning() << "检测到特定屏幕布局，禁用缩放";
        QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    } else {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }

    // 平板模式
    Utils::isTabletEnvironment = false; // DGuiApplicationHelper::isTabletEnvironment();
    //qInfo() << "Is Table:" << Utils::isTabletEnvironment;

    // wayland 协议
    Utils::isWaylandMode = isWaylandProtocol();
    //qInfo() << "Is Wayland:" << Utils::isWaylandMode;

    Utils::isRootUser = (getuid() == 0);
    //qInfo() << "Is Root User:" << Utils::isRootUser;

    if (Utils::isWaylandMode) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        QSurfaceFormat format;
        format.setAlphaBufferSize(8);
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
    }

    //检查是否包含ffmpeg相关库true：包含 false：不包含
    Utils::isFFmpegEnv =  CheckFFmpegEnv();
//    Utils::isFFmpegEnv = false;
    //qInfo() << "Is Exists FFmpeg Lib:" << Utils::isFFmpegEnv;

    // 适配deepin-turbo 启动加速
#if(DTK_VERSION < DTK_VERSION_CHECK(5,4,0,0))
    DApplication::loadDXcbPlugin();
    QScopedPointer<DApplication> app(new DApplication(argc, argv));
#else
    QScopedPointer<DApplication> app(DApplication::globalApplication(argc, argv));
#endif

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
        app->setAttribute(Qt::AA_UseHighDpiPixmaps);
//        static const QDate buildDate = QLocale(QLocale::English).
//                                       toDate(QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
//        QString t_date = buildDate.toString("MMdd");

        // Version Time
        app->setApplicationVersion(DApplication::buildVersion(APP_VERSION));

        using namespace Dtk::Core;
        Dtk::Core::DLogManager::registerConsoleAppender();
        Dtk::Core::DLogManager::registerFileAppender();

        QCommandLineOption delayOption(QStringList() << "d" << "delay", "Take a screenshot after NUM seconds.", "NUM");
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

        // 主题设置
        DGuiApplicationHelper::ColorType t_type = DGuiApplicationHelper::instance()->themeType();
        DGuiApplicationHelper::instance()->setPaletteType(t_type);
        Utils::themeType = t_type;

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
        qInfo() << "Is Wayland:" << Utils::isWaylandMode;
        qInfo() << "Is Root User:" << Utils::isRootUser;
        qInfo() << "Is Exists FFmpeg Lib:" << Utils::isFFmpegEnv;
        if (cmdParser.isSet(useGStreamer)) {
            Utils::isFFmpegEnv = false;
        }

        // 在 Wayland 环境下，如果是全屏截图，使用快速方式，避免创建 Screenshot 对象
        if (Utils::isWaylandMode && cmdParser.isSet(fullscreenOption)) {
            qInfo() << "检测到 Wayland 环境下的全屏截图，使用快速方式";
            if (ScreenGrabber::quickFullScreenshot()) {
                qInfo() << "快速全屏截图成功完成";
                return 0;
            } else {
                qWarning() << "快速全屏截图失败，退出应用";
                return 1;
            }
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
                qDebug() << "cmd fullscreen screenshot";
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
