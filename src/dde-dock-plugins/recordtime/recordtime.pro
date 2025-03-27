equals(QT_MAJOR_VERSION, 6) {
    QT          += widgets dbus
    PKGCONFIG += dtk6widget dtk6core
} else {
    QT          += widgets dbus
    QT          += dtkgui
    QT          += dtkwidget
}

TEMPLATE    = lib
CONFIG      += c++17 plugin link_pkgconfig

TARGET      = $$qtLibraryTarget(deepin-screen-recorder-plugin)
DISTFILES   += recordtime.json

HEADERS += \
    recordtimeplugin.h \
    timewidget.h \
    dbusservice.h \
    timewidget_interface.h

SOURCES += \
    recordtimeplugin.cpp \
    timewidget.cpp \
    dbusservice.cpp \
    timewidget_interface.cpp

isEmpty(PREFIX) {
    PREFIX = /usr
}

target.path = $$PREFIX/lib/dde-dock/plugins/

INSTALLS += target

RESOURCES += res.qrc
