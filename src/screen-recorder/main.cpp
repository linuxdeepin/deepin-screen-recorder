#include "config.h"
#include "screenshot.h"

#include "utils.h"

#include <QSurfaceFormat>
#include <QGuiApplication>
#include <QDBusConnection>
#include <QDBusInterface>

int main(int argc, char *argv[])
{
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    Utils::checkWaylandMode();
    if (Utils::isWaylandMode()) { //是否开启wayland
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell"); //add wayland parameter
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
    }
    
    QGuiApplication *app = new QGuiApplication(argc, argv);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService("com.deepin.ScreenRecorder")) {
        app->setOrganizationName("deepin");
        app->setApplicationName("deepin-screen-recorder");
        app->setApplicationVersion(VERSION);
        app->setAttribute(Qt::AA_UseHighDpiPixmaps);

        Screenshot *screenshot  = Screenshot::instance();
        // Register debus service.
        dbus.registerObject("/com/deepin/ScreenRecorder", screenshot, QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportScriptableSlots);
        QDBusConnection conn = QDBusConnection::sessionBus();

        if (!conn.registerService("com.deepin.Screenshot") || !conn.registerObject("/com/deepin/Screenshot", screenshot)) {
            qDebug() << "deepin-screenshot is running!";
            qApp->quit();
            if (Utils::isWaylandMode()) {
                _Exit(0);
            }
            return 0;
        }

        screenshot->parser(app->arguments());
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
