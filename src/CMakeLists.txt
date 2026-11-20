# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(DSR_MAIN_TARGET deepin-screen-recorder)

set(DSR_MAIN_SOURCES
    accessibility/acObjectList.cpp
    main.cpp
    capture.cpp
    utils/logger.cpp
    main_window.cpp
    qwayland-treeland-capture-unstable-v1.cpp
    record_process.cpp
    utils.cpp
    utils/log.cpp
    countdown_tooltip.cpp
    constant.cpp
    event_monitor.cpp
    button_feedback.cpp
    show_buttons.cpp
    menucontroller/menucontroller.cpp
    utils/proxyaudioport.cpp
    utils/shapesutils.cpp
    utils/imageeffects.cpp
    utils/tempfile.cpp
    utils/calculaterect.cpp
    utils/shortcut.cpp
    utils/configsettings.cpp
    utils/dbusutils.cpp
    utils/baseutils.cpp
    utils/voicevolumewatcher_interface.cpp
    utils/x_multi_screen_info.cpp
    utils_interface.cpp
    wayland-treeland-capture-unstable-v1-protocol.c
    widgets/toptips.cpp
    widgets/fineadjusttips.cpp
    widgets/shapeswidget.cpp
    widgets/textedit.cpp
    widgets/zoomIndicator.cpp
    widgets/toolbar.cpp
    widgets/toolbarhost.cpp
    widgets/subtoolwidget.cpp
    widgets/aiassistantwidget.cpp
    widgets/keybuttonwidget.cpp
    widgets/sidebar.cpp
    widgets/shottoolwidget.cpp
    widgets/roundedcornerwidget.cpp
    widgets/colortoolwidget.cpp
    widgets/shapetoolwidget.cpp
    widgets/savebutton.cpp
    widgets/savemenumanager.cpp
    widgets/toolbutton.cpp
    dbusinterface/dbusnotify.cpp
    dbusservice/dbusscreenshotservice.cpp
    widgets/camerawidget.cpp
    screenshot.cpp
    utils/voicevolumewatcher.cpp
    utils/camerawatcher.cpp
    widgets/tooltips.cpp
    widgets/filter.cpp
    utils/screengrabber.cpp
    RecorderRegionShow.cpp
    recordertablet.cpp
    dbusinterface/ocrinterface.cpp
    dbusinterface/pinscreenshotsinterface.cpp
    dbusinterface/aiassistantinterface.cpp
    widgets/imagemenu.cpp
    utils/borderprocessinterface.cpp
    widgets/zoomIndicatorGL.cpp
    gstrecord/gstrecordx.cpp
    utils/audioutils.cpp
    gstrecord/gstinterface.cpp
    camera/devnummonitor.cpp
    camera/majorimageprocessingthread.cpp
    camera/LPF_V4L2.c
    camera/v4l2devicehandle.c
    utils/eventlogutils.cpp
    dbus_name.cpp
    ext-image-capture/manager/extcapturemanager.cpp
    ext-image-capture/manager/extoutputsourcemanager.cpp
    ext-image-capture/session/extcapturesession.cpp
    ext-image-capture/frame/extcaptureframe.cpp
    ext-image-capture/extcaptureintegration.cpp
    ext-image-capture/multiscreencapturecoordinator.cpp
    ext-image-capture/multiscreenframecompositor.cpp
    ext-image-capture/extcapturerecorder.cpp
    ext-image-capture/framepumpworker.cpp
    ext-image-capture/capturepipelineworker.cpp
    ext-image-capture/dmaframegpuconverter.cpp
    ext-image-capture/extcaptureframebuffer.cpp
    ext-image-capture/extcapturebridge.cpp
    protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.cpp
    protocols/ext-image-copy-capture/wayland-ext-image-copy-capture-v1-protocol.c
    protocols/ext-image-copy-capture/qwayland-ext-image-capture-source-v1.cpp
    protocols/ext-image-copy-capture/wayland-ext-image-capture-source-v1-protocol.c
    protocols/ext-image-copy-capture/qwayland-ext-foreign-toplevel-list-v1.cpp
    protocols/ext-image-copy-capture/wayland-ext-foreign-toplevel-list-v1-protocol.c
    protocols/linux-dmabuf/qwayland-linux-dmabuf-unstable-v1.cpp
    protocols/linux-dmabuf/wayland-linux-dmabuf-unstable-v1-protocol.c
    protocols/wlr-screencopy/wayland-wlr-screencopy-unstable-v1-protocol.c
    protocols/treeland-snap-target/qwayland-treeland-snap-target-unstable-v1.cpp
    protocols/treeland-snap-target/wayland-treeland-snap-target-unstable-v1-protocol.c
    protocols/treeland-snap-target/qwayland-xdg-toplevel-tag-v1.cpp
    protocols/treeland-snap-target/wayland-xdg-toplevel-tag-v1-protocol.c
    protocols/treeland-snap-target/wayland-xdg-shell-protocol.c
    screenshot/core/selectionmodel.cpp
    screenshot/core/selectionoverlaygeometry.cpp
    screenshot/core/selectionconstraints.cpp
    screenshot/adapters/shapeswidgetannotationeditor.cpp
    screenshot/export/screenshotexportservice.cpp
    screenshot/export/localscreenshotresultsink.cpp
    screenshot/session/annotationcoordinator.cpp
    screenshot/session/screenshotsessioncontroller.cpp
    screenshot/session/screenshotsessionfactory.cpp
    screenshot/merge/scrollmergecoordinator.cpp
    screenshot/platform/treeland/treelandstillcapture.cpp
    screenshot/platform/treeland/treelandsnapproxy.cpp
    screenshot/platform/treeland/treelandsnaphintsource.cpp
    screenshot/platform/treeland/treelandwindowtagger.cpp
    screenshot/platform/treeland/treelandannotationeditor.cpp
    screenshot/platform/treeland/treelandmaskwindow.cpp
    screenshot/platform/treeland/treelandmaskoverlayhost.cpp
    screenshot/platform/treeland/treelandrecordingsession.cpp
    screenshot/platform/treeland/treelandscrollcapture.cpp
    screenshot/platform/treeland/treelandscrollcompositor.cpp
    screenshot/platform/treeland/treelandoutputcapture.cpp
    screenshot/platform/treeland/recordingfiletransaction.cpp
    screenshot/platform/treeland/treelandscreenshotbackend.cpp
    utils/cpuvendor.cpp
    utils/ximageconverter.cpp
)

set(DSR_MAIN_HEADERS
    capture.h
    main_window.h
    qwayland-treeland-capture-unstable-v1.h
    record_process.h
    utils.h
    utils/log.h
    utils/logger.h
    countdown_tooltip.h
    constant.h
    event_monitor.h
    button_feedback.h
    show_buttons.h
    keydefine.h
    menucontroller/menucontroller.h
    utils/proxyaudioport.h
    utils/voicevolumewatcher_interface.h
    utils/x_multi_screen_info.h
    utils_interface.h
    utils/baseutils.h
    utils/configsettings.h
    utils/dbusutils.h
    utils/shortcut.h
    utils/tempfile.h
    utils/calculaterect.h
    utils/imageeffects.h
    utils/saveutils.h
    utils/shapesutils.h
    wayland-treeland-capture-unstable-v1-client-protocol.h
    widgets/zoomIndicator.h
    widgets/textedit.h
    widgets/toptips.h
    widgets/fineadjusttips.h
    widgets/toolbar.h
    widgets/toolbarhost.h
    widgets/shapeswidget.h
    widgets/toolbutton.h
    widgets/subtoolwidget.h
    widgets/aiassistantwidget.h
    widgets/keybuttonwidget.h
    widgets/sidebar.h
    widgets/shottoolwidget.h
    widgets/roundedcornerwidget.h
    widgets/colortoolwidget.h
    widgets/shapetoolwidget.h
    widgets/savebutton.h
    widgets/savemenumanager.h
    dbusinterface/dbusnotify.h
    dbusservice/dbusscreenshotservice.h
    widgets/camerawidget.h
    screenshot.h
    utils/voicevolumewatcher.h
    utils/camerawatcher.h
    widgets/tooltips.h
    widgets/filter.h
    utils/screengrabber.h
    RecorderRegionShow.h
    recordertablet.h
    dbusinterface/ocrinterface.h
    dbusinterface/pinscreenshotsinterface.h
    dbusinterface/aiassistantinterface.h
    widgets/imagemenu.h
    utils/borderprocessinterface.h
    widgets/zoomIndicatorGL.h
    gstrecord/gstrecordx.h
    utils/audioutils.h
    gstrecord/gstinterface.h
    camera/devnummonitor.h
    camera/LPF_V4L2.h
    camera/v4l2devicehandle.h
    camera/majorimageprocessingthread.h
    utils/eventlogutils.h
    widgets/slider.h
    ext-image-capture/manager/extcapturemanager.h
    ext-image-capture/manager/extoutputsourcemanager.h
    ext-image-capture/shmformat.h
    ext-image-capture/session/extcapturesession.h
    ext-image-capture/frame/extcaptureframe.h
    ext-image-capture/extcaptureintegration.h
    ext-image-capture/multiscreencapturecoordinator.h
    ext-image-capture/multiscreenframecompositor.h
    ext-image-capture/extcapturerecorder.h
    ext-image-capture/framepumpworker.h
    ext-image-capture/capturepipelineworker.h
    ext-image-capture/dmaframegpuconverter.h
    ext-image-capture/extcaptureframebuffer.h
    ext-image-capture/extcapturebridge.h
    protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h
    protocols/ext-image-copy-capture/wayland-ext-image-copy-capture-v1-client-protocol.h
    protocols/ext-image-copy-capture/qwayland-ext-image-capture-source-v1.h
    protocols/ext-image-copy-capture/wayland-ext-image-capture-source-v1-client-protocol.h
    protocols/ext-image-copy-capture/qwayland-ext-foreign-toplevel-list-v1.h
    protocols/ext-image-copy-capture/wayland-ext-foreign-toplevel-list-v1-client-protocol.h
    protocols/linux-dmabuf/qwayland-linux-dmabuf-unstable-v1.h
    protocols/linux-dmabuf/wayland-linux-dmabuf-unstable-v1-client-protocol.h
    protocols/wlr-screencopy/wayland-wlr-screencopy-unstable-v1-client-protocol.h
    protocols/treeland-snap-target/qwayland-treeland-snap-target-unstable-v1.h
    protocols/treeland-snap-target/wayland-treeland-snap-target-unstable-v1-client-protocol.h
    protocols/treeland-snap-target/qwayland-xdg-toplevel-tag-v1.h
    protocols/treeland-snap-target/wayland-xdg-toplevel-tag-v1-client-protocol.h
    protocols/treeland-snap-target/wayland-xdg-shell-client-protocol.h
    screenshot/core/screenshottypes.h
    screenshot/core/coordinatemapper.h
    screenshot/core/selectionoverlaygeometry.h
    screenshot/core/selectionmodel.h
    screenshot/core/selectionconstraints.h
    screenshot/ports/iscreenshotbackend.h
    screenshot/ports/iscreenshotoverlayhost.h
    screenshot/ports/iannotationeditor.h
    screenshot/ports/irecordingsession.h
    screenshot/ports/iscrollcapture.h
    screenshot/ports/iscrollmerger.h
    screenshot/ports/iscreenshotresultsink.h
    screenshot/ports/isnaphintsource.h
    screenshot/ports/iwindowtagger.h
    screenshot/adapters/shapeswidgetannotationeditor.h
    screenshot/core/inputregiongeometry.h
    screenshot/session/annotationcoordinator.h
    screenshot/merge/scrollmergecoordinator.h
    screenshot/export/screenshotexportservice.h
    screenshot/export/localscreenshotresultsink.h
    screenshot/export/aiscreenshotfile.h
    screenshot/session/screenshotsessioncontroller.h
    screenshot/session/screenshotsessionfactory.h
    screenshot/session/platformdetection.h
    screenshot/platform/treeland/treelandstillcapture.h
    screenshot/platform/treeland/treelandsnapproxy.h
    screenshot/platform/treeland/treelandsnaphintsource.h
    screenshot/platform/treeland/treelandwindowtagger.h
    screenshot/platform/treeland/treelandannotationeditor.h
    screenshot/platform/treeland/treelandmaskwindow.h
    screenshot/platform/treeland/treelandmaskoverlayhost.h
    screenshot/platform/treeland/treelandrecordingsession.h
    screenshot/platform/treeland/treelandscrollcapture.h
    screenshot/platform/treeland/treelandscrollcompositor.h
    screenshot/platform/treeland/treelandoutputcapture.h
    screenshot/platform/treeland/recordingfiletransaction.h
    screenshot/platform/treeland/treelandscreenshotbackend.h
    utils/cpuvendor.h
    utils/ximageconverter.h
    accessibility/accessiblefunctions.h
    accessibility/acObjectList.h
    accessibility/acTextDefine.h
)

if(DSR_ENABLE_OCR_SCROLL)
    list(APPEND DSR_MAIN_SOURCES
        widgets/scrollshottip.cpp
        utils/pixmergethread.cpp
        utils/scrollScreenshot.cpp
        widgets/previewwidget.cpp
    )
    list(APPEND DSR_MAIN_HEADERS
        widgets/scrollshottip.h
        utils/pixmergethread.h
        utils/scrollScreenshot.h
        widgets/previewwidget.h
    )
endif()

file(GLOB DSR_LIBCAM_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/3rdparty/libcam/libcam/*.c
    ${CMAKE_SOURCE_DIR}/3rdparty/libcam/libcam_v4l2core/*.c
    ${CMAKE_SOURCE_DIR}/3rdparty/libcam/load_libs.c
)

set(DSR_MAIN_RESOURCES
    ${CMAKE_SOURCE_DIR}/assets/image/deepin-screen-recorder.qrc
    ${CMAKE_SOURCE_DIR}/assets/icons/icons.qrc
)

add_executable(${DSR_MAIN_TARGET}
    ${DSR_MAIN_SOURCES}
    ${DSR_MAIN_HEADERS}
    ${DSR_LIBCAM_SOURCES}
    ${DSR_MAIN_RESOURCES}
)

dsr_set_common_compile_options(${DSR_MAIN_TARGET})
dsr_apply_platform_definitions(${DSR_MAIN_TARGET})
dsr_apply_executable_hardening(${DSR_MAIN_TARGET})
dsr_apply_arch_flags(${DSR_MAIN_TARGET})
dsr_add_dtk_fallback_includes(${DSR_MAIN_TARGET})
set_target_properties(${DSR_MAIN_TARGET} PROPERTIES AUTOMOC OFF)

set(_dsr_main_include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/accessibility
    ${CMAKE_SOURCE_DIR}/3rdparty/libcam
    ${CMAKE_SOURCE_DIR}/3rdparty/libcam/libcam
    ${CMAKE_SOURCE_DIR}/3rdparty/libcam/libcam_v4l2core
)
target_include_directories(${DSR_MAIN_TARGET} PRIVATE ${_dsr_main_include_dirs})
include_directories(${_dsr_main_include_dirs})

# The combined mocs_compilation.cpp generated by CMake leaks Xlib's
# object-like macros (None, CursorShape, ...) into unrelated headers. Generate
# one moc unit per header like qmake did, so each translation unit keeps its
# own include order.
set(_dsr_main_moc_headers "")
foreach(_header IN LISTS DSR_MAIN_HEADERS)
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/${_header}" _moc_marker
        REGEX "Q_OBJECT|Q_GADGET|Q_NAMESPACE" LIMIT_COUNT 1)
    if(_moc_marker)
        list(APPEND _dsr_main_moc_headers "${CMAKE_CURRENT_SOURCE_DIR}/${_header}")
    endif()
endforeach()
qt6_wrap_cpp(DSR_MAIN_MOC_SOURCES ${_dsr_main_moc_headers})
target_sources(${DSR_MAIN_TARGET} PRIVATE ${DSR_MAIN_MOC_SOURCES})

pkg_check_modules(DSR_MAIN_DEPENDENCIES REQUIRED IMPORTED_TARGET
    xcb
    xcb-util
    gstreamer-app-1.0
    libusb-1.0
)
pkg_check_modules(DSR_MAIN_XLIBS REQUIRED IMPORTED_TARGET
    x11
    xext
    xtst
    xfixes
    xcursor
    xinerama
    libdrm
    gbm
)

target_link_libraries(${DSR_MAIN_TARGET} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::DBus
    Qt6::Sql
    Qt6::Xml
    Qt6::PrintSupport
    Qt6::Svg
    Qt6::Concurrent
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::WaylandClient
    Qt6::WaylandClientPrivate
    Dtk6::Widget
    Dtk6::Gui
    Dtk6::Core
    Dtk6::Log
    PkgConfig::DSR_MAIN_DEPENDENCIES
    PkgConfig::DSR_MAIN_XLIBS
    OpenGL::GL
    OpenGL::EGL
    ${CMAKE_DL_LIBS}
)

if(DSR_ENABLE_OCR_SCROLL)
    find_library(DSR_OPENCV_SMALL NAMES opencv_small REQUIRED)
    if(EXISTS /usr/include/opencv_mobile)
        target_include_directories(${DSR_MAIN_TARGET} PRIVATE /usr/include/opencv_mobile)
    endif()
    target_link_libraries(${DSR_MAIN_TARGET} PRIVATE ${DSR_OPENCV_SMALL})
endif()

install(TARGETS ${DSR_MAIN_TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
