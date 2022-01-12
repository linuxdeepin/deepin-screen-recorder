/*

* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.

*

* Author:     wangcong <wangcong@uniontech.com>

*

* Maintainer: wangcong <wangcong@uniontech.com>

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
#include "pinscreenshots.h"
#include "service/pinscreenshotsinterface.h"
#include "service/dbuspinscreenshotsadaptor.h"

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
#include <QDesktopWidget>

DWIDGET_USE_NAMESPACE

bool isWaylandProtocol()
{
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||  WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
}


int main(int argc, char *argv[])
{

    if (argc < 2) {
        qDebug() << "Cant open a null file";
        return 0;
    }


    if(isWaylandProtocol()) {
       qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }

    DGuiApplicationHelper::setUseInactiveColorGroup(false);
#if(DTK_VERSION < DTK_VERSION_CHECK(5,4,0,0))
    DApplication::loadDXcbPlugin();
    QScopedPointer<DApplication> app(new DApplication(argc, argv));
#else
    QScopedPointer<DApplication> app(DApplication::globalApplication(argc, argv));
#endif
    app->setOrganizationName("deepin");
    app->setApplicationName("deepin-pin-screenshots");
    app->setProductName(QObject::tr("Pin screenshots"));
    app->setApplicationVersion("1.0");

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    QCommandLineOption dbusOption(QStringList() << "u" << "dbus", "Start  from dbus.");
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("deepin-pin-screenshots");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.addOption(dbusOption);
    cmdParser.process(*app);

    app->loadTranslator();


    PinScreenShots instance;
    QDBusConnection dbus = QDBusConnection::sessionBus();

    if (dbus.registerService("com.deepin.PinScreenShots")) {
        // 第一次启动
        // 注册Dbus服务和对象
        dbus.registerObject("/com/deepin/PinScreenShots", &instance);
        // 初始化适配器
        new DbusPinScreenShotsAdaptor(&instance);

        if (cmdParser.isSet(dbusOption)) {
            // 第一次调用以 --dbus参数启动
            qDebug() << "dbus register waiting!";
            return app->exec();
        }

        instance.openFile(QString(argv[1]));
        qDebug() << "贴图dbus服务已注册";

    } else {
        // 第二次运行此应用，
        // 调用DBus接口，处理交给第一次调用的进程
        // 本进程退退出
        PinScreenShotsInterface *pinS = new PinScreenShotsInterface("com.deepin.PinScreenShots", "/com/deepin/PinScreenShots", QDBusConnection::sessionBus(), &instance);
        qDebug() << __FUNCTION__ << __LINE__;
        pinS->openFile(QString(argv[1]));
        delete pinS;
        return 0;
    }

    return app->exec();
}


