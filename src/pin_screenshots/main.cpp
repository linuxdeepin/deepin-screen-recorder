// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pinscreenshots.h"
#include "service/pinscreenshotsinterface.h"
#include "service/dbuspinscreenshotsadaptor.h"
#include "putils.h"
#include "../utils/log.h"

#include <DWidget>
#include <DLog>
#include <DWindowManagerHelper>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <DApplication>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>

#include <QScreen>
#include <QGuiApplication>

DWIDGET_USE_NAMESPACE

bool isWaylandProtocol()
{
    qCDebug(dsrApp) << "Checking if Wayland protocol is in use.";
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();

    // check is treeland environment.
    if (e.value(QStringLiteral("DDE_CURRENT_COMPOSITOR")) == QStringLiteral("TreeLand")) {
        qCDebug(dsrApp) << "DDE_CURRENT_COMPOSITOR is TreeLand, not Wayland.";
        return false;
    }

    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    bool isWayland = XDG_SESSION_TYPE == QLatin1String("wayland") ||  WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
    qCDebug(dsrApp) << "XDG_SESSION_TYPE:" << XDG_SESSION_TYPE << ", WAYLAND_DISPLAY:" << WAYLAND_DISPLAY << ", Is Wayland:" << isWayland;
    return isWayland;
}


int main(int argc, char *argv[])
{

    if (argc < 2) {
        qCDebug(dsrApp) << "Cant open a null file";
        return 0;
    }
    PUtils::isWaylandMode = isWaylandProtocol();
    if (PUtils::isWaylandMode) {
        qCDebug(dsrApp) << "Setting Wayland shell integration";
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::UnknownType);
    qCDebug(dsrApp) << "Set palette type for Qt6.";
#else
    DGuiApplicationHelper::setUseInactiveColorGroup(false);
    qCDebug(dsrApp) << "Set inactive color group for Qt5.";
#endif

#if(DTK_VERSION < DTK_VERSION_CHECK(5,4,0,0))
    DApplication::loadDXcbPlugin();
    qCDebug(dsrApp) << "Loaded DXcb plugin.";
    QScopedPointer<DApplication> app(new DApplication(argc, argv));
    qCDebug(dsrApp) << "Created DApplication for DTK < 5.4.0.0.";
#else
    QScopedPointer<DApplication> app(DApplication::globalApplication(argc, argv));
    qCDebug(dsrApp) << "Created DApplication for DTK >= 5.4.0.0.";
#endif
    app->setOrganizationName("deepin");
    app->setApplicationName("deepin-screen-recorder");
    app->setProductName(QObject::tr("Pin Screenshots"));
    app->setApplicationVersion("1.0");
    qCDebug(dsrApp) << "Application metadata set.";
#if (QT_VERSION_MAJOR == 5)
    app->setAttribute(Qt::AA_UseHighDpiPixmaps);
    qCDebug(dsrApp) << "Set Qt::AA_UseHighDpiPixmaps for Qt5.";
#endif
    QString logFilePath = Dtk::Core::DLogManager::getlogFilePath();
    QStringList list = logFilePath.split("/");
    if (!list.isEmpty()) {
        qCDebug(dsrApp) << "Log file path list is not empty.";
        list[list.count() - 1] = "deepin-pin-screen.log";
        logFilePath = list.join("/");
        Dtk::Core::DLogManager::setlogFilePath(logFilePath);
        qCInfo(dsrApp) << "Log file path set to:" << logFilePath;
    }

    Dtk::Core::DLogManager::registerConsoleAppender();
    qCDebug(dsrApp) << "Console appender registered.";
    Dtk::Core::DLogManager::registerFileAppender();
    qCDebug(dsrApp) << "File appender registered.";

    QString logPath = Dtk::Core::DLogManager::getlogFilePath();
    logPath.replace("deepin-screen-recorder.log", "deepin-pin-screen.log");
    Dtk::Core::DLogManager::setlogFilePath(logPath);
    //qCDebug(dsrApp) << "日志位置: " << Dtk::Core::DLogManager::getlogFilePath();
    QCommandLineOption dbusOption(QStringList() << "u" << "dbus", "Start  from dbus.");
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("deepin-pin-screenshots");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.addOption(dbusOption);
    cmdParser.process(*app);
    qCDebug(dsrApp) << "Command line options processed.";

    app->loadTranslator();
    qCDebug(dsrApp) << "Translator loaded.";

    qCDebug(dsrApp) << "Current log file path:" << Dtk::Core::DLogManager::getlogFilePath();

    PinScreenShots instance;
    qCDebug(dsrApp) << "PinScreenShots instance created.";
    QDBusConnection dbus = QDBusConnection::sessionBus();
    qCDebug(dsrApp) << "Connected to DBus session bus.";

    if (dbus.registerService("com.deepin.PinScreenShots")) {
        qCDebug(dsrApp) << "DBus service 'com.deepin.PinScreenShots' registered successfully.";
        // 第一次启动
        // 注册Dbus服务和对象
        dbus.registerObject("/com/deepin/PinScreenShots", &instance);
        qCDebug(dsrApp) << "DBus object '/com/deepin/PinScreenShots' registered.";
        // 初始化适配器
        new DbusPinScreenShotsAdaptor(&instance);
        qCDebug(dsrApp) << "DbusPinScreenShotsAdaptor initialized.";
        qCInfo(dsrApp) << "DBus service registered successfully";

        if (cmdParser.isSet(dbusOption)) {
            qCDebug(dsrApp) << "Application started with --dbus option, waiting for D-Bus registration.";
            // 第一次调用以 --dbus参数启动
            qCDebug(dsrApp) << "dbus register waiting!";
            return app->exec();
        }

        instance.openFile(QString(argv[1]));
        qCDebug(dsrApp) << "DBus service already registered.";

    } else {
        qCDebug(dsrApp) << "DBus service 'com.deepin.PinScreenShots' already running.";
        // 第二次运行此应用，
        // 调用DBus接口，处理交给第一次调用的进程
        // 本进程退退出
        PinScreenShotsInterface *pinS = new PinScreenShotsInterface("com.deepin.PinScreenShots", "/com/deepin/PinScreenShots", QDBusConnection::sessionBus(), &instance);
        qCDebug(dsrApp) << "Calling openFile on existing DBus service.";
        pinS->openFile(QString(argv[1]));
        delete pinS;
        qCDebug(dsrApp) << "PinScreenShotsInterface deleted.";
        return 0;
    }

    qCDebug(dsrApp) << "Starting application event loop.";
    return app->exec();
}


