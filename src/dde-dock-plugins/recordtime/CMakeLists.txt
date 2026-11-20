# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(DSR_PLUGIN_TARGET deepin-screen-recorder-plugin)

add_library(${DSR_PLUGIN_TARGET} SHARED
    recordtimeplugin.cpp
    timewidget.cpp
    dbusservice.cpp
    timewidget_interface.cpp
    recordtimeplugin.h
    timewidget.h
    dbusservice.h
    timewidget_interface.h
    res.qrc
)

dsr_set_common_compile_options(${DSR_PLUGIN_TARGET})
dsr_add_dtk_fallback_includes(${DSR_PLUGIN_TARGET})
target_compile_definitions(${DSR_PLUGIN_TARGET} PRIVATE QT_PLUGIN)

target_include_directories(${DSR_PLUGIN_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${DSR_PLUGIN_TARGET} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::DBus
    Qt6::PrintSupport
    Dtk6::Widget
    Dtk6::Gui
    Dtk6::Core
    Dtk6::Log
)

install(TARGETS ${DSR_PLUGIN_TARGET}
    LIBRARY DESTINATION lib/dde-dock/plugins
)
