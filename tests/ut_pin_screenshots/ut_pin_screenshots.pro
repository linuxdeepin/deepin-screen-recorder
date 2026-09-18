#-------------------------------------------------
#
# Project created by QtCreator 2021-12-21T10:22:41
#
#-------------------------------------------------

QT              += core gui widgets dbus testlib
CONFIG          += c++11 plugin link_pkgconfig
PKGCONFIG += dtk6gui dtk6widget
include(../../3rdparty/stub_linux/stub.pri)

TARGET = ut_pin_screenshots

QMAKE_CXXFLAGS += -g -Wno-error=deprecated-declarations -Wno-deprecated-declarations -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

#内存检测标签
# TSAN/ASAN disabled: DMenu construction under offscreen platform triggers
# ASan false-positives that abort the process before tests run.
TSAN_TOOL_ENABLE = false
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

INCLUDEPATH += ../../src/pin_screenshots
INCLUDEPATH += ../../src/pin_screenshots/ui
INCLUDEPATH += ../../src/pin_screenshots/service
INCLUDEPATH += ../../src/utils

SOURCES += \
    main.cpp \
    ../../src/pin_screenshots/settings.cpp \
    ../../src/pin_screenshots/putils.cpp \
    ../../src/pin_screenshots/ui/pinsavemenumanager.cpp \
    ../../src/pin_screenshots/service/pinscreenshotsinterface.cpp \
    ../../src/pin_screenshots/service/dbuspinscreenshotsadaptor.cpp \
    ../../src/utils/log.cpp

HEADERS += \
    ut_pin_screenshots.h \
    ut_pin_settings.h \
    ut_pinsavemenumanager.h \
    ut_putils.h \
    ut_pinscreenshotsinterface.h \
    ut_dbusadaptor.h \
    test_all_interfaces.h \
    ../../src/pin_screenshots/settings.h \
    ../../src/pin_screenshots/putils.h \
    ../../src/pin_screenshots/ui/pinsavemenumanager.h \
    ../../src/pin_screenshots/service/pinscreenshotsinterface.h \
    ../../src/pin_screenshots/service/dbuspinscreenshotsadaptor.h \
    ../../src/utils/log.h

include(../../3rdparty/googletest/gtest_dependency.pri)
