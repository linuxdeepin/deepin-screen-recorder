project(libdscreen-recorder)
#工程名称
set(CMD_NAME dscreen-recorder)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(./utils)
include_directories(./platforms)

aux_source_directory(./utils ALL_SOURCES)
aux_source_directory(./platforms ALL_SOURCES)
aux_source_directory(. ALL_SOURCES)

#使用第三方库需要用到的一个包
find_package(PkgConfig REQUIRED)

find_package(
    Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED
        Core
        Concurrent
        Widgets
        DBus
        PrintSupport
        Test
        WaylandClient
        Multimedia
)

qt6_add_resources(QRC_FILES ${QRC_SOURCE_FILES})

add_library(${CMD_NAME} SHARED ${ALL_SOURCES})
set_target_properties(${CMD_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1.0)

target_link_libraries(${CMD_NAME} PRIVATE
    Qt::Concurrent
    Qt::DBus
    Qt::PrintSupport
    Qt::GuiPrivate
    Qt::WaylandClient
    Wayland::Client
)

include(GNUInstallDirs)
install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
