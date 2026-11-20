# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Keep the installed translation set identical to the qmake baseline, which is
# defined by translations.pri. The directory also contains a template and
# translations that are not part of the packaged list.
set(DSR_TS_FILES "")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/translations.pri" _dsr_translations_pri)
file(GLOB _dsr_all_ts CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")
foreach(_ts IN LISTS _dsr_all_ts)
    get_filename_component(_ts_name "${_ts}" NAME)
    string(FIND "${_dsr_translations_pri}" "translations/${_ts_name}" _ts_pos)
    if(NOT _ts_pos EQUAL -1)
        list(APPEND DSR_TS_FILES "${_ts}")
    endif()
endforeach()

if(DSR_TS_FILES)
    qt6_add_lrelease(
        TS_FILES ${DSR_TS_FILES}
        QM_FILES_OUTPUT_VARIABLE DSR_QM_FILES
    )

    install(FILES ${DSR_QM_FILES}
        DESTINATION share/deepin-screen-recorder/translations
    )
endif()

add_custom_target(update_translations
    COMMAND $<TARGET_FILE:Qt6::lupdate>
        -no-obsolete
        -locations none
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        -ts ${DSR_TS_FILES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Updating translation sources from src/"
    VERBATIM
)
