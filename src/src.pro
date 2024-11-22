######################################################################
# Automatically generated by qmake (3.0) ?? 2? 4 17:49:37 2017
######################################################################

#兼容性 编译宏定义
# 1050 新添加Wayland相关库
# 103x 不支持滚动截图和ocr
# 102x DBus接口Server中不支持startdde相关接口
# /etc/os-version 依赖包deepin-desktop-base
SYS_INFO=$$system("cat /etc/os-version")
message("SYS_INFO: " $$SYS_INFO)

SYS_EDITION=$$system("cat /etc/os-version | grep 'Community'")
message("SYS_EDITION: " $$SYS_EDITION)

SYS_VERSION=$$system("cat /etc/os-version | grep 'MinorVersion' | grep -o '\-\?[0-9]\+'")
message("SYS_VERSION: " $$SYS_VERSION)

#cat /etc/os-version 中的OsBuild
SYS_BUILD=$$system("cat /etc/os-version | grep 'OsBuild' | grep -o '[0-9]\+\.\?[0-9]\+'")
SYS_BUILD_PREFIX=$$system("cat /etc/os-version | grep 'OsBuild' | grep -o '[0-9]\+\.' | grep -o '[0-9]\+'")
SYS_BUILD_SUFFIX=$$system("cat /etc/os-version | grep 'OsBuild' | grep -o '\.[0-9]\+' | grep -o '[0-9]\+'")
message("SYS_BUILD: " $$SYS_BUILD)
message("SYS_BUILD_PREFIX: " $$SYS_BUILD_PREFIX)
#SYS_BUILD_SUFFIX 系统镜像批次号
message("SYS_BUILD_SUFFIX: " $$SYS_BUILD_SUFFIX)


DEFINES += APP_VERSION=\\\"$$VERSION\\\"

if (!equals(SYS_EDITION, "")) {
# 社区版
        message("Community")

        DEFINES += DDE_START_FLAGE_ON
        message("startdde support: OK!!!")

        DEFINES += OCR_SCROLL_FLAGE_ON
        message("ocr and scroll support: OK!!!")
} else {
    message("not Community")

    #1030 支持startdde加速
    greaterThan(SYS_VERSION, 1029) {
        DEFINES += DDE_START_FLAGE_ON
        message("startdde support: OK!!!")
    }

    #1040 兼容支持ocr和滚动截图
    greaterThan(SYS_VERSION, 1039) {
        DEFINES += OCR_SCROLL_FLAGE_ON
        message("ocr and scroll support: OK!!!")
    }

    #1050支持Wayland
    greaterThan(SYS_VERSION, 1049) {
        DEFINES += KF5_WAYLAND_FLAGE_ON
        message("wayland support: OK!!!")
        #1054Wayland remote协议新增接口 109表示1053
        greaterThan(SYS_BUILD_SUFFIX, 109) {
            DEFINES += KWAYLAND_REMOTE_FLAGE_ON
            message("wayland remote support: OK!!!")
        }
        #1055Wayland remote协议新增release接口 110表示1054
        greaterThan(SYS_BUILD_SUFFIX, 110) {
            DEFINES += KWAYLAND_REMOTE_BUFFER_RELEASE_FLAGE_ON
            message("wayland remote support: OK!!!")
        }
        greaterThan(SYS_VERSION, 1059) {
            DEFINES += KWAYLAND_REMOTE_FLAGE_ON
            DEFINES += KWAYLAND_REMOTE_BUFFER_RELEASE_FLAGE_ON
            message("wayland remote buffer release support: OK!!!")
        }
    }
}



QT += dtkgui dtkwidget
contains( DEFINES, KF5_WAYLAND_FLAGE_ON ) {
    QT += KWaylandClient KI18n
    LIBS += -lepoxy
}
TEMPLATE = app
TARGET = deepin-screen-recorder
INCLUDEPATH += . \
            /usr/include/gstreamer-1.0 \
            /usr/include/opencv_mobile \

INCLUDEPATH += ../3rdparty/libcam/libcam/ \
               ../3rdparty/libcam/libcam_v4l2core/ \
               ../3rdparty/libcam/libcam_render/ \
               ../3rdparty/libcam/libcam_encoder/ \
               ../3rdparty/libcam/libcam_audio/ \
               ../3rdparty/libcam/load_libs.h \
               ../3rdparty/displayjack/wayland_client.h

message("PWD: " $$PWD)

include(accessibility/accessible.pri)
include(../3rdparty/libcam/libcam.pri)

QMAKE_CXX += -Wl,--as-need -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,-O1
QMAKE_CXXFLAGS += -Wl,--as-need -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,-O1
QMAKE_LFLAGS += -Wl,--as-needed -pie -z noexecstack -fstack-protector-all -z now

ARCH = $$QMAKE_HOST.arch
isEqual(ARCH, mips64) {
    QMAKE_CXX += -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi
    QMAKE_CXXFLAGS+= -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi
}

CONFIG += link_pkgconfig c++11

PKGCONFIG +=xcb xcb-util dframeworkdbus gobject-2.0 libusb-1.0 gstreamer-app-1.0

RESOURCES = ../assets/image/deepin-screen-recorder.qrc \
    ../assets/resources/resources.qrc \
    ../assets/icons/icons.qrc

HEADERS += main_window.h \
    record_process.h \
    utils.h \
    countdown_tooltip.h \
    constant.h \
    event_monitor.h \
    button_feedback.h \
    show_buttons.h \
    keydefine.h   \
    utils/audioutils.h \
    menucontroller/menucontroller.h \
    utils/baseutils.h \
    utils/configsettings.h \
    utils/shortcut.h \
    utils/tempfile.h \
    utils/calculaterect.h \
    utils/saveutils.h \
    utils/shapesutils.h \
    utils/eventlogutils.h \
    widgets/zoomIndicator.h \
    widgets/zoomIndicatorGL.h \
    widgets/textedit.h \
    widgets/toptips.h \
    widgets/toolbar.h \
    widgets/shapeswidget.h \
    widgets/toolbutton.h \
    widgets/maintoolwidget.h \
    widgets/subtoolwidget.h \
    widgets/keybuttonwidget.h \
    widgets/sidebar.h \
    widgets/shottoolwidget.h \
    widgets/colortoolwidget.h \
    dbusinterface/dbusnotify.h \
    dbusservice/dbusscreenshotservice.h \
    widgets/camerawidget.h \
    screenshot.h \
    utils/voicevolumewatcher.h \
    utils/camerawatcher.h \
    widgets/tooltips.h \
    widgets/filter.h \
    utils/screengrabber.h \
    RecorderRegionShow.h \
    recordertablet.h \
    dbusinterface/ocrinterface.h \
    dbusinterface/pinscreenshotsinterface.h \
    gstrecord/gstrecordx.h \
    gstrecord/gstinterface.h \
    camera/majorimageprocessingthread.h \
    camera/LPF_V4L2.h \
    camera/devnummonitor.h \
    utils/delaytime.h
contains(DEFINES , OCR_SCROLL_FLAGE_ON) {
    HEADERS += widgets/scrollshottip.h \
    utils/pixmergethread.h \
    utils/scrollScreenshot.h \
    widgets/previewwidget.h
}

contains( DEFINES, KF5_WAYLAND_FLAGE_ON ) {
    HEADERS += waylandrecord/writeframethread.h \
    waylandrecord/waylandintegration.h \
    waylandrecord/waylandintegration_p.h \
    waylandrecord/recordadmin.h \
    waylandrecord/avoutputstream.h \
    waylandrecord/avinputstream.h \
    utils/waylandscrollmonitor.h \
    waylandrecord/avlibinterface.h \
}

SOURCES += main.cpp \
    main_window.cpp \
    record_process.cpp \
    utils.cpp \
    countdown_tooltip.cpp \
    constant.cpp \
    event_monitor.cpp \
    button_feedback.cpp \
    show_buttons.cpp  \
    utils/audioutils.cpp \
    menucontroller/menucontroller.cpp \
    utils/shapesutils.cpp \
    utils/tempfile.cpp \
    utils/calculaterect.cpp \
    utils/shortcut.cpp \
    utils/configsettings.cpp \
    utils/baseutils.cpp \
    utils/eventlogutils.cpp \
    widgets/toptips.cpp \
    widgets/shapeswidget.cpp \
    widgets/textedit.cpp \
    widgets/zoomIndicator.cpp \
    widgets/zoomIndicatorGL.cpp \
    widgets/toolbar.cpp \
    widgets/maintoolwidget.cpp \
    widgets/subtoolwidget.cpp \
    widgets/keybuttonwidget.cpp \
    widgets/sidebar.cpp \
    widgets/shottoolwidget.cpp \
    widgets/colortoolwidget.cpp \
    dbusinterface/dbusnotify.cpp \
    dbusservice/dbusscreenshotservice.cpp \
    widgets/camerawidget.cpp \
    screenshot.cpp \
    utils/voicevolumewatcher.cpp \
    utils/camerawatcher.cpp \
    widgets/tooltips.cpp \
    widgets/filter.cpp \
    utils/screengrabber.cpp \
    RecorderRegionShow.cpp \
    recordertablet.cpp \
    dbusinterface/ocrinterface.cpp \
    dbusinterface/pinscreenshotsinterface.cpp \
    gstrecord/gstrecordx.cpp \
    gstrecord/gstinterface.cpp \
    camera/majorimageprocessingthread.cpp \
    camera/LPF_V4L2.c \
    camera/devnummonitor.cpp \
    utils/delaytime.cpp

contains(DEFINES , OCR_SCROLL_FLAGE_ON) {
    SOURCES += widgets/scrollshottip.cpp \
    utils/pixmergethread.cpp \
    utils/scrollScreenshot.cpp \
    widgets/previewwidget.cpp \
}

contains( DEFINES, KF5_WAYLAND_FLAGE_ON ) {
    SOURCES += waylandrecord/writeframethread.cpp \
    waylandrecord/waylandintegration.cpp \
    waylandrecord/recordadmin.cpp \
    waylandrecord/avinputstream.cpp \
    waylandrecord/avoutputstream.cpp \
    utils/waylandscrollmonitor.cpp \
    waylandrecord/avlibinterface.cpp
}


QT += core
QT += widgets
QT += gui
QT += x11extras
QT += dbus
QT += multimedia
QT += multimediawidgets
QT += concurrent
LIBS += -lX11 -lXext -lXtst -lXfixes -lXcursor -ldl -limagevisualresult -lXinerama

contains(DEFINES , OCR_SCROLL_FLAGE_ON) {
    LIBS += -lopencv_small
}

QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -Wno-error=deprecated-declarations -Wno-deprecated-declarations

isEmpty(PREFIX){
    PREFIX = /usr
}

isEmpty(BINDIR):BINDIR=/usr/bin
isEmpty(ICONDIR):ICONDIR=/usr/share/icons/hicolor/scalable/apps
isEmpty(APPDIR):APPDIR=/usr/share/applications
isEmpty(DSRDIR):DSRDIR=/usr/share/deepin-screen-recorder
isEmpty(DOCDIR):DOCDIR=/usr/share/dman/deepin-screen-recorder
isEmpty(ETCDIR):ETCDIR=/etc

target.path = $$INSTROOT$$BINDIR
icon.path = $$INSTROOT$$ICONDIR
desktop.path = $$INSTROOT$$APPDIR

icon.files = ../assets/image/deepin-screen-recorder.svg ../assets/image/deepin-screenshot.svg
desktop.files = ../deepin-screen-recorder.desktop



dbus_service.files = $$PWD/../assets/com.deepin.ScreenRecorder.service $$PWD/../assets/com.deepin.Screenshot.service
contains(DEFINES, DDE_START_FLAGE_ON ) {
    dbus_service.files = $$PWD/../com.deepin.ScreenRecorder.service $$PWD/../com.deepin.Screenshot.service
}
dbus_service.path = $$PREFIX/share/dbus-1/services


manual_dir.files = $$PWD/../assets/deepin-screen-recorder
manual_dir.path= /usr/share/deepin-manual/manual-assets/application/

INSTALLS += target icon desktop dbus_service manual_dir

#使用DConfig需要判断是否存在dtk_install_dconfig.prf文件（/usr/lib/aarch64-linux-gnu/qt5/mkspecs/features/dtk_install_dconfig.prf）
exists($$[QT_INSTALL_LIBS]/qt5/mkspecs/features/dtk_install_dconfig.prf){
    message("-- DConfig is supported by DTK")
    meta_file.files = $$PWD/../assets/org.deepin.screen-recorder.record.json
    meta_file.appid = org.deepin.screen-recorder
    message("meta_file.appid: " $$meta_file.appid)
    message("meta_file.files: " $$meta_file.files)
    DCONFIG_META_FILES += meta_file
    load(dtk_install_dconfig)
}

# 检查集成测试标签
AC_FUNC_ENABLE = true
# 获取标签系统设置
#AC_FUNC_ENABLE = $$(ENABLE_AC_FUNC)
equals(AC_FUNC_ENABLE, true ){
    DEFINES += ENABLE_ACCESSIBILITY
    message("deepin-screen-recorder enabled accessibility function with set: " $$AC_FUNC_ENABLE)
}
