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

SYS_EDITION=$$system("cat /etc/os-version | grep 'Community'")
message("SYS_EDITION: " $$SYS_EDITION)

SYS_VERSION=$$system("cat /etc/os-version | grep 'MinorVersion' | grep -o '\-\?[0-9]\+'")
message("SYS_VERSION: " $$SYS_VERSION)

if (!equals(SYS_EDITION, "")) {
# 社区版
    message("Community")
    DEFINES += DDE_START_PLUGIN_ON
} else {
# 专业版
    message("not Community")
    #1050支持Wayland
    greaterThan(SYS_VERSION, 1052) {
        DEFINES += DDE_START_PLUGIN_ON
        message("wayland support: OK!!!")
    }
}
contains(DEFINES, DDE_START_PLUGIN_ON) {
    INSTALLS += target
}
RESOURCES += res.qrc
