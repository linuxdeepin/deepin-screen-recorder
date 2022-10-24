set(BIN deepin-pin-screenshots)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core DBus Gui Widgets)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#include(../accessibility/accessible.cmake)
set(screenshots_HEAD
  service/dbuspinscreenshotsadaptor.h
  service/pinscreenshotsinterface.h
  service/ocrinterface.h
  ui/menucontroller.h
  ui/toolbarwidget.h
  ui/toolbar.h
  ui/subtoolwidget.h
  mainwindow.h
  pinscreenshots.h
  putils.h
)
set(screenshots_SRC
  service/dbuspinscreenshotsadaptor.cpp
  service/pinscreenshotsinterface.cpp
  service/ocrinterface.cpp
  ui/menucontroller.cpp
  ui/toolbarwidget.cpp
  ui/toolbar.cpp
  ui/subtoolwidget.cpp
  mainwindow.cpp
  main.cpp
  pinscreenshots.cpp
  putils.cpp
)

add_executable(${BIN}
  ${screenshots_HEAD}
  ${screenshots_SRC}
  accessibility/acObjectList.h
  accessibility/acTextDefine.h
  accessibility/acObjectList.h
  ../assets/icons/pin_icons.qrc
  ../assets/image/pin_screenshots.qrc
)

target_link_libraries(${BIN}
  ${DtkWidget_LIBRARIES}
  ${DtkGui_LIBRARIES}
  ${DtkCore_LIBRARIES}
  Qt5::DBus
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
)
install(FILES com.deepin.PinScreenShots.service DESTINATION "${CMAKE_INSTALL_DATADIR}/dbus-1/services")
install(TARGETS ${BIN} DESTINATION "${CMAKE_INSTALL_BINDIR}")
