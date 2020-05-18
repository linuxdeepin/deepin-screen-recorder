add_definitions(-DTRANSLATION_DOMAIN="xdg-desktop-portal-kde")

include_directories(${Qt5PrintSupport_PRIVATE_INCLUDE_DIRS})

include_directories(/usr/include/aarch64-linux-gnu ${CMAKE_CURRENT_SOURCE_DIR})

LINK_DIRECTORIES(/usr/lib/aarch64-linux-gnu)
FIND_LIBRARY(LIB_AVUTIL libavutil.so /usr/lib/aarch64-linux-gnu NO_DEFAULT_PATH)
link_libraries(${LIB_AVUTIL})
FIND_LIBRARY(LIB_AVFORMAT libavformat.so /usr/lib/aarch64-linux-gnu NO_DEFAULT_PATH)
link_libraries(${LIB_AVFORMAT})
FIND_LIBRARY(LIB_AVCODEC libavcodec.so /usr/lib/aarch64-linux-gnu NO_DEFAULT_PATH)
link_libraries(${LIB_AVCODEC})
FIND_LIBRARY(LIB_SWSALE libswscale.so /usr/lib/aarch64-linux-gnu NO_DEFAULT_PATH)
link_libraries(${LIB_SWSALE})
FIND_LIBRARY(LIB_FILTER libavfilter.so /usr/lib/aarch64-linux-gnu NO_DEFAULT_PATH)
link_libraries(${LIB_FILTER})
set(xdg_desktop_portal_kde_SRCS
    xdg-desktop-portal-kde.cpp
    access.cpp
    accessdialog.cpp
    appchooser.cpp
    appchooserdialog.cpp
    appchooserdialogitem.cpp
    desktopportal.cpp
    email.cpp
    filechooser.cpp
    inhibit.cpp
    notification.cpp
    print.cpp
    request.cpp
    session.cpp
    screenshot.cpp
    screenshotdialog.cpp
    settings.cpp
    utils.cpp
    MainLoop.cpp
    AVOutputStream.cpp
    AVInputStream.cpp
    writeframetostreamthread.cpp
)

if (SCREENCAST_ENABLED)
    set (xdg_desktop_portal_kde_SRCS
         ${xdg_desktop_portal_kde_SRCS}
         screencast.cpp
         screencaststream.cpp
         screencastwidget.cpp
         screenchooserdialog.cpp
         remotedesktop.cpp
         remotedesktopdialog.cpp
         waylandintegration.cpp)

    ki18n_wrap_ui(xdg_desktop_portal_kde_SRCS
                  screenchooserdialog.ui
                  remotedesktopdialog.ui)
endif()

ki18n_wrap_ui(xdg_desktop_portal_kde_SRCS
    accessdialog.ui
    screenshotdialog.ui
)

add_executable(xdg-desktop-portal-kde ${xdg_desktop_portal_kde_SRCS})

target_link_libraries(xdg-desktop-portal-kde
    Qt5::Core
    Qt5::DBus
    Qt5::Concurrent
    Qt5::PrintSupport
    Qt5::Widgets
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    KF5::KIOFileWidgets
    KF5::Notifications
    KF5::WaylandClient
    KF5::WidgetsAddons
    KF5::WindowSystem
     "pthread" "swresample" "avdevice"
)

if (SCREENCAST_ENABLED)
    target_link_libraries(xdg-desktop-portal-kde
                          PipeWire::PipeWire
                          GLIB2::GLIB2
                          ${Epoxy_LIBRARIES}
                          GBM::GBM)
endif()

install(TARGETS xdg-desktop-portal-kde DESTINATION ${KDE_INSTALL_LIBEXECDIR})
