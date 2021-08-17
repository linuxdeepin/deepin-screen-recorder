/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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
#include "accessibility/acTextDefine.h"
#include "keydefine.h"

#include <DWidget>
#include <DWindowManagerHelper>
#include <DForeignWindow>
#include <DLineEdit>
#include <DInputDialog>
#include <DDesktopServices>
#include <DDialog>
#include <DHiDPIHelper>

#include <QBitmap>
#include <QApplication>
#include <QTimer>
#include <QKeyEvent>
#include <QObject>
#include <QPainter>
#include <QDebug>
#include <QVBoxLayout>
#include <QProcess>
#include <QMouseEvent>
#include <QClipboard>
#include <QFileDialog>
#include <QShortcut>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QGestureEvent>

#include <X11/Xcursor/Xcursor.h>

//const int MainWindow::CURSOR_BOUND = 5;
const int MainWindow::RECORD_MIN_SIZE = 580;
const int MainWindow::RECORD_MIN_HEIGHT = 280;
const int MainWindow::RECORD_MIN_SHOT_SIZE = 10;
const int MainWindow::DRAG_POINT_RADIUS = 7;

const int MainWindow::RECORD_BUTTON_NORMAL = 0;
const int MainWindow::RECORD_BUTTON_WAIT = 1;
const int MainWindow::RECORD_BUTTON_RECORDING = 2;
const int MainWindow::RECORD_BUTTON_SAVEING = 3;

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
const int TOOLBAR_X_SPACING = 85;
const int TOOLBAR_Y_SPACING = 3;
const int SIDEBAR_X_SPACING = 8;
const int CURSOR_WIDTH = 8;
const int CURSOR_HEIGHT = 18;
const int INDICATOR_WIDTH =  110;
}

//DWM_USE_NAMESPACE
MainWindow::MainWindow(DWidget *parent) :
    DWidget(parent),
    m_wmHelper(DWindowManagerHelper::instance()),
    m_hasComposite(DWindowManagerHelper::instance()->hasComposite()),
    m_initScreenShot(false),
    m_initScreenRecorder(false)
{
    if (Utils::isTabletEnvironment) {
        m_cursorBound = 20;
    } else {
        m_cursorBound = 5;
    }
    setDragCursor();
    //QScreen *screen = qApp->primaryScreen();
    m_pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    // 监控录屏过程中， 特效窗口的变化。
    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &MainWindow::compositeChanged);

    connect(qApp, &QGuiApplication::screenAdded, this, &MainWindow::onExit);
    connect(qApp, &QGuiApplication::screenRemoved, this, &MainWindow::onExit);
    m_pScreenShotEvent =  new ScreenShotEvent();
    m_pScreenRecordEvent = new EventMonitor();

    m_screenCount = QApplication::desktop()->screenCount();
    QList<QScreen *> screenList = qApp->screens();
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        QRect rect = (*it)->geometry();
        qDebug() << (*it)->name() << rect;
        ScreenInfo screenInfo;
        screenInfo.x = rect.x();
        screenInfo.y = rect.y();
        screenInfo.height =  static_cast<int>(rect.height() * m_pixelRatio);
        screenInfo.width = static_cast<int>(rect.width() * m_pixelRatio);
        screenInfo.name = (*it)->name();
        m_screenInfo.append(screenInfo);
    }

    m_screenSize.setWidth(m_screenInfo[0].x + m_screenInfo[0].width);
    m_screenSize.setHeight(m_screenInfo[0].y + m_screenInfo[0].height);

    // 通过每个屏幕， 右下角的坐标来计算屏幕总大小。
    for (int i = 1; i < m_screenInfo.size(); ++i) {
        if ((m_screenInfo[i].height + m_screenInfo[i].y) > m_screenSize.height())
            m_screenSize.setHeight(m_screenInfo[i].height + m_screenInfo[i].y);

        if ((m_screenInfo[i].width + m_screenInfo[i].x) > m_screenSize.width())
            m_screenSize.setWidth(m_screenInfo[i].width + m_screenInfo[i].x);
    }

    qDebug() << m_screenSize;
    if (m_screenInfo.size() > 1) {
        // 排序
        qSort(m_screenInfo.begin(), m_screenInfo.end(), [ = ](const ScreenInfo info1, const ScreenInfo info2) {
            return info1.x < info2.x;
        });
    }


    m_screenCount = QApplication::desktop()->screenCount();
}

void MainWindow::initAttributes()
{
    qDebug() << "FunctionName: " << __func__;
    setWindowTitle(tr("Screen Capture"));
    m_keyButtonList.clear();
    checkCpuIsZhaoxin();

    m_screenHeight = QApplication::desktop()->screen()->height();
    QRect t_screenRect;

    //多屏情况下累加宽度
    if (m_screenCount == 1) {
        m_screenWidth = QApplication::desktop()->screen()->width();

        t_screenRect.setX(0);
        t_screenRect.setY(0);
        t_screenRect.setWidth(m_screenWidth);
        t_screenRect.setHeight(m_screenHeight);
    } else if (m_screenCount > 1) {
        QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
        t_screenRect = QRect(0, 0, static_cast<int>(m_screenSize.width() / m_pixelRatio), static_cast<int>(m_screenSize.height() / m_pixelRatio));
        qDebug() << "screen size" << t_primaryScreen->virtualGeometry() << t_screenRect;
    }

    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);   // make MouseMove can response
    installEventFilter(this);  // add event filter
    createWinId();

    //多屏情况下累加窗口大小
    if (m_screenCount == 1) {
        //        DScreenWindowsUtil *screenWin = DScreenWindowsUtil::instance(curPos);

        //        screenRect = screenWin->backgroundRect();
        screenRect = QApplication::desktop()->screen()->geometry();
        screenRect = QRect(screenRect.topLeft() / m_pixelRatio, screenRect.size());
        this->move(static_cast<int>(screenRect.x() * m_pixelRatio),
                   static_cast<int>(screenRect.y() * m_pixelRatio));
        this->setFixedSize(screenRect.width(), screenRect.height());
        rootWindowRect = QApplication::desktop()->screen()->geometry();
    }

    else if (m_screenCount > 1) {
        //        QPoint pos = this->cursor().pos();
        //        DScreenWindowsUtil *screenWin = DScreenWindowsUtil::instance(curPos);
        screenRect = t_screenRect;

        screenRect = QRect(screenRect.topLeft() / m_pixelRatio, screenRect.size());
        this->move(static_cast<int>(screenRect.x() * m_pixelRatio),
                   static_cast<int>(screenRect.y() * m_pixelRatio));
        this->setFixedSize(t_screenRect.size());
        rootWindowRect = t_screenRect;
    }

    m_screenHeight = m_screenSize.height();
    m_screenWidth = m_screenSize.width();
    DForeignWindow *prewindow = nullptr;
    for (auto wid : DWindowManagerHelper::instance()->currentWorkspaceWindowIdList()) {
        if (wid == winId()) continue;
        if (prewindow) {
            delete prewindow;
            prewindow = nullptr;
        }

        DForeignWindow *window = DForeignWindow::fromWinId(wid);//sanitizer dtk

        prewindow = window;

        //判断窗口是否被最小化
        if (window->windowState() == Qt::WindowState::WindowMinimized) {
            continue;
        }

        if (window) {
            int t_tempWidth = 0;
            int t_tempHeight = 0;
            //window:后面代码有使用
            //window->deleteLater();
            //修改部分窗口显示不全，截图框识别问题
            //x坐标小于0时
            if (window->frameGeometry().x() < 0) {
                if (window->frameGeometry().y() < 0) {

                    //x,y为负坐标情况
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    //windowRects << Dtk::Wm::WindowRect {0, 0, t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= m_screenHeight - window->frameGeometry().height()) {
                    //x为负坐标，y在正常屏幕区间内
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height();

                    //windowRects << Dtk::Wm::WindowRect {0, window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > m_screenHeight - window->frameGeometry().height()) {
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
                } else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= m_screenHeight - window->frameGeometry().height()) {
                    //y在正常屏幕区间内
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = window->frameGeometry().height();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > m_screenHeight - window->frameGeometry().height()) {
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
                } else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= m_screenHeight - window->frameGeometry().height()) {
                    //y在正常屏幕区间内
                    t_tempWidth = m_screenWidth - window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > m_screenHeight - window->frameGeometry().height()) {
                    //y方向窗口超出屏幕底部
                    t_tempWidth = m_screenWidth - window->frameGeometry().x();
                    t_tempHeight = m_screenHeight - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }
        }
    }
    if (prewindow) {
        delete prewindow;
        prewindow = nullptr;
    }
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

    connect(m_toolBar, &ToolBar::currentFunctionToMain, this, &MainWindow::changeFunctionButton);
    connect(m_toolBar, &ToolBar::keyBoardCheckedToMain, this, &MainWindow::changeKeyBoardShowEvent);
    connect(m_toolBar, &ToolBar::mouseCheckedToMain, this, &MainWindow::changeMouseShowEvent);
    connect(m_toolBar, &ToolBar::mouseShowCheckedToMain, this, &MainWindow::changeShowMouseShowEvent);
    connect(m_toolBar, &ToolBar::microphoneActionCheckedToMain, this, &MainWindow::changeMicrophoneSelectEvent);
    connect(m_toolBar, &ToolBar::systemAudioActionCheckedToMain, this, &MainWindow::changeSystemAudioSelectEvent);
    connect(m_toolBar, &ToolBar::cameraActionCheckedToMain, this, &MainWindow::changeCameraSelectEvent);
    connect(m_toolBar, &ToolBar::shotToolChangedToMain, this, &MainWindow::changeShotToolEvent);
    connect(m_toolBar, &ToolBar::closeButtonToMain, this, &MainWindow::exitApp);
    connect(m_sideBar, &SideBar::changeArrowAndLineToMain, this, &MainWindow::changeArrowAndLineEvent);

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
    m_recordButton->setIcon(QIcon(":/newUI/checked/screencap-checked.svg"));
    m_recordButton->setFixedSize(76, 58);
    //m_recordButton->setObjectName("mainRecordBtn");
    Utils::setAccessibility(m_recordButton, AC_MAINWINDOW_MAINRECORDBTN);


    m_shotButton = new DPushButton(this);
    m_shotButton->setFocusPolicy(Qt::NoFocus);
    pa = m_shotButton->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(0, 129, 255, 204));
    pa.setColor(DPalette::Light, QColor(0, 129, 255, 204));
    m_shotButton->setPalette(pa);
    m_shotButton->setIconSize(QSize(38, 38));
    m_shotButton->setIcon(QIcon(":/newUI/checked/screenshot-checked.svg"));
    m_shotButton->setFixedSize(76, 58);
    //m_shotButton->setObjectName("mainShotBtn");
    Utils::setAccessibility(m_shotButton, AC_MAINWINDOW_MAINSHOTBTN);

    m_recordButton->hide();
    m_shotButton->hide();

    m_backgroundRect = QApplication::desktop()->screen()->geometry();
    m_backgroundRect = QRect(m_backgroundRect.topLeft() / m_pixelRatio, m_backgroundRect.size());

    connect(m_recordButton, SIGNAL(clicked()), this, SLOT(startCountdown()));
    connect(m_shotButton, SIGNAL(clicked()), this, SLOT(saveScreenShot()));

    if (m_screenCount == 1) {
        m_backgroundRect = QApplication::desktop()->screen()->geometry();
        m_backgroundRect = QRect(m_backgroundRect.topLeft() / m_pixelRatio, m_backgroundRect.size());

        move(m_backgroundRect.topLeft() * m_pixelRatio);
        this->setFixedSize(m_backgroundRect.size());
    } else if (m_screenCount > 1) {
        m_backgroundRect = t_screenRect;
        m_backgroundRect = QRect(m_backgroundRect.topLeft() / m_pixelRatio, m_backgroundRect.size());
        move(m_backgroundRect.topLeft() * m_pixelRatio);
        this->setFixedSize(m_backgroundRect.size());
    }
    initBackground();
    initShortcut();


    if (m_screenCount > 1 && m_pixelRatio  > 1) {
        if (m_screenInfo[0].width < m_screenInfo[1].width)
            // QT bug，这里暂时做特殊处理
            // 多屏放缩情况下，小屏在前，整体需要偏移一定距离
            this->move(m_screenInfo[0].width - static_cast<int>(m_screenInfo[0].width / m_pixelRatio), 0);
    }
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
        << QString(tr("Saving the screen recording file, please wait..."))  // body
        << actions                                               // actions
        << QVariantMap()                                         // hints
        << timeout;                                           // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
}

void MainWindow::forciblySavingNotify()
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
        << QString(tr("As the window effect is disabled during the process, the recording has to be stopped"))  // body
        << actions                                               // actions
        << QVariantMap()                                         // hints
        << timeout;                                           // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
}

void MainWindow::onExit()
{
    if (RECORD_BUTTON_RECORDING == recordButtonStatus) {
        stopRecord();
    } else {
        exitApp();
    }
}

void MainWindow::initShortcut()
{
    //滚动截图应用内快捷键
    QShortcut *scrollShotSC = new QShortcut(QKeySequence("Alt+I"), this);
    //ocr应用内快捷键
    QShortcut *ocrSC = new QShortcut(QKeySequence("Alt+O"), this);

    QShortcut *rectSC = new QShortcut(QKeySequence("R"), this);
    QShortcut *ovalSC = new QShortcut(QKeySequence("O"), this);
    QShortcut *arrowSC = new QShortcut(QKeySequence("L"), this);
    QShortcut *lineSC = new QShortcut(QKeySequence("P"), this);
    QShortcut *textSC = new QShortcut(QKeySequence("T"), this);
    //转全局事件，此处未使用
    //QShortcut *optionSC = new QShortcut(QKeySequence("F3"), this);
    QShortcut *keyBoardSC = new QShortcut(QKeySequence("K"), this);
    //QShortcut *mouseSC = new QShortcut(QKeySequence("C"), this);
    QShortcut *cameraSC = new QShortcut(QKeySequence("W"), this);
    //转全局事件，此处未使用
    //QShortcut *audioSC = new QShortcut(QKeySequence("S"), this);
    //    QShortcut *colorSC = new QShortcut(QKeySequence("Alt+6"), this);

    connect(scrollShotSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            m_toolBar->shapeClickedFromMain("scrollShot");
        }

    });

    connect(ocrSC, &QShortcut::activated, this, [ = ] {
        //滚动截图及普通截图都可以通过快捷键触发ocr
        if (status::shot == m_functionType || status::scrollshot == m_functionType)
        {
            m_toolBar->shapeClickedFromMain("ocr");
        }

    });

    connect(rectSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            m_toolBar->shapeClickedFromMain("rect");
        }

    });
    connect(ovalSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            m_toolBar->shapeClickedFromMain("circ");
        }
    });
    connect(arrowSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            m_toolBar->shapeClickedFromMain("line");
        }
    });
    connect(lineSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            m_toolBar->shapeClickedFromMain("pen");
        }
    });
    connect(textSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            m_toolBar->shapeClickedFromMain("text");
        }
    });
//    connect(optionSC, &QShortcut::activated, this, [ = ] {
//        if (status::shot == m_functionType)
//            emit m_toolBar->shapeClickedFromMain("option");
//    });
    connect(keyBoardSC, &QShortcut::activated, this, [ = ] {
        if (status::record == m_functionType && RECORD_BUTTON_NORMAL == recordButtonStatus)
            m_toolBar->shapeClickedFromMain("keyBoard");
    });
    /*
    connect(mouseSC, &QShortcut::activated, this, [ = ] {
        if (status::record == m_functionType && RECORD_BUTTON_NORMAL == recordButtonStatus)
            emit m_toolBar->shapeClickedFromMain("mouse");
    });
    */
    connect(cameraSC, &QShortcut::activated, this, [ = ] {
        if (status::record == m_functionType && RECORD_BUTTON_NORMAL == recordButtonStatus)
            m_toolBar->shapeClickedFromMain("camera");
    });
//    connect(audioSC, &QShortcut::activated, this, [ = ] {
//        if (status::record == m_functionType && RECORD_BUTTON_NORMAL == recordButtonStatus)
//            emit m_toolBar->shapeClickedFromMain("audio");
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
        // 录屏的时候，如果焦点还在录屏应用上，会导致录屏退出。添加条件判断，修复。
        if ((status::shot == m_functionType) || (status::record == m_functionType && RECORD_BUTTON_RECORDING != recordButtonStatus)) {
            exitApp();
        }
    } else {
        qWarning() << "manual service not available, cannot open manual";
    }
}

void MainWindow::initResource()
{
    m_showButtons = new ShowButtons(this);
    if (!m_pScreenShotEvent || !m_showButtons)
        return;

    connect(m_showButtons, SIGNAL(keyShowSignal(const QString &)),
            this, SLOT(showKeyBoardButtons(const QString &)));
    //    resizeHandleBigImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("resize_handle_big.svg"));
    resizeHandleBigImg = DHiDPIHelper::loadNxPixmap(":/newUI/normal/node.svg");
    //resizeHandleSmallImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("resize_handle_small.svg"));

//　　　dde-dock显示时长插件代替系统托盘
//    trayIcon = new QSystemTrayIcon(this);
//    trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon1.svg"))));
//    trayIcon->setToolTip(tr("Screen Capture"));
//    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    //    setDragCursor();

    buttonFeedback = new ButtonFeedback();

    connect(m_pScreenShotEvent, SIGNAL(activateWindow()), this, SLOT(onActivateWindow()), Qt::QueuedConnection);
    connect(m_pScreenShotEvent, SIGNAL(shotKeyPressEvent(const unsigned char &)), this, SLOT(onShotKeyPressEvent(const unsigned char &)), Qt::QueuedConnection);
    qDebug() << "截图事件监控线程启动！";
    m_pScreenShotEvent->start();
}

void MainWindow::initScreenShot()
{
    if (!m_initScreenShot) {
        m_initScreenShot = true;
    } else {
        return;
    }
    connect(this, &MainWindow::releaseEvent, this, [ = ] {
        qDebug() << "release event !!!";
        removeEventFilter(this);
    });
    m_functionType = 1;
    m_keyBoardStatus = false;
    m_mouseStatus = 0;
    //m_multiKeyButtonsInOnSec = false;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    m_screenWidth = m_backgroundRect.width();
    m_screenHeight = m_backgroundRect.height();

    m_shotStatus = ShotMouseStatus::Normal;


    isPressButton = false;
    isReleaseButton = false;


    recordButtonStatus = RECORD_BUTTON_NORMAL;

    flashTrayIconCounter = 0;

    //    selectAreaName = "";

    //隐藏键盘按钮控件
    if (m_keyButtonList.count() > 0) {
        for (int i = 0; i < m_keyButtonList.count(); i++) {
            m_keyButtonList.at(i)->hide();
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
        m_sizeTips->setRecorderTipsInfo(false);
        m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
    }

    //recordButton->hide();
    //recordOptionPanel->hide();


    if (m_firstShot == 0) {
        m_selectedMic = true;
        m_selectedSystemAudio = true;
    }
    //    eventMonitor.quit();
    //    emit releaseEvent();
    //初始化截图，不退出录屏全局事件监控
    //exitScreenRecordEvent();
    connect(this, &MainWindow::hideScreenshotUI, this, &MainWindow::hide);

    //初始化ocr
    m_ocrInterface = nullptr;
    m_toolBar->setFocus();
}

//初始化录屏窗口
void MainWindow::initScreenRecorder()
{
    if (!m_pScreenRecordEvent)
        return;

    m_functionType = status::record;
    m_keyBoardStatus = false;
    m_mouseStatus = 0;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    m_screenWidth = m_backgroundRect.width();
    m_screenHeight = m_backgroundRect.height();

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

    recordButtonStatus = RECORD_BUTTON_NORMAL;

    flashTrayIconCounter = 0;

    //    selectAreaName = "";

    if (m_isShapesWidgetExist) {
        m_shapesWidget->hide();
    }

    m_isShapesWidgetExist = false;
    //m_needDrawSelectedPoint = false;


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
        m_sizeTips->setRecorderTipsInfo(true);
        m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
    }

    //recordButton->hide();
    //recordOptionPanel->hide();


    m_zoomIndicator->hide();

    if (m_firstShot == 0) {
        m_selectedMic = true;
        m_selectedSystemAudio = true;
    }
    //录屏初次进来此字段为false，后面进来此字段为ture故不会改变默认框选区域大小
    if (!m_initScreenRecorder) {
        m_initScreenRecorder = true;
    } else {
        return;
    }
    connect(m_pScreenRecordEvent, SIGNAL(buttonedPress(int, int)), this, SLOT(showPressFeedback(int, int)), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(buttonedDrag(int, int)), this, SLOT(showDragFeedback(int, int)), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(buttonedRelease(int, int)), this, SLOT(showReleaseFeedback(int, int)), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(pressEsc()), this, SLOT(responseEsc()), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(pressKeyButton(unsigned char)), m_showButtons,
            SLOT(showContentButtons(unsigned char)), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(pressKeyButton(unsigned char)), this,
            SLOT(onRecordKeyPressEvent(const unsigned char &)), Qt::QueuedConnection);
    connect(m_pScreenRecordEvent, SIGNAL(releaseKeyButton(unsigned char)), m_showButtons,
            SLOT(releaseContentButtons(unsigned char)), Qt::QueuedConnection);
    m_pScreenRecordEvent->start();

    m_toolBar->setFocus();
}

//滚动截图的初始化函数
void MainWindow::initScrollShot()
{
    //滚动截图激活鼠标点击事件moveMouse
    connect(m_pScreenShotEvent, SIGNAL(buttonedPress(int, int)), this, SLOT(onScrollShotButtonPressEvent(int, int)), Qt::QueuedConnection);
    connect(m_pScreenShotEvent, SIGNAL(moveMouse(int, int)), this, SLOT(onScrollShotMoveMouseEvent(int, int)), Qt::QueuedConnection);
    //定时器，滚动截图模式下每0.5秒减少一次鼠标点击次数
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [ = ]() {
        m_scrollShotMouseClick -= 1;
        if (m_scrollShotMouseClick < 0) {
            m_scrollShotMouseClick = 0;
        }
        //qDebug() << "0.5s定时结束！ m_scrollShotMouseClick： " << m_scrollShotMouseClick;
    });
    timer->start(500);

    //设置当前功能类型
    m_functionType = status::scrollshot;
    m_keyBoardStatus = false;
    m_mouseStatus = 0;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    m_screenWidth = m_backgroundRect.width();
    m_screenHeight = m_backgroundRect.height();
    isPressButton = false;
    isReleaseButton = false;

    //隐藏工具栏矩形、圆形、箭头、笔画、选项中裁切选项-显示光标
    m_toolBar->hideSomeToolBtn();
    update();

    //捕捉区域不能进行拖动
    recordButtonStatus = RECORD_BUTTON_WAIT;

    //重新设置鼠标形状
    resetCursor();

    m_toolBar->hide();
    m_shotButton -> hide();
    m_sizeTips->hide();

    //滚动预览开启初始化
    if (!m_previewWidget) {
        QRect previewRecordRect {
            static_cast<int>(recordX),
            static_cast<int>(recordY),
            static_cast<int>(recordWidth),
            static_cast<int>(recordHeight)
        };
        m_previewWidget = new PreviewWidget(previewRecordRect, this);
        m_previewWidget->setScreenWidth(m_screenWidth);
        m_previewWidget->initPreviewWidget();
        m_previewWidget->show();
    }

    //提示开始滚动截图的方法
    m_scrollShotTip = new ScrollShotTip(this);
    //链接拼接失败提示，点击打开帮助
    connect(m_scrollShotTip, &ScrollShotTip::openScrollShotHelp, this, &MainWindow::openScrollShotHelp);
    //选择提示类型
    m_scrollShotTip->showTip(TipType::StartScrollShotTip);
    const QPoint topLeft = geometry().topLeft();
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio + topLeft.x()),
        static_cast<int>(recordY * m_pixelRatio + topLeft.y()),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    int leftTopX = 0, leftTopY = 0;
    int screenWidth = 0, screenHeight = 0;
    //单个屏幕的长宽
    screenWidth = m_screenWidth / m_screenCount;
    screenHeight = m_screenHeight;
    //捕捉区域的宽小于300或者高小于100 则提示内容在屏幕中间且与捕捉区域左上角在一个屏幕
    if (recordWidth < 300 || recordHeight < 100) {
        leftTopX = static_cast<int>((recordRect.x() / screenWidth) * screenWidth + (screenWidth - m_scrollShotTip->width()) / 2);
        leftTopY = static_cast<int>((screenHeight - m_scrollShotTip->height()) / 2);
    } else {
        leftTopX = static_cast<int>((recordRect.x() / m_pixelRatio + (recordRect.width() / m_pixelRatio  - m_scrollShotTip->width()) / 2));
        //工具栏在捕捉区域下
        if(m_toolBar->y() > recordRect.y()){
            leftTopY = static_cast<int>(m_toolBar->y() / m_pixelRatio - 55 / m_pixelRatio );
        }
        //工具栏在捕捉区域上
        else {
            leftTopY = static_cast<int>((m_toolBar->y() + m_toolBar->height()) / m_pixelRatio + 15 / m_pixelRatio );
        }
        //qDebug() << "m_toolBar.x(): " << m_toolBar->x() << ",m_toolBar.y(): " <<m_toolBar->y() <<",m_toolBar.width(): " << m_toolBar->width() <<",m_toolBar.height(): " << m_toolBar->height() ;
        //qDebug() << "leftTopX: " << leftTopX << ",leftTopY: " <<leftTopY;
        //leftTopY = static_cast<int>((recordRect.y() / m_pixelRatio + (recordRect.height() / m_pixelRatio - m_scrollShotTip->height()) / 100 * 97));
    }
    //提示信息移动到指定位置
    m_scrollShotTip->move(leftTopX, leftTopY);
    //滚动截图的处理类
    m_scrollShot = new ScrollScreenshot();
    qRegisterMetaType<PixMergeThread::MergeErrorValue>("MergeErrorValue");
    //链接滚动拼接过程显示预览窗口和图片
    connect(m_scrollShot, &ScrollScreenshot::updatePreviewImg, this, &MainWindow::showPreviewWidgetImage);
    //链接滚动截图抓取当前捕捉区图片进行图片拼接
    connect(m_scrollShot, &ScrollScreenshot::getOneImg, this, [ = ] {
        bool ok;
        if (m_previewWidget)
        {
            m_previewWidget->hide();
        }
        QRect rect(recordX + 1, recordY + 1, recordWidth - 2, recordHeight - 2);
        QPixmap img = m_screenGrabber.grabEntireDesktop(ok, rect, m_pixelRatio);
        m_scrollShot->addPixmap(img);
        if (m_previewWidget)
        {
            m_previewWidget->show();
        }
    });
    //链接滚动截图拼接过程中返回的错误状态
    connect(m_scrollShot, SIGNAL(merageError(PixMergeThread::MergeErrorValue)), this, SLOT(scrollShotMerageImgState(PixMergeThread::MergeErrorValue)));
    //滚动截图左上角当前图片的大小
    m_scrollShotSizeTips = new TopTips(this);
    m_scrollShotSizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
    m_scrollShotSizeTips->hide();
    //检测锁屏的属性是否发生改变
    QDBusConnection::sessionBus().connect("com.deepin.SessionManager",
                                          "/com/deepin/SessionManager",
                                          "org.freedesktop.DBus.Properties",
                                          "PropertiesChanged",
                                          "sa{sv}as",
                                          this,
                                          SLOT(onLockScreenEvent(QDBusMessage))
                                         );

    //延时100ms之后使预览窗口显示第一张预览图
    QTimer::singleShot(100, this, [ = ] {
        //打开滚动截图时，预览窗口显示的第一张图片
        bool ok;
        QRect rect(recordX + 1, recordY + 1, recordWidth - 2, recordHeight - 2);
        //截取指定区域的图片
        //预览区域显示当前指定区域的图片
        QPixmap img = m_screenGrabber.grabEntireDesktop(ok, rect, m_pixelRatio);
        m_previewWidget->updateImage(img.toImage());
        //打开工具栏显示 需放在更新工具栏之前，避免出现工具栏没显示但是已经执行位置更新
        m_toolBar->show();
        //打开截图保存按钮显示
        m_shotButton->show();
        //打开滚动截图左上角当前图片的大小显示
        m_scrollShotSizeTips->show();
        //显示开始滚动 截图的提示
        m_scrollShotTip->show();
        repaint();
        //延时50ms之后更新工具栏及截图保存按钮的位置
        QTimer::singleShot(50, this, [ = ] {
            if (m_toolBar->isVisible())
            {
                updateToolBarPos();
                updateShotButtonPos();
            }
        });
    });
}
//显示预览窗口和图片
void MainWindow::showPreviewWidgetImage(QImage img)
{
    m_scrollShotSizeTips ->updateTips(QPoint(recordX, recordY), QSize(img.width(), img.height()));
    m_previewWidget->updateImage(img);
}

void MainWindow::initLaunchMode(const QString &launchMode)
{
    if (launchMode == "screenRecord") {
        m_sizeTips->setRecorderTipsInfo(true);
        m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
        m_launchWithRecordFunc = true;
        m_shotButton->hide();
        m_recordButton->show();
        m_functionType = status::record;
        initScreenRecorder();
        if (m_sideBar->isVisible()) {
            m_sideBar->hide();
        }
    } else {
        m_launchWithRecordFunc = false;
        m_recordButton->hide();
        m_shotButton->show();
        m_functionType = 1;
        initScreenShot();
    }
}
/*
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
*/
void MainWindow::fullScreenshot()
{
    //DDesktopServices::playSystemSoundEffect(DDesktopServices::SEE_Screenshot);
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

    // 多屏截取全屏

    if (m_screenCount == 1) {
        m_backgroundRect = QApplication::desktop()->screen()->geometry();
        m_backgroundRect = QRect(m_backgroundRect.topLeft(), m_backgroundRect.size());
    } else if (m_screenCount > 1) {
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

    shotFullScreen(true);


    TempFile::instance()->setFullScreenPixmap(m_resultPixmap);
    const auto r = saveAction(m_resultPixmap);
    sendNotify(m_saveIndex, m_saveFileName, r);
}
void MainWindow::topWindow()
{
    //DDesktopServices::playSystemSoundEffect(DDesktopServices::SEE_Screenshot);
    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();

    int t_windowCount = DWindowManagerHelper::instance()->allWindowIdList().size();
    DForeignWindow *prewindow = nullptr;
    for (int i = t_windowCount - 1; i >= 0; i--) {
        auto wid = DWindowManagerHelper::instance()->allWindowIdList().at(i);
        if (wid == winId()) continue;
        if (prewindow) {
            delete prewindow;
            prewindow = nullptr;
        }
        DForeignWindow *window = DForeignWindow::fromWinId(wid);
        prewindow = window;
        //if (window->type() == Qt::Window || window->type() == Qt::Desktop) {
        // 经DTK确认，type存在bug。用flags替换，获取窗口类型功能。bug 77300；
        if (window->flags().testFlag(Qt::Window) || window->flags().testFlag(Qt::Desktop)) {
            // 排除dde-dock作为顶层窗口
            if (window->wmClass() == "dde-dock") {
                continue;
            }
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
    if (prewindow) {
        delete prewindow;
        prewindow = nullptr;
    }
    // 放缩情况下，修正顶层窗口位置。
    if (!qFuzzyCompare(1.0, m_pixelRatio) && m_screenCount > 1) {
        int x = recordX;
        int y = recordY;
        if (x >= m_screenInfo[1].x) {
            recordX = static_cast<int>(m_screenInfo[1].x / m_pixelRatio + (x - m_screenInfo[1].x));
        }
        if (y >= m_screenInfo[1].y) {
            recordY = static_cast<int>(m_screenInfo[1].y / m_pixelRatio + (y - m_screenInfo[1].y));
        }
    } else {
        recordX = recordX - static_cast<int>(screenRect.x() * m_pixelRatio);
        recordY = recordY - static_cast<int>(screenRect.y() * m_pixelRatio);
    }

    this->hide();
    emit this->hideScreenshotUI();
    QRect target(static_cast<int>(recordX * m_pixelRatio),
                 static_cast<int>(recordY * m_pixelRatio),
                 static_cast<int>(recordWidth * m_pixelRatio),
                 static_cast<int>(recordHeight * m_pixelRatio));

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

void MainWindow::startScreenshotFor3rd(const QString &path)
{
    m_shotSavePath = path;
    if (path == "" || (!QDir(path).exists())) {
        // 传入的文件目录不存在，保存在系统pictures路径下
        qDebug() << path << "not exist! change path to QStandardPaths::PicturesLocation";
        m_shotSavePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();
    m_shotWithPath = true; // 自带路径
    m_noNotify = true; // 关闭通知

}

void MainWindow::noNotify()
{
    m_noNotify = true;

    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();
}

void MainWindow::initBackground()
{
    //    QTimer::singleShot(200, this, [ = ] {
    m_backgroundPixmap = getPixmapofRect(m_backgroundRect);
    qDebug() << "screen rect:" << m_backgroundPixmap.rect();
    if (m_backgroundPixmap.isNull()) {
        DBusNotify shotFailedNotify;
        QString tips = QString(tr("Screenshot failed."));
        shotFailedNotify.Notify(QCoreApplication::applicationName(), 0, "deepin-screen-recorder", QString(), tips, QStringList(), QVariantMap(), 5000);
        exit(0);
    }
    m_resultPixmap = m_backgroundPixmap;
    TempFile::instance()->setFullScreenPixmap(m_backgroundPixmap);
    //    });
}

QPixmap MainWindow::getPixmapofRect(const QRect &rect)
{
    bool ok;
    return m_screenGrabber.grabEntireDesktop(ok, rect, m_pixelRatio);
}

bool MainWindow::saveImg(const QPixmap &pix, const QString &fileName, const char *format)
{
    int quality = -1;
    //qt5环境，经测试quality值对png效果明显，对jpg和bmp不明显
    if (pix.width() * pix.height() > 1920 * 1080 && QString("PNG") == QString(format).toUpper()) {
        if (QSysInfo::currentCpuArchitecture().startsWith("x86") && !m_isZhaoxin) {
            quality = 60;
        } else if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin) {
            quality = 70;
        } else if (QSysInfo::currentCpuArchitecture().startsWith("arm")) {
            quality = 75;
        } else if (QSysInfo::currentCpuArchitecture().startsWith("mips")) {
            quality = 80;
        }
    }
    return pix.save(fileName, format, quality);
}

//滚动截图时鼠标穿透设置之所以需要单独用来设置，因为有些时候捕捉区域太大，工具栏在捕捉区域内部，需要将工具栏这片区域给排除掉
void MainWindow::setInputEvent()
{
    //将当前捕捉区域画为一个矩形
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    //当前鼠标点击的点
    QPoint toolBarPoint(m_toolBar->x() * m_pixelRatio, m_toolBar->y() * m_pixelRatio);
    //判断工具栏位置是否在捕捉区域内部
    if (recordRect.contains(toolBarPoint)) {
        //工具栏位置在捕捉区域内部，穿透的位置下移一断距离
        Utils::getInputEvent(
            static_cast<int>(this->winId()),
            static_cast<short>(recordX * m_pixelRatio),
            static_cast<short>((recordY + m_toolBar->height()) * m_pixelRatio),
            static_cast<unsigned short>(recordWidth * m_pixelRatio),
            static_cast<unsigned short>((recordHeight - m_toolBar->height()) * m_pixelRatio));
    } else {
        //捕捉区域穿透
        Utils::getInputEvent(
            static_cast<int>(this->winId()),
            static_cast<short>(recordX * m_pixelRatio),
            static_cast<short>(recordY * m_pixelRatio),
            static_cast<unsigned short>(recordWidth * m_pixelRatio),
            static_cast<unsigned short>(recordHeight * m_pixelRatio));
    }
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
    if (0 == m_functionType && RECORD_BUTTON_RECORDING != recordButtonStatus) {
        emit releaseEvent();
        exitScreenRecordEvent();
        exitScreenShotEvent();
        QApplication::quit();
    }
}

void MainWindow::onActivateWindow()
{
    activateWindow();
}

void MainWindow::compositeChanged()
{

    // 在非录屏状态下，通过快捷键关闭特效模式
    if (recordButtonStatus != RECORD_BUTTON_RECORDING) {
        m_hasComposite = m_wmHelper->hasComposite();
        update();
        return;
    }

    if (m_hasComposite == true  && !m_wmHelper->hasComposite()) {
        // 录屏过程中 由初始3D转2D模式, 强制暂停录屏.
        // 如果录屏由 由初始2D转3D模式, 则不强制退出录屏.
        // 强制退出通知
        forciblySavingNotify();
        if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
            // 录屏过程中， 从3D切换回2D， 停止录屏。
            stopRecord();
            return;
        } else {
            // 倒计时3s内， 从3D切换回2D直接退出。
            emit releaseEvent();
            exitScreenRecordEvent();
            QApplication::quit();
        }

        /*
        qDebug() << "have no Composite";
        Utils::warnNoComposite();
        emit releaseEvent();
        if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
            m_pScreenRecordEvent->terminate();
            m_pScreenRecordEvent->wait();
        }
        QApplication::quit();
        */
    }
    //2D录屏, 切换模式后,更新当前按钮的样式
    if (m_keyBoardStatus && m_pRecorderRegion) {
        m_pRecorderRegion->updateKeyBoardButtonStyle();
    }
}

void MainWindow::updateToolBarPos()
{
    if (m_shotflag == 1) {
        return;
    }
    m_isToolBarInside = false;
    if (m_toolBarInit == false) {
        m_toolBar->initToolBar(this);
        m_toolBar->setRecordLaunchMode(m_launchWithRecordFunc);
        //m_toolBar->setIsZhaoxinPlatform(m_isZhaoxin);

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
    for (int i = 0; i < m_screenInfo.size(); ++i) {
        if (toolbarPoint.x() + m_toolBar->width() >= m_screenInfo[i].x && toolbarPoint.x() + m_toolBar->width() <= m_screenInfo[i].x + m_screenInfo[i].width) {
            if (toolbarPoint.y() < m_screenInfo[i].y + TOOLBAR_Y_SPACING) {
                // 屏幕上超出
                toolbarPoint.setY(m_screenInfo[i].y + TOOLBAR_Y_SPACING);
            } else if (toolbarPoint.y() > m_screenInfo[i].y + m_screenInfo[i].height / m_pixelRatio - m_toolBar->height() - TOOLBAR_Y_SPACING) {
                // 屏幕下超出
                int y = std::max(recordY - m_toolBar->height() - TOOLBAR_Y_SPACING, 0);
                if (y > m_screenInfo[i].y + m_screenInfo[i].height / m_pixelRatio - m_toolBar->height() - TOOLBAR_Y_SPACING)
                    y = m_screenInfo[i].y + static_cast<int>(m_screenInfo[i].height / m_pixelRatio) - m_toolBar->height() - TOOLBAR_Y_SPACING;
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
                        sidebarPoint.setX(recordX - m_sideBar->width() - SIDEBAR_X_SPACING);
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
    for (int i = 0; i < m_screenInfo.size(); ++i) {
        if (sidebarPoint.x() + m_sideBar->width() >= m_screenInfo[i].x && sidebarPoint.x() + m_sideBar->width() <= m_screenInfo[i].x + m_screenInfo[i].width) {
            if (sidebarPoint.y() < m_screenInfo[i].y + TOOLBAR_Y_SPACING) {
                // 屏幕上超出
                sidebarPoint.setY(m_screenInfo[i].y + TOOLBAR_Y_SPACING);
            } else if (sidebarPoint.y() > m_screenInfo[i].y + m_screenInfo[i].height - m_sideBar->height() - TOOLBAR_Y_SPACING) {
                // 屏幕下超出
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

    if (status::record == m_functionType) {
        if (!m_recordButton->isVisible()) {
            m_recordButton->show();
        }
    }

    // 根据屏幕的具体实际坐标修正Y值
    for (int i = 0; i < m_screenInfo.size(); ++i) {
        if (recordButtonBarPoint.x() > m_screenInfo[i].x && recordButtonBarPoint.x() < m_screenInfo[i].x + m_screenInfo[i].width) {
            if (recordButtonBarPoint.y() < m_screenInfo[i].y + TOOLBAR_Y_SPACING) {
                // 屏幕上超出
                recordButtonBarPoint.setY(m_screenInfo[i].y + TOOLBAR_Y_SPACING + 6);
            } else if (recordButtonBarPoint.y() > m_screenInfo[i].y + m_screenInfo[i].height / m_pixelRatio - m_recordButton->height() - TOOLBAR_Y_SPACING) {
                // 屏幕下超出
                int y = std::max(recordY - m_recordButton->height() - TOOLBAR_Y_SPACING - 6, 0);
                if (y > m_screenInfo[i].y + m_screenInfo[i].height / m_pixelRatio - m_recordButton->height() - TOOLBAR_Y_SPACING - 6)
                    y = m_screenInfo[i].y + static_cast<int>(m_screenInfo[i].height / m_pixelRatio) - m_recordButton->height() - TOOLBAR_Y_SPACING - 6;
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

    if (status::shot == m_functionType) {
        if (!m_shotButton->isVisible()) {
            m_shotButton->show();
        }
    }

    // 根据屏幕的具体实际坐标修正Y值
    for (int i = 0; i < m_screenInfo.size(); ++i) {
        if (shotButtonBarPoint.x() > m_screenInfo[i].x && shotButtonBarPoint.x() < m_screenInfo[i].x + m_screenInfo[i].width) {
            if (shotButtonBarPoint.y() < m_screenInfo[i].y + TOOLBAR_Y_SPACING) {
                // 屏幕上超出
                shotButtonBarPoint.setY(m_screenInfo[i].y + TOOLBAR_Y_SPACING + 6);
            } else if (shotButtonBarPoint.y() > m_screenInfo[i].y + m_screenInfo[i].height / m_pixelRatio - m_shotButton->height() - TOOLBAR_Y_SPACING) {
                // 屏幕下超出
                int y = std::max(recordY - m_shotButton->height() - TOOLBAR_Y_SPACING - 6, 0);
                if (y > m_screenInfo[i].y + m_screenInfo[i].height / m_pixelRatio - m_shotButton->height() - TOOLBAR_Y_SPACING - 6)
                    y = m_screenInfo[i].y + static_cast<int>(m_screenInfo[i].height / m_pixelRatio) - m_shotButton->height() - TOOLBAR_Y_SPACING - 6;
                shotButtonBarPoint.setY(y);
            }
            break;
        }
    }

    m_shotButton->move(shotButtonBarPoint.x(), shotButtonBarPoint.y());
}
void MainWindow::updateCameraWidgetPos()
{
    if (m_cameraWidget == nullptr || m_shotflag == 1) {
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
//切换截图功能或者录屏功能
void MainWindow::changeFunctionButton(QString type)
{
    if (type == "record") {
        if (status::record == m_functionType) {
            return;
        }
        m_sizeTips->setRecorderTipsInfo(true);
        m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
        m_shotButton->hide();
        updateRecordButtonPos();
        m_recordButton->show();
        m_functionType = status::record;
        initScreenRecorder();
        if (m_sideBar->isVisible()) {
            m_sideBar->hide();
        }
    }

    else if (type == "shot") {
        if (status::shot == m_functionType) {
            return;
        }
        m_sizeTips->setRecorderTipsInfo(false);
        m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
        m_toolBar->setVideoButtonInit();
        if (m_cameraWidget && m_cameraWidget->isVisible()) {
            m_cameraWidget->cameraStop();
            m_cameraWidget->hide();
        }
        m_recordButton->hide();
        updateShotButtonPos();
        m_shotButton->show();
        m_functionType = 1;
        updateToolBarPos();
        initScreenShot();
    }

    update();
    repaint();
}

void MainWindow::showKeyBoardButtons(const QString &key)
{
    //键盘按钮启用状态下创建按键控件
    qDebug() << this->geometry();
    if (m_keyBoardStatus) {
        if (m_hasComposite == false && RECORD_BUTTON_RECORDING == recordButtonStatus) {
            // 2D 录屏下将按键发送至m_pRecorderRegion区域。
            m_pRecorderRegion->showKeyBoardButtons(key);
            return;
        }
        KeyButtonWidget *t_keyWidget = new KeyButtonWidget(this);
        t_keyWidget->setKeyLabelWord(key);
        m_keyButtonList.append(t_keyWidget);

        if (m_keyButtonList.count() > 5) {
            delete m_keyButtonList.first();
            m_keyButtonList.pop_front();
        }
        qDebug() << "aaa key count:" << m_keyButtonList.count();
        //更新多按钮的位置
        updateMultiKeyBoardPos();
        repaint();
    }
}

void MainWindow::changeKeyBoardShowEvent(bool checked)
{
    qDebug() << "keyboard" << checked;
    m_keyBoardStatus = checked;
    if (m_keyButtonList.count() > 0) {
        for (int t_index = 0; t_index < m_keyButtonList.count(); t_index++) {
            m_keyButtonList.at(t_index)->setVisible(checked);
        }
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

void MainWindow::changeShowMouseShowEvent(bool checked)
{
    qDebug() << "show mouse" << checked;
    m_mouseShowStatus = checked;
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
void MainWindow::changeCameraSelectEvent(bool checked)
{
    m_recordButton->setEnabled(false);
    if (m_cameraWidget == nullptr) {
        m_cameraWidget = new CameraWidget(this);
        m_cameraWidget->hide();
        // 摄像头界面层级下调,防止遮住工具栏
        m_cameraWidget->lower();
        m_cameraWidget->initCamera();
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
    m_recordButton->setEnabled(true);
}
/*
 * never used
void MainWindow::showMultiKeyBoardButtons()
{
    m_multiKeyButtonsInOnSec = false;
}
*/
void MainWindow::updateMultiKeyBoardPos()
{
    QPoint t_keyPoint[5];
    static float posfix[5][5] = {{-0.5f, 0}, {-(0.5f + 1 / 1.5f), (1 / 1.5f - 0.5f), 0}, {-1.8f, -0.5f, 0.8f, 0}, {-2.5f, -(0.5f + 1 / 1.5f), (1 / 1.5f - 0.5f), 1.5, 0}, {-3.1f, -1.8f, -0.5, 0.8f, 2.1f}};
    if (!m_keyButtonList.isEmpty()) {
        int count = m_keyButtonList.count();
        for (int j = 0; j < count; ++j) {
            m_keyButtonList.at(j)->hide();
            t_keyPoint[j] = QPoint(static_cast<int>(recordX + recordWidth / 2 + m_keyButtonList.at(j)->width() * posfix[count - 1][j]), std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(j)->move(t_keyPoint[j].x(), t_keyPoint[j].y());
            m_keyButtonList.at(j)->show();
        }
    }
    /*
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
    */
}

void MainWindow::changeShotToolEvent(const QString &func)
{

    qDebug() << "MainWindow::changeShotToolEvent >> func: " << func;
    //调用ocr功能时先截图后，退出截图录屏，将刚截图的图片串递到ocr识别界面；
    if (func == "ocr") {
        //qDebug() << "m_saveFileName: " << m_saveFileName;
        // 调起OCR识别界面， 传入截图路径
        m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), this);
        saveScreenShot();

    } else if (func == "scrollShot") { //点击滚动截图
        //初始化滚动截图
        initScrollShot();

    } else {
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

        //禁用滚动截图按钮
        m_toolBar->setScrollShotDisabled();

    }
}

void MainWindow::saveScreenShot()
{
    if (m_pScreenShotEvent) {
        m_CursorImage = m_pScreenShotEvent->getCursorImage();
    }

    emit releaseEvent();
    m_shotflag = 1;
    emit saveActionTriggered();
    hideAllWidget();

    m_toolBar->setVisible(false);
    m_sizeTips->setVisible(false);
    m_sideBar->setVisible(false);
    m_shotButton->setVisible(false);
    m_recordButton->setVisible(false);
    m_sizeTips->setVisible(false);
    if (m_scrollShotTip) {
        m_scrollShotTip->setVisible(false);
        m_scrollShotTip->hide();
    }
    update();


    if (status::scrollshot == m_functionType && m_scrollShotStatus != 0) {
        m_resultPixmap = QPixmap::fromImage(m_scrollShot->savePixmap());
    } else {
        shotCurrentImg();
    }
    const bool r = saveAction(m_resultPixmap);
    sendNotify(m_saveIndex, m_saveFileName, r);
}

void MainWindow::sendNotify(SaveAction saveAction, QString saveFilePath, const bool succeed)
{
    Q_UNUSED(saveAction);
    if (Utils::is3rdInterfaceStart) {
        QDBusMessage msg = QDBusMessage::createSignal("/com/deepin/Screenshot", "com.deepin.Screenshot", "Done");
        msg << saveFilePath;
        QDBusConnection::sessionBus().send(msg);
        qApp->quit();
        return;
    }
    if (m_noNotify) {
        qApp->quit();
        return;
    }
    // failed notify
    if (!succeed) {
        DBusNotify saveFailedNotify;
        QString tips = QString(tr("Save failed. Please save it in your home directory."));
        saveFailedNotify.Notify(QCoreApplication::applicationName(), 0, "deepin-screen-recorder", QString(), tips, QStringList(), QVariantMap(), 5000);
        qApp->quit();
        return;
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
    QString tips;
    if (remote_dde_notify_obj_exist && saveFilePath.compare(QString(tr("Clipboard")))) {
        actions << "_open" << tr("View");

        //QString fileDir  = QUrl::fromLocalFile(QFileInfo(saveFilePath).absoluteDir().absolutePath()).toString();
        //QString filePath = QUrl::fromLocalFile(saveFilePath).toString();

        QString command;

        tips = QString(tr("Saved to %1")).arg(saveFilePath);
        if (Utils::isTabletEnvironment && QFile("/usr/bin/deepin-album").exists()) {
            command = QString("deepin-album,%1").arg(saveFilePath);
            tips = tr("The screenshot has been saved in the album");
        } else if (Utils::isTabletEnvironment && !QFile("/usr/bin/deepin-album").exists() && QFile("/usr/bin/deepin-image-viewer").exists()) {
            command = QString("deepin-image-viewer,%1").arg(saveFilePath);
        } else if (!Utils::isTabletEnvironment && QFile("/usr/bin/dde-file-manager").exists()) {
            command = QString("dde-file-manager,--show-item,%1").arg(saveFilePath);
        } else {
            command = QString("xdg-open,%1").arg(saveFilePath);
        }

        hints["x-deepin-action-_open"] = command;
    }

    qDebug() << "saveFilePath:" << saveFilePath;
    QList<QVariant> arg;
    int timeout = 5000;
    unsigned int id = 0;
    arg << (QCoreApplication::applicationName())                 // appname
        << id                                                    // id
        << QString("deepin-screen-recorder")                     // icon
        << tr("Screenshot finished")                             // summary
        << tips                                                  // body
        << actions                                               // actions
        << hints                                                 // hints
        << timeout;
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);// timeout
    //    }

    QTimer::singleShot(2, [ = ] {
        emit releaseEvent();
        exitScreenRecordEvent();
        exitScreenShotEvent();
        qApp->quit();
    });
}

bool MainWindow::saveAction(const QPixmap &pix)
{
    emit releaseEvent();

    //    using namespace utils;
    //不必要的拷贝，浪费时间
    //QPixmap screenShotPix = pix;
    QDateTime currentDate;
    QString currentTime =  currentDate.currentDateTime().
                           toString("yyyyMMddHHmmss");
    m_saveFileName = "";
    QString functionTypeStr = tr("Screenshot");
    if (status::scrollshot == m_functionType) {
        functionTypeStr = functionTypeStr + "_" + tr("Scrollshot");
        selectAreaName.clear();
    }

    QString tempFileName = "";
    QStandardPaths::StandardLocation saveOption = QStandardPaths::TempLocation;
    //    bool copyToClipboard = true;

    int t_pictureFormat = ConfigSettings::instance()->value("save", "format").toInt();

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
    case SaveToSpecificDir: {
        this->hide();
        this->releaseKeyboard();

        QString path = ConfigSettings::instance()->value("common", "default_savepath").toString();
        QString fileName = selectAreaName;

        if (path.isEmpty() || !QDir(path).exists()) {
            path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        }

        if (fileName.isEmpty()) {
            fileName = QString("%1_%2").arg(functionTypeStr).arg(currentTime);
        } else {
            fileName = QString("%1_%2_%3").arg(functionTypeStr).arg(selectAreaName).arg(currentTime);
        }
        QString lastFileName;

        // 自动化测试反馈, dde-desktop里面有2个computer_window. 修改直接调用QFileDialog类的静态函数. 不用创建其对象
        //QFileDialog fileDialog;
        switch (t_pictureFormat) {
        case 0:
            lastFileName    = QString("%1/%2.png").arg(path).arg(fileName);
            m_saveFileName =  QFileDialog::getSaveFileName(this, tr("Save"),  lastFileName,
                                                           tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
            break;
        case 1:
            lastFileName    = QString("%1/%2.jpg").arg(path).arg(fileName);
            m_saveFileName =  QFileDialog::getSaveFileName(this, tr("Save"),  lastFileName,
                                                           tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)"));
            break;
        case 2:
            lastFileName    = QString("%1/%2.bmp").arg(path).arg(fileName);
            m_saveFileName =  QFileDialog::getSaveFileName(this, tr("Save"),  lastFileName,
                                                           tr("BMP (*.bmp);;JPEG (*.jpg *.jpeg);;PNG (*.png)"));
            break;
        default:
            lastFileName    = QString("%1/%2.png").arg(path).arg(fileName);
            m_saveFileName =  QFileDialog::getSaveFileName(this, tr("Save"),  lastFileName,
                                                           tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
            break;
        }




        if (m_saveFileName.isEmpty() || QFileInfo(m_saveFileName).isDir()) {
            // 保存到指定位置, 用户在选择保存目录时，点击取消。保存失败，且不显示通知信息
            m_noNotify = true;
            return false;
        }

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
        } else if (!isValidFormat(fileSuffix)) {
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
    case SaveToClipboard: {
        qDebug() << SaveToClipboard << "SaveToClipboard";
        break;
    }
    case PadDefaultPath: {
        QDir dir;
        QString padImgPath = QString("%1%2%3")
                             .arg(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first())
                             .arg(QDir::separator())
                             .arg(functionTypeStr);
        if (!dir.exists(padImgPath)) {
            dir.mkpath(padImgPath);
        }

        if (selectAreaName.isEmpty()) {
            m_saveFileName = QString("%1%2%3_%4.png").arg(padImgPath, QDir::separator(), functionTypeStr, currentTime);
        } else {
            m_saveFileName = QString("%1%2%3_%4_%5.png").arg(padImgPath, QDir::separator(), functionTypeStr, selectAreaName, currentTime);
        }

        break;
    }
    default:
        break;
    }
    if (m_saveIndex == SaveToSpecificDir && m_saveFileName.isEmpty()) {
        return false;
    } else if (m_saveIndex == SaveToSpecificDir || !m_saveFileName.isEmpty()) {
        if (!saveImg(pix, m_saveFileName, QFileInfo(m_saveFileName).suffix().toLocal8Bit()))
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
            m_saveFileName = QString("%1/%2_%3.%4").arg(savePath, functionTypeStr, currentTime, t_formatBuffix);
        } else {
            m_saveFileName = QString("%1/%2_%3_%4.%5").arg(savePath, functionTypeStr, selectAreaName, currentTime, t_formatBuffix);
        }

        if (!saveImg(pix, m_saveFileName, t_formatStr.toLatin1().data()))
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
                m_saveFileName = QString("%1/%2_%3.%4").arg(savePath, functionTypeStr, currentTime, t_formatBuffix);
            } else {
                m_saveFileName = QString("%1/%2_%3_%4.%5").arg(savePath, functionTypeStr, selectAreaName, currentTime, t_formatBuffix);
            }
        }


        if (!saveImg(pix, m_saveFileName, t_formatStr.toLatin1().data()))
            return false;
    } else if (m_saveIndex == SaveToClipboard) {
        if (selectAreaName.isEmpty()) {
            tempFileName = QString("%1_%2_%3").arg(tr("Clipboard"), functionTypeStr, currentTime);
        } else {
            tempFileName = QString("%1_%2_%3_%4").arg(tr("Clipboard"), functionTypeStr, selectAreaName, currentTime);
        }
        qDebug() << "m_saveFileName: " << m_saveFileName;
        m_saveFileName = QString(tr("Clipboard"));
    }
    // 保存到剪贴板
    if (Utils::is3rdInterfaceStart == false) {
        QMimeData *t_imageData = new QMimeData;
        t_imageData->setImageData(pix);
        Q_ASSERT(!pix.isNull());
        QClipboard *cb = qApp->clipboard();
        cb->setMimeData(t_imageData, QClipboard::Clipboard);
        /*
        // 调起画板， 传入截图路径
        int t_openWithDraw = ConfigSettings::instance()->value("open", "draw").toInt();
        if (t_openWithDraw == 1) {
            DrawInterface *m_draw = new DrawInterface("com.deepin.Draw", "/com/deepin/Draw", QDBusConnection::sessionBus(), this);
            QList<QImage> list;
            list.append(screenShotPix.toImage());
            m_draw->openImages(list);
            delete m_draw;
        }
        */
    }
    if (m_ocrInterface != nullptr) {
        if (m_saveIndex == SaveToClipboard) {
            m_ocrInterface->openImageAndName(pix.toImage(), tempFileName);
        } else {
            m_ocrInterface->openImageAndName(pix.toImage(), m_saveFileName);
        }
        //m_ocrInterface->openFile(m_saveFileName);
    }
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
        m_backgroundPixmap.setDevicePixelRatio(m_pixelRatio);
        painter.drawPixmap(backgroundRect, m_backgroundPixmap);
        //        DWidget::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 2D窗管模式下，录屏背景用截图背景。
    if (status::shot == m_functionType || m_hasComposite == false) {
        painter.setRenderHint(QPainter::Antialiasing, true);
        QRect backgroundRect;

        backgroundRect = QRect(0, 0, rootWindowRect.width(), rootWindowRect.height());
        // FIXME: Under the magnifying glass, it seems to be magnified two times.
        m_backgroundPixmap.setDevicePixelRatio(m_pixelRatio);
        painter.drawPixmap(backgroundRect, m_backgroundPixmap);
    }

    if (recordWidth > 0 && recordHeight > 0) {
        if (Utils::isTabletEnvironment && (status::record == m_functionType || status::scrollshot == m_functionType)) {
            // 平板环境屏蔽录屏和滚动截图， 不绘制线框
            return;
        }
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
        }
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
        //滚动截图情况下键盘按键操作
        if (status::scrollshot == m_functionType) {
            if (keyEvent->key() == Qt::Key_Escape) {
                qDebug() << "Key_Escape pressed: app quit!";
                emit releaseEvent();
                exitScreenRecordEvent();
                exitScreenShotEvent();
                qApp->quit();
            }else if (keyEvent->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (keyEvent->key() == Qt::Key_Question) {
                    onViewShortcut();
                }
            }else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {//滚动截图模式下，按下enter按键进行保存截图
                saveScreenShot();
            }
        }

        if (status::shot == m_functionType) {
            if (keyEvent->key() == Qt::Key_Escape) {
                if (m_isShapesWidgetExist) {
                    if (m_shapesWidget->textEditIsReadOnly()) {
                        return false;
                    }
                }
                qDebug() << "Key_Escape pressed: app quit!";
                emit releaseEvent();
                exitScreenRecordEvent();
                exitScreenShotEvent();
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
            } else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                saveScreenShot();
            }

            bool needRepaint = false;
            if (m_isShapesWidgetExist) {
                if (keyEvent->key() == Qt::Key_Escape) {
                    emit releaseEvent();
                    exitScreenRecordEvent();
                    exitScreenShotEvent();
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

                if (!m_needSaveScreenshot) {
                    m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
                    if (m_toolBar->isVisible()) {
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

                    if (m_cameraWidget && m_cameraWidget->isVisible()) {
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

                m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
                if (m_toolBar->isVisible()) {
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

                if (m_cameraWidget && m_cameraWidget->isVisible()) {
                    updateCameraWidgetPos();
                }


                if (recordButtonStatus == RECORD_BUTTON_NORMAL && needRepaint) {
                    //hideRecordButton();
                }
            }
        }

    } else if (event->type() == QEvent::KeyRelease) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (status::shot == m_functionType) {
            bool isNeedRepaint = false;

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
                    isNeedRepaint = true;
                }
            }
            if (isNeedRepaint) {
                update();
            }

        }

        else {
            if (!keyEvent->isAutoRepeat()) {
                if (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right || keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
                    needRepaint = true;
                }

                if (recordButtonStatus == RECORD_BUTTON_NORMAL && needRepaint) {
                    //showRecordButton();
                    updateToolBarPos();
                    if (status::shot == m_functionType && m_sideBar->isVisible()) {
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
            if (status::shot == m_functionType || status::scrollshot == m_functionType) {
                qDebug() << "双击鼠标按钮！进行截图保存！";
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
                        //hideRecordButton();
                        hideAllWidget();
                        if (m_cameraWidget && m_cameraWidget->isVisible()) {
                            m_cameraWidget->hide();
                        }
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
                if (status::shot == m_functionType) {
                    if (m_menuController == nullptr) {
                        m_menuController = new MenuController(this);
                        connect(m_menuController, &MenuController::saveAction, this, &MainWindow::saveScreenShot);
                        connect(m_menuController, &MenuController::closeAction, this, &MainWindow::exitApp);
                    }
                    m_menuController->showMenu(QPoint(mapToGlobal(mouseEvent->pos())));
                }
            }
        }


    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (!m_isShapesWidgetExist) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                //滚动截图的图片大小提示不使用此方法
                if (status::scrollshot != m_functionType) {
                    m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
                }
                if (!isFirstReleaseButton) {
                    isFirstReleaseButton = true;

                    updateCursor(event);
                    updateToolBarPos();
                    if (status::shot == m_functionType && m_sideBar->isVisible()) {
                        updateSideBarPos();
                        m_zoomIndicator->hide();
                    }
                    updateRecordButtonPos();
                    updateShotButtonPos();
                    // Record select area name with window name if just click (no drag).
                    if (!isFirstDrag) {
                        //QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                        for (auto it = windowRects.rbegin(); it != windowRects.rend(); ++it) {
                            if (QRect(it->x(), it->y(), it->width(), it->height()).contains(this->cursor().pos() + screenRect.topLeft())) {
                                selectAreaName = windowNames[windowRects.rend() - it - 1];
                                break;
                            }
                        }
                    }

                    if (status::record == m_functionType) {
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

                    else if (status::shot == m_functionType) {
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



                    //showRecordButton();
                    updateToolBarPos();
                    if (status::shot == m_functionType && m_sideBar->isVisible()) {
                        updateSideBarPos();
                    }
                    updateRecordButtonPos();
                    updateShotButtonPos();

                    needRepaint = true;
                } else {
                    if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                        //showRecordButton();
                        updateToolBarPos();
                        if (status::shot == m_functionType && m_sideBar->isVisible()) {
                            updateSideBarPos();
                        }
                        updateRecordButtonPos();
                        updateShotButtonPos();
                        updateCameraWidgetPos();

                    }
                }
                if (m_sizeTips->isVisible()) {
                    //滚动截图的图片大小提示不使用此方法
                    if (status::scrollshot != m_functionType) {
                        m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
                    }
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
                if (status::shot == m_functionType) {
                    if (!m_toolBar->isVisible() && !isFirstReleaseButton) {
                        //QPoint curPos = this->cursor().pos(); 采用全局坐标，替换局部坐标
                        QPoint curPos = mouseEvent->globalPos();
                        QPoint tmpPos;
                        QPoint topLeft = m_backgroundRect.topLeft() * m_pixelRatio;

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
                    if (isPressButton && !isReleaseButton && !Utils::isTabletEnvironment) {
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
                if (!Utils::isTabletEnvironment) {
                    const QPoint mousePoint = QCursor::pos();
                    for (auto it = windowRects.rbegin(); it != windowRects.rend(); ++it) {
                        if (QRect(it->x(), it->y(), it->width(), it->height()).contains(mousePoint)) {
                            if (!qFuzzyCompare(1.0, m_pixelRatio) && m_screenCount > 1) {
                                int x = it->x();
                                int y = it->y();
                                if (x < m_screenInfo[1].x) {
                                    recordX = x;
                                } else {
                                    recordX = static_cast<int>(m_screenInfo[1].x / m_pixelRatio + (x - m_screenInfo[1].x));
                                }
                                if (y < m_screenInfo[1].y) {
                                    recordY = y;
                                } else {
                                    recordY = static_cast<int>(m_screenInfo[1].y / m_pixelRatio + (y - m_screenInfo[1].y));
                                }
                            } else {
                                recordX = it->x() - static_cast<int>(screenRect.x() * m_pixelRatio);
                                recordY = it->y() - static_cast<int>(screenRect.y() * m_pixelRatio);
                            }
                            recordWidth = it->width();
                            recordHeight = it->height();
                            needRepaint = true;
                            break;
                        }
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
            //滚动截图的图片大小提示不使用此方法
            if (status::scrollshot != m_functionType) {
                m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
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
void MainWindow::tableRecordSet()
{
    m_tabletRecorderHandle = new RecorderTablet(nullptr);

    recordX = 0;
    recordY = 0;
    recordWidth = m_screenSize.width();
    recordHeight = m_screenSize.height();

    // 鼠标点击状态录制
    m_mouseStatus = 1;

    // 录制系统音频
    m_selectedMic = false;
    m_selectedSystemAudio = true;

    startCountdown();
}

//滚动截图鼠标按钮事件
void MainWindow::onScrollShotButtonPressEvent(int x, int y)
{
    //滚动截图出现异常时屏蔽鼠标点击事件
    if (m_isErrorWithScrollShot) return;

    //鼠标点击次数
    m_scrollShotMouseClick += 1;
    if (m_scrollShotMouseClick > 2) {
        m_scrollShotMouseClick = 2;
    }

    //将当前捕捉区域画为一个矩形
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    //qDebug() << "m_pixelRatio :  " << m_pixelRatio;
    //qDebug() << "recordX: " << recordX << ",recordY: " << recordY <<",recordWidth: " << recordWidth <<",recordHeight: " << recordHeight ;
    //qDebug() << "recordRect.x(): " << recordRect.x() << ",recordRect.y(): " <<recordRect.y() <<",recordRect.width(): " << recordRect.width() <<",recordRect.height(): " << recordRect.height() ;

    //当前鼠标点击的点
    QPoint mouseClickPoint(x , y);
    //qDebug() << "鼠标按键 x,y :  " << x << " , " << y;
    //qDebug() << "mouseClickPoint x,y :  " << mouseClickPoint.x() << " , " << mouseClickPoint.y();
    //判断当前点击的点是否在捕捉区域内部,不在捕捉区域内则不做以下处理
    if (!recordRect.contains(mouseClickPoint)) {
        return;
    }

    //qDebug() << "m_scrollShotMouseClick: " << m_scrollShotMouseClick;

    //鼠标单击
    if (m_scrollShotMouseClick == 1) {
        qDebug() << "鼠标单击!";
        //第一次进入滚动截图，开始滚动截图
        if (m_scrollShotStatus == 0) {
            m_scrollShotTip->hide();
            update();
            m_scrollShotStatus = 1;
            startScrollShot();
        }
        //第n次进入 n不等于1，暂停滚动截图
        else if (1 == m_scrollShotStatus || 2 == m_scrollShotStatus) {
            m_scrollShotStatus = 3;
            pauseScrollShot();
        }
        //第n次进入 n不等于1,继续滚动截图
        else if (3 == m_scrollShotStatus) {
            m_scrollShotStatus = 2;
            continueScrollShot();
        }
    }
    //鼠标双击
    else if (m_scrollShotMouseClick == 2) {
        qDebug() << "鼠标双击!";
        //不是第一次进入滚动截图，则保存当前滚动截图
        //saveScreenShot();

    }
}

//滚动截图鼠标移动事件处理
void MainWindow::onScrollShotMoveMouseEvent(int x, int y)
{
    //滚动截图出现异常时屏蔽鼠标移动事件
    if (m_isErrorWithScrollShot) return;

    //将当前捕捉区域画为一个矩形
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    //当前鼠标的点
    QPoint mouseMovePoint(x, y);
    //判断当前点是否在捕捉区域内部,不在捕捉区域内则暂停滚动
    if (!recordRect.contains(mouseMovePoint)) {
        if (1 == m_scrollShotStatus || 2 == m_scrollShotStatus || 3 == m_scrollShotStatus) {
            m_scrollShotStatus = 4;
            pauseScrollShot();
        }
    }
    //判断当前点是否在捕捉区域内部,在捕捉区域内则继续滚动
    else {
        //鼠标点击触发的暂停，不论鼠标在捕捉区域内如何移动都不继续
        if (4 == m_scrollShotStatus) {
            m_scrollShotStatus = 2;
            continueScrollShot();
        }
    }

}

//滚动截图时，锁屏处理事件
void MainWindow::onLockScreenEvent(QDBusMessage msg)
{
    bool isLocked = false;
    QList<QVariant> arguments = msg.arguments();
    //参数固定长度
    if (3 != arguments.count()) {
        qDebug() << "锁屏处理出现异常！";
        return;
    }
    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName == "com.deepin.SessionManager") {
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        QStringList keys =  changedProps.keys();
        foreach (const QString &prop, keys) {
            if (prop == "Locked") {
                //qDebug() << "Locked:" <<  changedProps[prop];
                isLocked = changedProps[prop].toBool();
            }
        }
    }
    //锁屏时暂停滚动
    if (isLocked) {
        m_scrollShotStatus = 3;
        pauseScrollShot();
    }
    //解锁时恢复滚动
    //else {
        //continueScrollShot();
    //}

}

//打开截图录屏帮助文档并定位到滚动截图
void MainWindow::openScrollShotHelp()
{


    QDBusInterface interFace("com.deepin.Manual.Open",
                             "/com/deepin/Manual/Open",
                             "com.deepin.Manual.Open",
                             QDBusConnection::sessionBus());
    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName())                  // 应用名称
        << QString(tr("Scrollshot"));                         // 帮助文案中的标题名称
    interFace.callWithArgumentList(QDBus::AutoDetect, "OpenTitle", arg);

    exitApp();
}



//滚动截图时，获取拼接时的状态
void MainWindow::scrollShotMerageImgState(PixMergeThread::MergeErrorValue state)
{
    //暂停滚动截图
    pauseScrollShot();

    //滚动截图出现异常
    m_isErrorWithScrollShot = true;

    qDebug() << "function:" << __func__ << " ,line: " << __LINE__ <<" , 拼接时的状态: " << state;
    //m_scrollShotTip = new ScrollShotTip (this);
    const QPoint topLeft = geometry().topLeft();
    //qDebug() << "function:" << __func__ << " ,line: " << __LINE__ <<"state: " << state;

    //捕捉区域
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio + topLeft.x()),
        static_cast<int>(recordY * m_pixelRatio + topLeft.y()),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    //qDebug() << "function:" << __func__ << " ,line: " << __LINE__ <<"state: " << state;

    //单个屏幕的长宽
    int screenWidth = m_screenWidth / m_screenCount;
    int screenHeight = m_screenHeight;

    //默认提示框的位置为捕捉区域左上角对应的屏幕的正中        //捕捉区域的宽小于300或者高小于100 则提示内容在屏幕中间且与捕捉区域左上角在一个屏幕
    int leftTopX = static_cast<int>((recordRect.x() / screenWidth) * screenWidth + (screenWidth - m_scrollShotTip->width()) / 2);
    int leftTopY = static_cast<int>((screenHeight - m_scrollShotTip->height()) / 2);
    //qDebug() << "function:" << __func__ << " ,line: " << __LINE__ <<"state: " << state;

    //1：拼接失败
    if (state == PixMergeThread::MergeErrorValue::Failed) {
        //提示滚动截图拼接失败的方法
        m_scrollShotTip->showTip(TipType::ErrorScrollShotTip);

        //捕捉区域的宽大于等于300或者高大于等于100 则提示内容在在捕捉区域内
        if (recordWidth >= 300 && recordHeight >= 100) {
            leftTopX = static_cast<int>((recordRect.x() / m_pixelRatio + (recordRect.width() / m_pixelRatio  - m_scrollShotTip->width()) / 2));
            leftTopY = static_cast<int>((recordRect.y() / m_pixelRatio + (recordRect.height() / m_pixelRatio - m_scrollShotTip->height()) / 100 * 97));
        }
        //qDebug() << "1：拼接失败" ;

        //拼接失败立即保存当前的截图
        //saveScreenShot();
    }
    //2：滚动到底部
    else if (state == PixMergeThread::MergeErrorValue::ReachBottom) {
        m_scrollShotTip->showTip(TipType::EndScrollShotTip);
        //捕捉区域的宽大于等于300或者高大于等于100 则提示内容在在捕捉区域内
        if (recordWidth >= 300 && recordHeight >= 100) {
            leftTopX = static_cast<int>((recordRect.x() / m_pixelRatio + (recordRect.width() / m_pixelRatio  - m_scrollShotTip->width()) / 2));
            leftTopY = static_cast<int>((recordRect.y() / m_pixelRatio + (recordRect.height() / m_pixelRatio - m_scrollShotTip->height()) / 100 * 3));
        }
        //qDebug() << "2：滚动到底部" ;

    }
    //3：拼接截图到截图最大限度
    else if (state == PixMergeThread::MergeErrorValue::MaxHeight) {
        m_scrollShotTip->showTip(TipType::MaxLengthScrollShotTip);
        //捕捉区域的宽大于等于300或者高大于等于100 则提示内容在在捕捉区域内
        if (recordWidth >= 300 && recordHeight >= 100) {
            leftTopX = static_cast<int>((recordRect.x() / m_pixelRatio + (recordRect.width() / m_pixelRatio  - m_scrollShotTip->width()) / 2));
            leftTopY = static_cast<int>((recordRect.y() / m_pixelRatio + (recordRect.height() / m_pixelRatio - m_scrollShotTip->height()) / 100 * 3));
        }
        //qDebug() << "3：拼接截图到截图最大限度" ;

    } else {
        return;
    }
    //qDebug() << "function:" << __func__ << " ,line: " << __LINE__ <<"state: " << state;

    m_scrollShotTip->move(leftTopX, leftTopY);
    //显示提示
    m_scrollShotTip->show();
    repaint();
    //qDebug() << "function:" << __func__ << " ,line: " << __LINE__ <<"state: " << state;



    //定时2s后提示消失
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [ = ]() {
        timer->stop();
        m_scrollShotTip->hide();
        //qDebug() << "提示已消失！" ;

    });
    timer->setInterval(2000);
    //qDebug() << "提示将在2s后消失！" ;
    timer->start();
}

void MainWindow::initPadShot()
{
    recordX = 0;
    recordY = 0;
    recordWidth = QApplication::desktop()->width();
    recordHeight = QApplication::desktop()->height();
    updateToolBarPos();
    updateShotButtonPos();
    QPoint toolbarPoint;
    toolbarPoint = QPoint(recordX + recordWidth - m_toolBar->width() - TOOLBAR_X_SPACING, std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));
    toolbarPoint.setY(recordY + TOOLBAR_Y_SPACING);
    m_toolBar->showAt(toolbarPoint);
}

void MainWindow::exitScreenRecordEvent()
{
    if (QSysInfo::currentCpuArchitecture().startsWith("x86")
            && !m_isZhaoxin
            && m_pScreenRecordEvent) {
        m_pScreenRecordEvent->terminate();
        m_pScreenRecordEvent->wait();
        delete m_pScreenRecordEvent;
        m_pScreenRecordEvent = nullptr;
    }
}

void MainWindow::exitScreenShotEvent()
{
    if (QSysInfo::currentCpuArchitecture().startsWith("x86")
            && !m_isZhaoxin
            && m_pScreenShotEvent) {
        m_pScreenShotEvent->terminate();
        m_pScreenShotEvent->wait();
        delete m_pScreenShotEvent;
        m_pScreenShotEvent = nullptr;
    }
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
        if (m_cameraWidget && !m_cameraOffFlag) {
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
    // 安全问题， 日志隐私，暴露cpu类型
    //qDebug() << "is zhao xin:" << str_output;
    if (str_output.length() == 0) {
        m_isZhaoxin = false;
    } else {
        m_isZhaoxin = true;
    }
    process.close();
}

//截图模式及滚动截图模式键盘按下执行的操作
void MainWindow::onShotKeyPressEvent(const unsigned char &keyCode)
{
    //滚动截图及普通截图都可以通过快捷键触发F3
    if (KEY_F3 == keyCode && ( status::shot == m_functionType || status::scrollshot == m_functionType)) {
        m_toolBar->shapeClickedFromMain("option");
    }

}

//录屏模式下键盘按下执行的操作
void MainWindow::onRecordKeyPressEvent(const unsigned char &keyCode)
{
    if (KEY_S == keyCode && status::record == m_functionType && RECORD_BUTTON_NORMAL == recordButtonStatus) {
        m_toolBar->shapeClickedFromMain("audio");
    }
    if (KEY_M == keyCode && status::record == m_functionType && RECORD_BUTTON_NORMAL == recordButtonStatus) {
        m_toolBar->shapeClickedFromMain("mouse");
    } else if (KEY_F3 == keyCode && status::record == m_functionType && RECORD_BUTTON_NORMAL == recordButtonStatus) {
        m_toolBar->shapeClickedFromMain("record_option");
    }
}

void MainWindow::startRecord()
{
    recordButtonStatus = RECORD_BUTTON_RECORDING;
    resetCursor();
    repaint();

//    trayIcon->show();
//    flashTrayIconTimer = new QTimer(this);
//    connect(flashTrayIconTimer, SIGNAL(timeout()), this, SLOT(flashTrayIcon()));
//    flashTrayIconTimer->start(800);
    // 状态栏闪烁
    if (Utils::isTabletEnvironment && m_tabletRecorderHandle) {
        m_tabletRecorderHandle->startStatusBar();
    }

    recordProcess.setRecordAudioInputType(getRecordInputType(m_selectedMic, m_selectedSystemAudio));
    recordProcess.setRecordMouse(m_mouseShowStatus);
    recordProcess.startRecord();
    // 录屏开始后，隐藏窗口。（2D窗管下支持录屏, 但是会导致摄像头录制不到）
    if (m_hasComposite == false) {
        hide();
        // 显示录屏框区域。
        m_pRecorderRegion->show();
    }
}

/**
 * @brief 开始滚动截图的方式：鼠标左键点击捕捉区域
 */
void MainWindow::startScrollShot()
{
    qDebug() << "开始滚动截图！";
    setInputEvent();

    bool ok;
    QRect rect(recordX + 1, recordY + 1, recordWidth - 2, recordHeight - 2);
    QPixmap img = m_screenGrabber.grabEntireDesktop(ok, rect, m_pixelRatio);
    //滚动截图添加第一张图片并启动
    m_scrollShot->addPixmap(img);
}

//暂停滚动截图
void MainWindow::pauseScrollShot()
{
    //qDebug() << "暂停滚动截图!";

    //将当前捕捉区域画为一个矩形
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    //当前鼠标点击的点
    QPoint toolBarPoint(m_toolBar->x() * m_pixelRatio, m_toolBar->y() * m_pixelRatio);
    //暂停滚动时取消窗口穿透
    Utils::cancelInputEvent(static_cast<int>(this->winId()), static_cast<short>(this->x()), static_cast<short>(this->y()), static_cast<unsigned short>(this->width() * m_pixelRatio), static_cast<unsigned short>(this->height() * m_pixelRatio));
    //滚动截图改变状态，暂停滚动
    m_scrollShot->changeState(true);
}

//继续滚动截图
void MainWindow::continueScrollShot()
{
    //qDebug() << "继续滚动截图!";
    //捕捉区域穿透
//    Utils::getInputEvent(static_cast<int>(this->winId()), static_cast<short>(recordX), static_cast<short>(recordY), static_cast<unsigned short>(recordWidth), static_cast<unsigned short>(recordHeight));
    setInputEvent();

    //滚动截图改变状态，继续滚动
    m_scrollShot->changeState(false);
}

void MainWindow::shotCurrentImg()
{
    if (recordWidth == 0 || recordHeight == 0)
        return;

    //m_needDrawSelectedPoint = false;
    //m_drawNothing = true;
    update();

    int eventTime = 60;
    QRect rect = QApplication::desktop()->screenGeometry();
    if (rect.width()*rect.height() > 1920 * 1080) {
        if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin) {
            eventTime = 120;
        } else if (QSysInfo::currentCpuArchitecture().startsWith("mips")) {
            eventTime = 260;
        } else if (QSysInfo::currentCpuArchitecture().startsWith("arm")) {
            eventTime = 220;
        }
    } else {
        if (QSysInfo::currentCpuArchitecture().startsWith("mips")) {
            eventTime = 160;
        } else if (QSysInfo::currentCpuArchitecture().startsWith("arm")) {
            eventTime = 120;
        }
    }
    QEventLoop eventloop1;
    QTimer::singleShot(eventTime, &eventloop1, SLOT(quit()));
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
    //qDebug() << recordX << "," << recordY << "," << recordWidth << "," << recordHeight << m_resultPixmap.rect() << m_pixelRatio;
    QRect target(static_cast<int>(recordX * m_pixelRatio),
                 static_cast<int>(recordY * m_pixelRatio),
                 static_cast<int>(recordWidth * m_pixelRatio),
                 static_cast<int>(recordHeight * m_pixelRatio));

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
    if (m_CursorImage == nullptr)
        return;
    const int dataSize = m_CursorImage->width * m_CursorImage->height * 4;
    uchar *pixels = new uchar[dataSize];
    int index = 0;
    for (int j = 0; j < m_CursorImage->width * m_CursorImage->height; ++j) {
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

void MainWindow::shotFullScreen(bool isFull)
{
    QRect target(m_backgroundRect.x(),
                 m_backgroundRect.y(),
                 m_backgroundRect.width(),
                 m_backgroundRect.height());
    qDebug() << "m_backgroundRect" << m_backgroundRect;

    //    m_resultPixmap = getPixmapofRect(m_backgroundRect);
    if (isFull) {
        m_resultPixmap = m_backgroundPixmap;
    } else {
        m_resultPixmap = getPixmapofRect(target);
    }
    qDebug() << "m_resultPixmap" << m_resultPixmap.rect();
}

//void MainWindow::flashTrayIcon()
//{
//    if (flashTrayIconCounter % 2 == 0) {
//        trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon2.svg"))));
//    } else {
//        trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon1.svg"))));
//    }

//    flashTrayIconCounter++;

//    if (flashTrayIconCounter > 10) {
//        flashTrayIconCounter = 1;
//    }
//}

void MainWindow::resizeTop(QMouseEvent *mouseEvent)
{
    if (status::record == m_functionType) {
        int offsetY = mouseEvent->y() - dragStartY;
        recordY = std::max(std::min(dragRecordY + offsetY, dragRecordY + dragRecordHeight - RECORD_MIN_HEIGHT), 1);
        recordHeight = std::max(std::min(dragRecordHeight - offsetY, rootWindowRect.height()), RECORD_MIN_HEIGHT);
    }

    else if (status::shot == m_functionType) {
        int offsetY = mouseEvent->y() - dragStartY;
        recordY = std::max(std::min(dragRecordY + offsetY, dragRecordY + dragRecordHeight - RECORD_MIN_SHOT_SIZE), 1);
        recordHeight = std::max(std::min(dragRecordHeight - offsetY, rootWindowRect.height()), RECORD_MIN_SHOT_SIZE);
    }

}

void MainWindow::resizeBottom(QMouseEvent *mouseEvent)
{
    if (status::record == m_functionType) {
        int offsetY = mouseEvent->y() - dragStartY;
        recordHeight = std::max(std::min(dragRecordHeight + offsetY, rootWindowRect.height()), RECORD_MIN_HEIGHT);
    } else if (status::shot == m_functionType) {
        int offsetY = mouseEvent->y() - dragStartY;
        recordHeight = std::max(std::min(dragRecordHeight + offsetY, rootWindowRect.height()), RECORD_MIN_SHOT_SIZE);
    }
}

void MainWindow::resizeLeft(QMouseEvent *mouseEvent)
{
    if (status::record == m_functionType) {
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
    if (status::record == m_functionType) {
        int offsetX = mouseEvent->x() - dragStartX;
        recordWidth = std::max(std::min(dragRecordWidth + offsetX, rootWindowRect.width()), RECORD_MIN_SIZE);
    } else if (status::shot == m_functionType) {
        int offsetX = mouseEvent->x() - dragStartX;
        recordWidth = std::max(std::min(dragRecordWidth + offsetX, rootWindowRect.width()), RECORD_MIN_SHOT_SIZE);
    }
}

int MainWindow::getAction(QEvent *event)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    int cursorX = mouseEvent->x();
    int cursorY = mouseEvent->y();

    if (cursorX > recordX - m_cursorBound
            && cursorX < recordX + m_cursorBound
            && cursorY > recordY - m_cursorBound
            && cursorY < recordY + m_cursorBound) {
        // Top-Left corner.
        return ACTION_RESIZE_TOP_LEFT;
    } else if (cursorX > recordX + recordWidth - m_cursorBound
               && cursorX < recordX + recordWidth + m_cursorBound
               && cursorY > recordY + recordHeight - m_cursorBound
               && cursorY < recordY + recordHeight + m_cursorBound) {
        // Bottom-Right corner.
        return ACTION_RESIZE_BOTTOM_RIGHT;
    } else if (cursorX > recordX + recordWidth - m_cursorBound
               && cursorX < recordX + recordWidth + m_cursorBound
               && cursorY > recordY - m_cursorBound
               && cursorY < recordY + m_cursorBound) {
        // Top-Right corner.
        return ACTION_RESIZE_TOP_RIGHT;
    } else if (cursorX > recordX - m_cursorBound
               && cursorX < recordX + m_cursorBound
               && cursorY > recordY + recordHeight - m_cursorBound
               && cursorY < recordY + recordHeight + m_cursorBound) {
        // Bottom-Left corner.
        return ACTION_RESIZE_BOTTOM_LEFT;
    } else if (cursorX > recordX - m_cursorBound
               && cursorX < recordX + m_cursorBound) {
        // Left.
        return ACTION_RESIZE_LEFT;
    } else if (cursorX > recordX + recordWidth - m_cursorBound
               && cursorX < recordX + recordWidth + m_cursorBound) {
        // Right.
        return ACTION_RESIZE_RIGHT;
    } else if (cursorY > recordY - m_cursorBound
               && cursorY < recordY + m_cursorBound) {
        // Top.
        return ACTION_RESIZE_TOP;
    } else if (cursorY > recordY + recordHeight - m_cursorBound
               && cursorY < recordY + recordHeight + m_cursorBound) {
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

        if (cursorX > recordX - m_cursorBound
                && cursorX < recordX + m_cursorBound
                && cursorY > recordY - m_cursorBound
                && cursorY < recordY + m_cursorBound) {
            // Top-Left corner.
            QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
        } else if (cursorX > recordX + recordWidth - m_cursorBound
                   && cursorX < recordX + recordWidth + m_cursorBound
                   && cursorY > recordY + recordHeight - m_cursorBound
                   && cursorY < recordY + recordHeight + m_cursorBound) {
            // Bottom-Right corner.
            QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
        } else if (cursorX > recordX + recordWidth - m_cursorBound
                   && cursorX < recordX + recordWidth + m_cursorBound
                   && cursorY > recordY - m_cursorBound
                   && cursorY < recordY + m_cursorBound) {
            // Top-Right corner.
            QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
        } else if (cursorX > recordX - m_cursorBound
                   && cursorX < recordX + m_cursorBound
                   && cursorY > recordY + recordHeight - m_cursorBound
                   && cursorY < recordY + recordHeight + m_cursorBound) {
            // Bottom-Left corner.
            QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
        } else if (cursorX > recordX - m_cursorBound
                   && cursorX < recordX + m_cursorBound) {
            // Left.
            QApplication::setOverrideCursor(Qt::SizeHorCursor);
        } else if (cursorX > recordX + recordWidth - m_cursorBound
                   && cursorX < recordX + recordWidth + m_cursorBound) {
            // Right.
            QApplication::setOverrideCursor(Qt::SizeHorCursor);
        } else if (cursorY > recordY - m_cursorBound
                   && cursorY < recordY + m_cursorBound) {
            // Top.
            QApplication::setOverrideCursor(Qt::SizeVerCursor);
        } else if (cursorY > recordY + recordHeight - m_cursorBound
                   && cursorY < recordY + recordHeight + m_cursorBound) {
            // Bottom.
            QApplication::setOverrideCursor(Qt::SizeVerCursor);

        }

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
/*
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason)
{
    stopRecord();
}
*/
void MainWindow::stopRecord()
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
        hide();
        emit releaseEvent();
        exitScreenRecordEvent();
        //正在保存录屏文件通知
        sendSavingNotify();
        // 停止闪烁
        //flashTrayIconTimer->stop();
        // 状态栏闪烁停止
        if (Utils::isTabletEnvironment && m_tabletRecorderHandle) {
            m_tabletRecorderHandle->stop();
        }
        recordButtonStatus = RECORD_BUTTON_SAVEING;
        recordProcess.stopRecord();
    }
}

void MainWindow::startCountdown()
{
    qDebug() << "function: " << __func__ ;
    if (nullptr != m_pScreenShotEvent) {
        m_pScreenShotEvent->terminate();
        if (!QSysInfo::currentCpuArchitecture().startsWith("mips")) {
            m_pScreenShotEvent->wait();
            delete  m_pScreenShotEvent;
            m_pScreenShotEvent = nullptr;
        }
    }

    recordButtonStatus = RECORD_BUTTON_WAIT;

    disconnect(m_recordButton, SIGNAL(clicked()), this, SLOT(startCountdown()));
    disconnect(m_shotButton, SIGNAL(clicked()), this, SLOT(saveScreenShot()));
    const QPoint topLeft = geometry().topLeft();
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio + topLeft.x()),
        static_cast<int>(recordY * m_pixelRatio + topLeft.y()),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    qDebug() << "record rect:" << recordRect;

    recordProcess.setRecordInfo(recordRect, selectAreaName);
    resetCursor();
    hideAllWidget();

    //释放正式录屏前显示的按钮
    for (int t_index = 0; t_index < m_keyButtonList.count(); t_index++) {
        delete m_keyButtonList.at(t_index);
    }
    m_keyButtonList.clear();

    //平板模式
    if (Utils::isTabletEnvironment && m_tabletRecorderHandle) {
        connect(m_tabletRecorderHandle, SIGNAL(finished()), this, SLOT(startRecord()));
        m_tabletRecorderHandle->start();
    } else {
        //QVBoxLayout *countdownLayout = new QVBoxLayout(this);
        //setLayout(countdownLayout);
        countdownTooltip = new CountdownTooltip(this);
        connect(countdownTooltip, SIGNAL(finished()), this, SLOT(startRecord()));

        //countdownLayout->addStretch();
        //countdownLayout->addWidget(countdownTooltip, 0, Qt::AlignCenter);
        //countdownLayout->addStretch();
        //adjustLayout(countdownLayout, countdownTooltip->rect().width(), countdownTooltip->rect().height());
        //countdownTooltip->move(recordRect.x() + recordRect.width() / 2 - countdownTooltip->width() / 2, recordRect.y() + recordRect.height() / 2 - countdownTooltip->height() / 2);

        countdownTooltip->move(static_cast<int>((recordRect.x() / m_pixelRatio + (recordRect.width() / m_pixelRatio  - countdownTooltip->width()) / 2)),
                               static_cast<int>((recordRect.y() / m_pixelRatio + (recordRect.height() / m_pixelRatio - countdownTooltip->height()) / 2)));

        countdownTooltip->start();
        countdownTooltip->show();
        m_pVoiceVolumeWatcher->setWatch(false);
        m_pCameraWatcher->setWatch(false);
    }

    if (m_hasComposite == false) {
        // 设置录屏框区域。
        m_pRecorderRegion =  new RecorderRegionShow();
        m_pRecorderRegion->resize(recordWidth + 2, recordHeight + 2);
        m_pRecorderRegion->move(std::max(recordX - 1, 0), std::max(recordY - 1, 0));
        if (m_cameraWidget && m_selectedCamera) {
            m_cameraWidget->cameraStop();
            m_cameraWidget->setCameraStop(true);
            m_pRecorderRegion->initCameraInfo(m_cameraWidget->postion(), m_cameraWidget->geometry().size());
        }
    }

    //先隐藏，再显示
    //目的是解决触控操作无法选中部份应用程序的 QLineEdit 控件的问题
    hide();
    show();

    Utils::passInputEvent(static_cast<int>(this->winId()));

    repaint();
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

    // Utils::clearBlur(windowManager, this->winId());
}
//void MainWindow::adjustLayout(QVBoxLayout *layout, int layoutWidth, int layoutHeight)
//{
//    Q_UNUSED(layoutWidth);
//    Q_UNUSED(layoutHeight);
//    layout->setContentsMargins(
//                recordX,
//                recordY,
//                rootWindowRect.width() - recordX - recordWidth,
//                rootWindowRect.height() - recordY - recordHeight);
//}

void MainWindow::initShapeWidget(QString type)
{
    qDebug() << "function: " << __func__ << " , line: " << __LINE__;
    m_shapesWidget = new ShapesWidget(this);
    m_shapesWidget->setShiftKeyPressed(m_isShiftPressed);

    if (type != "color")
        m_shapesWidget->setCurrentShape(type);

    m_shapesWidget->show();

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
    //m_needDrawSelectedPoint = false;
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
    connect(this, &MainWindow::unDo, m_shapesWidget, &ShapesWidget::undoDrawShapes);
    connect(this, &MainWindow::unDoAll, m_shapesWidget, &ShapesWidget::undoAllDrawShapes);
    connect(this, &MainWindow::saveActionTriggered,
            m_shapesWidget, &ShapesWidget::saveActionTriggered);
    connect(m_shapesWidget, &ShapesWidget::menuNoFocus, this, &MainWindow::activateWindow);
}

void MainWindow::exitApp()
{
    emit releaseEvent();
    exitScreenShotEvent();
    exitScreenRecordEvent();
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
    if (!tmpImg.isNull()) {
        tmpImg = tmpImg.scaled(imgWidth / radius, imgHeight / radius, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (effect == "blur") {
            tmpImg = tmpImg.scaled(imgWidth, imgHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            tempFile->setBlurPixmap(tmpImg);
        } else {
            tmpImg = tmpImg.scaled(imgWidth, imgHeight);
            tempFile->setMosaicPixmap(tmpImg);
        }
    }
}

void MainWindow::shotImgWidthEffect()
{
    if (recordWidth == 0 || recordHeight == 0)
        return;
    QRect target(static_cast<int>(m_shapesWidget->geometry().x() * m_pixelRatio),
                 static_cast<int>(m_shapesWidget->geometry().y() * m_pixelRatio),
                 static_cast<int>(m_shapesWidget->geometry().width() * m_pixelRatio),
                 static_cast<int>(m_shapesWidget->geometry().height() * m_pixelRatio));

    m_resultPixmap = m_backgroundPixmap.copy(target);
    //m_drawNothing = false;
    update();
}

void MainWindow::changeArrowAndLineEvent(int line)
{
    qDebug() << "line :" << line;
    m_toolBar->changeArrowAndLineFromMain(line);
}
