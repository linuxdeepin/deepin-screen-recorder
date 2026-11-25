# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

set(src ${CMAKE_SOURCE_DIR}/src)
add_executable(ut_screenshot_session
    main.cpp
    ${src}/screenshot/core/selectionconstraints.cpp
    ${src}/screenshot/core/selectionmodel.cpp
    ${src}/screenshot/core/selectionoverlaygeometry.cpp
    ${src}/screenshot/export/screenshotexportservice.cpp
    ${src}/screenshot/merge/scrollmergecoordinator.cpp
    ${src}/screenshot/ports/iannotationeditor.h
    ${src}/screenshot/ports/irecordingsession.h
    ${src}/screenshot/ports/iscrollcapture.h
    ${src}/screenshot/ports/iscrollmerger.h
    ${src}/screenshot/ports/iscreenshotbackend.h
    ${src}/screenshot/ports/iscreenshotoverlayhost.h
    ${src}/screenshot/ports/iscreenshotresultsink.h
    ${src}/screenshot/ports/isnaphintsource.h
    ${src}/screenshot/ports/iwindowtagger.h
    ${src}/screenshot/session/annotationcoordinator.cpp
    ${src}/screenshot/session/screenshotsessioncontroller.cpp
    ${src}/screenshot/platform/treeland/recordingfiletransaction.cpp
    ${src}/screenshot/platform/treeland/treelandmaskwindow.cpp
    ${src}/screenshot/platform/treeland/treelandscrollcompositor.cpp
    ${src}/ext-image-capture/framepumpworker.cpp
    ${src}/recordertablet.cpp
    ${src}/utils/baseutils.cpp
    ${src}/utils/configsettings.cpp
    ${src}/utils/imageeffects.cpp
    ${src}/utils/log.cpp
    ${src}/utils/pixmergethread.cpp
    ${src}/utils/shapesutils.cpp
)
dsr_configure_unit_test(ut_screenshot_session)
dsr_add_qt_test(ut_screenshot_session)
target_include_directories(ut_screenshot_session PRIVATE ${src} ${DSR_OPENCV_MOBILE_INCLUDE_DIR})
target_link_libraries(ut_screenshot_session PRIVATE Qt6::WaylandClientPrivate Wayland::Client ${DSR_TEST_OPENCV})
