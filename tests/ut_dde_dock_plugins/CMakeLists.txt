# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

set(src ${CMAKE_SOURCE_DIR}/src)
set(dock ${src}/dde-dock-plugins)

function(dsr_add_dock_test target)
    add_executable(${target} ${target}/main.cpp ${ARGN})
    dsr_configure_unit_test(${target})
    dsr_use_gtest(${target})
    target_include_directories(${target} PRIVATE /usr/include/dde-dock)
endfunction()

dsr_add_dock_test(ut_record_time
    ${dock}/recordtime/recordtimeplugin.cpp
    ${dock}/recordtime/timewidget.cpp
    ${dock}/recordtime/timewidget_interface.cpp
    ${dock}/recordtime/dbusservice.cpp
    ut_record_time/ut_mock_stub_recordtimeplugin.cpp
    ut_record_time/ut_timewidget.cpp
    ut_record_time/ut_timewidget_interface.cpp
    ut_record_time/ut_dbusservice.cpp
    ut_record_time/ut_record_time_cov2.cpp
)
dsr_add_dock_test(ut_shot_start
    ${src}/dbus_name.cpp
    ${dock}/shotstart/iconwidget.cpp
    ${dock}/shotstart/iconwidget_interface.cpp
    ${dock}/shotstart/shotstartplugin.cpp
    ${dock}/shotstart/tipswidget.cpp
    ${dock}/shotstart/quickpanelwidget.cpp
    ${dock}/shotstart/commoniconbutton.cpp
    ut_shot_start/ut_iconwidget.cpp
    ut_shot_start/ut_iconwidget_interface.cpp
    ut_shot_start/ut_commoniconbutton_cov.cpp
    ut_shot_start/ut_shotstartplugin.cpp
    ut_shot_start/ut_tipswidget.cpp
    ut_shot_start/ut_shot_start_cov2.cpp
)
dsr_add_dock_test(ut_shot_start_record
    ${src}/dbus_name.cpp
    ${dock}/shotstartrecord/recordiconwidget.cpp
    ${dock}/shotstartrecord/recordiconwidget_interface.cpp
    ${dock}/shotstartrecord/shotstartrecordplugin.cpp
    ${dock}/shotstartrecord/tipswidget.cpp
    ${dock}/shotstartrecord/quickpanelwidget.cpp
    ${dock}/shotstartrecord/commoniconbutton.cpp
    ut_shot_start_record/ut_tipswidget.cpp
    ut_shot_start_record/ut_shotstartrecordplugin.cpp
    ut_shot_start_record/ut_recordiconwidget.cpp
    ut_shot_start_record/ut_recordiconwidget_interface.cpp
    ut_shot_start_record/ut_commoniconbutton_cov.cpp
    ut_shot_start_record/ut_shot_start_record_cov2.cpp
)
target_compile_definitions(ut_shot_start PRIVATE UNIT_TEST)
target_compile_definitions(ut_shot_start_record PRIVATE UNIT_TEST)
target_compile_options(ut_shot_start PRIVATE -fno-access-control)
target_compile_options(ut_shot_start_record PRIVATE -fno-access-control)
