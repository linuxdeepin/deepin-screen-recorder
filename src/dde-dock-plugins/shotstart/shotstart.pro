equals(QT_MAJOR_VERSION, 6) {
    QT              += widgets dbus
    PKGCONFIG += dtk6widget dtk6core
} else {
    QT              += widgets dbus
    QT              += dtkgui
    QT              += dtkwidget
    
    # Qt5 specific configurations
    PKGCONFIG       += dframeworkdbus
}

TEMPLATE        = lib
CONFIG          += c++17 plugin link_pkgconfig

TARGET          = $$qtLibraryTarget(shot-start-plugin)
DISTFILES       += shotstart.json \
    res/screen-recording.svg

HEADERS += \
    iconwidget_interface.h \
    shotstartplugin.h \
    iconwidget.h \
    tipswidget.h \
    quickpanelwidget.h \
    commoniconbutton.h

SOURCES += \
    iconwidget_interface.cpp \
    shotstartplugin.cpp \
    iconwidget.cpp \
    tipswidget.cpp \
    quickpanelwidget.cpp \
    commoniconbutton.cpp

isEmpty(PREFIX) {
    PREFIX = /usr
}
target.path = $$PREFIX/lib/dde-dock/plugins/
file.files += $$PWD/com.deepin.dde.dock.module.shot-start-plugin.gschema.xml
file.path += $$PREFIX/share/glib-2.0/schemas/

INSTALLS += target file
RESOURCES += res.qrc
