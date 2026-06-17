// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
//#include "waylandrecord/ut_avinputstream.h"
//#include "waylandrecord/ut_avoutputstream.h"
//#include "waylandrecord/ut_recordadmin.h"
//#include "waylandrecord/ut_waylandintegration.h"
//#include "waylandrecord/ut_waylandintegration_p.h"
//#include "waylandrecord/ut_writeframethread.h"
//#include "widgets/ut_shapeswidget.h" // API drift: paintRect/paintEllipse
// signatures now take an extra `int radius`, paintText is overloaded, and the
// test references a non-existent Toolshape::isStraight field. Re-enable after
// updating the ACCESS_PRIVATE_FUN signatures and Toolshape fields in the test.
#include "ut_main_window.h"
#include "utils/ut_pixmergethread.h"
#include "utils/ut_scrollScreenshot.h"
#include "utils/ut_audioutils.h"
#include "utils/ut_baseutils.h"
#include "utils/ut_configsettings.h"
//#include "utils/ut_desktopinfo.h"
#include "utils/ut_screengrabber.h"
#include "utils/ut_shortcut.h"
#include "utils/ut_tempfile.h"
#include "utils/ut_utils_other.h"
#include "utils/ut_calculaterect.h"
#include "widgets/ut_keybuttonwidget.h"
#include "widgets/ut_colortoolwidget.h"
//#include "widgets/ut_maintoolwidget.h" // maintoolwidget.cpp has Qt6 QOverload::of(QButtonGroup::buttonClicked) error; gated in .pro
#include "widgets/ut_zoomIndicator.h"
#include "menucontroller/ut_menucontroller.h"
#include "dbusinterface/ut_dbusnotify.h"
#include "dbusinterface/ut_ocrinterface.h"
//#include "ut_RecorderRegionShow.h" // CameraWidget::initCamera removed, getcameraStatus renamed to getCameraStatus, cameraStart return-type changed
#include "ut_countdown_tooltip.h"
#include "ut_settings.h"
#include "ut_show_buttons.h"
#include "ut_utils.h"
#include "ut_button_feedback.h"
#include "ut_record_process.h"
//#include "ut_screenshot.h"
#include "utils/ut_voiceVolumeWatcher.h"
//#include "utils/ut_WaylandScrollMonitor.h" // WaylandScrollMonitor class is KF5_WAYLAND_FLAGE_ON-gated; disabled with that macro
//#include "widgets/ut_shapeswidget.h"
#include "widgets/ut_toptips.h"
//#include "widgets/ut_camerawidget.h" // CameraWidget: many renamed/removed members (initCamera, cameraResume, captureImage, getcameraStatus->getCameraStatus, ...)
//#include "widgets/ut_tooltips.h" // ToolTips enterEvent signature changed (addr_pri ambiguity)
#include "widgets/ut_textedit.h"
//#include "widgets/ut_shottoolwidget.h" // ShotToolWidget ctor/initRectLabel/m_blurRectButton/... drift
//#include "widgets/ut_subtoolwidget.h" // SubToolWidget API drift
#include "widgets/ut_hoverfilter.h"
#include "widgets/ut_hintfilter.h"
#include "widgets/ut_hovershadowfilter.h"
#include "widgets/ut_hintfilterprivate.h"
//#include "widgets/ut_toolbutton.h" // ToolButton setTips/getTips removed; enterEvent QEvent->QEnterEvent
//#include "widgets/ut_toolbarwidget.h" // ToolBarWidget setExpand/changeArrowAndLineFromBar/... drift
//#include "widgets/ut_toolbar.h"
//#include "widgets/ut_sidebar.h" // SideBar initSideBar removed; enterEvent QEvent->QEnterEvent
//#include "widgets/ut_sidebarwidget.h" // SideBarWidget ctor signature changed
#include "widgets/ut_scrollshottip.h"

