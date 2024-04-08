QT              += widgets dbus
TEMPLATE        = lib
CONFIG          += c++11 plugin link_pkgconfig
QT              += dtkgui
QT              += dtkwidget
PKGCONFIG += dframeworkdbus

TARGET          = $$qtLibraryTarget(shot-start-record-plugin)
DISTFILES       += recordstart.json \
    res/screen-recording.svg

HEADERS += \
    tipswidget.h \
    quickpanelwidget.h \
    commoniconbutton.h \
    shotstartrecordplugin.h \
    recordiconwidget.h

SOURCES += \
    tipswidget.cpp \
    quickpanelwidget.cpp \
    commoniconbutton.cpp \
    shotstartrecordplugin.cpp \
    recordiconwidget.cpp


target.path = /usr/lib/dde-dock/plugins/
file.files += $$PWD/com.deepin.dde.dock.module.shot-start-record-plugin.gschema.xml
file.path += /usr/share/glib-2.0/schemas/

INSTALLS += target file
RESOURCES += res.qrc
