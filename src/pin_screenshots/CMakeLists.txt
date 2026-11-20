# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(DSR_PIN_TARGET deepin-pin-screenshots)

set(DSR_PIN_SOURCES
    service/dbuspinscreenshotsadaptor.cpp
    service/pinscreenshotsinterface.cpp
    mainwindow.cpp
    platform/treeland/treelandpinplacement.cpp
    main.cpp
    pinscreenshots.cpp
    service/ocrinterface.cpp
    ui/menucontroller.cpp
    ui/toolbarwidget.cpp
    ui/subtoolwidget.cpp
    ui/pinsavemenumanager.cpp
    putils.cpp
    settings.cpp
    ui/mainToolWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets/toolbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets/savebutton.cpp
)

set(DSR_PIN_HEADERS
    service/dbuspinscreenshotsadaptor.h
    service/pinscreenshotsinterface.h
    mainwindow.h
    platform/treeland/treelandpinplacement.h
    pinscreenshots.h
    service/ocrinterface.h
    ui/menucontroller.h
    ui/toolbarwidget.h
    ui/subtoolwidget.h
    ui/pinsavemenumanager.h
    putils.h
    settings.h
    ui/mainToolWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/log.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets/toolbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets/savebutton.h
    accessibility/accessiblefunctions.h
    accessibility/acObjectList.h
    accessibility/acTextDefine.h
)

if(DSR_ENABLE_TABLE_RECOGNIZER)
    list(APPEND DSR_PIN_SOURCES
        table/tablerecognizerloader.cpp
        ui/tableglasspanel.cpp
        ui/tablerecognizingwidget.cpp
        ui/tablerecognizetoast.cpp
        ui/tableresultdialog.cpp
    )
    list(APPEND DSR_PIN_HEADERS
        table/tablerecognizerloader.h
        ui/tableglasspanel.h
        ui/tablerecognizingwidget.h
        ui/tablerecognizetoast.h
        ui/tableresultdialog.h
    )
endif()

add_executable(${DSR_PIN_TARGET}
    ${DSR_PIN_SOURCES}
    ${DSR_PIN_HEADERS}
    icons/icons.qrc
)

qt6_generate_wayland_protocol_client_sources(${DSR_PIN_TARGET}
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/../protocols/treeland-dde-shell/treeland-dde-shell-v2.xml
)

dsr_set_common_compile_options(${DSR_PIN_TARGET})
dsr_apply_platform_definitions(${DSR_PIN_TARGET})
dsr_apply_executable_hardening(${DSR_PIN_TARGET})
dsr_add_dtk_fallback_includes(${DSR_PIN_TARGET})

target_compile_definitions(${DSR_PIN_TARGET} PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

target_include_directories(${DSR_PIN_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/accessibility
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets
)

target_link_libraries(${DSR_PIN_TARGET} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::DBus
    Qt6::PrintSupport
    Qt6::WaylandClientPrivate
    Wayland::Client
    Dtk6::Widget
    Dtk6::Gui
    Dtk6::Core
    Dtk6::Log
)

if(DSR_ENABLE_TABLE_RECOGNIZER)
    target_link_libraries(${DSR_PIN_TARGET} PRIVATE
        PkgConfig::DSR_TABLE_RECOGNIZER
    )
endif()

install(TARGETS ${DSR_PIN_TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES com.deepin.PinScreenShots.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)
