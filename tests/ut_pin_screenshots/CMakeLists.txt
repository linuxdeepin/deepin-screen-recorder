#cmake_minimum_required(VERSION 3.10)
set(BIN_NAME ut_pin_screenshots)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "-fno-access-control")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
add_definitions(-DSN_API_NOT_YET_FROZEN)

include(../../3rdparty/stub_linux/stub.cmake)

find_package(DFrameworkdbus REQUIRED)
find_package(PkgConfig REQUIRED)
#find_package(Qt5Widgets REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
#find_package(OpenCV REQUIRED)
find_package(GTest REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS
  Core Widgets DBus)

add_executable(${BIN_NAME}
  ${stub_HEAD}
  ${stud_SRC}
  main.cpp 
  ut_pin_screenshots.cpp
)
target_link_libraries(${BIN_NAME}
  Qt5::Core 
  Qt5::Widgets 
  Qt5::DBus
  ${DFrameworkdbus_LIBRARIES}
  ${DtkWidget_LIBRARIES}
  ${DtkGui_LIBRARIES}
  ${GTEST_LIBRARIES}
  -lpthread
  -lm 
  -lgcov
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()
add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
