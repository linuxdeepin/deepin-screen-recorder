# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

add_subdirectory(startup)
add_subdirectory(ut_screenshot_session)
add_subdirectory(ut_pin_screenshots)
add_subdirectory(ut_dde_dock_plugins)

add_custom_target(dsr_unit_tests DEPENDS
    ut_screen_shot_recorder ut_screenshot_session ut_pin_screenshots
    ut_record_time ut_shot_start ut_shot_start_record
    test_startup test_cpuvendor
)
