# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(DSR_PLUGIN_TARGET shot-start-record-plugin)

add_library(${DSR_PLUGIN_TARGET} SHARED
    recordiconwidget_interface.cpp
    tipswidget.cpp
    quickpanelwidget.cpp
    commoniconbutton.cpp
    shotstartrecordplugin.cpp
    recordiconwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../dbus_name.cpp
    recordiconwidget_interface.h
    tipswidget.h
    quickpanelwidget.h
    commoniconbutton.h
    shotstartrecordplugin.h
    recordiconwidget.h
    res.qrc
)

dsr_set_common_compile_options(${DSR_PLUGIN_TARGET})
dsr_add_dtk_fallback_includes(${DSR_PLUGIN_TARGET})
target_compile_definitions(${DSR_PLUGIN_TARGET} PRIVATE QT_PLUGIN)

target_include_directories(${DSR_PLUGIN_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
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

install(FILES com.deepin.dde.dock.module.shot-start-record-plugin.gschema.xml
    DESTINATION share/glib-2.0/schemas
)
install(FILES res/dcc-shot-start-record-plugin.dci
    DESTINATION share/dde-dock/icons/dcc-setting
)
