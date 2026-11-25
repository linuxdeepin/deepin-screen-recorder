# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

set(pin ${CMAKE_SOURCE_DIR}/src/pin_screenshots)
set(utils ${CMAKE_SOURCE_DIR}/src/utils)
add_executable(ut_pin_screenshots
    main.cpp ut_pin_geometry_receiver.h
    ${pin}/settings.cpp
    ${pin}/putils.cpp
    ${pin}/ui/pinsavemenumanager.cpp
    ${pin}/service/pinscreenshotsinterface.cpp
    ${pin}/service/dbuspinscreenshotsadaptor.cpp
    ${utils}/log.cpp
)
dsr_configure_unit_test(ut_pin_screenshots)
dsr_use_gtest(ut_pin_screenshots)
target_compile_definitions(ut_pin_screenshots PRIVATE _GNU_SOURCE)
target_include_directories(ut_pin_screenshots PRIVATE ${pin} ${pin}/ui ${pin}/service ${utils})
