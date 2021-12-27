#-------------------------------------------------
#
# Project created by QtCreator 2021-12-16T10:00:05
#
#-------------------------------------------------

QT       += dtkgui dtkwidget dbus
TARGET = deepin-pin-screenshots
TEMPLATE = app
INCLUDEPATH += .

#下面的定义会让你的编译器在你使用任何被标记为deprecated的Qt特性时发出警告(确切的警告取决于你的编译器)。
#请查阅已弃用API的文档，以了解如何将代码从它那里移植出去。
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    service/dbuspinscreenshotsadaptor.cpp \
    service/pinscreenshotsinterface.cpp \
    mainwindow.cpp \
    main.cpp \
    pinscreenshots.cpp

HEADERS += \
    service/dbuspinscreenshotsadaptor.h \
    service/pinscreenshotsinterface.h \
    mainwindow.h \
    pinscreenshots.h

##安装路径
target.path=/usr/bin
isEmpty(DSRDIR):DSRDIR=/usr/share/deepin-pin-screenshots
##翻译
isEmpty(TRANSLATIONS) {
    include(./translations.pri)
}
TRANSLATIONS_COMPILED = $$TRANSLATIONS
TRANSLATIONS_COMPILED ~= s/\.ts/.qm/g
translations.path=/usr/share/deepin-pin-screenshots/translations
translations.files = $$TRANSLATIONS_COMPILED

#Dbus文件
dbus_service.path=/usr/share/dbus-1/services
dbus_service.files=./com.deepin.PinScreenShots.service

#安装
INSTALLS += target dbus_service translations

#指定工程配置和编译参数
CONFIG *= update_translations release_translations
CONFIG(update_translations) {
    isEmpty(lupdate):lupdate=lupdate
    system($$lupdate -no-obsolete -locations none $$_PRO_FILE_)
}

CONFIG(release_translations) {
    isEmpty(lrelease):lrelease=lrelease
    system($$lrelease $$_PRO_FILE_)
}

#DEFINES：指定预定义预处理器符号
DSR_LANG_PATH += $$DSRDIR/translations
DEFINES += "DSR_LANG_PATH=\\\"$$DSR_LANG_PATH\\\""

#RESOURCES += \
#    resource.qrc
