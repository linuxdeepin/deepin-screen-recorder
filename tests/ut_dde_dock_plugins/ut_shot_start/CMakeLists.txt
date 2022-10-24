set(BIN ut_shot_start)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "-fno-access-control")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
add_definitions(-DSN_API_NOT_YET_FROZEN)

include(../../../3rdparty/stub_linux/stub.cmake)
find_package(Qt5 REQUIRED COMPONENTS
  Core Widgets DBus Test)
find_package(DFrameworkdbus REQUIRED)
#find_package(Qt5Widgets REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(GTest REQUIRED)

add_executable(${BIN}
  ${stub_HEAD}
  ${stud_SRC}
  ut_mock_pluginproxyinterface.h
  ../../../../dde-dock-plugins/shotstart/iconwidget.h
  ../../../../dde-dock-plugins/shotstart/shotstartplugin.h
  ../../../../dde-dock-plugins/shotstart/tipswidget.h
  main.cpp
  ../../../../dde-dock-plugins/shotstart/iconwidget.cpp
  ../../../../dde-dock-plugins/shotstart/shotstartplugin.cpp
  ../../../../dde-dock-plugins/shotstart/tipswidget.cpp
  ut_iconwidget.cpp
  ut_shotstartplugin.cpp
  ut_tipswidget.cpp
)
#target_include_directories(${BIN} PUBLIC
#  ../../../../interfaces
#)

target_link_libraries(${BIN}
  Qt5::Core 
  Qt5::Test 
  Qt5::Widgets 
  Qt5::DBus
  ${DFrameworkdbus_LIBRARIES}
  ${DtkWidget_LIBRARIES}
  ${DtkGui_LIBRARIES}
  ${GTEST_LIBRARIES}
  -lpthread
  -lgmock
  -lm 
  -lgcov
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${BIN} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${BIN} PRIVATE -fprofile-arcs -ftest-coverage)
endif()
add_test(NAME ${BIN} COMMAND ${BIN})
