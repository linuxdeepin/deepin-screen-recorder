# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

include(cmake/DSRHelpers.cmake)

dsr_detect_app_version(_dsr_default_version)
project(deepin-screen-recorder LANGUAGES C CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(DSR_APP_VERSION "${_dsr_default_version}" CACHE STRING "Application version reported by the binaries")
if(DSR_APP_VERSION STREQUAL "")
    set(DSR_APP_VERSION "0.0.0")
endif()

dsr_detect_platform()
option(DSR_ENABLE_DDE_START "Enable startdde acceleration support" ${DSR_DETECTED_DDE_START})
option(DSR_ENABLE_OCR_SCROLL "Enable OCR and scroll screenshot support" ${DSR_DETECTED_OCR_SCROLL})
option(DSR_ENABLE_KF5_WAYLAND "Enable the legacy KF5 Wayland recording path" ${DSR_DETECTED_KF5_WAYLAND})
option(DSR_ENABLE_KWAYLAND_REMOTE "Enable the legacy KWayland remote extension" ${DSR_DETECTED_KWAYLAND_REMOTE})
option(DSR_ENABLE_KWAYLAND_RELEASE "Enable the legacy KWayland buffer release extension" ${DSR_DETECTED_KWAYLAND_RELEASE})

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    DBus
    Sql
    Xml
    PrintSupport
    Svg
    Concurrent
    LinguistTools
    Multimedia
    MultimediaWidgets
    OpenGL
    OpenGLWidgets
    WaylandClient
)
# qt6_add_lrelease (used for translations) is available since Qt 6.2.
if(Qt6_VERSION VERSION_LESS 6.2)
    message(FATAL_ERROR "Qt 6.2 or newer is required, found ${Qt6_VERSION}")
endif()

find_package(Dtk6 REQUIRED COMPONENTS Widget Gui Core Log)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DSR_TABLE_RECOGNIZER QUIET IMPORTED_TARGET dtk6tablerecognizer)
if(DSR_TABLE_RECOGNIZER_FOUND)
    set(_dsr_table_default ON)
else()
    set(_dsr_table_default OFF)
endif()
option(DSR_ENABLE_TABLE_RECOGNIZER "Enable table recognition in the pin-screenshots process" ${_dsr_table_default})
if(DSR_ENABLE_TABLE_RECOGNIZER AND NOT DSR_TABLE_RECOGNIZER_FOUND)
    message(FATAL_ERROR "DSR_ENABLE_TABLE_RECOGNIZER=ON but dtk6tablerecognizer was not found")
endif()

message(STATUS "deepin-screen-recorder ${DSR_APP_VERSION}")
message(STATUS "  DDE start acceleration : ${DSR_ENABLE_DDE_START}")
message(STATUS "  OCR / scroll capture   : ${DSR_ENABLE_OCR_SCROLL}")
message(STATUS "  Legacy KF5 Wayland     : ${DSR_ENABLE_KF5_WAYLAND}")
message(STATUS "  Table recognition      : ${DSR_ENABLE_TABLE_RECOGNIZER}")

add_subdirectory(src)
add_subdirectory(src/pin_screenshots)
add_subdirectory(src/dde-dock-plugins)

option(DSR_BUILD_TESTS "Build the CMake-native regression tests" OFF)
if(DSR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/startup)
endif()

include(cmake/DSRTranslations.cmake)
include(cmake/DSRInstall.cmake)
