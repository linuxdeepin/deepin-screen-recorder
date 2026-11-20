# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

if(DSR_ENABLE_DDE_START)
    set(_dsr_dbus_services
        "${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.ScreenRecorder.service"
        "${CMAKE_CURRENT_SOURCE_DIR}/com.deepin.Screenshot.service"
    )
else()
    set(_dsr_dbus_services
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/com.deepin.ScreenRecorder.service"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/com.deepin.Screenshot.service"
    )
endif()

install(FILES ${_dsr_dbus_services}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/image/deepin-screen-recorder.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/image/deepin-screenshot.svg"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/deepin-screen-recorder.desktop"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/deepin-screen-recorder"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-manual/manual-assets/application
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/configs/org.deepin.screen-recorder.json"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dsg/configs/org.deepin.screen-recorder
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/treeland/org.deepin.dde.treeland/org.deepin.dde.treeland.app/deepin-screen-recorder/90-disable-splash.json"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dsg/configs/overrides/org.deepin.dde.treeland/org.deepin.dde.treeland.app/deepin-screen-recorder
)
