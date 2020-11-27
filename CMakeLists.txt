cmake_minimum_required( VERSION 3.1.0 )
set (PROJECT_NAME deepin-screen-recorder)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QtModule Core Gui Test Widgets Network DBus Sql PrintSupport Svg Concurrent LinguistTools Multimedia MultimediaWidgets X11Extras)

project( ${PROJECT_NAME} )

#add_subdirectory( googletest )

#INCLUDE_DIRECTORIES(googletest/googletest/include)

set(LINK_DIR X11 Xext Xtst Xfixes Xcursor procps)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 CONFIG REQUIRED ${QtModule})

set ( deepin-screen-recorder_HDRS
    src/constant.h
    src/keydefine.h
    src/utils/baseutils.h
    src/utils/calculaterect.h
    src/utils/saveutils.h
    src/utils/shapesutils.h
    src/utils/dbusutils.h
    src/utils/desktopinfo.h
    src/RecorderRegionShow.h
    )

set ( deepin-screen-recorder_SRCS
    src/main.cpp
    src/main_window.cpp
    src/record_process.cpp
    src/settings.cpp
    src/utils.cpp
    src/record_button.cpp
    src/record_option_panel.cpp
    src/countdown_tooltip.cpp
    src/constant.cpp
    src/event_monitor.cpp
    src/button_feedback.cpp
    src/process_tree.cpp
    src/camera_process.cpp
    src/show_buttons.cpp
    src/utils/audioutils.cpp
    src/menucontroller/menucontroller.cpp
    src/utils/shapesutils.cpp
    src/utils/tempfile.cpp
    src/utils/calculaterect.cpp
    src/utils/shortcut.cpp
    src/utils/configsettings.cpp
    src/utils/baseutils.cpp
    src/widgets/savebutton.cpp
    src/widgets/toptips.cpp
    src/widgets/shapeswidget.cpp
    src/widgets/textbutton.cpp
    src/widgets/colorbutton.cpp
    src/widgets/fontsizewidget.cpp
    src/widgets/textedit.cpp
    src/widgets/bigcolorbutton.cpp
    src/widgets/zoomIndicator.cpp
    src/widgets/majtoolbar.cpp
    src/widgets/savetips.cpp
    src/widgets/toolbar.cpp
    src/widgets/maintoolwidget.cpp
    src/widgets/subtoolwidget.cpp
    src/widgets/keybuttonwidget.cpp
    src/widgets/sidebar.cpp
    src/widgets/shottoolwidget.cpp
    src/widgets/colortoolwidget.cpp
    src/dbusinterface/dbusnotify.cpp
    src/dbusinterface/dbuszone.cpp
    src/dbusinterface/dbuscontrolcenter.cpp
    src/dbusinterface/dbussoundeffect.cpp
    src/dbusservice/dbusscreenshotservice.cpp
    src/dbusservice/dbusscreenshot.cpp
    src/widgets/camerawidget.cpp
    src/utils/dbusutils.cpp
    src/screenshot.cpp
    src/utils/voicevolumewatcher.cpp
    src/utils/camerawatcher.cpp
    src/widgets/tooltips.cpp
    src/widgets/filter.cpp
    src/utils/desktopinfo.cpp
    src/utils/screengrabber.cpp
    src/dbusinterface/drawinterface.cpp
    src/screen_shot_event.cpp
    src/RecorderRegionShow.cpp
    )

set ( deepin-screen-recorder_RSCS
    deepin-screen-recorder.qrc
    src/widgets/resources.qrc
    src/icons/icons.qrc
    )

set ( deepin-screen-recorder_MOCS
    src/main_window.h
    src/record_process.h
    src/settings.h
    src/utils.h
    src/record_button.h
    src/record_option_panel.h
    src/countdown_tooltip.h
    src/event_monitor.h
    src/button_feedback.h
    src/process_tree.h
    src/camera_process.h
    src/show_buttons.h
    src/utils/audioutils.h
    #src/voice_record_process.h
    src/menucontroller/menucontroller.h
    #src/utils/screenutils.h
    src/utils/configsettings.h
    src/utils/shortcut.h
    src/utils/tempfile.h
    src/widgets/savetips.h
    src/widgets/fontsizewidget.h
    src/widgets/colorbutton.h
    src/widgets/zoomIndicator.h
    src/widgets/majtoolbar.h
    src/widgets/textedit.h
    src/widgets/bigcolorbutton.h
    src/widgets/toptips.h
    src/widgets/toolbar.h
    src/widgets/savebutton.h
    src/widgets/textbutton.h
    src/widgets/shapeswidget.h
    src/widgets/toolbutton.h
    src/widgets/maintoolwidget.h
    src/widgets/subtoolwidget.h
    src/widgets/keybuttonwidget.h
    src/widgets/sidebar.h
    src/widgets/shottoolwidget.h
    src/widgets/colortoolwidget.h
    src/dbusinterface/dbuszone.h
    src/dbusinterface/dbuscontrolcenter.h
    src/dbusinterface/dbusnotify.h
    src/dbusinterface/dbussoundeffect.h
    src/dbusservice/dbusscreenshotservice.h
    src/dbusservice/dbusscreenshot.h
    src/widgets/camerawidget.h
    src/screenshot.h
    src/utils/voicevolumewatcher.h
    src/utils/camerawatcher.h
    src/widgets/tooltips.h
    src/widgets/filter.h
    src/utils/screengrabber.h
    src/dbusinterface/drawinterface.h
    src/screen_shot_event.h
    )

add_executable(${PROJECT_NAME}
    ${deepin-screen-recorder_HDRS} ${deepin-screen-recorder_SRCS}
    ${deepin-screen-recorder_MOCS} ${deepin-screen-recorder_RSCS}
    )

qt5_use_modules(${PROJECT_NAME} ${QtModule})
find_package(PkgConfig REQUIRED)
pkg_check_modules(3rd_lib REQUIRED
    dtkwidget dtkgui dframeworkdbus xcb xcb-util
    )
target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES} ${LINK_DIR} )

