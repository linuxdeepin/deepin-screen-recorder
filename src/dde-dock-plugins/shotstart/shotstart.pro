QT              += widgets dbus
TEMPLATE        = lib
CONFIG          += c++11 plugin link_pkgconfig
QT              += dtkgui
QT              += dtkwidget
PKGCONFIG += dframeworkdbus

TARGET          = $$qtLibraryTarget(shot-start-plugin)
DISTFILES       += shotstart.json

HEADERS += \
    shotstartplugin.h \
    iconwidget.h \
    tipswidget.h

SOURCES += \
    shotstartplugin.cpp \
    iconwidget.cpp \
    tipswidget.cpp


target.path = /usr/lib/dde-dock/plugins/

INSTALLS += target
RESOURCES += res.qrc
