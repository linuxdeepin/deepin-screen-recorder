# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

include(GoogleTest)

pkg_check_modules(DSR_GTEST REQUIRED IMPORTED_TARGET gtest gmock)
find_path(DSR_OPENCV_MOBILE_INCLUDE_DIR opencv2/core.hpp PATH_SUFFIXES opencv_mobile)
find_library(DSR_TEST_OPENCV NAMES opencv_small)
if(NOT DSR_OPENCV_MOBILE_INCLUDE_DIR OR NOT DSR_TEST_OPENCV)
    message(FATAL_ERROR "Unit tests require libopencv-mobile-dev (headers and opencv_small)")
endif()

# Common configuration shared by every unit-test executable. Test registration
# is done by dsr_use_gtest() (per-test process) or dsr_add_qt_test().
function(dsr_configure_unit_test target)
    dsr_add_dtk_fallback_includes(${target})
    target_link_libraries(${target} PRIVATE
        Qt6::Test Qt6::Widgets Qt6::DBus Dtk6::Widget Dtk6::Gui Dtk6::Core
    )
    # The qmake tests were built with the default release config, so QT_NO_DEBUG
    # disabled Q_ASSERT. Keep that so tests using out-of-range access to probe
    # defensive behaviour do not abort on the Qt debug assertions.
    target_compile_definitions(${target} PRIVATE QT_NO_DEBUG)
    target_compile_options(${target} PRIVATE -Wall -Wno-deprecated-declarations)
    if(DSR_TEST_COVERAGE)
        target_compile_options(${target} PRIVATE --coverage -O0 -g1)
        target_link_options(${target} PRIVATE --coverage)
    endif()
endfunction()

# Register every GoogleTest case in its own process, matching the isolation the
# qmake coverage script used so one crashing/hanging case cannot take down the
# rest of the suite.
function(dsr_use_gtest target)
    cmake_parse_arguments(ARG "SERIAL" "FILTER" "" ${ARGN})

    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/stub_linux)
    target_link_libraries(${target} PRIVATE PkgConfig::DSR_GTEST)

    # Optional gtest FILTER (e.g. "*-Suite.Case") skips cases that hang in the
    # headless environment, mirroring the per-case timeout skips the original
    # qmake coverage harness applied. SERIAL runs cases one at a time for
    # suites that share global state and are not safe to run concurrently.
    set(_discover_args "")
    if(ARG_FILTER AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        list(APPEND _discover_args TEST_FILTER "${ARG_FILTER}")
    endif()
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.20)
        set(_props ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 180)
        if(ARG_SERIAL)
            list(APPEND _props RUN_SERIAL TRUE)
        endif()
        gtest_discover_tests(${target}
            DISCOVERY_TIMEOUT 60 ${_discover_args}
            PROPERTIES ${_props}
        )
    else()
        gtest_discover_tests(${target} DISCOVERY_TIMEOUT 60 ${_discover_args})
    endif()
endfunction()

# QtTest-style or plain executables run as a single test case.
function(dsr_add_qt_test target)
    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 180)
endfunction()
