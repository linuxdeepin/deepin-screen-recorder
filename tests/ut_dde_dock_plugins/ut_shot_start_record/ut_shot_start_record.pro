QT              += core widgets dbus testlib
CONFIG          += c++11 plugin link_pkgconfig
PKGCONFIG += dtkgui dtkwidget
DEFINES += UNIT_TEST

include(../../../3rdparty/stub_linux/stub.pri)

TARGET = ut_shot_start_record

QMAKE_CXXFLAGS += -g -Wno-error=deprecated-declarations -Wno-deprecated-declarations -Wall -fprofile-arcs -ftest-coverage -O0 -fno-access-control
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
    ../../../src/dde-dock-plugins/shotstartrecord/recordiconwidget.h \
    ../../../src/dde-dock-plugins/shotstartrecord/shotstartrecordplugin.h \
    ../../../src/dde-dock-plugins/shotstartrecord/tipswidget.h \
    ../../../src/dde-dock-plugins/shotstartrecord/quickpanelwidget.h \
    ../../../src/dde-dock-plugins/shotstartrecord/commoniconbutton.h

SOURCES += \
    main.cpp \
    ../../../src/dde-dock-plugins/shotstartrecord/recordiconwidget.cpp \
    ../../../src/dde-dock-plugins/shotstartrecord/shotstartrecordplugin.cpp \
    ../../../src/dde-dock-plugins/shotstartrecord/tipswidget.cpp \
    ../../../src/dde-dock-plugins/shotstartrecord/quickpanelwidget.cpp \
    ../../../src/dde-dock-plugins/shotstartrecord/commoniconbutton.cpp \
    ut_tipswidget.cpp \
    ut_shotstartrecordplugin.cpp \
    ut_recordiconwidget.cpp

INCLUDEPATH += /usr/include/dde-dock
include(../../../3rdparty/googletest/gtest_dependency.pri)

