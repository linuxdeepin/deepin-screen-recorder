# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(PLUGIN_NAME "deepin-screen-shotcut-plugin")

file(GLOB SRCS "*.h" "*.cpp")

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DFrameworkdbus REQUIRED)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} res.qrc)
target_include_directories(${PLUGIN_NAME} PUBLIC
  ${QGSettings_INCLUDE_DIRS}
  #../../../interfaces
)
target_link_libraries(${PLUGIN_NAME} PRIVATE
  Qt5::DBus
  Qt5::Widgets
  Qt5::Svg
  ${Qt5Widgets_LIBRARIES}
  ${DFrameworkdbus_LIBRARIES}
  ${QGSettings_LIBRARIES}
)
install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins)
