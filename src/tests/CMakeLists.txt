cmake_minimum_required( VERSION 3.1.0 )
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
set (PROJECT_NAME ut-deepin-screen-recorder)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QtModule Core Gui Test Widgets Network DBus Sql PrintSupport Svg Concurrent LinguistTools Multimedia MultimediaWidgets X11Extras)

project( ${PROJECT_NAME} )

INCLUDE_DIRECTORIES(googletest/googletest/include)
INCLUDE_DIRECTORIES(googletest/googlemock/include)

set(LINK_DIR X11 Xext Xtst Xfixes Xcursor procps gtest)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 CONFIG REQUIRED ${QtModule})

set (ut-deepin-screen-recorder_HDRS
    ../widgets/toolbutton.h
    )

set ( ut-deepin-screen-recorder_SRCS
    main.cpp
    ../utils/audioutils.cpp
    ../utils/baseutils.cpp
    ../utils/calculaterect.cpp
    ../utils/configsettings.cpp
    ../utils/dbusutils.cpp
    ../utils/desktopinfo.cpp
    ../utils/screengrabber.cpp
    ../utils/shortcut.cpp
    ../utils/tempfile.cpp
    ../utils/shapesutils.cpp
    ../utils/camerawatcher.cpp
    ../utils/voicevolumewatcher.cpp
    ../widgets/colorbutton.cpp
    ../widgets/colortoolwidget.cpp
    ../widgets/keybuttonwidget.cpp
    ../widgets/maintoolwidget.cpp
    ../widgets/textbutton.cpp
    ../widgets/zoomIndicator.cpp
    ../widgets/filter.cpp
    ../widgets/tooltips.cpp
    ../widgets/sidebar.cpp
    ../widgets/toolbar.cpp
    ../widgets/subtoolwidget.cpp
    ../widgets/camerawidget.cpp
    ../widgets/toptips.cpp
    ../widgets/shapeswidget.cpp
    ../widgets/textedit.cpp
    ../widgets/shottoolwidget.cpp
    ../menucontroller/menucontroller.cpp
    ../dbusinterface/dbusnotify.cpp
    ../dbusinterface/drawinterface.cpp
    ../constant.cpp
    ../utils.cpp
    ../button_feedback.cpp
    ../countdown_tooltip.cpp
    ../main_window.cpp
    ../record_process.cpp
    ../RecorderRegionShow.cpp
    ../screenshot.cpp
    ../settings.cpp
    ../show_buttons.cpp
    ../process_tree.cpp
    ../screen_shot_event.cpp
    ../event_monitor.cpp
    )

add_executable(${PROJECT_NAME}
    ${ut-deepin-screen-recorder_HDRS}
    ${ut-deepin-screen-recorder_SRCS}
    )

qt5_use_modules(${PROJECT_NAME} ${QtModule})
find_package(PkgConfig REQUIRED)
pkg_check_modules(3rd_lib REQUIRED
    dtkwidget dtkgui dframeworkdbus xcb xcb-util
    )
target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES} ${LINK_DIR} )

add_custom_target(test
    COMMAND mkdir -p coverageResult
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_command(TARGET test
    COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "
    COMMAND lcov --directory ./tests/CMakeFiles/${PROJECT_NAME}.dir --zerocounters
    COMMAND ./tests/${PROJECT_NAME}
    COMMAND lcov --directory ./tests --capture --output-file ./coverageResult/${PROJECT_NAME}_Coverage.info
    COMMAND echo " =================== do filter begin ==================== "
    COMMAND lcov --remove ./coverageResult/${PROJECT_NAME}_Coverage.info
    '*/${PROJECT_NAME}_autogen/*' '*/${PROJECT_NAME}_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*'
    -o ./coverageResult/${PROJECT_NAME}_Coverage.info
    COMMAND echo " =================== do filter end ==================== "
    COMMAND genhtml -o ./coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage.info
    COMMAND echo " =================== CREAT LCOV REPROT END ==================== "
    COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

add_dependencies(test ${PROJECT_NAME})
