#-------------------------------------------------
#
# Project created by QtCreator 2021-12-16T10:00:05
#
#-------------------------------------------------

# Qt 模块配置
equals(QT_MAJOR_VERSION, 6) {
    QT += core gui widgets dbus
    
    # Qt6 specific configurations
    PKGCONFIG += dtk6widget dtk6core
} else {
    QT += core gui widgets dbus
    
    # Qt5 specific configurations
    PKGCONFIG += dtkwidget dtkcore
}

TARGET = deepin-pin-screenshots
TEMPLATE = app
INCLUDEPATH += .

#下面的定义会让你的编译器在你使用任何被标记为deprecated的Qt特性时发出警告(确切的警告取决于你的编译器)。
#请查阅已弃用API的文档，以了解如何将代码从它那里移植出去。
DEFINES += QT_DEPRECATED_WARNINGS

# 编译器配置
CONFIG += c++17 link_pkgconfig

SOURCES += \
    service/dbuspinscreenshotsadaptor.cpp \
    service/pinscreenshotsinterface.cpp \
    mainwindow.cpp \
    main.cpp \
    pinscreenshots.cpp \
    service/ocrinterface.cpp \
    ui/menucontroller.cpp \
    ui/toolbarwidget.cpp \
    ui/subtoolwidget.cpp \
    putils.cpp \
    settings.cpp \
    ui/mainToolWidget.cpp \
    ../utils/log.cpp

HEADERS += \
    service/dbuspinscreenshotsadaptor.h \
    service/pinscreenshotsinterface.h \
    mainwindow.h \
    pinscreenshots.h \
    service/ocrinterface.h \
    ui/menucontroller.h \
    ui/toolbarwidget.h \
    ui/subtoolwidget.h \
    putils.h \
    settings.h \
    ui/toolbutton.h \
    ui/mainToolWidget.h \
    ../utils/log.h

QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -Wno-error=deprecated-declarations -Wno-deprecated-declarations

equals(QT_MAJOR_VERSION, 6) {
    # Qt6 特定的编译选项
    QMAKE_CXXFLAGS += -DQT_DISABLE_DEPRECATED_BEFORE=0x060000
}

##安装路径
isEmpty(PREFIX){
    PREFIX = /usr
}

target.path=$$PREFIX/bin
isEmpty(DSRDIR):DSRDIR=$$PREFIX/share/deepin-pin-screenshots

#Dbus文件
dbus_service.path=$$PREFIX/share/dbus-1/services
dbus_service.files=./com.deepin.PinScreenShots.service

include(accessibility/accessible.pri)

#安装
INSTALLS += target dbus_service

RESOURCES += \
    ./icons/icons.qrc

INCLUDEPATH += ../utils


