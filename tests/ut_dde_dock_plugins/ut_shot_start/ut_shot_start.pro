QT              += core widgets dbus testlib
CONFIG          += c++11 plugin link_pkgconfig
PKGCONFIG += dtkgui dtkwidget
DEFINES += UNIT_TEST

include(../../../3rdparty/stub_linux/stub.pri)s

TARGET = ut_shot_start

QMAKE_CXXFLAGS += -g -Wno-error=deprecated-declarations -Wno-deprecated-declarations -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

#内存检测标签
TSAN_TOOL_ENABLE = true
equals(TSAN_TOOL_ENABLE, true ){
    #DEFINES += TSAN_THREAD #互斥
    DEFINES += ENABLE_TSAN_TOOL
    message("deepin-screen-recorder enabled TSAN function with set: " $$TSAN_TOOL_ENABLE)
    contains(DEFINES, TSAN_THREAD){
       QMAKE_CXXFLAGS+="-fsanitize=thread"
       QMAKE_CFLAGS+="-fsanitize=thread"
       QMAKE_LFLAGS+="-fsanitize=thread"
    } else {
       QMAKE_CXXFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_CFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_LFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
    }
}

HEADERS += \
    ut_mock_pluginproxyinterface.h \
    ../../../src/dde-dock-plugins/shotstart/iconwidget.h \
    ../../../src/dde-dock-plugins/shotstart/shotstartplugin.h \
    ../../../src/dde-dock-plugins/shotstart/tipswidget.h \
    ../../../src/dde-dock-plugins/shotstart/quickpanelwidget.h \
    ../../../src/dde-dock-plugins/shotstart/commoniconbutton.h

SOURCES += \
    main.cpp \
    ../../../src/dde-dock-plugins/shotstart/iconwidget.cpp \
    ../../../src/dde-dock-plugins/shotstart/shotstartplugin.cpp \
    ../../../src/dde-dock-plugins/shotstart/tipswidget.cpp \
    ../../../src/dde-dock-plugins/shotstart/quickpanelwidget.cpp \
    ../../../src/dde-dock-plugins/shotstart/commoniconbutton.cpp \
    ut_iconwidget.cpp \
    ut_shotstartplugin.cpp \
    ut_tipswidget.cpp

INCLUDEPATH += /usr/include/dde-dock
include(../../../3rdparty/googletest/gtest_dependency.pri)
#include(../../../src/dde-dock-plugins/shotstart/iconwidget.h)

