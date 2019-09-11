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

#include <dscreenwindowsutil.h>
#include <QApplication>
#include <QTimer>
#include <QKeyEvent>
#include <QObject>
#include <QPainter>
#include <DWidget>
#include <DWindowManagerHelper>
#include <DForeignWindow>
#include <QDebug>
#include <QVBoxLayout>
#include <QProcess>
#include <DHiDPIHelper>
#include <QMouseEvent>

#include "main_window.h"
#include "utils.h"
#include "record_button.h"
#include "record_option_panel.h"
#include "countdown_tooltip.h"
#include "constant.h"
#include "utils/tempfile.h"
#include "utils/configsettings.h"


const int MainWindow::CURSOR_BOUND = 5;
const int MainWindow::RECORD_MIN_SIZE = 200;
const int MainWindow::DRAG_POINT_RADIUS = 8;

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
const int RECORD_MIN_SIZE = 10;
const int SPACING = 10;
const int TOOLBAR_X_SPACING = 70;
const int TOOLBAR_Y_SPACING = 8;
const int SIDEBAR_X_SPACING = 8;
const int SIDEBAR_Y_SPACING = 1;
const int CURSOR_WIDTH = 8;
const int CURSOR_HEIGHT = 18;
const int INDICATOR_WIDTH =  110;
const qreal RESIZEPOINT_WIDTH = 15;
}

DWM_USE_NAMESPACE

MainWindow::MainWindow(DWidget *parent) :
    DWidget(parent),
    m_wmHelper(DWindowManagerHelper::instance())
{
    initAttributes();

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &MainWindow::compositeChanged);
}

void MainWindow::initAttributes()
{
    // Init attributes.
    setWindowTitle(tr("Deepin screen recorder"));
//    m_functionType = 0;
    m_keyBoardStatus = 0;
    m_mouseStatus = 0;
    m_multiKeyButtonsInOnSec = false;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    m_gifMode = true;
    m_mp4Mode = false;
    m_keyBoardTimer = new QTimer(this);
    m_frameRate = RecordProcess::RECORD_FRAMERATE_24;
    m_keyButtonList.clear();
    m_tempkeyButtonList.clear();
    m_screenWidth = QApplication::desktop()->screen()->width();
    m_screenHeight = QApplication::desktop()->screen()->height();

    // Add Qt::WindowDoesNotAcceptFocus make window not accept focus forcely, avoid conflict with dde hot-corner.
//    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus | Qt::X11BypassWindowManagerHint);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
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

    // Get all windows geometry.
    // Below code must execute before `window.showFullscreen,
    // otherwise deepin-screen-recorder window will add in window lists.
    QPoint pos = this->cursor().pos();
    const qreal ratio = devicePixelRatioF();
    DScreenWindowsUtil *screenWin = DScreenWindowsUtil::instance(pos);
    screenRect = screenWin->backgroundRect();
    screenRect = QRect(screenRect.topLeft() / ratio, screenRect.size());
    this->move(static_cast<int>(screenRect.x() * ratio),
               static_cast<int>(screenRect.y() * ratio));
    this->setFixedSize(screenRect.width(), screenRect.height());

    windowManager = new DWindowManager();
    windowManager->setRootWindowRect(screenRect);
    QList<xcb_window_t> windows = windowManager->getWindows();
    rootWindowRect = windowManager->getRootWindowRect();

    for (auto wid : DWindowManagerHelper::instance()->currentWorkspaceWindowIdList()) {
        if (wid == winId()) continue;

        DForeignWindow *window = DForeignWindow::fromWinId(wid);
        if (window) {
            window->deleteLater();
            windowRects << Dtk::Wm::WindowRect { window->x(), window->y(), window->width(), window->height() };
            windowNames << window->wmClass();
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

    pa = m_recordButton->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(229, 70, 61, 204));
    pa.setColor(DPalette::Light, QColor(229, 70, 61, 204));
    m_recordButton->setPalette(pa);
    m_recordButton->setIconSize(QSize(30, 30));
    m_recordButton->setIcon(QIcon(":/image/newUI/checked/screencap-checked.svg"));

    m_recordButton->setFixedSize(60, 47);
//    m_recordButton->setText(tr("Record"));
    m_recordButton->setObjectName("mainRecordBtn");

    m_shotButton = new DPushButton(this);

    pa = m_shotButton->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(0, 129, 255, 204));
    pa.setColor(DPalette::Light, QColor(0, 129, 255, 204));
    m_shotButton->setPalette(pa);
    m_shotButton->setIconSize(QSize(30, 30));
    m_shotButton->setIcon(QIcon(":/image/newUI/checked/screenshot-checked.svg"));

    m_shotButton->setFixedSize(60, 47);
//    m_shotButton->setText(tr("Shot"));
    m_shotButton->setObjectName("mainShotBtn");

    m_recordButton->hide();
    m_shotButton->hide();

    QPoint curPos = this->cursor().pos();
    m_swUtil = DScreenWindowsUtil::instance(curPos);
    m_backgroundRect = m_swUtil->backgroundRect();
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
    if (QSysInfo::currentCpuArchitecture().startsWith("x86")) {
        recordButtonLayout->addWidget(recordOptionPanel, 0, Qt::AlignCenter);
    }
    recordButtonLayout->addStretch();

    recordButton->hide();
    recordOptionPanel->hide();

    m_selectedMic = true;
    m_selectedSystemAudio = false;

    m_swUtil = DScreenWindowsUtil::instance(curPos);
    m_backgroundRect = m_swUtil->backgroundRect();
    m_backgroundRect = QRect(m_backgroundRect.topLeft() / ratio, m_backgroundRect.size());

    move(m_backgroundRect.topLeft() * ratio);
    this->setFixedSize(m_backgroundRect.size());
    initBackground();

    // Just use for debug.
    // repaintCounter = 0;
    m_cameraWidget = new CameraWidget(this);
    hideCameraWidget();

    m_menuController = new MenuController(this);

    connect(m_menuController, &MenuController::saveAction,
            this, &MainWindow::saveScreenShot);
    connect(m_menuController, &MenuController::closeAction,
            this, &MainWindow::exitApp);
//    connect(m_menuController, &MenuController::unDoAction,
//            this, &ShapesWidget::undoDrawShapes);

    m_functionType = 1;
    initScreenShot();

}

void MainWindow::initResource()
{
    m_showButtons = new ShowButtons(this);
    connect(m_showButtons, SIGNAL(keyShowSignal(const QString &)),
            this, SLOT(showKeyBoardButtons(const QString &)));
    resizeHandleBigImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("resize_handle_big.svg"));
    resizeHandleSmallImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("resize_handle_small.svg"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon1.svg"))));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    setDragCursor();

    buttonFeedback = new ButtonFeedback();

    connect(&eventMonitor, SIGNAL(buttonedPress(int, int)), this, SLOT(showPressFeedback(int, int)), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(buttonedDrag(int, int)), this, SLOT(showDragFeedback(int, int)), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(buttonedRelease(int, int)), this, SLOT(showReleaseFeedback(int, int)), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(pressEsc()), this, SLOT(responseEsc()), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(pressKeyButton(unsigned char)), m_showButtons,
            SLOT(showContentButtons(unsigned char)), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(releaseKeyButton(unsigned char)), m_showButtons,
            SLOT(releaseContentButtons(unsigned char)), Qt::QueuedConnection);
    eventMonitor.start();

//    startTooltip->show();
    //    startTooltip->windowHandle()->raise();
}

void MainWindow::initScreenShot()
{

//    installEventFilter(this);

//    connect(this, &MainWindow::releaseEvent, this, [ = ] {
//        qDebug() << "release event !!!";
//        m_keyboardReleased = true;
//        m_keyboardGrabbed =  windowHandle()->setKeyboardGrabEnabled(false);
//        qDebug() << "keyboardGrabbed:" << m_keyboardGrabbed;
//        removeEventFilter(this);
//    });

//    connect(this, &MainWindow::hideScreenshotUI, this, &MainWindow::hide);

    m_functionType = 1;
    m_keyBoardStatus = 0;
    m_mouseStatus = 0;
    m_multiKeyButtonsInOnSec = false;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    m_gifMode = true;
    m_mp4Mode = false;
    m_keyBoardTimer = new QTimer(this);
    m_frameRate = RecordProcess::RECORD_FRAMERATE_24;
    m_screenWidth = QApplication::desktop()->screen()->width();
    m_screenHeight = QApplication::desktop()->screen()->height();

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

    //隐藏键盘按钮控件
    if (m_keyButtonList.count() > 0) {
        for (int t_index = 0; t_index < m_keyButtonList.count(); t_index++) {
            m_keyButtonList.at(t_index)->hide();
        }
    }
    //构建截屏工具栏按钮 by zyg
    m_toolBar->hide();
    m_sideBar->hide();

    m_recordButton->hide();
    m_shotButton->hide();

    recordButton->hide();
    recordOptionPanel->hide();



    m_selectedMic = true;
    m_selectedSystemAudio = false;

    setDragCursor();
    eventMonitor.quit();

    connect(this, &MainWindow::hideScreenshotUI, this, &MainWindow::hide);


}

void MainWindow::initScreenRecorder()
{
    m_functionType = 0;
    m_keyBoardStatus = 0;
    m_mouseStatus = 0;
    m_multiKeyButtonsInOnSec = false;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    m_gifMode = true;
    m_mp4Mode = false;
    m_keyBoardTimer = new QTimer(this);
    m_frameRate = RecordProcess::RECORD_FRAMERATE_24;

    m_screenWidth = QApplication::desktop()->screen()->width();
    m_screenHeight = QApplication::desktop()->screen()->height();

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

    if (m_isShapesWidgetExist) {
        m_shapesWidget->hide();
    }

    m_isShapesWidgetExist = false;
    m_needDrawSelectedPoint = false;


    //构建截屏工具栏按钮 by zyg
    m_toolBar->hide();
    m_sideBar->hide();

    m_recordButton->hide();
    m_shotButton->hide();

    recordButton->hide();
    recordOptionPanel->hide();

    m_sizeTips->hide();
    m_zoomIndicator->hide();

    m_selectedMic = true;
    m_selectedSystemAudio = false;

    setDragCursor();
    eventMonitor.start();
}

void MainWindow::initBackground()
{
    m_backgroundPixmap = getPixmapofRect(m_backgroundRect);
    m_resultPixmap = m_backgroundPixmap;
    TempFile::instance()->setFullScreenPixmap(m_backgroundPixmap);
}

QPixmap MainWindow::getPixmapofRect(const QRect &rect)
{
    QRect r(rect.topLeft() * devicePixelRatioF(), rect.size());

    QList<QScreen *> screenList = qApp->screens();
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        if ((*it)->geometry().contains(r)) {
            return (*it)->grabWindow(m_swUtil->rootWindowId(), rect.x(), rect.y(), rect.width(), rect.height());
        }
    }

    return QPixmap();
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
    if (m_functionType == 0) {
        if (recordButtonStatus != RECORD_BUTTON_RECORDING) {
            QApplication::quit();
        }
    }

}

void MainWindow::compositeChanged()
{
    if (!m_wmHelper->hasComposite()) {
        Utils::warnNoComposite();
        QApplication::quit();
    }
}

void MainWindow::updateToolBarPos()
{
    if (m_toolBarInit == false) {
        m_toolBar->initToolBar();
        m_toolBarInit = true;
    }

    QPoint toolbarPoint;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    toolbarPoint = QPoint(recordX + recordWidth - m_toolBar->width() - TOOLBAR_X_SPACING,
                          std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));

    if (toolbarPoint.x() <= 0) {
        m_repaintMainButton = true;
        toolbarPoint.setX(recordX);
    }
    if (toolbarPoint.y() >= m_backgroundRect.y() + m_backgroundRect.height()
            - m_toolBar->height() - 28) {
        m_repaintSideBar = true;
        if (recordY > 28 * 2 + 10) {
            toolbarPoint.setY(recordY - m_toolBar->height() - TOOLBAR_Y_SPACING);
        } else {
            toolbarPoint.setY(recordY + TOOLBAR_Y_SPACING);
        }
    }
    m_toolBar->showAt(toolbarPoint);
}

void MainWindow::updateSideBarPos()
{
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
            if (sidebarPoint.y() <= 0) {
                sidebarPoint.setX(recordX + recordWidth - m_sideBar->width() - SIDEBAR_X_SPACING);
                sidebarPoint.setY(recordY + recordHeight + m_toolBar->height() + TOOLBAR_Y_SPACING * 2);
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

    m_sideBar->showAt(sidebarPoint);
}

void MainWindow::updateRecordButtonPos()
{
    QPoint recordButtonBarPoint;
    recordButtonBarPoint = QPoint(recordX + recordWidth - m_shotButton->width(),
                                  std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));

    if (m_repaintMainButton == true) {
        recordButtonBarPoint.setX(recordX + m_toolBar->width() + TOOLBAR_X_SPACING - m_recordButton->width());
    }

    if (recordButtonBarPoint.y() >= m_backgroundRect.y() + m_backgroundRect.height()
            - m_shotButton->height() - 28) {
        if (recordY > 28 * 2 + 10) {
            recordButtonBarPoint.setY(recordY - m_shotButton->height() - TOOLBAR_Y_SPACING);
        } else {
            recordButtonBarPoint.setY(recordY + TOOLBAR_Y_SPACING);
        }
    }

    if (m_functionType == 0) {
        if (!m_recordButton->isVisible()) {
            m_recordButton->show();
        }
    }

    m_recordButton->move(recordButtonBarPoint.x(), recordButtonBarPoint.y());
}

void MainWindow::updateShotButtonPos()
{
    QPoint shotButtonBarPoint;
    shotButtonBarPoint = QPoint(recordX + recordWidth - m_shotButton->width(),
                                std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));

    if (m_repaintMainButton == true) {
        shotButtonBarPoint.setX(recordX + m_toolBar->width() + TOOLBAR_X_SPACING - m_shotButton->width());
    }

    if (shotButtonBarPoint.y() >= m_backgroundRect.y() + m_backgroundRect.height()
            - m_shotButton->height() - 28) {
        if (recordY > 28 * 2 + 10) {
            shotButtonBarPoint.setY(recordY - m_shotButton->height() - TOOLBAR_Y_SPACING);
        } else {
            shotButtonBarPoint.setY(recordY + TOOLBAR_Y_SPACING);
        }
    }

    if (m_functionType == 1) {
        if (!m_shotButton->isVisible()) {
            m_shotButton->show();
        }
    }
    m_shotButton->move(shotButtonBarPoint.x(), shotButtonBarPoint.y());
}
void MainWindow::updateCameraWidgetPos()
{
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
        int x = recordX + recordWidth - cameraWidgetWidth;
        int y = recordY + recordHeight - cameraWidgetHeight;
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
        m_recordButton->hide();
//        updateShotButtonPos();
        m_shotButton->show();
        m_functionType = 1;
        initScreenShot();


    }

    update();
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
    m_selectedCamera = checked;
    if (checked) {
        int cameraWidgetWidth = recordWidth * 2 / 5;
        if (cameraWidgetWidth > CAMERA_WIDGET_MAX_WIDTH)
            cameraWidgetWidth = CAMERA_WIDGET_MAX_WIDTH;

        int cameraWidgetHeight = recordHeight * 1 / 4;
        if (cameraWidgetHeight > CAMERA_WIDGET_MAX_HEIGHT)
            cameraWidgetHeight = CAMERA_WIDGET_MAX_HEIGHT;
        int tempHeight = cameraWidgetWidth * 3 / 4;
        int tempWidth = cameraWidgetHeight * 4 / 3;
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
        m_cameraWidget->cameraStart();
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
            t_keyPoint1 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() / 1.5,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 + m_keyButtonList.at(1)->width() / 1.5,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();
            break;
        //三个按键的情况
        case 3:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 1.3,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2 + m_keyButtonList.at(2)->width() * 1.3,
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
            t_keyPoint2 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 - m_keyButtonList.at(1)->width() / 1.5,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2 + m_keyButtonList.at(2)->width() / 1.5,
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
            t_keyPoint1 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 2.6,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 - m_keyButtonList.at(1)->width() * 1.3,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();

            m_keyButtonList.at(3)->hide();
            t_keyPoint4 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(3)->width() * 1.3,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(3)->move(t_keyPoint4.x(), t_keyPoint4.y());
            m_keyButtonList.at(3)->show();

            m_keyButtonList.at(4)->hide();
            t_keyPoint5 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(4)->width() * 2.6,
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
    emit releaseEvent();
    emit saveActionTriggered();

//    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Screenshot);
//    if (m_hotZoneInterface->isValid())
//        m_hotZoneInterface->asyncCall("EnableZoneDetected",  true);
//    m_needSaveScreenshot = true;

    m_toolBar->setVisible(false);
    m_sizeTips->setVisible(false);
    m_sideBar->setVisible(false);

    shotCurrentImg();

    const bool r = saveAction(m_resultPixmap);
    sendNotify(m_saveIndex, m_saveFileName, r);
}

void MainWindow::sendNotify(SaveAction saveAction, QString saveFilePath, const bool succeed)
{
    // failed notify
    if (!succeed) {
        const auto tips = tr("Save failed. Please save it in your home directory.");
        m_notifyDBInterface->Notify("Deepin Screenshot", 0, "deepin-screen-recorder", QString(), tips, QStringList(), QVariantMap(), 0);

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

    if (remote_dde_notify_obj_exist) {
        actions << "_open" << tr("View");

        QString fileDir  = QUrl::fromLocalFile(QFileInfo(saveFilePath).absoluteDir().absolutePath()).toString();
        QString filePath = QUrl::fromLocalFile(saveFilePath).toString();
        QString command;
        if (QFile("/usr/bin/dde-file-manager").exists()) {
            command = QString("/usr/bin/dde-file-manager,%1?selectUrl=%2").arg(fileDir).arg(filePath);
        } else {
            command = QString("xdg-open,%1").arg(filePath);
        }

        hints["x-deepin-action-_open"] = command;
    }

    qDebug() << "saveFilePath:" << saveFilePath;

    QString summary;
    if (saveAction == SaveAction::SaveToClipboard) {
        summary = QString(tr("Picture has been saved to clipboard"));
    } else {
        summary = QString(tr("Picture has been saved to %1")).arg(saveFilePath);
    }

    if (saveAction == SaveAction::SaveToClipboard && !m_noNotify) {

        QVariantMap emptyMap;
        m_notifyDBInterface->Notify("Deepin Screenshot", 0,  "deepin-screen-recorder", "",
                                    summary,  QStringList(), emptyMap, 0);
    }  else if ( !m_noNotify &&  !(m_saveIndex == SaveAction::SaveToSpecificDir && m_saveFileName.isEmpty())) {

//        m_notifyDBInterface->Notify("Deepin Screenshot", 0,  "deepin-screen-recorder", "",
//                                    summary, actions, hints, 0);
        QList<QVariant> arg;
        arg << (QCoreApplication::applicationName())                 // appname
            << ((unsigned int) 0)                                    // id
            << QString("Deepin Screenshot")                     // icon
            << tr("Shotting finished")                              // summary
            << QString("%1 %2").arg(tr("Saved to")).arg(saveFilePath) // body
            << actions                                               // actions
            << hints                                                 // hints
            << (int) -1;
        notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);// timeout
    }

    QTimer::singleShot(2, [ = ] {
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
    bool copyToClipboard = false;

//    std::pair<bool, SaveAction> temporarySaveAction = ConfigSettings::instance()->getTemporarySaveAction();
//    if (temporarySaveAction.first) {
//        m_saveIndex = temporarySaveAction.second;
//    } else {
//        m_saveIndex = ConfigSettings::instance()->value("save", "save_op").value<SaveAction>();
//    }
    //for test
    m_saveIndex = SaveToDesktop;
    switch (m_saveIndex) {
    case SaveToDesktop: {
        saveOption = QStandardPaths::DesktopLocation;
//        ConfigSettings::instance()->setValue("common", "default_savepath", QStandardPaths::writableLocation(
//                                                 QStandardPaths::DesktopLocation));
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
//    case SaveToSpecificDir: {
//        this->hide();
//        this->releaseKeyboard();

//        QString path = ConfigSettings::instance()->value("common", "default_savepath").toString();
//        QString fileName = m_selectAreaName;

//        if (path.isEmpty() || !QDir(path).exists()) {
//            path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
//        }

//        if (fileName.isEmpty()) {
//            fileName = QString("%1_%2").arg(tr("DeepinScreenshot")).arg(currentTime);
//        } else {
//            fileName = QString("%1_%2_%3").arg(tr("DeepinScreenshot")).arg(m_selectAreaName).arg(currentTime);
//        }

//        QString lastFileName = QString("%1/%2.png").arg(path).arg(fileName);

//        QFileDialog fileDialog;
//        m_saveFileName =  fileDialog.getSaveFileName(this, "Save",  lastFileName,
//                                                     tr("PNG (*.png);;JPEG (*.jpg *.jpeg);; BMP (*.bmp);; PGM (*.pgm);;"
//                                                        "XBM (*.xbm);;XPM(*.xpm)"));

//        if (m_saveFileName.isEmpty() || QFileInfo(m_saveFileName).isDir())
//            return false;

//        QString fileSuffix = QFileInfo(m_saveFileName).completeSuffix();
//        if (fileSuffix.isEmpty()) {
//            m_saveFileName = m_saveFileName + ".png";
//        } else if ( !isValidFormat(fileSuffix)) {
//            qWarning() << "The fileName has invalid suffix!" << fileSuffix << m_saveFileName;
//            return false;
//        }

//        ConfigSettings::instance()->setValue("common", "default_savepath",
//                                             QFileInfo(m_saveFileName).dir().absolutePath());
//        break;
//    }
//    case SaveToClipboard: {
//        copyToClipboard = true;
//        ConfigSettings::instance()->setValue("common", "default_savepath",   "clipboard");
//        break;
//    }
//    case SaveToAutoClipboard: {
//        copyToClipboard = true;
//        QString defaultSaveDir = ConfigSettings::instance()->value("common", "default_savepath").toString();
//        if (defaultSaveDir.isEmpty()) {
//            saveOption = QStandardPaths::DesktopLocation;
//        } else if (defaultSaveDir == "clipboard") {
//            m_saveIndex = SaveToSpecificDir;
//        } else  {
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
    default:
        break;
    }

//    int toolBarSaveQuality = std::min(ConfigSettings::instance()->value("save",
//                                                                        "save_quality").toInt(), 100);
    int toolBarSaveQuality = 100;
    if (toolBarSaveQuality != 100) {
        qreal saveQuality = qreal(toolBarSaveQuality) * 5 / 1000 + 0.5;

        int pixWidth = screenShotPix.width();
        int pixHeight = screenShotPix.height();
        screenShotPix = screenShotPix.scaled(pixWidth * saveQuality, pixHeight * saveQuality,
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
        QString savePath = QStandardPaths::writableLocation(saveOption);
        QDir saveDir(savePath);
        if (!saveDir.exists()) {
            bool mkdirSucc = saveDir.mkpath(".");
            if (!mkdirSucc) {
                qCritical() << "Save path not exist and cannot be created:" << savePath;
                qCritical() << "Fall back to temp location!";
                savePath = QDir::tempPath();
            }
        }
        if (selectAreaName.isEmpty()) {
            m_saveFileName = QString("%1/%2_%3.png").arg(savePath, tr("DeepinScreenshot"), currentTime);
        } else {
            m_saveFileName = QString("%1/%2_%3_%4.png").arg(savePath, tr("DeepinScreenshot"), selectAreaName, currentTime);
        }
        if (!screenShotPix.save(m_saveFileName,  "PNG"))
            return false;
    }

//    if (copyToClipboard) {
//        Q_ASSERT(!screenShotPix.isNull());
//        QClipboard *cb = qApp->clipboard();
//        cb->setPixmap(screenShotPix, QClipboard::Clipboard);
//    }

    return true;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    // Just use for debug.
    // repaintCounter++;
    // qDebug() << repaintCounter;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_functionType == 1) {
        painter.setRenderHint(QPainter::Antialiasing, true);
        QRect backgroundRect = QRect(0, 0, rootWindowRect.width, rootWindowRect.height);
        // FIXME: Under the magnifying glass, it seems to be magnified two times.
        m_backgroundPixmap.setDevicePixelRatio(devicePixelRatioF());
        painter.drawPixmap(backgroundRect, m_backgroundPixmap);
    }

    if (recordWidth > 0 && recordHeight > 0) {

        QRect backgroundRect = QRect(0, 0, rootWindowRect.width, rootWindowRect.height);
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
            QPen framePen(QColor("#01bdff"));
            framePen.setWidth(2);
            painter.setOpacity(1);
            painter.setBrush(QBrush());  // clear brush
            painter.setPen(framePen);
            painter.drawRect(QRect(
                                 std::max(frameRect.x(), 1),
                                 std::max(frameRect.y(), 1),
                                 std::min(frameRect.width() - 1, rootWindowRect.width - 2),
                                 std::min(frameRect.height() - 1, rootWindowRect.height - 2)));
            painter.setRenderHint(QPainter::Antialiasing, true);
        }

        // Draw drag pint.
        if (recordButtonStatus == RECORD_BUTTON_NORMAL && drawDragPoint) {
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleBigImg);

            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleSmallImg);
        }



//        if (m_functionType == 0) {
//            QRect backgroundRect = QRect(0, 0, rootWindowRect.width, rootWindowRect.height);
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
        if (m_functionType == 1) {
            if (keyEvent->key() == Qt::Key_Escape ) {
                if (m_isShapesWidgetExist) {
                    if (m_shapesWidget->textEditIsReadOnly()) {
                        return false;
                    }
                }
                qDebug() << "Key_Escape pressed: app quit!";
                qApp->quit();
            } else if (keyEvent->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (keyEvent->key() == Qt::Key_Question) {
//                    onViewShortcut();
                }
            } else if (qApp->keyboardModifiers() & Qt::ControlModifier) {
                if (keyEvent->key() == Qt::Key_Z) {
                    qDebug() << "SDGF: ctrl+z !!!";
                    emit unDo();
                }
            }

            bool needRepaint = false;
            if (m_isShapesWidgetExist) {
                if (keyEvent->key() == Qt::Key_Escape) {
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
                        ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToClipboard);
//                        saveScreenshot();
                    } else if (keyEvent->key() == Qt::Key_S) {
//                        expressSaveScreenshot();
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
                        recordX = std::max(0, recordX + 1);
                        recordWidth = std::max(std::min(recordWidth - 1,
                                                        m_backgroundRect.width()), RECORD_MIN_SIZE);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        recordWidth = std::max(std::min(recordWidth - 1,
                                                        m_backgroundRect.width()), RECORD_MIN_SIZE);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        recordY = std::max(0, recordY + 1);
                        recordHeight = std::max(std::min(recordHeight - 1,
                                                         m_backgroundRect.height()), RECORD_MIN_SIZE);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        recordHeight = std::max(std::min(recordHeight - 1,
                                                         m_backgroundRect.height()), RECORD_MIN_SIZE);

                        needRepaint = true;
                    }
                } else if (qApp->keyboardModifiers() & Qt::ControlModifier) {
                    if (keyEvent->key() == Qt::Key_S) {
//                        expressSaveScreenshot();
                    }

                    if (keyEvent->key() == Qt::Key_C) {
                        ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToClipboard);
//                        saveScreenshot();
                    }

                    if (keyEvent->key() == Qt::Key_Left) {
                        recordX = std::max(0, recordX - 1);
                        recordWidth = std::max(std::min(recordWidth + 1,
                                                        m_backgroundRect.width()), RECORD_MIN_SIZE);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        recordWidth = std::max(std::min(recordWidth + 1,
                                                        m_backgroundRect.width()), RECORD_MIN_SIZE);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        recordY = std::max(0, recordY - 1);
                        recordHeight = std::max(std::min(recordHeight + 1,
                                                         m_backgroundRect.height()), RECORD_MIN_SIZE);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        recordHeight = std::max(std::min(recordHeight + 1,
                                                         m_backgroundRect.height()), RECORD_MIN_SIZE);

                        needRepaint = true;
                    }
                } else {
                    if (keyEvent->key() == Qt::Key_Left) {
                        recordX = std::max(0, recordX - 1);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        recordX = std::min(m_backgroundRect.width() - recordWidth,
                                           recordX + 1);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        recordY = std::max(0, recordY - 1);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        recordY = std::min(m_backgroundRect.height() -
                                           recordHeight, recordY + 1);

                        needRepaint = true;
                    }
                }

                if ( !m_needSaveScreenshot) {
                    m_sizeTips->updateTips(QPoint(recordX, recordY),
                                           QString("%1X%2").arg(recordWidth).arg(recordHeight));
                    updateToolBarPos();
                    if (m_recordButton->isVisible()) {
                        updateRecordButtonPos();
                    }

                    if (m_sideBar->isVisible()) {
                        updateSideBarPos();
                    }

                    if (m_shotButton->isVisible()) {
                        updateShotButtonPos();
                    }
                }
            }

            if (needRepaint) {
                update();
            }
        }

//        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        else {
            if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                    if (keyEvent->key() == Qt::Key_Left) {
                        recordX = std::max(0, recordX - 1);
                        recordWidth = std::min(recordWidth + 1, rootWindowRect.width);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        recordWidth = std::min(recordWidth + 1, rootWindowRect.width);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        recordY = std::max(0, recordY - 1);
                        recordHeight = std::min(recordHeight + 1, rootWindowRect.height);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        recordHeight = std::min(recordHeight + 1, rootWindowRect.height);

                        needRepaint = true;
                    }
                } else {
                    if (keyEvent->key() == Qt::Key_Left) {
                        recordX = std::max(0, recordX - 1);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Right) {
                        recordX = std::min(rootWindowRect.width - recordWidth, recordX + 1);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Up) {
                        recordY = std::max(0, recordY - 1);

                        needRepaint = true;
                    } else if (keyEvent->key() == Qt::Key_Down) {
                        recordY = std::min(rootWindowRect.height - recordHeight, recordY + 1);

                        needRepaint = true;
                    }
                }

                m_sizeTips->updateTips(QPoint(recordX, recordY),
                                       QString("%1X%2").arg(recordWidth).arg(recordHeight));
                updateToolBarPos();
                if (m_recordButton->isVisible()) {
                    updateRecordButtonPos();
                }

                if (m_sideBar->isVisible()) {
                    updateSideBarPos();
                }

                if (m_shotButton->isVisible()) {
                    updateShotButtonPos();
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
            DWidget::keyReleaseEvent(keyEvent);
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
                }
            }
        }
        // NOTE: must be use 'isAutoRepeat' to filter KeyRelease event send by Qt.
    }

    if (event->type() == QEvent::MouseButtonPress) {
        if (!m_isShapesWidgetExist) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragStartX = mouseEvent->x();
                dragStartY = mouseEvent->y();
                if (!isFirstPressButton) {
                    isFirstPressButton = true;

                    Utils::clearBlur(windowManager, this->winId());
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
                if (!isFirstReleaseButton) {
                    isFirstReleaseButton = true;

                    updateCursor(event);
                    updateToolBarPos();
                    if (m_functionType == 1 && m_sideBar->isVisible()) {
                        updateSideBarPos();
                        m_zoomIndicator->hide();
                        if (m_sizeTips->isVisible()) {
                            m_sizeTips->updateTips(QPoint(recordX, recordY),
                                                   QString("%1X%2").arg(recordWidth).arg(recordHeight));
                        }
                    }
                    updateRecordButtonPos();
                    updateShotButtonPos();
                    // Record select area name with window name if just click (no drag).
                    if (!isFirstDrag) {
                        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                        for (auto it = windowRects.rbegin(); it != windowRects.rend(); ++it) {
                            if (QRect(it->x, it->y, it->width, it->height).contains(mouseEvent->pos() + screenRect.topLeft())) {
                                selectAreaName = windowNames[windowRects.rend() - it - 1];
                                break;
                            }
                        }
                    } else {
                        // Make sure record area not too small.
                        recordWidth = recordWidth < RECORD_MIN_SIZE ? RECORD_MIN_SIZE : recordWidth;
                        recordHeight = recordHeight < RECORD_MIN_SIZE ? RECORD_MIN_SIZE : recordHeight;

                        if (recordX + recordWidth > rootWindowRect.width) {
                            recordX = rootWindowRect.width - recordWidth;
                        }

                        if (recordY + recordHeight > rootWindowRect.height) {
                            recordY = rootWindowRect.height - recordHeight;
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

                isPressButton = false;
                isReleaseButton = true;

                needRepaint = true;
            }
        }


    } else if (event->type() == QEvent::MouseMove) {
        if (!m_isShapesWidgetExist) {
            m_sizeTips->updateTips(QPoint(recordX, recordY),
                                   QString("%1X%2").arg(recordWidth).arg(recordHeight));

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
                        QPoint curPos = this->cursor().pos();
                        QPoint tmpPos;
                        QPoint topLeft = m_backgroundRect.topLeft() * devicePixelRatioF();

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

//                        m_zoomIndicator->showMagnifier(QPoint(
//                                                           std::max(tmpPos.x() - topLeft.x(), 0),
//                                                           std::max(tmpPos.y() - topLeft.y(), 0)));
                    }

                }
            }

            if (isPressButton && isFirstPressButton) {
                if (!isFirstDrag) {
                    isFirstDrag = true;

                    selectAreaName = tr("Select area");
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
                            recordX = std::max(std::min(dragRecordX + mouseEvent->x() - dragStartX, rootWindowRect.width - recordWidth), 1);
                            recordY = std::max(std::min(dragRecordY + mouseEvent->y() - dragStartY, rootWindowRect.height - recordHeight), 1);
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
                const qreal ratio = devicePixelRatioF();
                const QPoint mousePoint = QCursor::pos();
                for (auto it = windowRects.rbegin(); it != windowRects.rend(); ++it) {
                    if (QRect(it->x, it->y, it->width, it->height).contains(mousePoint)) {
                        recordX = it->x - static_cast<int>(screenRect.x() * ratio);
                        recordY = it->y - static_cast<int>(screenRect.y() * ratio);
                        recordWidth = it->width;
                        recordHeight = it->height;
                        needRepaint = true;
                        break;
                    }
                }
            }
        }

    }

    // Use flag instead call `repaint` directly,
    // to avoid repaint many times in one event function.
    if (needRepaint) {
        repaint();
    }

    return false;
}

void MainWindow::startRecord()
{
    Utils::clearBlur(windowManager, this->winId());
    recordButtonStatus = RECORD_BUTTON_RECORDING;

    resetCursor();

    repaint();

    trayIcon->show();

    flashTrayIconTimer = new QTimer(this);
    connect(flashTrayIconTimer, SIGNAL(timeout()), this, SLOT(flashTrayIcon()));
    flashTrayIconTimer->start(800);

    recordProcess.setRecordAudioInputType(getRecordInputType(m_selectedMic, m_selectedSystemAudio));
    recordProcess.startRecord();
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
    QTimer::singleShot(100, &eventloop1, SLOT(quit()));
    eventloop1.exec();

    qDebug() << "shotCurrentImg shotFullScreen";
//    using namespace utils;
    shotFullScreen();
    if (m_isShapesWidgetExist) {
        m_shapesWidget->hide();
    }

    this->hide();
    emit hideScreenshotUI();

    const qreal ratio = this->devicePixelRatioF();
    QRect target( recordX * ratio,
                  recordY * ratio,
                  recordWidth * ratio,
                  recordHeight * ratio );

    m_resultPixmap = m_resultPixmap.copy(target);
}

void MainWindow::shotFullScreen()
{
    m_resultPixmap = getPixmapofRect(m_backgroundRect);
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
    int offsetY = mouseEvent->y() - dragStartY;
    recordY = std::max(std::min(dragRecordY + offsetY, dragRecordY + dragRecordHeight - RECORD_MIN_SIZE), 1);
    recordHeight = std::max(std::min(dragRecordHeight - offsetY, rootWindowRect.height), RECORD_MIN_SIZE);
}

void MainWindow::resizeBottom(QMouseEvent *mouseEvent)
{
    int offsetY = mouseEvent->y() - dragStartY;
    recordHeight = std::max(std::min(dragRecordHeight + offsetY, rootWindowRect.height), RECORD_MIN_SIZE);
}

void MainWindow::resizeLeft(QMouseEvent *mouseEvent)
{
    int offsetX = mouseEvent->x() - dragStartX;
    recordX = std::max(std::min(dragRecordX + offsetX, dragRecordX + dragRecordWidth - RECORD_MIN_SIZE), 1);
    recordWidth = std::max(std::min(dragRecordWidth - offsetX, rootWindowRect.width), RECORD_MIN_SIZE);
}

void MainWindow::resizeRight(QMouseEvent *mouseEvent)
{
    int offsetX = mouseEvent->x() - dragStartX;
    recordWidth = std::max(std::min(dragRecordWidth + offsetX, rootWindowRect.width), RECORD_MIN_SIZE);
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

        } else if (recordButton->geometry().contains(cursorX, cursorY) || recordOptionPanel->geometry().contains(cursorX, cursorY)) {
            // Record area.
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        } else if (t_rectbuttonRect.contains(cursorX, cursorY) || m_shotButton->geometry().contains(cursorX, cursorY)) {
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
    QApplication::overrideCursor();
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason)
{
    stopRecord();
}

void MainWindow::stopRecord()
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
        hide();
        recordProcess.stopRecord();
//        voiceRecordProcess.stopRecord();
    }
}

void MainWindow::startCountdown()
{
    recordButtonStatus = RECORD_BUTTON_WAIT;

    const qreal ratio = devicePixelRatioF();
    const QPoint topLeft = geometry().topLeft();

    QRect recordRect {
        static_cast<int>(recordX * ratio + topLeft.x()),
        static_cast<int>(recordY * ratio + topLeft.y()),
        static_cast<int>(recordWidth * ratio),
        static_cast<int>(recordHeight * ratio)
    };

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
    if (QSysInfo::currentCpuArchitecture().startsWith("x86")) {
        if (t_saveGif) {
            qDebug() << "record format is gif";
            recordProcess.setRecordType(RecordProcess::RECORD_TYPE_GIF);
        } else if (t_saveGif == false) {
            qDebug() << "record format is mp4";
            recordProcess.setRecordType(RecordProcess::RECORD_TYPE_VIDEO);
        }
    } else {
        recordProcess.setRecordType(RecordProcess::RECORD_TYPE_GIF);
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


    Utils::passInputEvent(this->winId());

    repaint();
}

void MainWindow::showRecordButton()
{
//    updateToolBarPos();
//    recordButton->show();
    if (QSysInfo::currentCpuArchitecture().startsWith("x86")) {
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

    Utils::clearBlur(windowManager, this->winId());
}

void MainWindow::hideAllWidget()
{
    m_toolBar->hide();
    m_sideBar->hide();
    m_recordButton->hide();
    m_shotButton->hide();

    //隐藏键盘按钮控件
    if (m_keyButtonList.count() > 0) {
        for (int t_index = 0; t_index < m_keyButtonList.count(); t_index++) {
            m_keyButtonList.at(t_index)->hide();
        }
    }

    Utils::clearBlur(windowManager, this->winId());
}
void MainWindow::hideCameraWidget()
{
    m_cameraWidget->hide();
}
void MainWindow::adjustLayout(QVBoxLayout *layout, int layoutWidth, int layoutHeight)
{
    if (recordHeight < layoutHeight) {
        if (recordY + layoutHeight > rootWindowRect.height) {
            layout->setContentsMargins(
                recordX,
                recordY - layoutHeight - Constant::RECTANGLE_PADDING,
                rootWindowRect.width - recordX - recordWidth,
                rootWindowRect.height - recordY + Constant::RECTANGLE_PADDING);
        } else {
            layout->setContentsMargins(
                recordX,
                recordY + recordHeight + Constant::RECTANGLE_PADDING,
                rootWindowRect.width - recordX - recordWidth,
                rootWindowRect.height - (recordY + recordHeight + layoutHeight) + Constant::RECTANGLE_PADDING);
        }
    } else if (recordWidth < layoutWidth) {
        if (recordX + layoutWidth > rootWindowRect.width) {
            layout->setContentsMargins(
                recordX - layoutWidth - Constant::RECTANGLE_PADDING,
                recordY,
                rootWindowRect.width - recordX + Constant::RECTANGLE_PADDING,
                rootWindowRect.height - recordY - recordHeight);
        } else {
            layout->setContentsMargins(
                recordX + recordWidth + Constant::RECTANGLE_PADDING,
                recordY,
                rootWindowRect.width - (recordX + recordWidth + layoutWidth + Constant::RECTANGLE_PADDING),
                rootWindowRect.height - recordY - recordHeight);
        }
    } else {
        layout->setContentsMargins(
            recordX,
            recordY,
            rootWindowRect.width - recordX - recordWidth,
            rootWindowRect.height - recordY - recordHeight);
    }
}

void MainWindow::initShapeWidget(QString type)
{
    qDebug() << "show shapesWidget";
    m_shapesWidget = new ShapesWidget(this);
    m_shapesWidget->setShiftKeyPressed(m_isShiftPressed);

    if (type != "color")
        m_shapesWidget->setCurrentShape(type);

    m_shapesWidget->show();
    if (m_isSideBarInside == false) {
        m_shapesWidget->setFixedSize(recordWidth - 4, recordHeight - 4);
    }

    else {
        m_shapesWidget->setFixedSize(recordWidth - m_sideBar->width(), recordHeight - 4);
    }

    m_shapesWidget->move(recordX + 2, recordY + 2);

    updateToolBarPos();
//    m_toolBar->raise();
    m_needDrawSelectedPoint = false;
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
//    connect(m_shapesWidget, &ShapesWidget::shapePressed,
//            m_toolBar, &ToolBar::shapePressed);
//    connect(m_shapesWidget, &ShapesWidget::saveBtnPressed,
//            m_toolBar, &ToolBar::saveBtnPressed);
//    connect(m_shapesWidget, &ShapesWidget::requestExit, this, &MainWindow::exitApp);
    connect(this, &MainWindow::unDo, m_shapesWidget, &ShapesWidget::undoDrawShapes);
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
    qApp->quit();
}

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

    update();

//    QEventLoop eventloop;
//    QTimer::singleShot(100, &eventloop, SLOT(quit()));
//    eventloop.exec();

    qDebug() << m_toolBar->isVisible() << m_sizeTips->isVisible();
    const qreal ratio = devicePixelRatioF();
    const QRect rect(m_shapesWidget->geometry().topLeft() * ratio, m_shapesWidget->geometry().size() * ratio);
    m_resultPixmap = m_backgroundPixmap.copy(rect);
    m_drawNothing = false;
    update();
}

void MainWindow::changeArrowAndLineEvent(int line)
{
    qDebug() << "line :" << line;
    m_toolBar->changeArrowAndLineFromMain(line);
}
