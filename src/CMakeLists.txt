#cmake_minimum_required( VERSION 3.1.0 )
set(BIN_NAME deepin-screen-recorder)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt5 REQUIRED COMPONENTS
  Core Gui Widgets DBus Sql Svg
  Concurrent LinguistTools Multimedia MultimediaWidgets X11Extras)


include(../3rdparty/libcam/libcam.cmake)
include(../3rdparty/stub_linux/stub.cmake)

include(accessibility/accessible.cmake)
include(gstrecord/gstrecord.cmake)
include(widgets/widgets.cmake)
include(utils/utils.cmake)
include(dbusservice/dbusserver.cmake)
include(dbusinterface/dbusinterface.cmake)
include(camera/camera.cmake)
include(menucontroller/menucontroller.cmake)
include(waylandrecord/waylandrecord.cmake)
include(glob.cmake)
#find_package(Qt5 CONFIG REQUIRED ${QtModule})

include(../translations/translate.cmake)
qt5_add_translation(TRANSLATEDFILES ${TRANSLATIONS})

find_package(DFrameworkdbus REQUIRED)
find_package(PkgConfig REQUIRED)
#find_package(Qt5Widgets REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DdeDock REQUIRED)
#find_package(DtkCMake REQUIRED)
pkg_check_modules(gstreamer-audio REQUIRED gstreamer-audio-1.0)
pkg_check_modules(libffmpegthumbnailer REQUIRED libffmpegthumbnailer)
pkg_check_modules(libusb REQUIRED libusb-1.0)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XEXT REQUIRED xext)
pkg_check_modules(XFIXES REQUIRED xfixes)
pkg_check_modules(libimagevisualresult REQUIRED libimagevisualresult)
pkg_check_modules(Xtst REQUIRED xtst)
add_definitions(-DAPP_VERSION="${PROJECT_VERSION}")
pkg_check_modules(libprocps REQUIRED libprocps)

set(deepin-screen-recorder_HDRS
  ${glob_HEAD}
  ${accessibility_HEAD}
  ${menucontroller_HEAD}
  ${camera_HEAD}
  ${dbusinterface_HEAD}
  ${dbusserver_HEAD}
  ${utils_HEAD}
  ${widgets_HEAD}
  ${gstrecord_HEAD}
)

set(deepin-screen-recorder_SRCS
  main.cpp
  ${glob_SRC}
  ${menucontroller_SRC}
  ${camera_SRC}
  ${dbusinterface_SRC}
  ${dbusserver_SRC}
  ${utils_SRC}
  ${widgets_SRC}
  ${gstrecord_SRC}
)

set (deepin-screen-recorder_RSCS
  ../assets/image/deepin-screen-recorder.qrc
  ../assets/resources/resources.qrc
  ../assets/icons/icons.qrc
)
set(COMPILE_SRC
  ${libcam_TOP_SRC}
  ${deepin-screen-recorder_HDRS}
  ${deepin-screen-recorder_SRCS}
  ${libcam_TOP_HEAD}
  ${deepin-screen-recorder_RSCS}
  ${TRANSLATEDFILES}
)
set(include_DIRS 
  #../../interfaces/
  ${libffmpegthumbnailer_INCLUDE_DIRS}
  ${gstreamer-audio_INCLUDE_DIRS}
  ${libusb_INCLUDE_DIRS}
  ${X11_INCLUDE_DIRS}
  ${XEXT_INCLUDE_DIRS}
  ${XFIXES_INCLUDE_DIRS}
  ${Xtst_INCLUDE_DIRS}
  ${libimagevisualresult_INCLUDE_DIRS}
  ${libprocps_INCLUDE_DIRS}
)
set(link_TARGETS
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
  ${DFrameworkdbus_LIBRARIES}
  ${libffmpegthumbnailer_LIBRARIES}
  ${gstreamer-audio_LIBRARIES}
  ${libusb_LIBRARIES}
  ${X11_LIBRARIES}
  ${XEXT_LIBRARIES}
  ${DtkWidget_LIBRARIES}
  ${Xtst_LIBRARIES}
  ${XFIXES_LIBRARIES}
  ${libimagevisualresult_LIBRARIES}
  ${libprocps_LIBRARIES}
)
if(OCR_SCROLL_FLAGE_ON)
  find_package(OpenCV REQUIRED)
  add_definitions(-DOPENCV_VERSION_MAR=${OpenCV_VERSION_MAJOR})
  add_definitions(-DOCR_SCROLL_FLAGE_ON)

  list(APPEND COMPILE_SRC
    ${utils_OPENCV}
    ${widgets_OPENCV}
  )
  list(APPEND include_DIRS
    ${OpenCV_INCLUDE_DIRS}
  )
  list(APPEND link_TARGETS
    ${OpenCV_LIBS}
  )
endif()
# TODO have not finished
if(USE_DEEPIN_WAYLAND)
  include(FeatureSummary)
  find_package(KF5I18n)
  find_package(KF5Wayland CONFIG)
  set_package_properties(KF5Wayland PROPERTIES TYPE OPTIONAL)
  message(${KF5Wayland_VERSION_MAJOR}) 
  add_definitions(-DKWAYLAND_VERSION_MAJOR=${KF5Wayland_VERSION_MAJOR})
  add_definitions(-DKWAYLAND_VERSION_MINOR=${KF5Wayland_VERSION_MINOR})
  add_definitions(-DKWAYLAND_VERSION_PATCH=${KF5Wayland_VERSION_PATCH})
  message("KWayland Version = ${KF5Wayland_VERSION}")
  add_feature_info("KF5Wayland" KF5Wayland_FOUND "Required for the awesome Wayland on Qt demo")
  pkg_check_modules(epoxy REQUIRED epoxy)
  if(UNUSE_DEFAULT_FFMPEG)
    set(ENV{PKG_CONFIG_PATH} "${FFMPEG_PATH}:$ENV{PKG_CONFIG_PATH}")
  endif()
  pkg_check_modules(libavutil REQUIRED libavutil)
  pkg_check_modules(libavcodec REQUIRED libavcodec)
  add_definitions(-DKF5_WAYLAND_FLAGE_ON)
  list(APPEND COMPILE_SRC 
    ${utils_WAYLAND}
    ${waylandrecord_HEAD}
    ${waylandrecord_SRC}
  )
  list(APPEND include_DIRS 
    ${epoxy_INCLUDE_DIRS}
    ${libavutil_INCLUDE_DIRS}
    ${libavcodec_INCLUDE_DIRS}
  )
  list(APPEND link_TARGETS
    KF5::I18n
    KF5::WaylandClient
    ${epoxy_LIBRARIES}
    ${libavutil_LIBRARIES}
    ${libavcodec_LIBRARIES}
  )
endif()

add_executable(${BIN_NAME}
  ${COMPILE_SRC}
)

target_include_directories(${BIN_NAME} PUBLIC 
  ${include_DIRS}
)
target_link_libraries(${BIN_NAME}
  ${link_TARGETS}
)
install(TARGETS ${BIN_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}")
#qt5_use_modules(${PROJECT_NAME} ${QtModule})
install(FILES ${TRANSLATEDFILES} DESTINATION "${CMAKE_INSTALL_DATADIR}/deepin-screen-recorder/translations")
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/deepin-screen-recorder DESTINATION "${CMAKE_INSTALL_DATADIR}/deepin-manual/manual-assets/application")
