cmake_minimum_required( VERSION 3.1.0 )
set (DVERSION "1.0.0" CACHE STRING "define project version")
project(deepin-screen-recorder
  VERSION ${DVERSION}
  DESCRIPTION "Screen shot and record"
  HOMEPAGE_URL "https://github.com/linuxdeepin/deepin-screen-recorder"
  LANGUAGES CXX C
)
# Not Finished
# FIXME KWayland/Client/clientmanagement.h not exist
set(USE_DEEPIN_WAYLAND OFF CACHE BOOL "Wayland Support")
set(UNUSE_DEFAULT_FFMPEG OFF CACHE BOOL "Not Use default ffmpeg")
set(FFMPEG_PATH "/usr/lib/ffmpeg4.4/pkgconfig" CACHE STRING "define project version")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# FINISHED
set (OCR_SCROLL_FLAGE_ON OFF CACHE BOOL "Opencv Support")
include(GNUInstallDirs)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fsanitize=address -fno-omit-frame-pointer")
  set(BUILD_TESTING ON)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()
add_subdirectory(src)
add_subdirectory(pin_screenshots)
add_subdirectory(dde-dock-plugins)
if(BUILD_TESTING)
  message("==================================")
  message("       Now Testing is enabled     ")
  message("==================================")
  enable_testing()
  add_subdirectory(tests)
endif()
