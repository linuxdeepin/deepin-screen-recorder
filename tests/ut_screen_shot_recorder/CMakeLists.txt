# 这个单元测试写的是什么鬼？怎么一堆h文件，这单元测试了个锤子
# 覆盖率东西搞不上去，不加test了
#cmake_minimum_required(VERSION 3.10)
set(BIN ut_screen_shot)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-fno-access-control")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_definitions(-DDDE_START_FLAGE_ON)
add_definitions(-DOCR_SCROLL_FLAGE_ON)
add_definitions(-DSN_API_NOT_YET_FROZEN)
find_package(Qt5 REQUIRED COMPONENTS
  Core Test Gui Widgets DBus Sql Svg
  Concurrent Multimedia MultimediaWidgets X11Extras)
find_package(DFrameworkdbus)
find_package(PkgConfig REQUIRED)
#find_package(Qt5Widgets REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GTest REQUIRED)
#find_package(KF5Wayland REQUIRED)
pkg_check_modules(gobject REQUIRED gobject-2.0)
pkg_check_modules(xcb_util REQUIRED xcb-util)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XEXT REQUIRED xext)
pkg_check_modules(XFIXES REQUIRED xfixes)
pkg_check_modules(Xtst REQUIRED xtst)

pkg_check_modules(gstreamer-app REQUIRED gstreamer-app-1.0)
pkg_check_modules(gstreamer-audio REQUIRED gstreamer-audio-1.0)

pkg_check_modules(libusb REQUIRED libusb-1.0)
pkg_check_modules(libprocps REQUIRED libprocps)
add_definitions(-DAPP_VERSION="1.0.0")
include(../../3rdparty/libcam/libcam.cmake)
include(../../3rdparty/stub_linux/stub.cmake)
include(../../src/glob.cmake)
include(../../src/dbusinterface/dbusinterface.cmake)
include(../../src/menucontroller/menucontroller.cmake)
include(../../src/widgets/widgets.cmake)
include(../../src/camera/camera.cmake)
include(../../src/utils/utils.cmake)
include(../../src/gstrecord/gstrecord.cmake)
include(utils/ut_utils.cmake)

# 修不好了
set(USE_DEEPIN_WAYLAND OFF CACHE BOOL "Wayland Support")

# 实际是强制开启的
set (OCR_SCROLL_FLAGE_ON ON CACHE BOOL "OpenCV Support")
add_definitions(-DOCR_SCROLL_FLAGE_ON=1)
add_definitions(-DOPENCV_VERSION_MAR=${OpenCV_VERSION_MAJOR})
add_definitions(-DENABLE_TSAN_TOOL)
#add_definitions(-DNONE=0)
add_definitions(-DDDE_START_FLAGE_ON)
file(GLOB test_SRC
  *.cpp
  dbusinterface/*.cpp
  menucontroller/*.cpp
  widgets/*.cpp
  gstrecord/ut_gstrecordx.cpp
)

set(COMPILE_SRC
  ${glob_HEAD}
  ${libcam_TOP_HEAD}
  ${menucontroller_HEAD}
  ${dbusinterface_HEAD}
  ${utils_HEAD}
  ${widgets_HEAD}
  ${camera_HEAD}
  ${gstrecord_HEAD}

  ${libcam_TOP_SRC}
  ${menucontroller_SRC}
  ${dbusinterface_SRC}
  ${utils_SRC}
  ${widgets_SRC}
  ${camera_SRC}
  ${gstrecord_SRC}
  ${glob_SRC}
  
  ${utils_OPENCV}
  ${widgets_OPENCV}
  ${test_SRC}
  ${ut_utils_HEAD}

  testImg.qrc
  ../../assets/image/deepin-screen-recorder.qrc
  ../../assets/resources/resources.qrc
  ../../assets/icons/icons.qrc

)
file(GLOB WAYLAND_SRC
  waylandrecord/*.h
)
if(USE_DEEPIN_WAYLAND)
  add_definitions(-DKF5_WAYLAND_FLAGE_ON})
  list(APPEND COMPILE_SRC
    ${WAYLAND_SRC}
  )
endif()
add_executable(${BIN}
  ${COMPILE_SRC}
)
target_link_libraries(${BIN}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::DBus
  Qt5::Sql
  Qt5::Svg
  Qt5::Concurrent
  Qt5::Multimedia
  Qt5::MultimediaWidgets
  Qt5::X11Extras
  Qt5::Test
  #KF5::WaylandClient
  ${DFrameworkdbus_LIBRARIES}
  ${DtkWidget_LIBRARIES}
  ${gobject_LIBRARIES}
  ${gstreamer-app_LIBRARIES}
  ${gstreamer-audio_LIBRARIES}
  ${xcb_util_LIBRARIES}
  ${libusb_LIBRARIES}
  ${libprocps_LIBRARIES}
  ${OpenCV_LIBS}
  ${GTEST_LIBRARIES}
  ${XEXT_LIBRARIES}
  ${X11_LIBRARIES}
  ${Xtst_LIBRARIES}
  ${XFIXES_LIBRARIES}
)
target_include_directories(${BIN} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/../../src
  ${gobject_INCLUDE_DIRS}
  ${xcb_util_INCLUDE_DIRS}
  ${gstreamer-app_INCLUDE_DIRS}
  ${gstreamer-audio_INCLUDE_DIRS}
  ${libusb_INCLUDE_DIRS}
  ${libprocps_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${X11_INCLUDE_DIRS}
  ${XEXT_INCLUDE_DIRS}
  ${XFIXES_INCLUDE_DIRS}
  ${Xtst_INCLUDE_DIRS}
  -lpthread
  -lm 
  -lgcov
)
#if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#  target_compile_options(${BIN} PRIVATE -fprofile-instr-generate -ftest-coverage)
#endif()
#if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#  target_compile_options(${BIN} PRIVATE -fprofile-arcs -ftest-coverage)
#endif()

add_test(NAME ${BIN} COMMAND ${BIN})
