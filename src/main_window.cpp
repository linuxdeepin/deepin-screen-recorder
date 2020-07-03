/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include <dscreenwindowsutil.h>
#include <DWidget>
#include <DWindowManagerHelper>
#include <DForeignWindow>
#include <DLineEdit>
#include <DInputDialog>
#include <DDesktopServices>
#include <DDialog>
#include <QBitmap>


#include <QApplication>
#include <QTimer>
#include <QKeyEvent>
#include <QObject>
#include <QPainter>
#include <QDebug>
#include <QVBoxLayout>
#include <QProcess>
#include <DHiDPIHelper>
#include <QMouseEvent>
#include <QClipboard>
#include <QFileDialog>
#include <QShortcut>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>


#include "main_window.h"
#include "utils.h"
#include "record_button.h"
#include "record_option_panel.h"
#include "countdown_tooltip.h"
#include "constant.h"
#include "utils/tempfile.h"
#include "utils/configsettings.h"
#include "utils/audioutils.h"
#include "utils/shortcut.h"
#include "utils/screengrabber.h"
#include "camera_process.h"
#include "widgets/tooltips.h"
#include "dbusinterface/drawinterface.h"



const int MainWindow::CURSOR_BOUND = 5;
const int MainWindow::RECORD_MIN_SIZE = 580;
const int MainWindow::RECORD_MIN_HEIGHT = 280;
const int MainWindow::RECORD_MIN_SHOT_SIZE = 10;
const int MainWindow::DRAG_POINT_RADIUS = 7;

const int MainWindow::RECORD_BUTTON_NORMAL = 0;
const int MainWindow::RECORD_BUTTON_WAIT = 1;
const int MainWindow::RECORD_BUTTON_RECORDING = 2;

const int MainWindow::ACTION_MOVE = 0;
const int MainWindow::ACTION_RESIZE_TOP_LEFT = 1;
const int MainWindow::ACTION_RESIZE_TOP_RIGHT = 2;
const int MainWindow::ACTION_RESIZE_BOTTOM_LEFT = 3;
const int MainWindow::ACTION_RESIZE_BOTTOM_RIGHT = 4;
const int MainWindow::ACTION_RESIZE_TOP = 5;
const int MainWindow::ACTION_RESIZE_BOTTOM = 6;
const int MainWindow::ACTION_RESIZE_LEFT = 7;
const int MainWindow::ACTION_RESIZE_RIGHT = 8;

const int MainWindow::RECORD_OPTIONAL_PADDING = 12;

const int MainWindow::CAMERA_WIDGET_MAX_WIDTH = 320;
const int MainWindow::CAMERA_WIDGET_MAX_HEIGHT = 180;
const int MainWindow::CAMERA_WIDGET_MIN_WIDTH = 80;
const int MainWindow::CAMERA_WIDGET_MIN_HEIGHT = 45;
DWIDGET_USE_NAMESPACE

namespace {
//const int RECORD_MIN_SIZE = 10;
//const int SPACING = 10;
const int TOOLBAR_X_SPACING = 85;
const int TOOLBAR_Y_SPACING = 3;
const int SIDEBAR_X_SPACING = 8;
//const int SIDEBAR_Y_SPACING = 1;
const int CURSOR_WIDTH = 8;
const int CURSOR_HEIGHT = 18;
const int INDICATOR_WIDTH =  110;
//const qreal RESIZEPOINT_WIDTH = 15;
}

//DWM_USE_NAMESPACE

MainWindow::MainWindow(DWidget *parent) :
    DWidget(parent),
    m_wmHelper(DWindowManagerHelper::instance())
{
//    initAttributes();

    m_pScreenShotEvent =  new ScreenShotEvent();
    m_pScreenRecordEvent = new EventMonitor();
    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &MainWindow::compositeChanged);


    // 初始化获取屏幕坐标信息
    QDBusInterface displayInterface("com.deepin.daemon.Display",
                                "/com/deepin/daemon/Display",
                                "com.deepin.daemon.Display",
                                QDBusConnection::sessionBus());

    if(!displayInterface.isValid()) {
        return;
    }
    int w = displayInterface.property("ScreenWidth").toInt();
    int h = displayInterface.property("ScreenHeight").toInt();
    m_screenSize.setWidth(w);
    m_screenSize.setHeight(h);
    QList<QDBusObjectPath> pathList = qvariant_cast< QList<QDBusObjectPath> >(displayInterface.property("Monitors"));
    for(int i = 0; i < pathList.size(); ++i) {
        QString path = pathList.at(i).path();
        QDBusInterface monitorInterface("com.deepin.daemon.Display",path,"com.deepin.daemon.Display.Monitor",
                                    QDBusConnection::sessionBus());

        if(!monitorInterface.isValid()) {
            continue;
        }
        ScreenInfo screenInfo;
        screenInfo.x = monitorInterface.property("X").toInt();
        screenInfo.y = monitorInterface.property("Y").toInt();
        screenInfo.height =  monitorInterface.property("Height").toInt();
        screenInfo.width = monitorInterface.property("Width").toInt();
        screenInfo.name = monitorInterface.property("Name").toString();
        m_screenInfo.append(screenInfo);
    }
}

void MainWindow::initAttributes()
{
    // Init attributes.
    hintFilter = new HintFilter;
    setWindowTitle(tr("Screen Capture"));
    m_functionType = 0;
    m_keyBoardStatus = 0;
    m_mouseStatus = 0;
    m_multiKeyButtonsInOnSec = false;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    m_gifMode = true;
    m_mp4Mode = false;
//    m_keyBoardTimer = new QTimer(this);
    m_frameRate = RecordProcess::RECORD_FRAMERATE_24;
    m_keyButtonList.clear();
    m_tempkeyButtonList.clear();

    checkCpuIsZhaoxin();

    int t_screenCount = QApplication::desktop()->screenCount();
    m_screenHeight = QApplication::desktop()->screen()->height();

    QRect t_screenRect;

    //多屏情况下累加宽度
    if (t_screenCount == 1) {
        m_screenWidth = QApplication::desktop()->screen()->width();

        t_screenRect.setX(0);
        t_screenRect.setY(0);
        t_screenRect.setWidth(m_screenWidth);
        t_screenRect.setHeight(m_screenHeight);
    }

    else if (t_screenCount > 1) {
//        for (t_indexScreen = 0; t_indexScreen < t_screenCount;  t_indexScreen++) {
//            m_screenWidth += QApplication::desktop()->screen(t_indexScreen)->width();
//        }
        QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
        const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
        t_screenRect = QRect(0, 0,m_screenSize.width() / ratio, m_screenSize.height() / ratio);
        qDebug() << "screen size" << t_primaryScreen->virtualGeometry() << t_screenRect;
    }


//    qDebug() << "screen width:" << m_screenWidth;


    // Add Qt::WindowDoesNotAcceptFocus make window not accept focus forcely, avoid conflict with dde hot-corner.
//    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus | Qt::X11BypassWindowManagerHint);
//    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint  | Qt::WindowStaysOnTopHint);
    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);   // make MouseMove can response
    installEventFilter(this);  // add event filter

    isFirstDrag = false;
    isFirstMove = false;
    isFirstPressButton = false;
    isFirstReleaseButton = false;
    dragStartX = 0;
    dragStartY = 0;

    isPressButton = false;
    isReleaseButton = false;

    recordX = 0;
    recordY = 0;
    recordWidth = 0;
    recordHeight = 0;

    dragRecordX = -1;
    dragRecordY = -1;

    drawDragPoint = false;

    recordButtonStatus = RECORD_BUTTON_NORMAL;

    flashTrayIconCounter = 0;

    selectAreaName = "";

    m_isShapesWidgetExist = false;
    m_needDrawSelectedPoint = false;


    createWinId();

    ConfigSettings::instance();
    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();

    // Get all windows geometry.
    // Below code must execute before `window.showFullscreen,
    // otherwise deepin-screen-recorder window will add in window lists.

    //多屏情况下累加窗口大小
    if (t_screenCount == 1) {
//        DScreenWindowsUtil *screenWin = DScreenWindowsUtil::instance(curPos);

//        screenRect = screenWin->backgroundRect();
        screenRect = QApplication::desktop()->screen()->geometry();
        screenRect = QRect(screenRect.topLeft() / ratio, screenRect.size());
        this->move(static_cast<int>(screenRect.x() * ratio),
                   static_cast<int>(screenRect.y() * ratio));
        this->setFixedSize(screenRect.width(), screenRect.height());
//        m_swUtil = DScreenWindowsUtil::instance(curPos);
//        m_screenNum =  m_swUtil->getScreenNum();

//        windowManager = new DWindowManager();
//        windowManager->setRootWindowRect(screenRect);
//        QList<xcb_window_t> windows = windowManager->getWindows();
//        rootWindowRect = windowManager->getRootWindowRect();
        rootWindowRect = QApplication::desktop()->screen()->geometry();
    }

    else if (t_screenCount > 1) {
//        QPoint pos = this->cursor().pos();
//        DScreenWindowsUtil *screenWin = DScreenWindowsUtil::instance(curPos);
        screenRect = t_screenRect;
        screenRect = QRect(screenRect.topLeft() / ratio, screenRect.size());
        this->move(static_cast<int>(screenRect.x() * ratio),
                   static_cast<int>(screenRect.y() * ratio));
        this->setFixedSize(screenRect.width(), screenRect.height());
//        m_swUtil = DScreenWindowsUtil::instance(curPos);
//        m_screenNum =  m_swUtil->getScreenNum();

//        windowManager = new DWindowManager();
//        windowManager->setRootWindowRect(t_screenRect);
//        QList<xcb_window_t> windows = windowManager->getWindows();
//        rootWindowRect = Dtk::Wm::WindowRect {t_screenRect.x(), t_screenRect.y(),
//                                              t_screenRect.width(), t_screenRect.height()};
        rootWindowRect = t_screenRect;
    }

        m_screenHeight = m_screenSize.height();
        m_screenWidth = m_screenSize.width();
        for (auto wid : DWindowManagerHelper::instance()->currentWorkspaceWindowIdList()) {
            if (wid == winId()) continue;

            DForeignWindow *window = DForeignWindow::fromWinId(wid);
//            qDebug() << window->wmClass() << window->windowState();
            //判断窗口是否被最小化
            if (window->windowState() == Qt::WindowState::WindowMinimized) {
                continue;
            }

            if (window) {
                int t_tempWidth = 0;
                int t_tempHeight = 0;
                window->deleteLater();
                //修改部分窗口显示不全，截图框识别问题
                //x坐标小于0时
                if (window->frameGeometry().x() < 0) {
                    if (window->frameGeometry().y() < 0) {

                        //x,y为负坐标情况
                        t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                        t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

//                        windowRects << Dtk::Wm::WindowRect {0, 0, t_tempWidth, t_tempHeight};
                        windowRects << QRect(0, 0, t_tempWidth, t_tempHeight);
                        windowNames << window->wmClass();
                        continue;
                    }

                    else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= m_screenHeight - window->frameGeometry().height()) {
                        //x为负坐标，y在正常屏幕区间内
                        t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                        t_tempHeight = window->frameGeometry().height();

//                        windowRects << Dtk::Wm::WindowRect {0, window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                        windowRects << QRect(0, window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                        windowNames << window->wmClass();
                        continue;
                    }

                    else if (window->frameGeometry().y() > m_screenHeight - window->frameGeometry().height()) {
                        //x为负坐标，y方向窗口超出屏幕底部
                        t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                        t_tempHeight = m_screenHeight - window->frameGeometry().y();

//                        windowRects << Dtk::Wm::WindowRect {0, window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                        windowRects << QRect(0, window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                        windowNames << window->wmClass();
                        continue;
                    }
                }

                //x坐标位于正常屏幕区间时
                else if (window->frameGeometry().x() >= 0 && window->frameGeometry().x() <= m_screenWidth - window->frameGeometry().width()) {
                    if (window->frameGeometry().y() < 0) {
                        //y为负坐标情况
                        t_tempWidth = window->frameGeometry().width();
                        t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

//                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight};
                        windowRects << QRect(window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight);
                        windowNames << window->wmClass();
                        continue;
                    }

                    else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= m_screenHeight - window->frameGeometry().height()) {
                        //y在正常屏幕区间内
                        t_tempWidth = window->frameGeometry().width();
                        t_tempHeight = window->frameGeometry().height();

//                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                        windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                        windowNames << window->wmClass();
                        continue;
                    }

                    else if (window->frameGeometry().y() > m_screenHeight - window->frameGeometry().height()) {
                        //y方向窗口超出屏幕底部
                        t_tempWidth = window->frameGeometry().width();
                        t_tempHeight = m_screenHeight - window->frameGeometry().y();

//                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                        windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                        windowNames << window->wmClass();
                        continue;
                    }
                }

                //x方向窗口超出屏幕右侧区域
                else if (window->frameGeometry().x() > m_screenWidth - window->frameGeometry().width()) {
                    if (window->frameGeometry().y() < 0) {
                        //y为负坐标情况
                        t_tempWidth = m_screenWidth - window->frameGeometry().x();
                        t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

//                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight};
                        windowRects << QRect(window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight);
                        windowNames << window->wmClass();
                        continue;
                    }

                    else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= m_screenHeight - window->frameGeometry().height()) {
                        //y在正常屏幕区间内
                        t_tempWidth = m_screenWidth - window->frameGeometry().x();
                        t_tempHeight = window->frameGeometry().height();

//                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                        windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                        windowNames << window->wmClass();
                        continue;
                    }

                    else if (window->frameGeometry().y() > m_screenHeight - window->frameGeometry().height()) {
                        //y方向窗口超出屏幕底部
                        t_tempWidth = m_screenWidth - window->frameGeometry().x();
                        t_tempHeight = m_screenHeight - window->frameGeometry().y();

//                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                        windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                        windowNames << window->wmClass();
                        continue;
                    }
                }
                //                windowRects << Dtk::Wm::WindowRect { window->x(), window->y(), window->width(), window->height() };
                //                windowNames << window->wmClass();
            }
        }

    recordButtonLayout = new QVBoxLayout();
    setLayout(recordButtonLayout);

    //屏蔽原有的录屏提示控件 by zyg
//    startTooltip = new StartTooltip();
//    startTooltip->setWindowManager(windowManager);

    //构建截屏工具栏按钮 by zyg
    m_toolBar = new ToolBar(this);
    m_toolBar->hide();

    m_sideBar = new SideBar(this);
    m_sideBar->hide();


    connect(m_sideBar, &SideBar::closeSideBarToMain, this, [ = ] {
        if (m_sideBar->isVisible())
        {
            m_sideBar->hide();
        }
    });

    m_sizeTips = new TopTips(this);
    m_sizeTips->hide();
    m_zoomIndicator = new ZoomIndicator(this);
    m_zoomIndicator->hide();

    countdownTooltip = new CountdownTooltip();
    connect(countdownTooltip, SIGNAL(finished()), this, SLOT(startRecord()));

    connect(m_toolBar, &ToolBar::currentFunctionToMain, this, &MainWindow::changeFunctionButton);
    connect(m_toolBar, &ToolBar::keyBoardCheckedToMain, this, &MainWindow::changeKeyBoardShowEvent);
    connect(m_toolBar, &ToolBar::mouseCheckedToMain, this, &MainWindow::changeMouseShowEvent);
    connect(m_toolBar, &ToolBar::microphoneActionCheckedToMain, this, &MainWindow::changeMicrophoneSelectEvent);
    connect(m_toolBar, &ToolBar::systemAudioActionCheckedToMain, this, &MainWindow::changeSystemAudioSelectEvent);
    connect(m_toolBar, &ToolBar::cameraActionCheckedToMain, this, &MainWindow::changeCameraSelectEvent);
    connect(m_toolBar, &ToolBar::gifActionCheckedToMain, this, &MainWindow::changeGifSelectEvent);
    connect(m_toolBar, &ToolBar::mp4ActionCheckedToMain, this, &MainWindow::changeMp4SelectEvent);
    connect(m_toolBar, &ToolBar::frameRateChangedToMain, this, &MainWindow::changeFrameRateEvent);
    connect(m_toolBar, &ToolBar::shotToolChangedToMain, this, &MainWindow::changeShotToolEvent);
    connect(m_toolBar, &ToolBar::closeButtonToMain, this, &MainWindow::exitApp);
    connect(m_sideBar, &SideBar::changeArrowAndLineToMain, this, &MainWindow::changeArrowAndLineEvent);
//    connect(m_toolBar, &ToolBar::shotToolChangedToMain, this,  [ = ](QString shape) {
//        if (m_isShapesWidgetExist && shape != "color") {
//            m_shapesWidget->setCurrentShape(shape);
//        } else if (shape != "color") {
//            initShapeWidget(shape);
//            m_isShapesWidgetExist = true;
//        }
//    });
    //构建截屏录屏功能触发按钮
    DPalette pa;
    m_recordButton = new DPushButton(this);
    m_recordButton->setFocusPolicy(Qt::NoFocus);

    pa = m_recordButton->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(229, 70, 61, 204));
    pa.setColor(DPalette::Light, QColor(229, 70, 61, 204));
    m_recordButton->setPalette(pa);
    m_recordButton->setIconSize(QSize(38, 38));
    m_recordButton->setIcon(QIcon(":/image/newUI/checked/screencap-checked.svg"));
//    m_recordButton->setToolTip(tr("Start Record"));
//    installTipHint(m_recordButton, tr("Start Record"));
//    m_recordButton->setToolTip(tr("Switch to record mode"));

    m_recordButton->setFixedSize(76, 58);
//    m_recordButton->setText(tr("Record"));
    m_recordButton->setObjectName("mainRecordBtn");

    m_recordButton->setFocusPolicy(Qt::ClickFocus);


    m_shotButton = new DPushButton(this);
    m_recordButton->setFocusPolicy(Qt::NoFocus);
//    m_shotButton->setFocusPolicy(Qt::ClickFocus);

    pa = m_shotButton->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(0, 129, 255, 204));
    pa.setColor(DPalette::Light, QColor(0, 129, 255, 204));
    m_shotButton->setPalette(pa);
    m_shotButton->setIconSize(QSize(38, 38));
    m_shotButton->setIcon(QIcon(":/image/newUI/checked/screenshot-checked.svg"));
//    m_shotButton->setToolTip(tr("Start Shot"));
//    installTipHint(m_shotButton, tr("Start Shot"));
//    m_shotButton->setToolTip(tr("Switch to shot mode"));

    m_shotButton->setFixedSize(76, 58);
//    m_shotButton->setText(tr("Shot"));
    m_shotButton->setObjectName("mainShotBtn");

    m_recordButton->hide();
    m_shotButton->hide();

//    m_backgroundRect = m_swUtil->backgroundRect();
    m_backgroundRect = QApplication::desktop()->screen()->geometry();
    m_backgroundRect = QRect(m_backgroundRect.topLeft() / ratio, m_backgroundRect.size());

    recordButton = new RecordButton();
    recordButton->setText(tr("Start recording"));
    connect(m_recordButton, SIGNAL(clicked()), this, SLOT(startCountdown()));
    connect(m_shotButton, SIGNAL(clicked()), this, SLOT(saveScreenShot()));

    recordOptionPanel = new RecordOptionPanel();

    recordOptionPanel->setFixedWidth(recordButton->width());

    recordButtonLayout->addStretch();
    recordButtonLayout->addWidget(recordButton, 0, Qt::AlignCenter);
    recordButtonLayout->addSpacing(RECORD_OPTIONAL_PADDING);

    if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
        recordButtonLayout->addWidget(recordOptionPanel, 0, Qt::AlignCenter);
    }
    recordButtonLayout->addStretch();

    recordButton->hide();
    recordOptionPanel->hide();

    m_selectedMic = true;
    m_selectedSystemAudio = true;

//    m_swUtil = DScreenWindowsUtil::instance(curPos);

    if (t_screenCount == 1) {
//        m_backgroundRect = m_swUtil->backgroundRect();
        m_backgroundRect = QApplication::desktop()->screen()->geometry();
        m_backgroundRect = QRect(m_backgroundRect.topLeft() / ratio, m_backgroundRect.size());

        move(m_backgroundRect.topLeft() * ratio);
        this->setFixedSize(m_backgroundRect.size());
    }

    else if (t_screenCount > 1) {
        m_backgroundRect = t_screenRect;
        m_backgroundRect = QRect(m_backgroundRect.topLeft() / ratio, m_backgroundRect.size());
        move(m_backgroundRect.topLeft() * ratio);
        this->setFixedSize(m_backgroundRect.size());
    }
    initBackground();

    // Just use for debug.
    // repaintCounter = 0;

//    if (CameraProcess::checkCameraAvailability()) {
    m_cameraWidget = new CameraWidget(this);
    hideCameraWidget();
    // 摄像头界面层级下调,防止遮住工具栏
    m_cameraWidget->lower();
//    }

    m_menuController = new MenuController(this);

    connect(m_menuController, &MenuController::saveAction,
            this, &MainWindow::saveScreenShot);
    connect(m_menuController, &MenuController::closeAction,
            this, &MainWindow::exitApp);
//    connect(m_menuController, &MenuController::unDoAction,
//            this, &ShapesWidget::undoDrawShapes);

//    m_functionType = 1;
//    initScreenShot();
    initShortcut();
}

void MainWindow::sendSavingNotify()
{
    // Popup notify.
    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());
    QStringList actions;
    actions << "_close" << tr("Ignore");
    int timeout = 3000;
    unsigned int id = 0;

    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName())                 // appname
        << id                                                   // id
        << QString("deepin-screen-recorder")                     // icon
        << QString(tr("Screen Capture"))                         // summary
        << QString(tr("Saving the screen recording,please wait..."))  // body
        << actions                                               // actions
        << QVariantMap()                                         // hints
        << timeout;                                           // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
}

void MainWindow::initShortcut()
{
    QShortcut *rectSC = new QShortcut(QKeySequence("R"), this);
    QShortcut *ovalSC = new QShortcut(QKeySequence("O"), this);
    QShortcut *arrowSC = new QShortcut(QKeySequence("L"), this);
    QShortcut *lineSC = new QShortcut(QKeySequence("P"), this);
    QShortcut *textSC = new QShortcut(QKeySequence("T"), this);
//    QShortcut *colorSC = new QShortcut(QKeySequence("Alt+6"), this);

    connect(rectSC, &QShortcut::activated, this, [ = ] {
        if (m_functionType == 1)
        {
            emit m_toolBar->shapeClickedFromMain("rect");
        }

    });
    connect(ovalSC, &QShortcut::activated, this, [ = ] {
        if (m_functionType == 1)
        {
            emit m_toolBar->shapeClickedFromMain("circ");
        }
    });
    connect(arrowSC, &QShortcut::activated, this, [ = ] {
        if (m_functionType == 1)
        {
            emit m_toolBar->shapeClickedFromMain("line");
        }
    });
    connect(lineSC, &QShortcut::activated, this, [ = ] {
        if (m_functionType == 1)
        {
            emit m_toolBar->shapeClickedFromMain("pen");
        }
    });
    connect(textSC, &QShortcut::activated, this, [ = ] {
        if (m_functionType == 1)
        {
            emit m_toolBar->shapeClickedFromMain("text");
        }
    });
//    connect(colorSC, &QShortcut::activated, this, [ = ] {
//        if (m_functionType == 1)
//        {
//            emit m_toolBar->shapeClickedFromMain("option");
//        }
//    });

    if (isCommandExist("dman")) {
        QShortcut *helpSC = new QShortcut(QKeySequence("F1"), this);
        helpSC->setAutoRepeat(false);
        connect(helpSC,  SIGNAL(activated()), this, SLOT(onHelp()));
    }
}

void MainWindow::onHelp()
{
    QDBusInterface iface("com.deepin.Manual.Open",
                         "/com/deepin/Manual/Open",
                         "com.deepin.Manual.Open");
    if (iface.isValid()) {
        iface.call("ShowManual", "deepin-screen-recorder");
        exitApp();
    } else {
        qWarning() << "manual service not available, cannot open manual";
    }
}

void MainWindow::initResource()
{
    m_showButtons = new ShowButtons(this);
    connect(m_showButtons, SIGNAL(keyShowSignal(const QString &)),
            this, SLOT(showKeyBoardButtons(const QString &)));
//    resizeHandleBigImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("resize_handle_big.svg"));
    resizeHandleBigImg = DHiDPIHelper::loadNxPixmap(":/image/newUI/normal/node.svg");
    resizeHandleSmallImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("resize_handle_small.svg"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon1.svg"))));
    trayIcon->setToolTip(tr("Screen Capture"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

//    setDragCursor();

    buttonFeedback = new ButtonFeedback();



//    startTooltip->show();
    //    startTooltip->windowHandle()->raise();
}

void MainWindow::initScreenShot()
{

//    installEventFilter(this);
    connect(m_pScreenShotEvent, SIGNAL(activateWindow()), this, SLOT(onActivateWindow()), Qt::QueuedConnection);
    m_pScreenShotEvent->start();

    connect(this, &MainWindow::releaseEvent, this, [ = ] {
        qDebug() << "release event !!!";
        removeEventFilter(this);
    });

//    connect(this, &MainWindow::hideScreenshotUI, this, &MainWindow::hide);

    m_functionType = 1;
    m_keyBoardStatus = 0;
    m_mouseStatus = 0;
    m_multiKeyButtonsInOnSec = false;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    if (m_firstShot == 0) {
        m_gifMode = true;
        m_mp4Mode = false;
//    m_keyBoardTimer = new QTimer(this);
        m_frameRate = RecordProcess::RECORD_FRAMERATE_24;
    }
    // 多屏情况下， 屏幕宽高，用背景宽高获取
    //m_screenWidth = QApplication::desktop()->screen()->width();
    //m_screenHeight = QApplication::desktop()->screen()->height();
    m_screenWidth = m_backgroundRect.width();
    m_screenHeight = m_backgroundRect.height();

    m_shotStatus = ShotMouseStatus::Normal;

//    isFirstDrag = false;
//    isFirstMove = false;
//    isFirstPressButton = false;
//    isFirstReleaseButton = false;
//    dragStartX = 0;
//    dragStartY = 0;

    isPressButton = false;
    isReleaseButton = false;


//    recordX = 0;
//    recordY = 0;
//    recordWidth = 0;
//    recordHeight = 0;

//    dragRecordX = -1;
//    dragRecordY = -1;

//    drawDragPoint = false;

    recordButtonStatus = RECORD_BUTTON_NORMAL;

    flashTrayIconCounter = 0;

//    selectAreaName = "";

    //隐藏键盘按钮控件
    if (m_keyButtonList.count() > 0) {
        for (int t_index = 0; t_index < m_keyButtonList.count(); t_index++) {
            m_keyButtonList.at(t_index)->hide();
        }
    }
    //构建截屏工具栏按钮 by zyg
    if (m_firstShot == 0) {
        m_toolBar->hide();
        m_sideBar->hide();

        m_recordButton->hide();
        m_shotButton->hide();
        m_sizeTips->hide();
    }

    else {
        m_toolBar->show();
        m_sideBar->hide();

        m_recordButton->hide();
        m_shotButton->show();
        m_sizeTips->show();


        updateToolBarPos();
        updateShotButtonPos();
        m_sizeTips->updateTips(QPoint(recordX, recordY),
                               QString("%1X%2").arg(recordWidth).arg(recordHeight));
    }

    recordButton->hide();
    recordOptionPanel->hide();


    if (m_firstShot == 0) {
        m_selectedMic = true;
        m_selectedSystemAudio = true;
    }

    if (m_firstShot == 0) {
        setDragCursor();
    }
//    eventMonitor.quit();
//    emit releaseEvent();
    if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
        m_pScreenRecordEvent->terminate();
        m_pScreenRecordEvent->wait();
    }


    connect(this, &MainWindow::hideScreenshotUI, this, &MainWindow::hide);

    m_toolBar->setFocus();
}

void MainWindow::initScreenRecorder()
{
    if (!DWindowManagerHelper::instance()->hasComposite()) {
        qDebug() << "no composite";
        Utils::warnNoComposite();
        if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
            if(nullptr != m_pScreenRecordEvent){
                m_pScreenRecordEvent->terminate();
                m_pScreenRecordEvent->wait();
                m_pScreenRecordEvent = nullptr;
            }
            if(nullptr != m_pScreenShotEvent){
                m_pScreenShotEvent->terminate();
                m_pScreenShotEvent->wait();
                m_pScreenShotEvent = nullptr;
            }
        }
        qApp->quit();
    }


    m_functionType = 0;
    m_keyBoardStatus = 0;
    m_mouseStatus = 0;
    m_multiKeyButtonsInOnSec = false;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    if (m_firstShot == 0) {
        m_gifMode = true;
        m_mp4Mode = false;
        //    m_keyBoardTimer = new QTimer(this);
        m_frameRate = RecordProcess::RECORD_FRAMERATE_24;
    }


    //m_screenWidth = QApplication::desktop()->screen()->width();
    //m_screenHeight = QApplication::desktop()->screen()->height();
    m_screenWidth = m_backgroundRect.width();
    m_screenHeight = m_backgroundRect.height();

//    isFirstDrag = false;
//    isFirstMove = false;
//    isFirstPressButton = false;
//    isFirstReleaseButton = false;
//    dragStartX = 0;
//    dragStartY = 0;

    isPressButton = false;
    isReleaseButton = false;

    if (m_firstShot == 1) {
        if (recordWidth < 580) {
            recordWidth = 580;
            if (recordX >= m_screenWidth - 580) {
                recordX = m_screenWidth - 581;
            }

        }

        if (recordHeight < 280) {
            recordHeight = 280;

            if (recordY >= m_screenHeight - 280) {
                recordY = m_screenHeight - 281;
            }
        }
    }

//    recordX = 0;
//    recordY = 0;
//    recordWidth = 0;
//    recordHeight = 0;

//    dragRecordX = -1;
//    dragRecordY = -1;

//    drawDragPoint = false;

    recordButtonStatus = RECORD_BUTTON_NORMAL;

    flashTrayIconCounter = 0;

//    selectAreaName = "";

    if (m_isShapesWidgetExist) {
        m_shapesWidget->hide();
    }

    m_isShapesWidgetExist = false;
    m_needDrawSelectedPoint = false;


    //构建截屏工具栏按钮 by zyg
    if (m_firstShot == 0) {
        m_toolBar->hide();
        m_sideBar->hide();

        m_recordButton->hide();
        m_shotButton->hide();
        m_sizeTips->hide();
    }

    else {
        m_toolBar->show();
        m_sideBar->hide();

        m_recordButton->show();
        m_shotButton->hide();
        m_sizeTips->show();

        updateToolBarPos();
        updateRecordButtonPos();
        m_sizeTips->updateTips(QPoint(recordX, recordY),
                               QString("%1X%2").arg(recordWidth).arg(recordHeight));
    }

    recordButton->hide();
    recordOptionPanel->hide();


    m_zoomIndicator->hide();

    if (m_firstShot == 0) {
        m_selectedMic = true;
        m_selectedSystemAudio = true;
    }

//    initVirtualCard();
    if (m_firstShot == 0) {
        setDragCursor();
    }
//    eventMonitor.start();


    connect(m_pScreenRecordEvent, SIGNAL(buttonedPress(int, int)), this, SLOT(showPressFeedback(int, int)), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(buttonedDrag(int, int)), this, SLOT(showDragFeedback(int, int)), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(buttonedRelease(int, int)), this, SLOT(showReleaseFeedback(int, int)), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(pressEsc()), this, SLOT(responseEsc()), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(pressKeyButton(unsigned char)), m_showButtons,
            SLOT(showContentButtons(unsigned char)), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(releaseKeyButton(unsigned char)), m_showButtons,
            SLOT(releaseContentButtons(unsigned char)), Qt::QueuedConnection);
    m_pScreenRecordEvent->start();

    m_toolBar->setFocus();
}

void MainWindow::initLaunchMode(const QString &launchMode)
{
    if (launchMode == "screenRecord") {
        m_launchWithRecordFunc = true;
        m_shotButton->hide();
        m_recordButton->show();
        m_functionType = 0;
        initScreenRecorder();
        if (m_sideBar->isVisible()) {
            m_sideBar->hide();
        }
    }

    else if (launchMode == "screenShot") {
        m_launchWithRecordFunc = false;
        m_recordButton->hide();
        m_shotButton->show();
        m_functionType = 1;
        initScreenShot();
    }

    else {
        m_launchWithRecordFunc = false;
        m_recordButton->hide();
        m_shotButton->show();
        m_functionType = 1;
        initScreenShot();
    }
}

void MainWindow::delayScreenshot(double num)
{
    QString summary = QString(tr("Screen Capture will start in %1 seconds").arg(num));
    QStringList actions = QStringList();
    QVariantMap hints;
    DBusNotify *notifyDBus = new DBusNotify(this);
    if (num >= 2) {
        notifyDBus->Notify("Deepin Screenshot", 0,  "deepin-screen-recorder", "",
                           summary, actions, hints, 0);
    }

    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    timer->start(int(1000 * num));
    connect(timer, &QTimer::timeout, this, [ = ] {
        notifyDBus->CloseNotification(0);
//        startScreenshot();
        this->initAttributes();
        this->initLaunchMode("screenShot");
        this->showFullScreen();
        this->initResource();
    });
}

void MainWindow::fullScreenshot()
{
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SEE_Screenshot);
    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();
    m_mouseStatus = ShotMouseStatus::Shoting;
    repaint();
    qApp->setOverrideCursor(setCursorShape("start"));
//    initDBusInterface();
    this->setFocus();
//    m_configSettings =  ConfigSettings::instance();
    installEventFilter(this);

//    if (m_hotZoneInterface->isValid())
//        m_hotZoneInterface->asyncCall("EnableZoneDetected", false);
    /*
        QPoint curPos = this->cursor().pos();
    //    m_swUtil = DScreenWindowsUtil::instance(curPos);
    //    m_screenNum = m_swUtil->getScreenNum();
    //    m_backgroundRect = m_swUtil->backgroundRect();
        int t_screen = QApplication::desktop()->screenNumber(curPos);
        m_backgroundRect = QApplication::desktop()->screen(t_screen)->geometry();
    */
    // 多屏截取全屏
    int t_screenCount = QApplication::desktop()->screenCount();
    if (t_screenCount == 1) {
        m_backgroundRect = QApplication::desktop()->screen()->geometry();
        m_backgroundRect = QRect(m_backgroundRect.topLeft(), m_backgroundRect.size());
    } else if (t_screenCount > 1) {
        QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
        m_backgroundRect = t_primaryScreen->virtualGeometry();;
        m_backgroundRect = QRect(m_backgroundRect.topLeft(), m_backgroundRect.size());
    }
    //
    this->move(m_backgroundRect.x(), m_backgroundRect.y());
    this->setFixedSize(m_backgroundRect.size());
    m_needSaveScreenshot = true;

    m_toolBar = new ToolBar(this);
    m_toolBar->hide();

    shotFullScreen();

//    if (m_hotZoneInterface->isValid())
//        m_hotZoneInterface->asyncCall("EnableZoneDetected",  true);

//    emit this->hideScreenshotUI();
//    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Screenshot);
//    using namespace utils;

    TempFile::instance()->setFullScreenPixmap(m_resultPixmap);
    const auto r = saveAction(m_resultPixmap);
    sendNotify(m_saveIndex, m_saveFileName, r);
}

void MainWindow::testScreenshot()
{
    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();

    m_shotWithPath = true;
    m_shotSavePath = QDir::homePath() + QDir::separator() + QString("testShot");
    QDir t_appDir;
    t_appDir.mkpath(m_shotSavePath);

    m_mouseStatus = ShotMouseStatus::Shoting;
    repaint();
    qApp->setOverrideCursor(setCursorShape("start"));
//    initDBusInterface();
    this->setFocus();
//    m_configSettings =  ConfigSettings::instance();
    installEventFilter(this);

//    if (m_hotZoneInterface->isValid())
//        m_hotZoneInterface->asyncCall("EnableZoneDetected", false);

    QPoint curPos = this->cursor().pos();
//    m_swUtil = DScreenWindowsUtil::instance(curPos);
//    m_screenNum = m_swUtil->getScreenNum();
//    m_backgroundRect = m_swUtil->backgroundRect();
    int t_screen = QApplication::desktop()->screenNumber(curPos);
    m_backgroundRect = QApplication::desktop()->screen(t_screen)->geometry();

    this->move(m_backgroundRect.x(), m_backgroundRect.y());
    this->setFixedSize(m_backgroundRect.size());
    m_needSaveScreenshot = true;
    shotFullScreen();
    m_toolBar = new ToolBar(this);
    m_toolBar->hide();

//    if (m_hotZoneInterface->isValid())
//        m_hotZoneInterface->asyncCall("EnableZoneDetected",  true);

//    emit this->hideScreenshotUI();
//    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Screenshot);
//    using namespace utils;

    TempFile::instance()->setFullScreenPixmap(m_resultPixmap);
    const auto r = saveAction(m_resultPixmap);
    sendNotify(m_saveIndex, m_saveFileName, r);
}

void MainWindow::topWindow()
{
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SEE_Screenshot);
    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();

//    QList<QRect> t_windowRects;
//    if (m_screenNum == 0) {
//        t_windowRects  = m_swUtil->windowsRect();

//        if (t_windowRects.length() > 1 && t_windowRects[0] == m_backgroundRect) {
//            recordX = t_windowRects[1].x();
//            recordY = t_windowRects[1].y();
//            recordWidth = t_windowRects[1].width();
//            recordHeight = t_windowRects[1].height();
//        } else {
//            recordX = t_windowRects[0].x();
//            recordY = t_windowRects[0].y();
//            recordWidth = t_windowRects[0].width();
//            recordHeight = t_windowRects[0].height();
//        }


//    } else {
//        recordX = m_backgroundRect.x();
//        recordY = m_backgroundRect.y();
//        recordWidth = m_backgroundRect.width();
//        recordHeight = m_backgroundRect.height();
//    }

//    for (auto wid : DWindowManagerHelper::instance()->allWindowIdList()) {
//        DForeignWindow *window = DForeignWindow::fromWinId(wid);
//        qDebug() << "window name:" << window->wmClass();
//    }


    int t_windowCount = DWindowManagerHelper::instance()->allWindowIdList().size();

    for (int i = t_windowCount - 1; i >= 0; i--) {
        auto wid = DWindowManagerHelper::instance()->allWindowIdList().at(i);
        if (wid == winId()) continue;
        DForeignWindow *window = DForeignWindow::fromWinId(wid);

        if (window->type() == Qt::Window || window->type() == Qt::Desktop) {
            //判断窗口是否被最小化
            if (window->windowState() == Qt::WindowState::WindowMinimized) {
                continue;
            }
            selectAreaName = window->wmClass();
            recordX = window->frameGeometry().x();
            recordY = window->frameGeometry().y();
            recordWidth = window->frameGeometry().width();
            recordHeight = window->frameGeometry().height();
            break;
        } else {
            continue;
        }
    }

    this->hide();
    emit this->hideScreenshotUI();

    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    QRect target( static_cast<int>(recordX * ratio),
                  static_cast<int>(recordY * ratio),
                  static_cast<int>(recordWidth * ratio),
                  static_cast<int>(recordHeight * ratio) );

//    using namespace utils;
    QPixmap screenShotPix =  m_backgroundPixmap.copy(target);
    qDebug() << "topWindow grabImage is null:" << m_backgroundPixmap.isNull()
             << QRect(recordX, recordY, recordWidth, recordHeight)
             << "\n"
             << "screenShot is null:" << screenShotPix.isNull();
    m_needSaveScreenshot = true;
    //    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Screenshot);

    const auto r = saveAction(screenShotPix);
    sendNotify(m_saveIndex, m_saveFileName, r);
}

void MainWindow::savePath(const QString &path)
{
    if (!QFileInfo(path).dir().exists()) {
        exitApp();
    }

    qDebug() << "path exist!";

    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();

    m_shotWithPath = true;
    m_shotSavePath = path;
}

void MainWindow::noNotify()
{
    m_noNotify = true;

    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();
}

void MainWindow::setConfigThemeType(int themeType)
{
    ConfigSettings::instance()->setValue("common", "themeType", themeType);
}

void MainWindow::initBackground()
{
//    QTimer::singleShot(200, this, [ = ] {
    m_backgroundPixmap = getPixmapofRect(m_backgroundRect);
    qDebug() << "screen rect:" << m_backgroundPixmap.rect();
    m_resultPixmap = m_backgroundPixmap;
    TempFile::instance()->setFullScreenPixmap(m_backgroundPixmap);
//    });
}

QPixmap MainWindow::getPixmapofRect(const QRect &rect)
{
    QScreen *screen = qApp->primaryScreen();
    const qreal dpiVal = screen->devicePixelRatio();
    bool ok;
    return m_screenGrabber.grabEntireDesktop(ok, rect, dpiVal);
}

void MainWindow::installTipHint(QWidget *w, const QString &hintstr)
{
    // TODO: parent must be mainframe
    auto hintWidget = new ToolTips("", this);
    hintWidget->hide();
    hintWidget->setText(hintstr);
    hintWidget->setFixedHeight(32);
    installHint(w, hintWidget);
}

void MainWindow::installHint(QWidget *w, QWidget *hint)
{
    w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    w->installEventFilter(hintFilter);
}

void MainWindow::showPressFeedback(int x, int y)
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING && m_mouseStatus == 1) {
        buttonFeedback->showPressFeedback(x, y);
    }
}

void MainWindow::showDragFeedback(int x, int y)
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING && m_mouseStatus == 1) {
        buttonFeedback->showDragFeedback(x, y);
    }
}

void MainWindow::showReleaseFeedback(int x, int y)
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING && m_mouseStatus == 1) {
        buttonFeedback->showReleaseFeedback(x, y);
    }
}

void MainWindow::responseEsc()
{
    if (0 == m_functionType && RECORD_BUTTON_RECORDING != recordButtonStatus){
        emit releaseEvent();
        if (QSysInfo::currentCpuArchitecture().startsWith("x86") && false == m_isZhaoxin){
            if (nullptr != m_pScreenRecordEvent){
                m_pScreenRecordEvent->terminate();
                m_pScreenRecordEvent->wait();
                m_pScreenRecordEvent = nullptr;
            }
            if(nullptr != m_pScreenShotEvent){
                m_pScreenShotEvent->terminate();
                m_pScreenShotEvent->wait();
                m_pScreenShotEvent = nullptr;
            }
        }
        QApplication::quit();
    }
}

void MainWindow::onActivateWindow()
{
    activateWindow();
}

void MainWindow::compositeChanged()
{
    if (!m_wmHelper->hasComposite()) {
        qDebug() << "have no Composite";
        Utils::warnNoComposite();
        emit releaseEvent();
        if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
            m_pScreenRecordEvent->terminate();
            m_pScreenRecordEvent->wait();
        }
        QApplication::quit();
    }
}

void MainWindow::updateToolBarPos()
{
    if (m_shotflag == 1) {
        return;
    }
    m_isToolBarInside = false;
    if (m_toolBarInit == false) {
        m_toolBar->initToolBar();
        m_toolBar->setRecordLaunchMode(m_launchWithRecordFunc);
        m_toolBar->setIsZhaoxinPlatform(m_isZhaoxin);

        m_pVoiceVolumeWatcher = new voiceVolumeWatcher(this);
        m_pVoiceVolumeWatcher->start();
        connect(m_pVoiceVolumeWatcher, SIGNAL(sigRecodeState(bool)), this, SLOT(on_CheckRecodeCouldUse(bool)));
        m_toolBarInit = true;

        m_pCameraWatcher = new CameraWatcher(this);
        m_pCameraWatcher->start();
        connect(m_pCameraWatcher, SIGNAL(sigCameraState(bool)), this, SLOT(on_CheckVideoCouldUse(bool)));
    }

    QPoint toolbarPoint;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    toolbarPoint = QPoint(recordX + recordWidth - m_toolBar->width() - TOOLBAR_X_SPACING,
                          std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));

    if (toolbarPoint.x() <= 0) {
        m_repaintMainButton = true;
        toolbarPoint.setX(recordX);
        if (recordX + m_toolBar->width() + TOOLBAR_X_SPACING + m_shotButton->width() > m_backgroundRect.width()) {

            toolbarPoint.setX(0);
        }
    }
    if (toolbarPoint.y() >= m_backgroundRect.y() + m_backgroundRect.height()
            - m_toolBar->height() - 28) {
        m_repaintSideBar = true;
        if (recordY > 28 * 2 + 10) {
            toolbarPoint.setY(recordY - m_toolBar->height() - TOOLBAR_Y_SPACING);

        } else {
            toolbarPoint.setY(recordY + TOOLBAR_Y_SPACING);
            m_isToolBarInside = true;
        }
    }
    // 根据屏幕的具体实际坐标修正Y值
    // 多屏情况下， 右下角有可能在屏幕外面。
    for(int i = 0; i < m_screenInfo.size(); ++i) {
        if(toolbarPoint.x() + m_toolBar->width() >= m_screenInfo[i].x && toolbarPoint.x() + m_toolBar->width() <= m_screenInfo[i].x + m_screenInfo[i].width) {
            if(toolbarPoint.y() < m_screenInfo[i].y + TOOLBAR_Y_SPACING){
                // 屏幕上超出            
                toolbarPoint.setY(m_screenInfo[i].y + TOOLBAR_Y_SPACING);
            }else if(toolbarPoint.y() > m_screenInfo[i].y + m_screenInfo[i].height - m_toolBar->height() - TOOLBAR_Y_SPACING) {
                // 屏幕下超出
                int y = std::max(recordY - m_toolBar->height() - TOOLBAR_Y_SPACING, 0);
                if(y > m_screenInfo[i].y + m_screenInfo[i].height - m_toolBar->height() - TOOLBAR_Y_SPACING)
                    y = m_screenInfo[i].y + m_screenInfo[i].height - m_toolBar->height() - TOOLBAR_Y_SPACING;
                toolbarPoint.setY(y);
            }
            break;
        }
    }
    m_toolBar->showAt(toolbarPoint);
}

void MainWindow::updateSideBarPos()
{
    if (m_shotflag == 1) {
        return;
    }
    m_isSideBarInside = false;
    if (m_sideBarInit == false) {
        m_sideBar->initSideBar();
        m_sideBarInit = true;
    }

    QPoint sidebarPoint;
    sidebarPoint = QPoint(recordX + recordWidth + SIDEBAR_X_SPACING,
                          std::max(recordY + (recordHeight / 2 - m_sideBar->height() / 2), 0));
    if (m_sideBar->height() < recordHeight) {
        if (sidebarPoint.x() >= m_screenWidth - m_sideBar->width() - SIDEBAR_X_SPACING) {
            //修改属性栏在截图区域内部，无法触发的bug
            sidebarPoint.setX(recordX + recordWidth - m_sideBar->width() - SIDEBAR_X_SPACING);
            m_isSideBarInside = true;
//            m_sideBar->hide();
//            m_shapesWidget->updateSideBarPosition();
//            return;
        }
    }

    else if (m_sideBar->height() >= recordHeight) {

        sidebarPoint.setY(recordY - (m_sideBar->height() - recordHeight));
        if (sidebarPoint.x() >= m_screenWidth - m_sideBar->width() - SIDEBAR_X_SPACING) {
            if (sidebarPoint.y() - recordHeight <= 0) {
                sidebarPoint.setX(recordX + recordWidth - m_sideBar->width() - SIDEBAR_X_SPACING);
                sidebarPoint.setY(recordY + recordHeight + m_toolBar->height() + TOOLBAR_Y_SPACING * 2);

                if (sidebarPoint.y() + m_sideBar->height() > m_screenHeight) {
                    sidebarPoint.setX(recordX - m_sideBar->width() - SIDEBAR_X_SPACING);
                    sidebarPoint.setY(recordY + recordHeight - m_sideBar->height() - TOOLBAR_Y_SPACING);

                    //分辨率过小的情况下
                    if (sidebarPoint.y() < 0) {
                        sidebarPoint.setX(m_toolBar->x() - m_sideBar->width() - SIDEBAR_X_SPACING);
                        if (sidebarPoint.x() <= m_screenWidth - recordX) {
                            sidebarPoint.setX(recordX - m_sideBar->width() - SIDEBAR_X_SPACING);
                            if (sidebarPoint.x() <= 0) {
                                sidebarPoint.setX(0);
                            }
                        }
                        if (sidebarPoint.x() <= 0) {
                            sidebarPoint.setX(0);
                        }
                        sidebarPoint.setY(m_toolBar->y() + m_toolBar->height() - m_sideBar->height());
                        if (sidebarPoint.y() <= 0) {
                            sidebarPoint.setY(0);
                        }
                    }

                    else {
                        sidebarPoint.setX(m_toolBar->x() - m_sideBar->width() - SIDEBAR_X_SPACING);
                        if (sidebarPoint.x() <= m_screenWidth - recordX) {
                            sidebarPoint.setX(recordX - m_sideBar->width() - SIDEBAR_X_SPACING);
                            if (sidebarPoint.x() <= 0) {
                                sidebarPoint.setX(0);
                            }
                        }
                        if (sidebarPoint.x() <= 0) {
                            sidebarPoint.setX(0);
                        }
                    }
                }
            }

            else {
                sidebarPoint.setX(recordX + recordWidth - m_sideBar->width() - SIDEBAR_X_SPACING);

                if (m_repaintSideBar == false) {
                    sidebarPoint.setY(recordY - m_sideBar->height() - TOOLBAR_Y_SPACING);
                }

                else {
                    sidebarPoint.setY(recordY - m_sideBar->height() - TOOLBAR_Y_SPACING - m_toolBar->height() - TOOLBAR_Y_SPACING);
                }

            }
        }

        else {
            if (m_repaintMainButton == false) {
                if (sidebarPoint.y() <= 0) {
                    sidebarPoint.setY(recordY);
                }
            }

            if (m_repaintMainButton == true) {
                sidebarPoint.setX(recordX + recordWidth + SIDEBAR_X_SPACING);
                if (sidebarPoint.y() <= 0) {
                    sidebarPoint.setX(recordX);
                    sidebarPoint.setY(recordY + recordHeight + m_toolBar->height() + TOOLBAR_Y_SPACING * 2);
                }

                else {
                    if (m_repaintSideBar == false) {
                        sidebarPoint.setY(recordY - (m_sideBar->height() - recordHeight));
                    }

                    else {
                        sidebarPoint.setX(recordX);
                        sidebarPoint.setY(recordY - m_sideBar->height() - TOOLBAR_Y_SPACING - m_toolBar->height() - TOOLBAR_Y_SPACING);
                    }

                }
            }
        }

    }

    // 根据屏幕的具体实际坐标修正Y值
    // 多屏情况下， 右下角有可能在屏幕外面。
    for(int i = 0; i < m_screenInfo.size(); ++i) {
        if(sidebarPoint.x() + m_sideBar->width() >= m_screenInfo[i].x && sidebarPoint.x() + m_sideBar->width() <= m_screenInfo[i].x + m_screenInfo[i].width) {
            if(sidebarPoint.y() < m_screenInfo[i].y + TOOLBAR_Y_SPACING){
                // 屏幕上超出
                sidebarPoint.setY(m_screenInfo[i].y + TOOLBAR_Y_SPACING);
            }else if(sidebarPoint.y() > m_screenInfo[i].y + m_screenInfo[i].height - m_sideBar->height() - TOOLBAR_Y_SPACING) {
                // 屏幕下超出
                //int y = std::max(recordY - m_sideBar->height() - TOOLBAR_Y_SPACING, 0);
                //if(y > m_screenInfo[i].y + m_screenInfo[i].height - m_sideBar->height() - TOOLBAR_Y_SPACING)
                    //y = m_screenInfo[i].y + m_screenInfo[i].height - m_sideBar->height() - TOOLBAR_Y_SPACING;
                sidebarPoint.setY(m_screenInfo[i].y + m_screenInfo[i].height - m_sideBar->height() - TOOLBAR_Y_SPACING);
            }
            break;
        }
    }
    m_sideBar->showAt(sidebarPoint);
}

void MainWindow::updateRecordButtonPos()
{

    if (m_shotflag == 1) {
        return;
    }
    QPoint recordButtonBarPoint;
    recordButtonBarPoint = QPoint(recordX + recordWidth - m_recordButton->width() + 3,
                                  std::max(recordY + recordHeight + TOOLBAR_Y_SPACING + 6, 0));

//    qDebug() << "recordButtonBarPoint y" << recordButtonBarPoint.y();

    if (m_repaintMainButton == true) {
        recordButtonBarPoint.setX(recordX + m_toolBar->width() + TOOLBAR_X_SPACING - m_recordButton->width() + 3);
        if (recordX + m_toolBar->width() + TOOLBAR_X_SPACING + m_shotButton->width() > m_backgroundRect.width()) {

            recordButtonBarPoint.setX(m_toolBar->width() + TOOLBAR_X_SPACING - m_recordButton->width() + 3);
        }
    }

    if (recordButtonBarPoint.y() >= m_backgroundRect.y() + m_backgroundRect.height()
            - m_toolBar->height() - 22) {
        if (recordY > 28 * 2 + 10) {
            recordButtonBarPoint.setY(recordY - m_recordButton->height() - TOOLBAR_Y_SPACING - 6);
        } else {
            recordButtonBarPoint.setY(recordY + TOOLBAR_Y_SPACING + 6);
        }
    }

    if (m_functionType == 0) {
        if (!m_recordButton->isVisible()) {
            m_recordButton->show();
        }
    }

    // 根据屏幕的具体实际坐标修正Y值
    for(int i = 0; i < m_screenInfo.size(); ++i) {
        if(recordButtonBarPoint.x() > m_screenInfo[i].x && recordButtonBarPoint.x() < m_screenInfo[i].x + m_screenInfo[i].width) {
            if(recordButtonBarPoint.y() < m_screenInfo[i].y + TOOLBAR_Y_SPACING){
                // 屏幕上超出
                recordButtonBarPoint.setY(m_screenInfo[i].y + TOOLBAR_Y_SPACING + 6);
            }else if(recordButtonBarPoint.y() > m_screenInfo[i].y + m_screenInfo[i].height - m_recordButton->height() - TOOLBAR_Y_SPACING) {
                // 屏幕下超出
                int y = std::max(recordY - m_recordButton->height() - TOOLBAR_Y_SPACING - 6, 0);
                if(y > m_screenInfo[i].y + m_screenInfo[i].height - m_recordButton->height() - TOOLBAR_Y_SPACING -6)
                    y = m_screenInfo[i].y + m_screenInfo[i].height - m_recordButton->height() - TOOLBAR_Y_SPACING -6;
                recordButtonBarPoint.setY(y);
            }
            break;
        }
    }

    m_recordButton->move(recordButtonBarPoint.x(), recordButtonBarPoint.y());
}

void MainWindow::updateShotButtonPos()
{
    if (m_shotflag == 1) {
        return;
    }
    QPoint shotButtonBarPoint;
    shotButtonBarPoint = QPoint(recordX + recordWidth - m_shotButton->width() + 3,
                                std::max(recordY + recordHeight + TOOLBAR_Y_SPACING + 6, 0));

    if (m_repaintMainButton == true) {
        shotButtonBarPoint.setX(recordX + m_toolBar->width() + TOOLBAR_X_SPACING - m_shotButton->width() + 3);
        if (recordX + m_toolBar->width() + TOOLBAR_X_SPACING + m_shotButton->width() > m_backgroundRect.width()) {

            shotButtonBarPoint.setX(m_toolBar->width() + TOOLBAR_X_SPACING - m_shotButton->width() + 3);
        }
    }

    if (shotButtonBarPoint.y() >= m_backgroundRect.y() + m_backgroundRect.height()
            - m_toolBar->height() - 22) {
        if (recordY > 28 * 2 + 10) {
            shotButtonBarPoint.setY(recordY - m_shotButton->height() - TOOLBAR_Y_SPACING - 6);
        } else {
            shotButtonBarPoint.setY(recordY + TOOLBAR_Y_SPACING + 6);
        }
    }

    if (m_functionType == 1) {
        if (!m_shotButton->isVisible()) {
            m_shotButton->show();
        }
    }

    // 根据屏幕的具体实际坐标修正Y值
    for(int i = 0; i < m_screenInfo.size(); ++i) {
        if(shotButtonBarPoint.x() > m_screenInfo[i].x && shotButtonBarPoint.x() < m_screenInfo[i].x + m_screenInfo[i].width) {
            if(shotButtonBarPoint.y() < m_screenInfo[i].y + TOOLBAR_Y_SPACING){
                // 屏幕上超出
                shotButtonBarPoint.setY(m_screenInfo[i].y + TOOLBAR_Y_SPACING + 6);
            }else if(shotButtonBarPoint.y() > m_screenInfo[i].y + m_screenInfo[i].height - m_shotButton->height() - TOOLBAR_Y_SPACING) {
                // 屏幕下超出
                int y = std::max(recordY - m_shotButton->height() - TOOLBAR_Y_SPACING - 6, 0);
                if(y > m_screenInfo[i].y + m_screenInfo[i].height - m_shotButton->height() - TOOLBAR_Y_SPACING - 6)
                    y = m_screenInfo[i].y + m_screenInfo[i].height - m_shotButton->height() - TOOLBAR_Y_SPACING -6;
                shotButtonBarPoint.setY(y);
            }
            break;
        }
    }

    m_shotButton->move(shotButtonBarPoint.x(), shotButtonBarPoint.y());
}
void MainWindow::updateCameraWidgetPos()
{
    if (m_shotflag == 1) {
        return;
    }
    if (!m_selectedCamera)
        return;
    bool isScaled = recordWidth != m_cameraWidget->getRecordWidth() || recordHeight != m_cameraWidget->getRecordHeight();
    if (isScaled) {
        int cameraWidgetWidth = recordWidth * 2 / 5;
        if (cameraWidgetWidth > CAMERA_WIDGET_MAX_WIDTH)
            cameraWidgetWidth = CAMERA_WIDGET_MAX_WIDTH;

        int cameraWidgetHeight = recordHeight * 1 / 4;
        if (cameraWidgetHeight > CAMERA_WIDGET_MAX_HEIGHT)
            cameraWidgetHeight = CAMERA_WIDGET_MAX_HEIGHT;
        int tempHeight = cameraWidgetWidth * 9 / 16;
        int tempWidth = cameraWidgetHeight * 16 / 9;
        if (tempHeight <= CAMERA_WIDGET_MAX_HEIGHT && tempHeight >= CAMERA_WIDGET_MIN_HEIGHT && tempHeight <= recordHeight) {
            cameraWidgetHeight = tempHeight;
        } else {
            cameraWidgetWidth = tempWidth;
        }
        int x = recordX;
        int y = recordY;
        switch (m_cameraWidget->postion()) {
        case CameraWidget::Position::leftTop:
            x = recordX;
            y = recordY;
            break;
        case CameraWidget::Position::leftBottom:
            x = recordX;
            y = recordY + recordHeight - cameraWidgetHeight;
            break;
        case CameraWidget::Position::rightTop:
            x = recordX + recordWidth - cameraWidgetWidth;
            y = recordY;
            break;
        case CameraWidget::Position::rightBottom:
            x = recordX + recordWidth - cameraWidgetWidth;
            y = recordY + recordHeight - cameraWidgetHeight;
            break;
        }
        m_cameraWidget->setRecordRect(recordX, recordY, recordWidth, recordHeight);
        m_cameraWidget->resize(cameraWidgetWidth, cameraWidgetHeight);
        m_cameraWidget->showAt(QPoint(x, y));
    } else {
        int x = recordX - m_cameraWidget->getRecordX();
        int y = recordY - m_cameraWidget->getRecordY();
        m_cameraWidget->showAt(QPoint(m_cameraWidget->x() + x, m_cameraWidget->y() + y));
        m_cameraWidget->setRecordRect(recordX, recordY, recordWidth, recordHeight);
    }
}
void MainWindow::changeFunctionButton(QString type)
{
    if (type == "record") {
        if (m_functionType == 0) {
            return;
        }
        m_shotButton->hide();
//        updateRecordButtonPos();
        m_recordButton->show();
        m_functionType = 0;
        initScreenRecorder();
        if (m_sideBar->isVisible()) {
            m_sideBar->hide();
        }
    }

    else if (type == "shot") {
        if (m_functionType == 1) {
            return;
        }
        m_toolBar->setVideoButtonInit();
        if (m_cameraWidget->isVisible()) {
            m_cameraWidget->cameraStop();
            m_cameraWidget->hide();
        }
        m_recordButton->hide();
//        updateShotButtonPos();
        m_shotButton->show();
        m_functionType = 1;
        initScreenShot();


    }

    update();
    repaint();
}

void MainWindow::showKeyBoardButtons(const QString &key)
{
    //键盘按钮启用状态下创建按键控件
    if (m_keyBoardStatus == 1) {
//        connect(m_keyBoardTimer, SIGNAL(timeout()), this, SLOT(showMultiKeyBoardButtons()));


//        if (m_keyBoardTimer->isActive()) {
//            qDebug() << "timer is actived!";
//        }

//        else {
//            qDebug() << "timer start!";
//            m_keyBoardTimer->start(1000);
//        }

        KeyButtonWidget *t_keyWidget = new KeyButtonWidget(this);
        t_keyWidget->setKeyLabelWord(key);
//        m_tempkeyButtonList.append(t_keyWidget);

//        QPoint t_keyPoint;
//        t_keyPoint = QPoint(recordX + recordWidth/2 - t_keyWidget->width()/2,
//                              std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));

//        t_keyWidget->show();

//        if (m_multiKeyButtonsInOnSec == true) {
        m_keyButtonList.append(t_keyWidget);

        if (m_keyButtonList.count() > 5) {
            delete m_keyButtonList.first();
            m_keyButtonList.pop_front();
        }
        qDebug() << "aaa key count:" << m_keyButtonList.count();
        //更新多按钮的位置
        updateMultiKeyBoardPos();
//        }

//        else {
//            if (!m_keyButtonList.isEmpty()) {
//                qDeleteAll(m_keyButtonList);
//                m_keyButtonList.clear();
//            }

//            m_keyButtonList.append(t_keyWidget);

//            if (m_keyButtonList.count() > 5) {
//                delete m_keyButtonList.first();
//                m_keyButtonList.pop_front();
//            }
//            qDebug() << "bbb key count:" << m_keyButtonList.count();
//            //更新多按钮的位置
//            updateMultiKeyBoardPos();
//            m_multiKeyButtonsInOnSec = true;

//        }
//        t_keyWidget->move(t_keyPoint.x(), t_keyPoint.y());

        repaint();
    }
}

void MainWindow::changeKeyBoardShowEvent(bool checked)
{
    qDebug() << "keyboard" << checked;

    if (checked == false) {
        m_keyBoardStatus = 0;
        if (m_keyButtonList.count() > 0) {
            for (int t_index = 0; t_index < m_keyButtonList.count(); t_index++) {
                m_keyButtonList.at(t_index)->hide();
            }
        }
    }

    else if (checked == true) {
        m_keyBoardStatus = 1;
    }
}

void MainWindow::changeMouseShowEvent(bool checked)
{
    qDebug() << "mouse" << checked;
    if (checked == false) {
        m_mouseStatus = 0;
    }

    else {
        m_mouseStatus = 1;
    }
    return;
}
void MainWindow::changeMicrophoneSelectEvent(bool checked)
{
    m_selectedMic = checked;
}
void MainWindow::changeSystemAudioSelectEvent(bool checked)
{
    m_selectedSystemAudio = checked;
}

void MainWindow::changeGifSelectEvent(bool checked)
{
    m_gifMode = true;
    m_mp4Mode = false;
    qDebug() << "gif: " << checked;
}

void MainWindow::changeMp4SelectEvent(bool checked)
{
    m_gifMode = false;
    m_mp4Mode = true;
    qDebug() << "mp4: " << checked;
}

void MainWindow::changeFrameRateEvent(int frameRate)
{
    qDebug() << "frameRate: " << frameRate;
    switch (frameRate) {
    case 5:
        m_frameRate = RecordProcess::RECORD_FRAMERATE_5;
        break;
    case 10:
        m_frameRate = RecordProcess::RECORD_FRAMERATE_10;
        break;
    case 20:
        m_frameRate = RecordProcess::RECORD_FRAMERATE_20;
        break;
    case 24:
        m_frameRate = RecordProcess::RECORD_FRAMERATE_24;
        break;
    case 30:
        m_frameRate = RecordProcess::RECORD_FRAMERATE_30;
        break;
    default:
        m_frameRate = RecordProcess::RECORD_FRAMERATE_24;
        break;
    }
}

void MainWindow::changeCameraSelectEvent(bool checked)
{
    if (m_initCamera == false) {
        m_cameraWidget->initCamera();
        m_initCamera = true;
    }

    m_selectedCamera = checked;
    if (checked) {
        qDebug() << "camera checked" << checked;
        int cameraWidgetWidth = recordWidth * 2 / 5;
        if (cameraWidgetWidth > CAMERA_WIDGET_MAX_WIDTH)
            cameraWidgetWidth = CAMERA_WIDGET_MAX_WIDTH;

        int cameraWidgetHeight = recordHeight * 1 / 4;
        if (cameraWidgetHeight > CAMERA_WIDGET_MAX_HEIGHT)
            cameraWidgetHeight = CAMERA_WIDGET_MAX_HEIGHT;
        int tempHeight = cameraWidgetWidth * 9 / 16;
        int tempWidth = cameraWidgetHeight * 16 / 9;
        if (tempHeight <= CAMERA_WIDGET_MAX_HEIGHT && tempHeight >= CAMERA_WIDGET_MIN_HEIGHT && tempHeight <= recordHeight) {
            cameraWidgetHeight = tempHeight;
        } else {
            cameraWidgetWidth = tempWidth;
        }
        int x = recordX + recordWidth - cameraWidgetWidth;
        int y = recordY + recordHeight - cameraWidgetHeight;
        m_cameraWidget->setRecordRect(recordX, recordY, recordWidth, recordHeight);
        m_cameraWidget->resize(cameraWidgetWidth, cameraWidgetHeight);
        m_cameraWidget->showAt(QPoint(x, y));
        if (!m_cameraWidget->cameraStart()) {
            m_cameraWidget->cameraStart();
        }
    } else {
        m_cameraWidget->cameraStop();
        m_cameraWidget->hide();
    }
}

void MainWindow::showMultiKeyBoardButtons()
{
//    m_keyBoardTimer->stop();
//    if(!m_keyButtonList.isEmpty())
//    {
//        m_keyButtonList.clear();
//    }

//    m_keyButtonList.append(m_tempkeyButtonList);
//    m_keyButtonList = m_tempkeyButtonList;

//    if(!m_keyButtonList.isEmpty())
//    {
//        for(int t_index = 0; t_index < m_keyButtonList.count(); t_index++)
//        {
//            QPoint t_keyPoint;
//            t_keyPoint = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(t_index)->width()/2,
//                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
//            m_keyButtonList.at(t_index)->show();

//            m_keyButtonList.at(t_index)->move(t_keyPoint.x(), t_keyPoint.y());

//            repaint();
//        }
//    }

//    return;

    m_multiKeyButtonsInOnSec = false;
}

void MainWindow::updateMultiKeyBoardPos()
{
    QPoint t_keyPoint1;
    QPoint t_keyPoint2;
    QPoint t_keyPoint3;
    QPoint t_keyPoint4;
    QPoint t_keyPoint5;

    if (!m_keyButtonList.isEmpty()) {
        switch (m_keyButtonList.count()) {
        //一个按键的情况
        case 1:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();
            break;
        //两个按键的情况
        case 2:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 + m_keyButtonList.at(1)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();
            break;
            //三个按键的情况
        case 3:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2 + m_keyButtonList.at(2)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();
            break;
        //四个按键的情况
        case 4:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 - m_keyButtonList.at(1)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2 + m_keyButtonList.at(2)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();

            m_keyButtonList.at(3)->hide();
            t_keyPoint4 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(3)->width() * 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(3)->move(t_keyPoint4.x(), t_keyPoint4.y());
            m_keyButtonList.at(3)->show();
            break;
            //五个按键的情况
        case 5:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 2.6),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 - m_keyButtonList.at(1)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();

            m_keyButtonList.at(3)->hide();
            t_keyPoint4 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(3)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(3)->move(t_keyPoint4.x(), t_keyPoint4.y());
            m_keyButtonList.at(3)->show();

            m_keyButtonList.at(4)->hide();
            t_keyPoint5 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(4)->width() * 2.6),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(4)->move(t_keyPoint5.x(), t_keyPoint5.y());
            m_keyButtonList.at(4)->show();
            break;
        default:
            break;
        }
    }

}

void MainWindow::changeShotToolEvent(const QString &func)
{

    if (!m_sideBar->isVisible()) {
        updateSideBarPos();
    }

    if (m_isShapesWidgetExist && func != "color") {
        m_shapesWidget->setCurrentShape(func);
    } else if (func != "color") {
        initShapeWidget(func);
        m_isShapesWidgetExist = true;
    }

    m_sideBar->changeShotToolFunc(func);
}

void MainWindow::saveScreenShot()
{
    m_CursorImage = m_pScreenRecordEvent->GetCursorImage();
    emit releaseEvent();
    m_shotflag = 1;
    emit saveActionTriggered();
    hideAllWidget();
    update();


//    if (m_hotZoneInterface->isValid())
//        m_hotZoneInterface->asyncCall("EnableZoneDetected",  true);
//    m_needSaveScreenshot = true;

    m_toolBar->setVisible(false);
    m_sizeTips->setVisible(false);
    m_sideBar->setVisible(false);
    m_shotButton->setVisible(false);
    m_recordButton->setVisible(false);
    m_sizeTips->setVisible(false);

    shotCurrentImg();

    const bool r = saveAction(m_resultPixmap);
    sendNotify(m_saveIndex, m_saveFileName, r);
}

void MainWindow::sendNotify(SaveAction saveAction, QString saveFilePath, const bool succeed)
{
    Q_UNUSED(saveAction);
    if(m_noNotify) {
        exit(0);
    }
    // failed notify
    if (!succeed) {
        const auto tips = tr("Save failed. Please save it in your home directory.");
        m_notifyDBInterface->Notify("Deepin Screenshot", 0, "deepin-screen-recorder", QString(), tips, QStringList(), QVariantMap(), 5000);

        exit(0);
    }

    QDBusInterface remote_dde_notify_obj("com.deepin.dde.Notification", "/com/deepin/dde/Notification",
                                         "com.deepin.dde.Notification");

    const bool remote_dde_notify_obj_exist = remote_dde_notify_obj.isValid();

    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());


    QStringList actions;
    QVariantMap hints;

    // 保存到剪贴板， 通知不用open
    if (remote_dde_notify_obj_exist && saveFilePath.compare(QString(tr("Clipboard")))) {
        actions << "_open" << tr("View");

        QString fileDir  = QUrl::fromLocalFile(QFileInfo(saveFilePath).absoluteDir().absolutePath()).toString();
        QString filePath = QUrl::fromLocalFile(saveFilePath).toString();

        QString command;
        if (QFile("/usr/bin/dde-file-manager").exists()) {
            command = QString("/usr/bin/dde-file-manager,--show-item,%1").arg(filePath);
        } else {
            command = QString("xdg-open,%1").arg(filePath);
        }

        hints["x-deepin-action-_open"] = command;
    }

    qDebug() << "saveFilePath:" << saveFilePath;

//    QString summary;
//    if (saveAction == SaveAction::SaveToClipboard) {
//        summary = QString(tr("Picture has been saved to clipboard"));
//    } else {
//        summary = QString(tr("Picture has been saved to %1")).arg(saveFilePath);
//    }

//    if (saveAction == SaveAction::SaveToClipboard && !m_noNotify) {

//        QVariantMap emptyMap;
//        m_notifyDBInterface->Notify("Deepin Screenshot", 0,  "deepin-screenshot", "",
//                                    summary,  QStringList(), emptyMap, 5000);
//    }  else if ( !m_noNotify &&  !(m_saveIndex == SaveAction::SaveToSpecificDir && m_saveFileName.isEmpty())) {

//        m_notifyDBInterface->Notify("Deepin Screenshot", 0,  "deepin-screen-recorder", "",
//                                    summary, actions, hints, 0);
    QList<QVariant> arg;
    int timeout = 5000;
    unsigned int id = 0;
    arg << (QCoreApplication::applicationName())                 // appname
        << id                                                    // id
        << QString("deepin-screenshot")                     // icon
        << tr("Screenshot finished")                              // summary
        << QString(tr("Saved to %1")).arg(saveFilePath) // body
        << actions                                               // actions
        << hints                                                 // hints
        << timeout;
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);// timeout
//    }

    QTimer::singleShot(2, [ = ] {
        emit releaseEvent();
        if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false)
        {
            if(nullptr != m_pScreenRecordEvent){
                m_pScreenRecordEvent->terminate();
                m_pScreenRecordEvent->wait();
                m_pScreenRecordEvent = nullptr;
            }
            if(nullptr != m_pScreenShotEvent){
                m_pScreenShotEvent->terminate();
                m_pScreenShotEvent->wait();
                m_pScreenShotEvent = nullptr;
            }
        }
        qApp->quit();
    });
}

bool MainWindow::saveAction(const QPixmap &pix)
{
    emit releaseEvent();

//    using namespace utils;
    QPixmap screenShotPix = pix;
    QDateTime currentDate;
    QString currentTime =  currentDate.currentDateTime().
                           toString("yyyyMMddHHmmss");
    m_saveFileName = "";

    QStandardPaths::StandardLocation saveOption = QStandardPaths::TempLocation;
//    bool copyToClipboard = true;

    int t_pictureFormat = ConfigSettings::instance()->value("save", "format").toInt();
    //int t_saveToClipBoard = ConfigSettings::instance()->value("save", "saveClip").toInt();

    //if (t_saveToClipBoard == 0) {
        //m_copyToClipboard = false;
    //}else {
        //m_copyToClipboard = true;
    //}

    std::pair<bool, SaveAction> temporarySaveAction = ConfigSettings::instance()->getTemporarySaveAction();
    if (temporarySaveAction.first) {
        m_saveIndex = temporarySaveAction.second;
    } else {
        m_saveIndex = ConfigSettings::instance()->value("save", "save_op").value<SaveAction>();
    }

    if (m_shotWithPath == true) {
        m_saveIndex = AutoSave;
    }

    //for test
//    m_saveIndex = SaveToImage;
    switch (m_saveIndex) {
    case SaveToDesktop: {
        saveOption = QStandardPaths::DesktopLocation;
        ConfigSettings::instance()->setValue("common", "default_savepath", QStandardPaths::writableLocation(
                                                 QStandardPaths::DesktopLocation));
        break;
    }
    case SaveToImage: {
        saveOption = QStandardPaths::PicturesLocation;
        ConfigSettings::instance()->setValue("common", "default_savepath", QStandardPaths::writableLocation(
                                                 QStandardPaths::PicturesLocation));
        break;
    }
//    case AutoSave: {
//        QString defaultSaveDir = ConfigSettings::instance()->value("common", "default_savepath").toString();
//        if (defaultSaveDir.isEmpty()) {
//            saveOption = QStandardPaths::DesktopLocation;
//        } else if (defaultSaveDir == "clipboard") {
//            copyToClipboard = true;
//            m_saveIndex = SaveToSpecificDir;
//        } else {
//            if (m_selectAreaName.isEmpty()) {
//                m_saveFileName = QString("%1/%2_%3.png").arg(defaultSaveDir).arg(tr(
//                                                                                     "DeepinScreenshot")).arg(currentTime);
//            } else {
//                m_saveFileName = QString("%1/%2_%3_%4.png").arg(defaultSaveDir).arg(tr(
//                                                                                        "DeepinScreenshot")).arg(m_selectAreaName).arg(currentTime);
//            }
//        }
//        break;
//    }
    case SaveToSpecificDir: {
        this->hide();
        this->releaseKeyboard();

        QString path = ConfigSettings::instance()->value("common", "default_savepath").toString();
        QString fileName = selectAreaName;

        if (path.isEmpty() || !QDir(path).exists()) {
            path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        }

        if (fileName.isEmpty()) {
            fileName = QString("%1_%2").arg(tr("Screen Capture")).arg(currentTime);
        } else {
            fileName = QString("%1_%2_%3").arg(tr("Screen Capture")).arg(selectAreaName).arg(currentTime);
        }
        QString lastFileName;
        QFileDialog fileDialog;

        switch (t_pictureFormat) {
        case 0:
            lastFileName    = QString("%1/%2.png").arg(path).arg(fileName);
            m_saveFileName =  fileDialog.getSaveFileName(this, tr("Save"),  lastFileName,
                                                         tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
            break;
        case 1:
            lastFileName    = QString("%1/%2.jpg").arg(path).arg(fileName);
            m_saveFileName =  fileDialog.getSaveFileName(this, tr("Save"),  lastFileName,
                                                         tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)"));
            break;
        case 2:
            lastFileName    = QString("%1/%2.bmp").arg(path).arg(fileName);
            m_saveFileName =  fileDialog.getSaveFileName(this, tr("Save"),  lastFileName,
                                                         tr("BMP (*.bmp);;JPEG (*.jpg *.jpeg);;PNG (*.png)"));
            break;
        default:
            lastFileName    = QString("%1/%2.png").arg(path).arg(fileName);
            m_saveFileName =  fileDialog.getSaveFileName(this, tr("Save"),  lastFileName,
                                                         tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
            break;
        }




        if (m_saveFileName.isEmpty() || QFileInfo(m_saveFileName).isDir())
            return false;

        QString fileSuffix = QFileInfo(m_saveFileName).completeSuffix();
        if (fileSuffix.isEmpty()) {
//            m_saveFileName = m_saveFileName + ".png";

            switch (t_pictureFormat) {
            case 0:
                m_saveFileName = m_saveFileName + ".png";
                break;
            case 1:
                m_saveFileName = m_saveFileName + ".jpg";
                break;
            case 2:
                m_saveFileName = m_saveFileName + ".bmp";
                break;
            default:
                m_saveFileName = m_saveFileName + ".png";
                break;
            }
        } else if ( !isValidFormat(fileSuffix)) {
            qWarning() << "The fileName has invalid suffix!" << fileSuffix << m_saveFileName;

            switch (t_pictureFormat) {
            case 0:
                m_saveFileName = m_saveFileName + ".png";
                break;
            case 1:
                m_saveFileName = m_saveFileName + ".jpg";
                break;
            case 2:
                m_saveFileName = m_saveFileName + ".bmp";
                break;
            default:
                m_saveFileName = m_saveFileName + ".png";
                break;
            }

//            return false;
        }

        ConfigSettings::instance()->setValue("common", "default_savepath",
                                             QFileInfo(m_saveFileName).dir().absolutePath());
        break;
    }
    case AutoSave:
        break;
    case SaveToClipboard:{
        qDebug() << SaveToClipboard << "SaveToClipboard";
        break;
    }
    default:
        break;
    }

//    DDesktopServices::playSystemSoundEffect(DDesktopServices::SEE_Screenshot);

//    int toolBarSaveQuality = std::min(ConfigSettings::instance()->value("save",
//                                                                        "save_quality").toInt(), 100);
    int toolBarSaveQuality = 100;
    if (toolBarSaveQuality != 100) {
        qreal saveQuality = qreal(toolBarSaveQuality) * 5 / 1000 + 0.5;

        int pixWidth = screenShotPix.width();
        int pixHeight = screenShotPix.height();
        screenShotPix = screenShotPix.scaled(static_cast<int>(pixWidth * saveQuality), static_cast<int>(pixHeight * saveQuality),
                                             Qt::KeepAspectRatio, Qt::FastTransformation);
        screenShotPix = screenShotPix.scaled(pixWidth,  pixHeight,
                                             Qt::KeepAspectRatio, Qt::FastTransformation);
    }
    if (m_saveIndex == SaveToSpecificDir && m_saveFileName.isEmpty()) {
        return false;
    } else if (m_saveIndex == SaveToSpecificDir || !m_saveFileName.isEmpty()) {
        if (!screenShotPix.save(m_saveFileName,  QFileInfo(m_saveFileName).suffix().toLocal8Bit()))
            return false;
    } else if (saveOption != QStandardPaths::TempLocation && m_saveFileName.isEmpty()) {
        QString savePath;
        if (m_shotWithPath == false) {
            savePath = QStandardPaths::writableLocation(saveOption);
        }

        else {
            savePath = m_shotSavePath;
        }

        QDir saveDir(savePath);
        if (!saveDir.exists()) {
            bool mkdirSucc = saveDir.mkpath(".");
            if (!mkdirSucc) {
                qCritical() << "Save path not exist and cannot be created:" << savePath;
                qCritical() << "Fall back to temp location!";
                savePath = QDir::tempPath();
            }
        }
        QString t_formatStr;
        QString t_formatBuffix;
        switch (t_pictureFormat) {
        case 0:
            t_formatStr = "PNG";
            t_formatBuffix = "png";
            break;
        case 1:
            t_formatStr = "JPEG";
            t_formatBuffix = "jpg";
            break;
        case 2:
            t_formatStr = "BMP";
            t_formatBuffix = "bmp";
            break;
        default:
            t_formatStr = "PNG";
            t_formatBuffix = "png";
            break;
        }
        if (selectAreaName.isEmpty()) {
            m_saveFileName = QString("%1/%2_%3.%4").arg(savePath, tr("Screen Capture"), currentTime, t_formatBuffix);
        } else {
            m_saveFileName = QString("%1/%2_%3_%4.%5").arg(savePath, tr("Screen Capture"), selectAreaName, currentTime, t_formatBuffix);
        }
        qDebug() << m_saveFileName;
        if (!screenShotPix.save(m_saveFileName,  t_formatStr.toLatin1().data()))
            return false;

    } else if (m_saveIndex == AutoSave && m_saveFileName.isEmpty()) {
        QString savePath;
//        if (m_shotWithPath == false) {
//            savePath = QStandardPaths::writableLocation(saveOption);
//        }

//        else {
        savePath = m_shotSavePath;
//        }
        QString t_fileName = "";
        if (savePath.contains(".png")) {
            t_pictureFormat = 0;
//            savePath.lastIndexOf("/");
            t_fileName = savePath;
        }

        if (savePath.contains(".jpg")) {
            t_pictureFormat = 1;
//            savePath.lastIndexOf("/");
            t_fileName = savePath;
        }

        if (savePath.contains(".bmp")) {
            t_pictureFormat = 2;
//            savePath.lastIndexOf("/");
            t_fileName = savePath;
        }

        if (t_fileName == "") {
            QDir saveDir(savePath);
            if (!saveDir.exists()) {
                bool mkdirSucc = saveDir.mkpath(".");
                if (!mkdirSucc) {
                    qCritical() << "Save path not exist and cannot be created:" << savePath;
                    qCritical() << "Fall back to temp location!";
                    savePath = QDir::tempPath();
                }
            }
        }
        QString t_formatStr;
        QString t_formatBuffix;
        switch (t_pictureFormat) {
        case 0:
            t_formatStr = "PNG";
            t_formatBuffix = "png";
            break;
        case 1:
            t_formatStr = "JPEG";
            t_formatBuffix = "jpg";
            break;
        case 2:
            t_formatStr = "BMP";
            t_formatBuffix = "bmp";
            break;
        default:
            t_formatStr = "PNG";
            t_formatBuffix = "png";
            break;
        }
        qDebug() << "save path" << savePath;

        if (t_fileName != "") {
            m_saveFileName = t_fileName;
        } else {
            if (selectAreaName.isEmpty()) {
                m_saveFileName = QString("%1/%2_%3.%4").arg(savePath, tr("Screen Capture"), currentTime, t_formatBuffix);
            } else {
                m_saveFileName = QString("%1/%2_%3_%4.%5").arg(savePath, tr("Screen Capture"), selectAreaName, currentTime, t_formatBuffix);
            }
        }


        if (!screenShotPix.save(m_saveFileName,  t_formatStr.toLatin1().data()))
            return false;
    }else if(m_saveIndex == SaveToClipboard){
       m_saveFileName = QString(tr("Clipboard"));
    }

    // 保存到剪贴板
    QMimeData *t_imageData = new QMimeData;
    t_imageData->setImageData(screenShotPix);
    Q_ASSERT(!screenShotPix.isNull());
    QClipboard *cb = qApp->clipboard();
    cb->setMimeData(t_imageData, QClipboard::Clipboard);
    //if (m_copyToClipboard) {}
    // 调起画板， 传入截图路径
    /*
    int t_saveCursor = ConfigSettings::instance()->value("open", "draw").toInt();
    if (t_saveCursor == 1) {
        DrawInterface *m_draw = new DrawInterface("com.deepin.Draw", "/com/deepin/Draw", QDBusConnection::sessionBus(), this);
        QList<QString> list;
        list.append(m_saveFileName);
        m_draw->openFiles(list);
    }
    */
    return true;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // Just use for debug.
    // repaintCounter++;
    // qDebug() << repaintCounter;

    if (m_shotflag == 1) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QRect backgroundRect = QRect(0, 0, rootWindowRect.width(), rootWindowRect.height());
        // FIXME: Under the magnifying glass, it seems to be magnified two times.
        QScreen *screen = qApp->primaryScreen();
        const qreal dpiVal = screen->devicePixelRatio();
        m_backgroundPixmap.setDevicePixelRatio(dpiVal);
        painter.drawPixmap(backgroundRect, m_backgroundPixmap);
//        DWidget::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_functionType == 1) {
        painter.setRenderHint(QPainter::Antialiasing, true);
        QRect backgroundRect;

        backgroundRect = QRect(0, 0, rootWindowRect.width(), rootWindowRect.height());


        // FIXME: Under the magnifying glass, it seems to be magnified two times.
        QScreen *screen = qApp->primaryScreen();
        const qreal dpiVal = screen->devicePixelRatio();
        m_backgroundPixmap.setDevicePixelRatio(dpiVal);
        painter.drawPixmap(backgroundRect, m_backgroundPixmap);
    }

    if (recordWidth > 0 && recordHeight > 0) {
        m_firstShot = 1;
        QRect backgroundRect = QRect(0, 0, rootWindowRect.width(), rootWindowRect.height());
        QRect frameRect = QRect(recordX, recordY, recordWidth, recordHeight);

        // Draw background.
        painter.setBrush(QBrush("#000000"));
        painter.setOpacity(0.2);

        painter.setClipping(true);
        painter.setClipRegion(QRegion(backgroundRect).subtracted(QRegion(frameRect)));
        painter.drawRect(backgroundRect);

        // Reset clip.
        painter.setClipRegion(QRegion(backgroundRect));

        // Draw frame.
        if (recordButtonStatus != RECORD_BUTTON_RECORDING) {
            painter.setRenderHint(QPainter::Antialiasing, false);
//            QPen framePen(QColor("#01bdff"));
            QPen framePen(Qt::white);
            framePen.setStyle(Qt::DashLine);
            framePen.setDashOffset(0);
            framePen.setWidth(1);
            painter.setOpacity(1);
            painter.setBrush(QBrush());  // clear brush
            painter.setPen(framePen);
            painter.drawRect(QRect(
                                 std::max(frameRect.x(), 1),
                                 std::max(frameRect.y(), 1),
                                 std::min(frameRect.width() - 1, rootWindowRect.width() - 2),
                                 std::min(frameRect.height() - 1, rootWindowRect.height() - 2)));
            painter.setRenderHint(QPainter::Antialiasing, true);
        }

        // Draw drag pint.
        if (recordButtonStatus == RECORD_BUTTON_NORMAL && drawDragPoint) {
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth - 1, recordY - DRAG_POINT_RADIUS), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth - 1, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth - 1, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleBigImg);

//            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS), resizeHandleSmallImg);
//            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS), resizeHandleSmallImg);
//            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleSmallImg);
//            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleSmallImg);
//            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleSmallImg);
//            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleSmallImg);
//            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS), resizeHandleSmallImg);
//            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleSmallImg);
        }



//        if (m_functionType == 0) {
//            QRect backgroundRect = QRect(0, 0, rootWindowRect.width(), rootWindowRect.height());
//            QRect frameRect = QRect(recordX, recordY, recordWidth, recordHeight);

//            // Draw background.
//            painter.setBrush(QBrush("#000000"));
//            painter.setOpacity(0.2);

//            painter.setClipping(true);
//            painter.setClipRegion(QRegion(backgroundRect).subtracted(QRegion(frameRect)));
//            painter.drawRect(backgroundRect);
//        }
    }
}
bool MainWindow::eventFilter(QObject *, QEvent *event)
{
    bool needRepaint = false;
#undef KeyPress
#undef KeyRelease
    if (event->type() == QEvent::KeyPress) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        qDebug() << "key press:" << keyEvent->key();
        if (m_functionType == 1) {
            if (keyEvent->key() == Qt::Key_Escape ) {
                if (m_isShapesWidgetExist) {
                    if (m_shapesWidget->textEditIsReadOnly()) {
                        return false;
                    }
                }
                qDebug() << "Key_Escape pressed: app quit!";
                emit releaseEvent();
                if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
                    if(nullptr != m_pScreenRecordEvent){
                        m_pScreenRecordEvent->terminate();
                        m_pScreenRecordEvent->wait();
                        m_pScreenRecordEvent = nullptr;
                    }
                    if(nullptr != m_pScreenShotEvent){
                        m_pScreenShotEvent->terminate();
                        m_pScreenShotEvent->wait();
                        m_pScreenShotEvent = nullptr;
                    }
                }
                qApp->quit();
            } else if (keyEvent->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (keyEvent->key() == Qt::Key_Question) {
                    onViewShortcut();
                } else if (keyEvent->key() == Qt::Key_Z) {
                    qDebug() << "SDGF: ctrl+shift+z !!!";
                    emit unDoAll();
                }
            } else if (qApp->keyboardModifiers() & Qt::ControlModifier) {
                if (keyEvent->key() == Qt::Key_C) {
//                    ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToClipboard);
                        //m_copyToClipboard = true;
                        //saveScreenShot();
                } else if (keyEvent->key() == Qt::Key_Z) {
                    qDebug() << "SDGF: ctrl+z !!!";
                    emit unDo();
                }
            } else if(keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter){
                saveScreenShot();
            }

            bool needRepaint = false;
            if (m_isShapesWidgetExist) {
                if (keyEvent->key() == Qt::Key_Escape) {
                    emit releaseEvent();
                    if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
                        if(nullptr != m_pScreenRecordEvent){
                            m_pScreenRecordEvent->terminate();
                            m_pScreenRecordEvent->wait();
                            m_pScreenRecordEvent = nullptr;
                        }
                        if(nullptr != m_pScreenShotEvent){
                            m_pScreenShotEvent->terminate();
                            m_pScreenShotEvent->wait();
                            m_pScreenShotEvent = nullptr;
                        }
                    }
                    qApp->quit();
                }

                if (keyEvent->key() == Qt::Key_Shift) {
                    m_isShiftPressed =  true;
                    m_shapesWidget->setShiftKeyPressed(m_isShiftPressed);
                }

                if (keyEvent->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
                    if (keyEvent->key() == Qt::Key_Left) {
                        m_shapesWidget->microAdjust("Ctrl+Shift+Left");
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        m_shapesWidget->microAdjust("Ctrl+Shift+Right");
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        m_shapesWidget->microAdjust("Ctrl+Shift+Up");
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        m_shapesWidget->microAdjust("Ctrl+Shift+Down");
                    }
                } else if (qApp->keyboardModifiers() & Qt::ControlModifier) {
                    if (keyEvent->key() == Qt::Key_Left) {
                        m_shapesWidget->microAdjust("Ctrl+Left");
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        m_shapesWidget->microAdjust("Ctrl+Right");
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        m_shapesWidget->microAdjust("Ctrl+Up");
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        m_shapesWidget->microAdjust("Ctrl+Down");
                    } else if (keyEvent->key() == Qt::Key_C) {
//                        ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToClipboard);
                        //m_copyToClipboard = true;
                        //saveScreenShot();
                    } else if (keyEvent->key() == Qt::Key_S) {
//                        expressSaveScreenshot();
                        saveScreenShot();
                    }
                }  else {
                    if (keyEvent->key() == Qt::Key_Left) {
                        m_shapesWidget->microAdjust("Left");
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        m_shapesWidget->microAdjust("Right");
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        m_shapesWidget->microAdjust("Up");
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        m_shapesWidget->microAdjust("Down");
                    }
                }

                if (keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace) {
                    emit  deleteShapes();
                } else {
                    qDebug() << "ShapeWidget Exist keyEvent:" << keyEvent->key();
                }
                return  false;
            }

            if (m_shotStatus == ShotMouseStatus::Normal) {
                if (keyEvent->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {

                    if (keyEvent->key() == Qt::Key_Left) {
                        if (recordWidth > RECORD_MIN_SHOT_SIZE) {
                            recordX = std::max(0, recordX + 1);
                            recordWidth = std::max(std::min(recordWidth - 1,
                                                            m_backgroundRect.width()), RECORD_MIN_SHOT_SIZE);
                            needRepaint = true;
                            selectAreaName = tr("select-area");
                        }

                    } else if (keyEvent->key() == Qt::Key_Right) {
                        if (recordWidth > RECORD_MIN_SHOT_SIZE) {
                            recordWidth = std::max(std::min(recordWidth - 1,
                                                            m_backgroundRect.width()), RECORD_MIN_SHOT_SIZE);
                            needRepaint = true;
                            selectAreaName = tr("select-area");
                        }
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        if (recordHeight > RECORD_MIN_SHOT_SIZE) {
                            recordY = std::max(0, recordY + 1);

                            recordHeight = std::max(std::min(recordHeight - 1,
                                                             m_backgroundRect.height()), RECORD_MIN_SHOT_SIZE);
                            needRepaint = true;
                            selectAreaName = tr("select-area");
                        }
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        if (recordHeight > RECORD_MIN_SHOT_SIZE) {
                            recordHeight = std::max(std::min(recordHeight - 1,
                                                             m_backgroundRect.height()), RECORD_MIN_SHOT_SIZE);
                            needRepaint = true;
                            selectAreaName = tr("select-area");
                        }
                    }
                } else if (qApp->keyboardModifiers() & Qt::ControlModifier) {
                    if (keyEvent->key() == Qt::Key_S) {
//                        expressSaveScreenshot();
                        saveScreenShot();
                    }

                    if (keyEvent->key() == Qt::Key_C) {
//                        ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToClipboard);
                        //m_copyToClipboard = true;
//                        saveScreenshot();
                        //saveScreenShot();
                    }
                    if (keyEvent->key() == Qt::Key_Left) {
                        recordX = std::max(0, recordX - 1);
                        recordWidth = std::min(recordWidth + 1, rootWindowRect.width());

                        needRepaint = true;
                        selectAreaName = tr("select-area");
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        if (recordX + recordWidth + 1 >= m_screenWidth) {
                            recordX = std::max(0, recordX - 1);
                        }
                        recordWidth = std::min(recordWidth + 1, rootWindowRect.width());

                        needRepaint = true;
                        selectAreaName = tr("select-area");
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        recordY = std::max(0, recordY - 1);
                        recordHeight = std::min(recordHeight + 1, rootWindowRect.height());

                        needRepaint = true;
                        selectAreaName = tr("select-area");
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        if (recordY + recordHeight + 1 >= m_screenHeight) {
                            recordY = std::max(0, recordY - 1);
                        }
                        recordHeight = std::min(recordHeight + 1, rootWindowRect.height());

                        needRepaint = true;
                        selectAreaName = tr("select-area");
                    }
                } else {
                    if (keyEvent->key() == Qt::Key_Left) {
                        recordX = std::max(0, recordX - 1);

                        needRepaint = true;
                        selectAreaName = tr("select-area");
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        recordX = std::min(m_backgroundRect.width() - recordWidth,
                                           recordX + 1);

                        needRepaint = true;
                        selectAreaName = tr("select-area");
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        recordY = std::max(0, recordY - 1);

                        needRepaint = true;
                        selectAreaName = tr("select-area");
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        recordY = std::min(m_backgroundRect.height() -
                                           recordHeight, recordY + 1);

                        needRepaint = true;
                        selectAreaName = tr("select-area");
                    }
                }

                if ( !m_needSaveScreenshot) {
                    m_sizeTips->updateTips(QPoint(recordX, recordY),
                                           QString("%1X%2").arg(recordWidth).arg(recordHeight));
                    if(m_toolBar->isVisible()) {
                        updateToolBarPos();
                    }
                    if (m_recordButton->isVisible()) {
                        updateRecordButtonPos();
                    }

                    if (m_sideBar->isVisible()) {
                        updateSideBarPos();
                    }

                    if (m_shotButton->isVisible()) {
                        updateShotButtonPos();
                    }

                    if (m_cameraWidget->isVisible()) {
                        updateCameraWidgetPos();
                    }
                }
            }

            if (needRepaint) {
                update();
            }
            DWidget::keyPressEvent(keyEvent);
        }

//        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        else {
            if (recordButtonStatus == RECORD_BUTTON_NORMAL) {

                if (keyEvent->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
                    if (keyEvent->key() == Qt::Key_Question) {
                        onViewShortcut();
                    }
                }

                if (keyEvent->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {

                    if (keyEvent->key() == Qt::Key_Left) {
                        if (recordWidth > RECORD_MIN_SIZE) {
                            recordX = std::max(0, recordX + 1);
                            recordWidth = std::max(std::min(recordWidth - 1,
                                                            m_backgroundRect.width()), RECORD_MIN_SIZE);
                            needRepaint = true;
                        }

                    } else if (keyEvent->key() == Qt::Key_Right) {
                        if (recordWidth > RECORD_MIN_SIZE) {
                            recordWidth = std::max(std::min(recordWidth - 1,
                                                            m_backgroundRect.width()), RECORD_MIN_SIZE);
                            needRepaint = true;
                        }
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        if (recordHeight > RECORD_MIN_HEIGHT) {
                            recordY = std::max(0, recordY + 1);

                            recordHeight = std::max(std::min(recordHeight - 1,
                                                             m_backgroundRect.height()), RECORD_MIN_HEIGHT);
                            needRepaint = true;
                        }
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        if (recordHeight > RECORD_MIN_HEIGHT) {
                            recordHeight = std::max(std::min(recordHeight - 1,
                                                             m_backgroundRect.height()), RECORD_MIN_HEIGHT);
                            needRepaint = true;
                        }
                    }
                } else if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                    if (keyEvent->key() == Qt::Key_Left) {
                        recordX = std::max(0, recordX - 1);
                        recordWidth = std::min(recordWidth + 1, rootWindowRect.width());

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        if (recordX + recordWidth + 1 >= m_screenWidth) {
                            recordX = std::max(0, recordX - 1);
                        }
                        recordWidth = std::min(recordWidth + 1, rootWindowRect.width());

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        recordY = std::max(0, recordY - 1);
                        recordHeight = std::min(recordHeight + 1, rootWindowRect.height());

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        if (recordY + recordHeight + 1 >= m_screenHeight) {
                            recordY = std::max(0, recordY - 1);
                        }
                        recordHeight = std::min(recordHeight + 1, rootWindowRect.height());

                        needRepaint = true;
                    }
                } else {
                    if (keyEvent->key() == Qt::Key_Left) {
                        recordX = std::max(0, recordX - 1);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        recordX = std::min(rootWindowRect.width() - recordWidth, recordX + 1);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        recordY = std::max(0, recordY - 1);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        recordY = std::min(rootWindowRect.height() - recordHeight, recordY + 1);

                        needRepaint = true;
                    }
                }

                m_sizeTips->updateTips(QPoint(recordX, recordY),
                                       QString("%1X%2").arg(recordWidth).arg(recordHeight));
                if(m_toolBar->isVisible()){
                    updateToolBarPos();
                }
                if (m_recordButton->isVisible()) {
                    updateRecordButtonPos();
                }

                if (m_sideBar->isVisible()) {
                    updateSideBarPos();
                }

                if (m_shotButton->isVisible()) {
                    updateShotButtonPos();
                }

                if (m_cameraWidget->isVisible()) {
                    updateCameraWidgetPos();
                }


                if (recordButtonStatus == RECORD_BUTTON_NORMAL && needRepaint) {
                    hideRecordButton();
                }
            }
        }

    } else if (event->type() == QEvent::KeyRelease) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (m_functionType == 1) {
            bool needRepaint = false;

            if (m_isShapesWidgetExist) {
                if (keyEvent->key() == Qt::Key_Shift) {
                    m_isShiftPressed =  false;
                    m_shapesWidget->setShiftKeyPressed(m_isShiftPressed);
                }
            }

            if (!keyEvent->isAutoRepeat()) {
                if (keyEvent->key() == Qt::Key_Left || keyEvent->key()
                        == Qt::Key_Right || keyEvent->key() == Qt::Key_Up ||
                        keyEvent->key() == Qt::Key_Down) {
                    needRepaint = true;
                }
            }
            if (needRepaint) {
                update();
            }

        }

        else {
            if (!keyEvent->isAutoRepeat()) {
                if (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right || keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
                    needRepaint = true;
                }

                if (recordButtonStatus == RECORD_BUTTON_NORMAL && needRepaint) {
                    showRecordButton();
                    updateToolBarPos();
                    if (m_functionType == 1 && m_sideBar->isVisible()) {
                        updateSideBarPos();
                    }
                    updateRecordButtonPos();
                    updateShotButtonPos();
                    updateCameraWidgetPos();
                }
            }
        }
        // NOTE: must be use 'isAutoRepeat' to filter KeyRelease event send by Qt.
        DWidget::keyReleaseEvent(keyEvent);
    }

    if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            if (m_functionType == 1) {
                saveScreenShot();
            }

        }
    }

    if (event->type() == QEvent::MouseButtonPress) {
        if (!m_isShapesWidgetExist) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragStartX = mouseEvent->x();
                dragStartY = mouseEvent->y();
                if (!isFirstPressButton) {
                    isFirstPressButton = true;

//                    Utils::clearBlur(windowManager, this->winId());
                } else {
                    dragAction = getAction(event);

                    dragRecordX = recordX;
                    dragRecordY = recordY;
                    dragRecordWidth = recordWidth;
                    dragRecordHeight = recordHeight;

                    if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                        hideRecordButton();
                        hideAllWidget();
                        hideCameraWidget();
                        //隐藏键盘按钮控件
                        if (m_keyButtonList.count() > 0) {
                            for (int t_index = 0; t_index < m_keyButtonList.count(); t_index++) {
                                m_keyButtonList.at(t_index)->hide();
                            }
                        }
                    }
                }

                isPressButton = true;
                isReleaseButton = false;
            }

            if (mouseEvent->button() == Qt::RightButton) {
                if (!isFirstPressButton) {
                    return false;
                }
                if (m_functionType == 1) {
                    m_menuController->showMenu(QPoint(mapToGlobal(mouseEvent->pos())));
                }
            }
        }


    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (!m_isShapesWidgetExist) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                if (m_sizeTips->isVisible()) {
                    m_sizeTips->updateTips(QPoint(recordX, recordY),
                                           QString("%1X%2").arg(recordWidth).arg(recordHeight));
                }
                if (!isFirstReleaseButton) {
                    isFirstReleaseButton = true;

                    updateCursor(event);
                    updateToolBarPos();
                    if (m_functionType == 1 && m_sideBar->isVisible()) {
                        updateSideBarPos();
                        m_zoomIndicator->hide();
                    }
                    updateRecordButtonPos();
                    updateShotButtonPos();
                    // Record select area name with window name if just click (no drag).
                    if (!isFirstDrag) {
                        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                        for (auto it = windowRects.rbegin(); it != windowRects.rend(); ++it) {
                            if (QRect(it->x(), it->y(), it->width(), it->height()).contains(mouseEvent->pos() + screenRect.topLeft())) {
                                selectAreaName = windowNames[windowRects.rend() - it - 1];
                                break;
                            }
                        }
                    }

                        if (m_functionType == 0) {
                            // Make sure record area not too small.
                            recordWidth = recordWidth < RECORD_MIN_SIZE ? RECORD_MIN_SIZE : recordWidth;
                            recordHeight = recordHeight < RECORD_MIN_HEIGHT ? RECORD_MIN_HEIGHT : recordHeight;

                            if (recordX + recordWidth > rootWindowRect.width()) {
                                recordX = rootWindowRect.width() - recordWidth;
                            }

                            if (recordY + recordHeight > rootWindowRect.height()) {
                                recordY = rootWindowRect.height() - recordHeight;
                            }
                        }

                        else if (m_functionType == 1) {
                            // Make sure record area not too small.
                            recordWidth = recordWidth < RECORD_MIN_SHOT_SIZE ? RECORD_MIN_SHOT_SIZE : recordWidth;
                            recordHeight = recordHeight < RECORD_MIN_SHOT_SIZE ? RECORD_MIN_SHOT_SIZE : recordHeight;

                            if (recordX + recordWidth > rootWindowRect.width()) {
                                recordX = rootWindowRect.width() - recordWidth;
                            }

                            if (recordY + recordHeight > rootWindowRect.height()) {
                                recordY = rootWindowRect.height() - recordHeight;
                            }
                        }



                    showRecordButton();
                    updateToolBarPos();
                    if (m_functionType == 1 && m_sideBar->isVisible()) {
                        updateSideBarPos();
                    }
                    updateRecordButtonPos();
                    updateShotButtonPos();

                    needRepaint = true;
                } else {
                    if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                        showRecordButton();
                        updateToolBarPos();
                        if (m_functionType == 1 && m_sideBar->isVisible()) {
                            updateSideBarPos();
                        }
                        updateRecordButtonPos();
                        updateShotButtonPos();
                        updateCameraWidgetPos();

                    }
                }
                if (m_sizeTips->isVisible()) {
                    m_sizeTips->updateTips(QPoint(recordX, recordY),
                                           QString("%1X%2").arg(recordWidth).arg(recordHeight));
                }

                isPressButton = false;
                isReleaseButton = true;

                needRepaint = true;
            }
        }


    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (!m_isShapesWidgetExist) {


            if (m_toolBar->isVisible()) {
                updateToolBarPos();
                m_zoomIndicator->hide();
            }

            if (!isFirstMove) {
                isFirstMove = true;
            }

            else {
                if (m_functionType == 1) {
                    if (!m_toolBar->isVisible() && !isFirstReleaseButton) {
                        QScreen *screen = qApp->primaryScreen();
                        const qreal dpiVal = screen->devicePixelRatio();
                        QPoint curPos = this->cursor().pos();
                        QPoint tmpPos;
                        QPoint topLeft = m_backgroundRect.topLeft() * dpiVal;

                        if (curPos.x() + INDICATOR_WIDTH + CURSOR_WIDTH > topLeft.x()
                                + m_backgroundRect.width()) {
                            tmpPos.setX(curPos.x() - INDICATOR_WIDTH);
                        } else {
                            tmpPos.setX(curPos.x() + CURSOR_WIDTH);
                        }

                        if (curPos.y() + INDICATOR_WIDTH > topLeft.y() + m_backgroundRect.height()) {
                            tmpPos.setY(curPos.y() - INDICATOR_WIDTH);
                        } else {
                            tmpPos.setY(curPos.y() + CURSOR_HEIGHT);
                        }

                        m_zoomIndicator->showMagnifier(QPoint(
                                                           std::max(tmpPos.x() - topLeft.x(), 0),
                                                           std::max(tmpPos.y() - topLeft.y(), 0)));
                    }

                }
            }

            if (isPressButton && isFirstPressButton) {
                if (!isFirstDrag) {
                    isFirstDrag = true;

                    selectAreaName = tr("select-area");
                }
            }


            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (isFirstPressButton) {
                if (!isFirstReleaseButton) {
                    if (isPressButton && !isReleaseButton) {
                        recordX = std::min(dragStartX, mouseEvent->x());
                        recordY = std::min(dragStartY, mouseEvent->y());
                        recordWidth = std::abs(dragStartX - mouseEvent->x());
                        recordHeight = std::abs(dragStartY - mouseEvent->y());

                        needRepaint = true;
                    }
                } else if (isPressButton) {
                    if (recordButtonStatus == RECORD_BUTTON_NORMAL && dragRecordX >= 0 && dragRecordY >= 0) {
                        if (dragAction == ACTION_MOVE) {
                            recordX = std::max(std::min(dragRecordX + mouseEvent->x() - dragStartX, rootWindowRect.width() - recordWidth), 0);
                            recordY = std::max(std::min(dragRecordY + mouseEvent->y() - dragStartY, rootWindowRect.height() - recordHeight), 0);
                        } else if (dragAction == ACTION_RESIZE_TOP_LEFT) {
                            resizeTop(mouseEvent);
                            resizeLeft(mouseEvent);
                        } else if (dragAction == ACTION_RESIZE_TOP_RIGHT) {
                            resizeTop(mouseEvent);
                            resizeRight(mouseEvent);
                        } else if (dragAction == ACTION_RESIZE_BOTTOM_LEFT) {
                            resizeBottom(mouseEvent);
                            resizeLeft(mouseEvent);
                        } else if (dragAction == ACTION_RESIZE_BOTTOM_RIGHT) {
                            resizeBottom(mouseEvent);
                            resizeRight(mouseEvent);
                        } else if (dragAction == ACTION_RESIZE_TOP) {
                            resizeTop(mouseEvent);
                        } else if (dragAction == ACTION_RESIZE_BOTTOM) {
                            resizeBottom(mouseEvent);
                        } else if (dragAction == ACTION_RESIZE_LEFT) {
                            resizeLeft(mouseEvent);
                        } else if (dragAction == ACTION_RESIZE_RIGHT) {
                            resizeRight(mouseEvent);
                        }

                        needRepaint = true;
                    }
                }

                updateCursor(event);

                int action = getAction(event);
                bool drawPoint = action != ACTION_MOVE;
                if (drawPoint != drawDragPoint) {
                    drawDragPoint = drawPoint;
                    needRepaint = true;
                }
            } else {
                // Select the first window where the mouse is located
                QScreen *screen = qApp->primaryScreen();
                const qreal ratio = screen->devicePixelRatio();
                const QPoint mousePoint = QCursor::pos();
                for (auto it = windowRects.rbegin(); it != windowRects.rend(); ++it) {
                    if (QRect(it->x(), it->y(), it->width(), it->height()).contains(mousePoint)) {
                        recordX = it->x() - static_cast<int>(screenRect.x() * ratio);
                        recordY = it->y() - static_cast<int>(screenRect.y() * ratio);
                        recordWidth = it->width();
                        recordHeight = it->height();
                        needRepaint = true;
                        break;
                    }
                }
            }
        }

        else {
            QRect t_rect;
            t_rect.setX(recordX);
            t_rect.setY(recordY);
            t_rect.setWidth(recordWidth);
            t_rect.setHeight(recordHeight);

            if (!t_rect.contains(mouseEvent->x(), mouseEvent->y())) {
                qApp->setOverrideCursor(Qt::ArrowCursor);
            }
        }
        if (m_shotflag == 0) {
            m_sizeTips->updateTips(QPoint(recordX, recordY),
                                   QString("%1X%2").arg(recordWidth).arg(recordHeight));
        }

    }

    // Use flag instead call `repaint` directly,
    // to avoid repaint many times in one event function.

    if (needRepaint) {
        repaint();
    }

    return false;
}

void MainWindow::onViewShortcut()
{
    //QRect rect = window()->geometry();
    //多屏情况下bug修复， 将快捷键预览框显示在主屏中央。
    QRect rect = QGuiApplication::primaryScreen()->geometry();
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
    Shortcut sc;
    QStringList shortcutString;
    QString param1 = "-j=" + sc.toStr();
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << "-b" << param1 << param2;

    QProcess *shortcutViewProc = new QProcess(this);
    shortcutViewProc->startDetached("killall deepin-shortcut-viewer");
    shortcutViewProc->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProc, SIGNAL(finished(int)), shortcutViewProc, SLOT(deleteLater()));

    if (m_isShapesWidgetExist) {
        m_isShiftPressed =  false;
        m_shapesWidget->setShiftKeyPressed(m_isShiftPressed);
    }


}

void MainWindow::shapeClickedSlot(QString shape)
{
    m_toolBar->shapeClickedFromMain(shape);
}

void MainWindow::on_CheckRecodeCouldUse(bool canUse)
{
    m_toolBar->setMicroPhoneEnable(canUse);
}

void MainWindow::on_CheckVideoCouldUse(bool canUse)
{
//
    if (!canUse) {
        if (m_initCamera && !m_cameraOffFlag) {
            if (m_cameraWidget->getcameraStatus() == false) {
                qDebug() << "camera canuse" << canUse;
                m_cameraWidget->cameraStop();
                m_cameraWidget->setCameraStop(canUse);
                m_cameraOffFlag = true;
                m_cameraWidget->hide();
                m_toolBar->setCameraDeviceEnable(canUse);
            }
        } else {
            m_toolBar->setCameraDeviceEnable(canUse);
        }
    } else if (canUse) {
        m_toolBar->setCameraDeviceEnable(canUse);
        if (m_cameraOffFlag) {
            m_cameraWidget->cameraResume();
            m_cameraOffFlag = false;
        }
    }
}

void MainWindow::checkCpuIsZhaoxin()
{
    QStringList options;
    options << "-c";
    options << "lscpu | grep 'CentaurHauls'";
    QProcess process;
    process.start("bash", options);
    process.waitForFinished();
    process.waitForReadyRead();
    QString str_output = process.readAllStandardOutput();
    qDebug() << "is zhao xin:" << str_output;

    if (str_output.length() == 0) {
        m_isZhaoxin = false;
    }

    else {
        m_isZhaoxin = true;
    }

    process.close();
}

void MainWindow::startRecord()
{
//    Utils::clearBlur(windowManager, this->winId());
//    emit releaseEvent();
    recordButtonStatus = RECORD_BUTTON_RECORDING;

    resetCursor();

    repaint();

    trayIcon->show();

    flashTrayIconTimer = new QTimer(this);
    connect(flashTrayIconTimer, SIGNAL(timeout()), this, SLOT(flashTrayIcon()));
    flashTrayIconTimer->start(800);

    recordProcess.setRecordAudioInputType(getRecordInputType(m_selectedMic, m_selectedSystemAudio));
    recordProcess.setIsZhaoXinPlatform(m_isZhaoxin);
    recordProcess.startRecord();

    m_pVoiceVolumeWatcher->stopWatch();

    m_pCameraWatcher->stopWatch();
//    voiceRecordProcess.startRecord();
}

void MainWindow::shotCurrentImg()
{
    if (recordWidth == 0 || recordHeight == 0)
        return;

    m_needDrawSelectedPoint = false;
    m_drawNothing = true;
    update();

    QEventLoop eventloop1;
    QTimer::singleShot(60, &eventloop1, SLOT(quit()));
    eventloop1.exec();

    qDebug() << "shotCurrentImg shotFullScreen";
    if (m_isShapesWidgetExist) {
        qDebug() << "hide shotFullScreen";
        m_shapesWidget->hide();
    }
    m_sizeTips->hide();
    shotFullScreen();


    this->hide();
    emit hideScreenshotUI();

    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    qDebug() << recordX << "," << recordY << "," << recordWidth << "," << recordHeight << m_resultPixmap.rect() << ratio;
    QRect target( static_cast<int>(recordX * ratio),
                  static_cast<int>(recordY * ratio),
                  static_cast<int>(recordWidth * ratio),
                  static_cast<int>(recordHeight * ratio));



    m_resultPixmap = m_resultPixmap.copy(target);
    addCursorToImage();
}
void MainWindow::addCursorToImage()
{
    //获取配置是否截取光标
    int t_saveCursor = ConfigSettings::instance()->value("save", "saveCursor").toInt();
    if (t_saveCursor == 0) {
        return;
    }
    QPoint coursePoint = this->cursor().pos();//获取当前光标的位置
    int x = coursePoint.x();
    int y = coursePoint.y();
    //光标是否在当前截取区域
    bool isUnderRect = ((x > recordX) && (x < recordX + recordWidth)) && ((y > recordY) && (y < recordY + recordHeight));
    if (isUnderRect == false) {
        return;
    }
    if(m_CursorImage == nullptr)
        return;
    const int dataSize = m_CursorImage->width * m_CursorImage->height * 4;
    uchar *pixels = new uchar[dataSize];
    int index = 0;
    for(int j = 0; j < m_CursorImage->width * m_CursorImage->height; ++j) {
        unsigned long curValue = m_CursorImage->pixels[j];
        pixels[index++] = static_cast<uchar>(curValue >> 0);
        pixels[index++] = static_cast<uchar>(curValue >> 8);
        pixels[index++] = static_cast<uchar>(curValue >> 16);
        pixels[index++] = static_cast<uchar>(curValue >> 24);
    }
    QImage cursorImage = QImage(pixels, m_CursorImage->width, m_CursorImage->height, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&m_resultPixmap);
    painter.drawImage(QRect(x - recordX - m_CursorImage->width / 2, y - recordY - m_CursorImage->height / 2, m_CursorImage->width, m_CursorImage->height), cursorImage);
    delete[] pixels;
    XFree(m_CursorImage);
    return;
}
void MainWindow::shotFullScreen()
{
    //const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    QRect target( m_backgroundRect.x(),
                  m_backgroundRect.y(),
                  m_backgroundRect.width(),
                  m_backgroundRect.height() );
    qDebug() << "m_backgroundRect" << m_backgroundRect;

//    m_resultPixmap = getPixmapofRect(m_backgroundRect);
    m_resultPixmap = getPixmapofRect(target);
    qDebug() << "m_resultPixmap" << m_resultPixmap.rect();
}

void MainWindow::flashTrayIcon()
{
    if (flashTrayIconCounter % 2 == 0) {
        trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon2.svg"))));
    } else {
        trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon1.svg"))));
    }

    flashTrayIconCounter++;

    if (flashTrayIconCounter > 10) {
        flashTrayIconCounter = 1;
    }
}

void MainWindow::resizeTop(QMouseEvent *mouseEvent)
{
    if (m_functionType == 0) {
        int offsetY = mouseEvent->y() - dragStartY;
        recordY = std::max(std::min(dragRecordY + offsetY, dragRecordY + dragRecordHeight - RECORD_MIN_HEIGHT), 1);
        recordHeight = std::max(std::min(dragRecordHeight - offsetY, rootWindowRect.height()), RECORD_MIN_HEIGHT);
    }

    else if (m_functionType == 1) {
        int offsetY = mouseEvent->y() - dragStartY;
        recordY = std::max(std::min(dragRecordY + offsetY, dragRecordY + dragRecordHeight - RECORD_MIN_SHOT_SIZE), 1);
        recordHeight = std::max(std::min(dragRecordHeight - offsetY, rootWindowRect.height()), RECORD_MIN_SHOT_SIZE);
    }

}

void MainWindow::resizeBottom(QMouseEvent *mouseEvent)
{
    if (m_functionType == 0) {
        int offsetY = mouseEvent->y() - dragStartY;
        recordHeight = std::max(std::min(dragRecordHeight + offsetY, rootWindowRect.height()), RECORD_MIN_HEIGHT);
    } else if (m_functionType == 1) {
        int offsetY = mouseEvent->y() - dragStartY;
        recordHeight = std::max(std::min(dragRecordHeight + offsetY, rootWindowRect.height()), RECORD_MIN_SHOT_SIZE);
    }
}

void MainWindow::resizeLeft(QMouseEvent *mouseEvent)
{
    if (m_functionType == 0) {
        int offsetX = mouseEvent->x() - dragStartX;
        recordX = std::max(std::min(dragRecordX + offsetX, dragRecordX + dragRecordWidth - RECORD_MIN_SIZE), 1);
        recordWidth = std::max(std::min(dragRecordWidth - offsetX, rootWindowRect.width()), RECORD_MIN_SIZE);
    } else if (m_functionType ==  1) {
        int offsetX = mouseEvent->x() - dragStartX;
        recordX = std::max(std::min(dragRecordX + offsetX, dragRecordX + dragRecordWidth - RECORD_MIN_SHOT_SIZE), 1);
        recordWidth = std::max(std::min(dragRecordWidth - offsetX, rootWindowRect.width()), RECORD_MIN_SHOT_SIZE);
    }

}

void MainWindow::resizeRight(QMouseEvent *mouseEvent)
{
    if (m_functionType == 0) {
        int offsetX = mouseEvent->x() - dragStartX;
        recordWidth = std::max(std::min(dragRecordWidth + offsetX, rootWindowRect.width()), RECORD_MIN_SIZE);
    } else if (m_functionType == 1) {
        int offsetX = mouseEvent->x() - dragStartX;
        recordWidth = std::max(std::min(dragRecordWidth + offsetX, rootWindowRect.width()), RECORD_MIN_SHOT_SIZE);
    }
}

int MainWindow::getAction(QEvent *event)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    int cursorX = mouseEvent->x();
    int cursorY = mouseEvent->y();

    if (cursorX > recordX - CURSOR_BOUND
            && cursorX < recordX + CURSOR_BOUND
            && cursorY > recordY - CURSOR_BOUND
            && cursorY < recordY + CURSOR_BOUND) {
        // Top-Left corner.
        return ACTION_RESIZE_TOP_LEFT;
    } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
               && cursorX < recordX + recordWidth + CURSOR_BOUND
               && cursorY > recordY + recordHeight - CURSOR_BOUND
               && cursorY < recordY + recordHeight + CURSOR_BOUND) {
        // Bottom-Right corner.
        return ACTION_RESIZE_BOTTOM_RIGHT;
    } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
               && cursorX < recordX + recordWidth + CURSOR_BOUND
               && cursorY > recordY - CURSOR_BOUND
               && cursorY < recordY + CURSOR_BOUND) {
        // Top-Right corner.
        return ACTION_RESIZE_TOP_RIGHT;
    } else if (cursorX > recordX - CURSOR_BOUND
               && cursorX < recordX + CURSOR_BOUND
               && cursorY > recordY + recordHeight - CURSOR_BOUND
               && cursorY < recordY + recordHeight + CURSOR_BOUND) {
        // Bottom-Left corner.
        return ACTION_RESIZE_BOTTOM_LEFT;
    } else if (cursorX > recordX - CURSOR_BOUND
               && cursorX < recordX + CURSOR_BOUND) {
        // Left.
        return ACTION_RESIZE_LEFT;
    } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
               && cursorX < recordX + recordWidth + CURSOR_BOUND) {
        // Right.
        return ACTION_RESIZE_RIGHT;
    } else if (cursorY > recordY - CURSOR_BOUND
               && cursorY < recordY + CURSOR_BOUND) {
        // Top.
        return ACTION_RESIZE_TOP;
    } else if (cursorY > recordY + recordHeight - CURSOR_BOUND
               && cursorY < recordY + recordHeight + CURSOR_BOUND) {
        // Bottom.
        return ACTION_RESIZE_BOTTOM;
    } else {
        return ACTION_MOVE;
    }
}

void MainWindow::updateCursor(QEvent *event)
{
    if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        int cursorX = mouseEvent->x();
        int cursorY = mouseEvent->y();

        QRect t_rectbuttonRect = m_recordButton->geometry();

        t_rectbuttonRect.setX(t_rectbuttonRect.x() - 5);
        t_rectbuttonRect.setY(t_rectbuttonRect.y() - 2);
        t_rectbuttonRect.setWidth(t_rectbuttonRect.width() + 6);
        t_rectbuttonRect.setHeight(t_rectbuttonRect.height() + 2);

        if (cursorX > recordX - CURSOR_BOUND
                && cursorX < recordX + CURSOR_BOUND
                && cursorY > recordY - CURSOR_BOUND
                && cursorY < recordY + CURSOR_BOUND) {
            // Top-Left corner.
            QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
        } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
                   && cursorX < recordX + recordWidth + CURSOR_BOUND
                   && cursorY > recordY + recordHeight - CURSOR_BOUND
                   && cursorY < recordY + recordHeight + CURSOR_BOUND) {
            // Bottom-Right corner.
            QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
        } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
                   && cursorX < recordX + recordWidth + CURSOR_BOUND
                   && cursorY > recordY - CURSOR_BOUND
                   && cursorY < recordY + CURSOR_BOUND) {
            // Top-Right corner.
            QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
        } else if (cursorX > recordX - CURSOR_BOUND
                   && cursorX < recordX + CURSOR_BOUND
                   && cursorY > recordY + recordHeight - CURSOR_BOUND
                   && cursorY < recordY + recordHeight + CURSOR_BOUND) {
            // Bottom-Left corner.
            QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
        } else if (cursorX > recordX - CURSOR_BOUND
                   && cursorX < recordX + CURSOR_BOUND) {
            // Left.
            QApplication::setOverrideCursor(Qt::SizeHorCursor);
        } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
                   && cursorX < recordX + recordWidth + CURSOR_BOUND) {
            // Right.
            QApplication::setOverrideCursor(Qt::SizeHorCursor);
        } else if (cursorY > recordY - CURSOR_BOUND
                   && cursorY < recordY + CURSOR_BOUND) {
            // Top.
            QApplication::setOverrideCursor(Qt::SizeVerCursor);
        } else if (cursorY > recordY + recordHeight - CURSOR_BOUND
                   && cursorY < recordY + recordHeight + CURSOR_BOUND) {
            // Bottom.
            QApplication::setOverrideCursor(Qt::SizeVerCursor);

        }
//        else if (recordButton->geometry().contains(cursorX, cursorY) || recordOptionPanel->geometry().contains(cursorX, cursorY)) {
//            // Record area.
//            QApplication::setOverrideCursor(Qt::ArrowCursor);
//        }
        else if (t_rectbuttonRect.contains(cursorX, cursorY) || m_shotButton->geometry().contains(cursorX, cursorY)) {
            // Record button.
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        } else {
            if (isPressButton) {
                QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            } else {
                QApplication::setOverrideCursor(Qt::OpenHandCursor);
            }
        }
    }
}

void MainWindow::setDragCursor()
{
    QApplication::setOverrideCursor(Qt::CrossCursor);
}

void MainWindow::resetCursor()
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason)
{
    stopRecord();
}

void MainWindow::stopRecord()
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
        hide();
        emit releaseEvent();
        if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
            m_pScreenRecordEvent->terminate();
            m_pScreenRecordEvent->wait();
        }
        //正在保存录屏文件通知
        sendSavingNotify();
        // 停止闪烁
        flashTrayIconTimer->stop();

        recordProcess.stopRecord();
//        voiceRecordProcess.stopRecord();
    }
}

void MainWindow::startCountdown()
{
    if(nullptr != m_pScreenShotEvent)
    {
        m_pScreenShotEvent->terminate();
        m_pScreenShotEvent->wait();
        m_pScreenShotEvent = nullptr;
    }

    recordButtonStatus = RECORD_BUTTON_WAIT;

    disconnect(m_recordButton, SIGNAL(clicked()), this, SLOT(startCountdown()));
    disconnect(m_shotButton, SIGNAL(clicked()), this, SLOT(saveScreenShot()));

    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    const QPoint topLeft = geometry().topLeft();

    QRect recordRect {
        static_cast<int>(recordX * ratio + topLeft.x()),
        static_cast<int>(recordY * ratio + topLeft.y()),
        static_cast<int>(recordWidth * ratio),
        static_cast<int>(recordHeight * ratio)
    };
    qDebug() << "record rect:" << recordRect;

    recordProcess.setRecordInfo(recordRect, selectAreaName);
//    if (QSysInfo::currentCpuArchitecture().startsWith("x86")) {
//        if (recordOptionPanel->isSaveAsGif()) {
//            recordProcess.setRecordType(RecordProcess::RECORD_TYPE_GIF);
//        } else {
//            recordProcess.setRecordType(RecordProcess::RECORD_TYPE_VIDEO);
//        }
//    } else {
//        recordProcess.setRecordType(RecordProcess::RECORD_TYPE_GIF);
//    }

    Settings *t_settings = new Settings();
    QVariant t_saveGifVar = t_settings->getOption("save_as_gif");
    QVariant t_frameRateVar = t_settings->getOption("mkv_framerate");
    //保持帧数的配置文件判断
    int t_frameRate = t_frameRateVar.toString().toInt();

    bool t_saveGif = false;
    //保持格式的配置文件判断
    if (t_saveGifVar.toString() == "true") {
        t_saveGif = true;
    } else {
        t_saveGif = false;
    }

    recordProcess.setFrameRate(t_frameRate);

    if (t_saveGif) {
        qDebug() << "record format is gif";
        recordProcess.setRecordType(RecordProcess::RECORD_TYPE_GIF);
    } else if (t_saveGif == false) {
        qDebug() << "record format is mp4";
        recordProcess.setRecordType(RecordProcess::RECORD_TYPE_VIDEO);
    }

    resetCursor();

//    hideRecordButton();
    //隐藏所有的控件
    hideAllWidget();


    delete recordButtonLayout;

    countdownLayout = new QVBoxLayout();
    setLayout(countdownLayout);

    countdownLayout->addStretch();
    countdownLayout->addWidget(countdownTooltip, 0, Qt::AlignCenter);
    countdownLayout->addStretch();
    adjustLayout(countdownLayout, countdownTooltip->rect().width(), countdownTooltip->rect().height());

    countdownTooltip->start();


    Utils::passInputEvent(static_cast<int>(this->winId()));

    repaint();
}

void MainWindow::showRecordButton()
{
//    updateToolBarPos();
//    recordButton->show();
    if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
//        recordOptionPanel->show();

        adjustLayout(recordButtonLayout,
                     recordButton->width(),
                     recordButton->height() + RECORD_OPTIONAL_PADDING + recordOptionPanel->height());
    } else {
        adjustLayout(recordButtonLayout,
                     recordButton->width(),
                     recordButton->height() + RECORD_OPTIONAL_PADDING);
    }
}

void MainWindow::hideRecordButton()
{
    recordButton->hide();
    recordOptionPanel->hide();

//    Utils::clearBlur(windowManager, this->winId());
}

void MainWindow::hideAllWidget()
{
    m_toolBar->hide();
    m_sideBar->hide();
    m_recordButton->hide();
    m_shotButton->hide();
    m_sizeTips->hide();
    m_zoomIndicator->hide();

    //隐藏键盘按钮控件
    if (m_keyButtonList.count() > 0) {
        for (int t_index = 0; t_index < m_keyButtonList.count(); t_index++) {
            m_keyButtonList.at(t_index)->hide();
        }
    }

//    Utils::clearBlur(windowManager, this->winId());
}
void MainWindow::hideCameraWidget()
{
    m_cameraWidget->hide();
}
void MainWindow::adjustLayout(QVBoxLayout *layout, int layoutWidth, int layoutHeight)
{
    Q_UNUSED(layoutWidth);
    Q_UNUSED(layoutHeight);
//    if (recordHeight < layoutHeight) {
//        if (recordY + layoutHeight > rootWindowRect.height()) {
//            layout->setContentsMargins(
//                recordX,
//                recordY - layoutHeight - Constant::RECTANGLE_PADDING,
//                rootWindowRect.width() - recordX - recordWidth,
//                rootWindowRect.height() - recordY + Constant::RECTANGLE_PADDING);
//        } else {
//            layout->setContentsMargins(
//                recordX,
//                recordY + recordHeight + Constant::RECTANGLE_PADDING,
//                rootWindowRect.width() - recordX - recordWidth,
//                rootWindowRect.height() - (recordY + recordHeight + layoutHeight) + Constant::RECTANGLE_PADDING);
//        }
//    } else if (recordWidth < layoutWidth) {
//        if (recordX + layoutWidth > rootWindowRect.width()) {
//            layout->setContentsMargins(
//                recordX - layoutWidth - Constant::RECTANGLE_PADDING,
//                recordY,
//                rootWindowRect.width() - recordX + Constant::RECTANGLE_PADDING,
//                rootWindowRect.height() - recordY - recordHeight);
//        } else {
//            layout->setContentsMargins(
//                recordX + recordWidth + Constant::RECTANGLE_PADDING,
//                recordY,
//                rootWindowRect.width() - (recordX + recordWidth + layoutWidth + Constant::RECTANGLE_PADDING),
//                rootWindowRect.height() - recordY - recordHeight);
//        }
//    } else {
    layout->setContentsMargins(
        recordX,
        recordY,
        rootWindowRect.width() - recordX - recordWidth,
        rootWindowRect.height() - recordY - recordHeight);
//}
}

void MainWindow::initShapeWidget(QString type)
{
    qDebug() << "show shapesWidget";
    m_shapesWidget = new ShapesWidget(this);
    m_shapesWidget->setShiftKeyPressed(m_isShiftPressed);

    if (type != "color")
        m_shapesWidget->setCurrentShape(type);

    m_shapesWidget->show();
//    if (m_isSideBarInside == false && m_isToolBarInside == false) {
//        m_shapesWidget->setFixedSize(recordWidth - 4, recordHeight - 4);
//        m_shapesWidget->move(recordX + 2, recordY + 2);
//    }

//    else if (m_isSideBarInside == true && m_isToolBarInside == false) {
//        m_shapesWidget->setFixedSize(recordWidth - m_sideBar->width(), recordHeight - 4);
//        m_shapesWidget->move(recordX + 2, recordY + 2);
//    }

//    else if (m_isSideBarInside == true && m_isToolBarInside == true) {
//        m_shapesWidget->setFixedSize(recordWidth - m_sideBar->width(), recordHeight - m_toolBar->height());
//        m_shapesWidget->move(recordX + 2, recordY + m_toolBar->height());
//    }

//    else if (m_isSideBarInside == false && m_isToolBarInside == true) {
//        m_shapesWidget->setFixedSize(recordWidth - 4, recordHeight - m_toolBar->height());
//        m_shapesWidget->move(recordX + 2, recordY + m_toolBar->height());
//    }

    m_shapesWidget->setFixedSize(recordWidth - 4, recordHeight - 4);
    m_shapesWidget->move(recordX + 2, recordY + 2);
    QRect t_rect;
    t_rect.setX(recordX);
    t_rect.setY(recordY);
    t_rect.setWidth(recordWidth);
    t_rect.setHeight(recordHeight);
    m_shapesWidget->setGlobalRect(t_rect);


    updateToolBarPos();
    m_toolBar->raise();
    m_sideBar->raise();
    m_shotButton->raise();
    m_needDrawSelectedPoint = false;
    m_toolBar->setRecordButtonDisable();
    update();

//    connect(m_toolBar, &ToolBar::updateColor,
//            m_shapesWidget, &ShapesWidget::setPenColor);
    connect(m_shapesWidget, &ShapesWidget::reloadEffectImg,
            this, &MainWindow::reloadImage);
    connect(this, &MainWindow::deleteShapes, m_shapesWidget,
            &ShapesWidget::deleteCurrentShape);
    connect(m_shapesWidget, &ShapesWidget::saveFromMenu,
            this, &MainWindow::saveScreenShot);
    connect(m_shapesWidget, &ShapesWidget::closeFromMenu,
            this, &MainWindow::exitApp);
    connect(m_shapesWidget, &ShapesWidget::shapeClicked,
            this, &MainWindow::shapeClickedSlot);
//    connect(m_shapesWidget, &ShapesWidget::shapePressed,
//            m_toolBar, &ToolBar::shapePressed);
//    connect(m_shapesWidget, &ShapesWidget::saveBtnPressed,
//            m_toolBar, &ToolBar::saveBtnPressed);
//    connect(m_shapesWidget, &ShapesWidget::requestExit, this, &MainWindow::exitApp);
    connect(this, &MainWindow::unDo, m_shapesWidget, &ShapesWidget::undoDrawShapes);
    connect(this, &MainWindow::unDoAll, m_shapesWidget, &ShapesWidget::undoAllDrawShapes);
    connect(this, &MainWindow::saveActionTriggered,
            m_shapesWidget, &ShapesWidget::saveActionTriggered);
    connect(m_shapesWidget, &ShapesWidget::menuNoFocus, this, &MainWindow::activateWindow);
}

void MainWindow::exitApp()
{
    if (m_interfaceExist && nullptr != m_hotZoneInterface) {
        if (m_hotZoneInterface->isValid())
            m_hotZoneInterface->asyncCall("EnableZoneDetected",  true);
    }
    emit releaseEvent();
    if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
        if(nullptr != m_pScreenRecordEvent){
            m_pScreenRecordEvent->terminate();
            m_pScreenRecordEvent->wait();
            m_pScreenRecordEvent = nullptr;
        }
        if(nullptr != m_pScreenShotEvent){
            m_pScreenShotEvent->terminate();
            m_pScreenShotEvent->wait();
            m_pScreenShotEvent = nullptr;
        }
    }
    qApp->quit();
}

//void MainWindow::initVirtualCard()
//{
//    if (AudioUtils().canVirtualCardOutput()) {
//        return;
//    }
//    bool isOk;

//    QString text = DInputDialog::getText(this, tr("Need authorization"), tr("Please enter your sudo password to be authorized"),
//                                         QLineEdit::Password, "", &isOk);

//    if (isOk) {

//        QProcess p(this);
//        QStringList arguments;
//        arguments << QString("-c");
//        arguments << QString("echo %1 | sudo -S modprobe snd-aloop pcm_substreams=1 ; sudo sed -i '$ a snd_aloop' /etc/modules").arg(text);
//        qDebug() << arguments;
//        p.start("/bin/bash", arguments);
//        p.waitForFinished();
////        p.waitForReadyRead();
//        p.close();
//        sleep(1);

//        if (!AudioUtils().canVirtualCardOutput()) {
//            DDialog warnDlg(this);
//            warnDlg.setIcon(QIcon::fromTheme("deepin-screen-recorder"));
//            warnDlg.setTitle(tr("Password Error!"));
//            warnDlg.addSpacing(20);
//            warnDlg.addButton(tr("Ok"));
//            warnDlg.exec();
//        }

//        else {
//            DDialog warnDlg(this);
//            warnDlg.setIcon(QIcon::fromTheme("deepin-screen-recorder"));
//            warnDlg.setTitle(tr("Authentication success!"));
//            warnDlg.addSpacing(20);
//            warnDlg.addButton(tr("Ok"));
//            warnDlg.exec();

//            if (m_toolBarInit) {
//                m_toolBar->setSystemAudioEnable(true);
//            }
//        }


//    }

//}

int MainWindow::getRecordInputType(bool selectedMic, bool selectedSystemAudio)
{
    if (selectedMic && selectedSystemAudio) {
        return RecordProcess::RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO;
    } else if (selectedMic) {
        return RecordProcess::RECORD_AUDIO_INPUT_MIC;
    } else if (selectedSystemAudio) {
        return RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO;
    }
    return 0;

}

void MainWindow::reloadImage(QString effect)
{
    //*save tmp image file
    shotImgWidthEffect();
    //using namespace utils;
    const int radius = 10;
    QPixmap tmpImg = m_resultPixmap;
    int imgWidth = tmpImg.width();
    int imgHeight = tmpImg.height();

    TempFile *tempFile = TempFile::instance();

    if (effect == "blur") {
        if (!tmpImg.isNull()) {
            tmpImg = tmpImg.scaled(imgWidth / radius, imgHeight / radius,
                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            tmpImg = tmpImg.scaled(imgWidth, imgHeight, Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);
            tempFile->setBlurPixmap(tmpImg);
        }
    } else {
        if (!tmpImg.isNull()) {
            tmpImg = tmpImg.scaled(imgWidth / radius, imgHeight / radius,
                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            tmpImg = tmpImg.scaled(imgWidth, imgHeight);
            tempFile->setMosaicPixmap(tmpImg);
        }
    }
}

void MainWindow::shotImgWidthEffect()
{
    if (recordWidth == 0 || recordHeight == 0)
        return;


//    QEventLoop eventloop;
//    QTimer::singleShot(100, &eventloop, SLOT(quit()));
//    eventloop.exec();

    qDebug() << m_toolBar->isVisible() << m_sizeTips->isVisible();
    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
//    const QRect rect(m_shapesWidget->geometry().topLeft() * ratio, m_shapesWidget->geometry().size() * ratio);

    QRect target( static_cast<int>(m_shapesWidget->geometry().x() * ratio),
                  static_cast<int>(m_shapesWidget->geometry().y() * ratio),
                  static_cast<int>(m_shapesWidget->geometry().width() * ratio),
                  static_cast<int>(m_shapesWidget->geometry().height() * ratio) );

    m_resultPixmap = m_backgroundPixmap.copy(target);
    m_drawNothing = false;
    update();
}

void MainWindow::changeArrowAndLineEvent(int line)
{
    qDebug() << "line :" << line;
    m_toolBar->changeArrowAndLineFromMain(line);
}
