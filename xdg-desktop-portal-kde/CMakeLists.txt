cmake_minimum_required(VERSION 3.0)

project(xdg-desktop-portal-kde)

set(PROJECT_VERSION "5.15.5")
set(PROJECT_VERSION_MAJOR 5)

set(QT_MIN_VERSION "5.11.0")
set(KF5_MIN_VERSION "5.54.0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fpermissive")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fpermissive")

################# set KDE specific information #################

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)

find_package(GLIB2)
set_package_properties(GLIB2 PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for screencast portal"
)

find_package(PipeWire)
set_package_properties(PipeWire PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for screencast portal"
)

find_package(GBM)
set_package_properties(GBM PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Required for screencast portal"
)

find_package(Epoxy)
set_package_properties(Epoxy PROPERTIES DESCRIPTION "libepoxy"
    URL "http://github.com/anholt/libepoxy"
    TYPE OPTIONAL
    PURPOSE "Required for screencast portal"
)

if (PipeWire_FOUND AND GLIB2_FOUND AND GBM_FOUND AND Epoxy_FOUND)
    set (SCREENCAST_ENABLED true)
else()
    set (SCREENCAST_ENABLED false)
endif()
add_definitions(-DSCREENCAST_ENABLED=${SCREENCAST_ENABLED})

add_feature_info ("Screencast portal" ${SCREENCAST_ENABLED} "Support for screen sharing")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Concurrent
    DBus
    PrintSupport
    Widgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED
    CoreAddons
    Config
    I18n
    KIO
    Notifications
    Wayland
    WidgetsAddons
    WindowSystem
)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

add_subdirectory(data)
add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
