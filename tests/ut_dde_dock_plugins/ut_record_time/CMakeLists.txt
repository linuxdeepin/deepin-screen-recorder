set(BIN ut_record_time)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "-fno-access-control")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
add_definitions(-DSN_API_NOT_YET_FROZEN)

include(../../../3rdparty/stub_linux/stub.cmake)
find_package(Qt5 REQUIRED COMPONENTS
  Core Widgets DBus)
find_package(DFrameworkdbus REQUIRED)
#find_package(Qt5Widgets REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(GTest REQUIRED)
add_executable(${BIN}
  ${stub_HEAD}
  ${stud_SRC}
  main.cpp 
  ut_timewidget.cpp
  ut_mock_pluginproxyinterface.h
  ../../../dde-dock-plugins/recordtime/timewidget.h
  ../../../dde-dock-plugins/recordtime/dbusservice.h
  ../../../dde-dock-plugins/recordtime/recordtimeplugin.h
  ../../../dde-dock-plugins/recordtime/recordtimeplugin.cpp
  ../../../dde-dock-plugins/recordtime/timewidget.cpp
  ../../../dde-dock-plugins/recordtime/dbusservice.cpp
  ut_mock_stub_recordtimeplugin.cpp
  ut_dbusservice.cpp
)
#target_include_directories(${BIN} PUBLIC
#  ../../../../interfaces
#)
target_link_libraries(${BIN}
  Qt5::Core 
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
