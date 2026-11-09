# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

QT += core gui widgets dbus testlib
CONFIG += c++17 testcase console link_pkgconfig
PKGCONFIG += dtk6widget dtk6gui dtk6core
TARGET = ut_screenshot_session

INCLUDEPATH += ../../src /usr/include/opencv_mobile

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
LIBS += -lopencv_small

SOURCES += \
    main.cpp \
    ../../src/screenshot/core/selectionconstraints.cpp \
    ../../src/screenshot/core/selectionmodel.cpp \
    ../../src/screenshot/core/selectionoverlaygeometry.cpp \
    ../../src/screenshot/export/screenshotexportservice.cpp \
    ../../src/screenshot/merge/scrollmergecoordinator.cpp \
    ../../src/screenshot/session/annotationcoordinator.cpp \
    ../../src/screenshot/session/screenshotsessioncontroller.cpp \
    ../../src/screenshot/platform/treeland/recordingfiletransaction.cpp \
    ../../src/screenshot/platform/treeland/treelandscrollcompositor.cpp \
    ../../src/recordertablet.cpp \
    ../../src/utils/configsettings.cpp \
    ../../src/utils/imageeffects.cpp \
    ../../src/utils/log.cpp \
    ../../src/utils/pixmergethread.cpp \
    ../../src/utils/shapesutils.cpp

HEADERS += \
    ../../src/screenshot/core/coordinatemapper.h \
    ../../src/screenshot/core/inputregiongeometry.h \
    ../../src/screenshot/core/screenshottypes.h \
    ../../src/screenshot/core/selectionconstraints.h \
    ../../src/screenshot/core/selectionmodel.h \
    ../../src/screenshot/core/selectionoverlaygeometry.h \
    ../../src/screenshot/export/screenshotexportservice.h \
    ../../src/screenshot/merge/scrollmergecoordinator.h \
    ../../src/recordertablet.h \
    ../../src/screenshot/ports/iannotationeditor.h \
    ../../src/screenshot/ports/irecordingsession.h \
    ../../src/screenshot/ports/iscrollcapture.h \
    ../../src/screenshot/ports/iscrollmerger.h \
    ../../src/screenshot/ports/iscreenshotbackend.h \
    ../../src/screenshot/ports/iscreenshotoverlayhost.h \
    ../../src/screenshot/ports/iscreenshotresultsink.h \
    ../../src/screenshot/ports/isnaphintsource.h \
    ../../src/screenshot/ports/iwindowtagger.h \
    ../../src/screenshot/session/annotationcoordinator.h \
    ../../src/screenshot/session/screenshotsessioncontroller.h \
    ../../src/screenshot/platform/treeland/recordingfiletransaction.h \
    ../../src/screenshot/platform/treeland/treelandscrollcompositor.h \
    ../../src/utils/configsettings.h \
    ../../src/utils/imageeffects.h \
    ../../src/utils/log.h \
    ../../src/utils/pixmergethread.h \
    ../../src/utils/shapesutils.h
