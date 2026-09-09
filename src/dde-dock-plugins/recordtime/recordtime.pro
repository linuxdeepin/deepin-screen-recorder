QT              += widgets dbus
TEMPLATE        = lib
CONFIG          += c++11 plugin link_pkgconfig
QT              += dtkgui
QT              += dtkwidget
PKGCONFIG += dframeworkdbus

#TARGET          = deepin-screen-recorder-plugin
TARGET          = $$qtLibraryTarget(deepin-screen-recorder-plugin)
DISTFILES       += recordtime.json

HEADERS += \
    recordtimeplugin.h \
    timewidget.h \
    dbusservice.h

SOURCES += \
    recordtimeplugin.cpp \
    timewidget.cpp \
    dbusservice.cpp

target.path = /usr/lib/dde-dock/plugins/
gschema.files += $$PWD/com.deepin.dde.dock.module.deepin-screen-recorder-plugin.gschema.xml
gschema.path += /usr/share/glib-2.0/schemas/

INSTALLS += target gschema

RESOURCES += res.qrc
