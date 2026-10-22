# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

QT += core gui testlib
CONFIG += c++17 testcase console
TARGET = ut_screenshot_session

INCLUDEPATH += ../../src

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0

SOURCES += \
    main.cpp \
    ../../src/screenshot/core/selectionconstraints.cpp \
    ../../src/screenshot/core/selectionmodel.cpp \
    ../../src/screenshot/export/screenshotexportservice.cpp \
    ../../src/screenshot/session/annotationcoordinator.cpp \
    ../../src/screenshot/session/screenshotsessioncontroller.cpp \
    ../../src/utils/log.cpp

HEADERS += \
    ../../src/screenshot/core/coordinatemapper.h \
    ../../src/screenshot/core/screenshottypes.h \
    ../../src/screenshot/core/selectionconstraints.h \
    ../../src/screenshot/core/selectionmodel.h \
    ../../src/screenshot/export/screenshotexportservice.h \
    ../../src/screenshot/ports/iannotationeditor.h \
    ../../src/screenshot/ports/iscreenshotbackend.h \
    ../../src/screenshot/ports/iscreenshotoverlayhost.h \
    ../../src/screenshot/ports/iscreenshotresultsink.h \
    ../../src/screenshot/ports/isnaphintsource.h \
    ../../src/screenshot/ports/iwindowtagger.h \
    ../../src/screenshot/session/annotationcoordinator.h \
    ../../src/screenshot/session/screenshotsessioncontroller.h \
    ../../src/utils/log.h
