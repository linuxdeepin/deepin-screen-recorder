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

TARGET          = $$qtLibraryTarget(shot-start-record-plugin)
DISTFILES       += recordstart.json \
    res/screen-recording.svg

HEADERS += \
    recordiconwidget_interface.h \
    tipswidget.h \
    quickpanelwidget.h \
    commoniconbutton.h \
    shotstartrecordplugin.h \
    recordiconwidget.h

SOURCES += \
    recordiconwidget_interface.cpp \
    tipswidget.cpp \
    quickpanelwidget.cpp \
    commoniconbutton.cpp \
    shotstartrecordplugin.cpp \
    recordiconwidget.cpp

target.path = /usr/lib/dde-dock/plugins/
file.files += $$PWD/com.deepin.dde.dock.module.shot-start-record-plugin.gschema.xml
file.path += /usr/share/glib-2.0/schemas/
recordicon.files += $$PWD/res/dcc-shot-start-record-plugin.dci
recordicon.path += /usr/share/dde-dock/icons/dcc-setting/

INSTALLS += target file recordicon
RESOURCES += res.qrc
