set(APP_BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin/)
set(BIN_NAME ${CMAKE_PROJECT_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQT_QML_DEBUG_NO_WARNING)
endif()

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Widgets Quick DBus)

find_package(PkgConfig REQUIRED)

# 保证 src 目录下头文件全局可见
include_directories(src)
include_directories(../libdscreen-recorder)
include_directories(../libdscreen-recorder/utils)
include_directories(../libdscreen-recorder/platforms)
include_directories(./dbusservice)
include_directories(./gui)

aux_source_directory(./ ALLSOURCE)
aux_source_directory(./dbusservice ALLSOURCE)
aux_source_directory(./gui ALLSOURCE)

add_executable(${BIN_NAME} ${ALLSOURCE})

qt_add_qml_module(${BIN_NAME}
    URI ${SCREENRECORDERCORE_QML_URI}
    VERSION 1.0
    QML_FILES gui/ImageOverlay.qml
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::DBus
    dscreen-recorder
)

# Add install rule
install(TARGETS ${BIN_NAME} DESTINATION ${APP_BIN_INSTALL_DIR})
# install Desktop
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/${CMAKE_PROJECT_NAME}.desktop DESTINATION share/applications)
# install Manual App icon
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/${CMAKE_PROJECT_NAME}.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)
# install service
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/com.deepin.Screenshot.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/com.deepin.ScreenRecorder.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)
