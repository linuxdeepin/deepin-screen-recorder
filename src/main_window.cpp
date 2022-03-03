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
#include <DHiDPIHelper>

#include <QApplication>
#include <QTimer>
#include <QKeyEvent>
#include <QObject>
#include <QPainter>
#include <QDebug>
#include <QProcess>
#include <QMouseEvent>
#include <QClipboard>
#include <QFileDialog>
#include <QShortcut>
#include <QDesktopWidget>
#include <QScreen>
#include <QtConcurrent>
#include <X11/Xcursor/Xcursor.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#define EPSILON 1e-10
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
//const int TOOLBAR_Y_SPACING = 3;
const int TOOLBAR_Y_SPACING = 5;
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
    m_initScreenRecorder(false),
    m_initScroll(false)
{
    initMainWindow();
}

void MainWindow::initMainWindow()
{
    if (Utils::isTabletEnvironment) {
        m_cursorBound = 20;
    } else {
        m_cursorBound = 5;
    }
    setDragCursor();
    m_pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    // 监控录屏过程中， 特效窗口的变化。
    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &MainWindow::compositeChanged);

    connect(qApp, &QGuiApplication::screenAdded, this, &MainWindow::onExit);
    connect(qApp, &QGuiApplication::screenRemoved, this, &MainWindow::onExit);

    m_pScreenCaptureEvent = new EventMonitor(this);
    connect(m_pScreenCaptureEvent, SIGNAL(activateWindow()), this, SLOT(onActivateWindow()), Qt::QueuedConnection);
    connect(m_pScreenCaptureEvent, SIGNAL(mouseDrag(int, int)), this, SLOT(onMouseDrag(int, int)), Qt::QueuedConnection);
    connect(m_pScreenCaptureEvent, SIGNAL(mousePress(int, int)), this, SLOT(onMousePress(int, int)), Qt::QueuedConnection);
    connect(m_pScreenCaptureEvent, SIGNAL(mouseRelease(int, int)), this, SLOT(onMouseRelease(int, int)), Qt::QueuedConnection);
    connect(m_pScreenCaptureEvent, SIGNAL(mouseMove(int, int)), this, SLOT(onMouseMove(int, int)), Qt::QueuedConnection);
    connect(m_pScreenCaptureEvent, SIGNAL(mouseScroll(int, int, int, int)), this, SLOT(onMouseScroll(int, int, int, int)), Qt::QueuedConnection);

    if (!Utils::isWaylandMode) {
        connect(m_pScreenCaptureEvent, SIGNAL(keyboardPress(unsigned char)), this, SLOT(onKeyboardPress(unsigned char)), Qt::QueuedConnection);
        connect(m_pScreenCaptureEvent, SIGNAL(keyboardRelease(unsigned char)), this, SLOT(onKeyboardRelease(unsigned char)), Qt::QueuedConnection);
        m_pScreenCaptureEvent->start();
    }

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
    if (Utils::isWaylandMode) {
        // Wayland 下窗口接收全局键盘
        create();
        windowHandle()->setProperty("_d_dwayland_global_keyevent", true);
    }
}

void MainWindow::initAttributes()
{
    qDebug() << "FunctionName: " << __func__;
    setWindowTitle(tr("Screen Capture"));
    m_keyButtonList.clear();
    m_isZhaoxin = Utils::checkCpuIsZhaoxin();


    rootWindowRect = QRect(0, 0, static_cast<int>(m_screenSize.width() / m_pixelRatio), static_cast<int>(m_screenSize.height() / m_pixelRatio));
    screenRect = QRect(screenRect.topLeft() / m_pixelRatio, screenRect.size());
    qDebug() << "screen size" << rootWindowRect;

    //Qt::FramelessWindowHint ： 设置窗口无边框，
    //Qt::WindowStaysOnTopHint： 通知窗口系统该窗口应位于所有其他窗口之上。请注意，在 X11 上的某些窗口管理器上，您还必须传递 Qt::X11BypassWindowManagerHint 以使此标志正常工作。
    //Qt::X11BypassWindowManagerHint : 完全绕过窗口管理器。
    if (Utils::isWaylandMode) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    }
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);   // make MouseMove can response
    installEventFilter(this);  // add event filter
    createWinId();



    m_screenHeight = m_screenSize.height();
    m_screenWidth = m_screenSize.width();

    //获取自动识别的窗口
    if (Utils::isWaylandMode) {
#ifdef KF5_WAYLAND_FLAGE_ON
        //wayland自动识别窗口
        m_connectionThread = new QThread(this);
        m_connectionThreadObject = new ConnectionThread();
        connect(m_connectionThreadObject, &ConnectionThread::connected, this,
        [this] {
            m_eventQueue = new EventQueue(this);
            m_eventQueue->setup(m_connectionThreadObject);

            Registry *registry = new Registry(this);
            setupRegistry(registry);
        },
        Qt::QueuedConnection
               );
        m_connectionThreadObject->moveToThread(m_connectionThread);
        m_connectionThread->start();
        m_connectionThreadObject->initConnection();
#endif
    } else {
        //x11自动识别窗口
        Utils::getAllWindowInfo(static_cast<quint32>(this->winId()), m_screenWidth, m_screenHeight, windowRects, windowNames);
    }
    //构建截屏工具栏按钮 by zyg
    m_toolBar = new ToolBar(this);
    m_toolBar->hide();

    m_sideBar = new SideBar(this);
    m_sideBar->hide();

    m_sizeTips = new TopTips(this);
    m_sizeTips->hide();

    m_zoomIndicator = new ZoomIndicator(this);
    m_zoomIndicator->hide();


    connect(m_toolBar, &ToolBar::mouseShowCheckedToMain, &recordProcess, &RecordProcess::onRecordMouse);// 鼠标显示
    connect(this, SIGNAL(changeMicrophoneSelectEvent(bool)), &recordProcess, SLOT(setMicrophone(bool)));// 麦克风音频
    connect(this, SIGNAL(changeSystemAudioSelectEvent(bool)), &recordProcess, SLOT(setSystemAudio(bool)));// 系统音频

    connect(m_toolBar, &ToolBar::currentFunctionToMain, this, &MainWindow::changeFunctionButton);
    connect(m_sideBar, &SideBar::changeArrowAndLineToMain, m_toolBar, &ToolBar::changeArrowAndLineFromMain);


    m_backgroundRect = rootWindowRect;
    m_backgroundRect = QRect(m_backgroundRect.topLeft() / m_pixelRatio, m_backgroundRect.size());
    move(m_backgroundRect.topLeft() * m_pixelRatio);
    this->setFixedSize(m_backgroundRect.size());

    initBackground();
    initShortcut();


    if (m_screenCount > 1 && m_pixelRatio  > 1) {

        int widthAfterFirst = 0;
        for (int index = 1; index < m_screenCount; ++index) {
            widthAfterFirst += m_screenInfo[index].width;
        }
        if (m_screenInfo[0].width < widthAfterFirst)
            // QT bug，这里暂时做特殊处理
            // 多屏放缩情况下，小屏在前，整体需要偏移一定距离
            this->move(m_screenInfo[0].width - static_cast<int>(m_screenInfo[0].width / m_pixelRatio), 0);
    }
    //检测锁屏的属性是否发生改变
    QDBusConnection::sessionBus().connect("com.deepin.SessionManager",
                                          "/com/deepin/SessionManager",
                                          "org.freedesktop.DBus.Properties",
                                          "PropertiesChanged",
                                          "sa{sv}as",
                                          this,
                                          SLOT(onLockScreenEvent(QDBusMessage))
                                         );
}
#ifdef KF5_WAYLAND_FLAGE_ON
void MainWindow::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::compositorAnnounced, this,
    [this, registry](quint32 name, quint32 version) {
        m_compositor = registry->createCompositor(name, version, this);
    }
           );

    connect(registry, &Registry::clientManagementAnnounced, this,
    [this, registry](quint32 name, quint32 version) {
        m_clientManagement = registry->createClientManagement(name, version, this);
        qDebug() << QDateTime::currentDateTime().toString(QLatin1String("hh:mm:ss.zzz ")) << "createClientManagement";
        connect(m_clientManagement, &ClientManagement::windowStatesChanged, this,
        [this] {
            m_windowStates = m_clientManagement->getWindowStates();
            qDebug() << "Get new window states";
            this->waylandwindowinfo(m_windowStates);
        }
               );
    }
           );

    connect(registry, &Registry::interfacesAnnounced, this,
    [this] {
        Q_ASSERT(m_compositor);
        Q_ASSERT(m_clientManagement);
        qDebug() << "request getWindowStates";
        m_windowStates = m_clientManagement->getWindowStates();
        this->waylandwindowinfo(m_windowStates);
    }
           );

    registry->setEventQueue(m_eventQueue);
    registry->create(m_connectionThreadObject);
    registry->setup();
}

void MainWindow::waylandwindowinfo(const QVector<ClientManagement::WindowState> &windowStates)
{
    if (windowStates.count() == 0) {
        return;
    }
    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    for (int i = 0; i < windowStates.count(); ++i) {
        if (windowStates.at(i).isMinimized == false && windowStates.at(i).pid != getpid() && windowStates.at(i).resourceName[0] != '\0') {
            if (m_screenInfo.size() > 1) {
                if (m_isVertical == false) {
                    if (windowStates.at(i).geometry.x < m_screenInfo[1].x)
                        windowRects << QRect(
                                        static_cast<int>(windowStates.at(i).geometry.x / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.y / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.width / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.height / ratio));
                    else
                        windowRects << QRect(
                                        static_cast<int>(m_screenInfo[1].x + (windowStates.at(i).geometry.x  - m_screenInfo[1].x) / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.y / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.width / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.height / ratio));
                } else {
                    if (windowStates.at(i).geometry.y <= m_screenInfo[1].y)
                        windowRects << QRect(
                                        static_cast<int>(windowStates.at(i).geometry.x / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.y / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.width / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.height / ratio));
                    else
                        windowRects << QRect(
                                        static_cast<int>(windowStates.at(i).geometry.x / ratio),
                                        static_cast<int>(m_screenInfo[1].y + (windowStates.at(i).geometry.y  - m_screenInfo[1].y) / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.width / ratio),
                                        static_cast<int>(windowStates.at(i).geometry.height / ratio));
                }
            } else {
                windowRects << QRect(
                                static_cast<int>(windowStates.at(i).geometry.x / ratio),
                                static_cast<int>(windowStates.at(i).geometry.y / ratio),
                                static_cast<int>(windowStates.at(i).geometry.width / ratio),
                                static_cast<int>(windowStates.at(i).geometry.height / ratio));
            }
            windowNames << windowStates.at(i).resourceName;
        }
    }
    if (windowStates.count() > 0) {
        m_connectionThread->quit();
        m_connectionThread->wait();
        m_connectionThreadObject->deleteLater();
        QRect screenRect(0, 0, static_cast<int>(m_screenSize.width() / ratio), static_cast<int>(m_screenSize.height() / ratio));
        //qDebug() << screenRect;
        //qDebug() << windowRects;

        for (int i = 0; i < windowRects.size(); ++i) {
            int x = windowRects[i].x();
            int y = windowRects[i].y();
            int x1 = x + windowRects[i].width();
            int y1 = y + windowRects[i].height();
            if (x < 0) {
                windowRects[i].setX(0);
                windowRects[i].setWidth(windowRects[i].width());
            }
            if (m_isVertical == true && x1 > screenRect.width()) {
                windowRects[i].setWidth(screenRect.width() - x);
            }
            if (m_isVertical == false && y1 > screenRect.height()) {
                windowRects[i].setHeight(screenRect.height() - y);
            }
        }
        if (m_isFullScreenShot) {
            saveTopWindow();
        }
    }
}
//1050wayland平台上，部分性能差的机型，采用线程循环监听文件（"/home/" + userName + "/.cache/deepin/deepin-screen-recorder/stopRecord.txt"）是否存在且内容是否为1
void MainWindow::checkTempFileArm()
{
    qDebug() << __LINE__ << __func__ ;
    QString userName = QDir::homePath().section("/", -1, -1);
    m_tempPath = ("/home/" + userName + "/.cache/deepin/deepin-screen-recorder/").toStdString();
    //判断文件是否存在，若存在则先删除文件，启动录屏时不应该存在该文件
    std::string tempFile = m_tempPath + "stopRecord.txt";
    QFile mFile(tempFile.c_str());
    if (mFile.exists()) {
        remove(tempFile.c_str());
    }
    qDebug() << __func__;
    QtConcurrent::run(this, &MainWindow::whileCheckTempFileArm);
}
void MainWindow::whileCheckTempFileArm()
{
    bool tempFlag = true;

    while (tempFlag) {
        QDir tdir(m_tempPath.c_str());
        //判断文件夹路径是否存在
        if (tdir.exists()) {
            std::string tempFile = m_tempPath + "stopRecord.txt";
            //打开文件
            int fd = open(tempFile.c_str(), O_RDWR, 0644);
            if (fd == -1) {
                //qDebug() << "open file fail!" << strerror(errno);
                ::close(fd);
                if (tempFlag) {
                    QThread::msleep(500);
                }
                continue;
            }
            //文件加锁
            int flock = lockf(fd, F_TLOCK, 0);
            if (flock == -1) {
                qDebug() << "lock file fail!" << strerror(errno);
                ::close(fd);
                if (tempFlag) {
                    QThread::msleep(500);
                }
                continue;
            }
            ssize_t ret = -1;
            char rBuffer[2];
            memset(rBuffer, 0, 2);
            //读文件
            ret = read(fd, rBuffer, 2);
            if (ret < 0) {
                qDebug() << "read file fail!";
            } else {
                //文件内容为1时会停止录屏
                if (QString(rBuffer).toInt() == 1) {
                    qDebug() << "read file to stop Record!" ;
                    //stopRecord();
                    emit stopRecordArm();
                    tempFlag = false;
                } else {
                    qDebug() << "file: " << rBuffer;
                }
            }
            //文件解锁
            flock = lockf(fd, F_ULOCK, 0);
            ::close(fd);
            qDebug() << "close file!";
            //移除文件
            remove(tempFile.c_str());
            qDebug() << "remove file!";
        }
    }
}
#endif
void MainWindow::checkIsLockScreen()
{
    QDBusInterface sessionManagerIntert("com.deepin.SessionManager",
                                        "/com/deepin/SessionManager",
                                        "com.deepin.SessionManager",
                                        QDBusConnection::sessionBus());
    if (!sessionManagerIntert.isValid()) {
        return;
    }
    bool isLockScreen = sessionManagerIntert.property("Locked").toBool();

    if (isLockScreen) {
        pinScreenshotsLockScreen(isLockScreen);
    }
}

void MainWindow::sendSavingNotify()
{
    if (Utils::isRootUser) {
        return;
    }
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
    if (Utils::isRootUser) {
        return;
    }
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

//初始化应用能快捷键 如果快捷键需要打开下拉列表，则不能使用全局快捷键处理，需使用此方法处理
//下拉列表会影响快捷键
void MainWindow::initShortcut()
{
    //截图模式 贴图 截图应用内快捷键
    QShortcut *pinScreenshotsSC = new QShortcut(QKeySequence("Alt+P"), this);
    //截图模式 滚动截图应用内快捷键
    QShortcut *scrollShotSC = new QShortcut(QKeySequence("Alt+I"), this);
    //截图模式/滚动模式 ocr应用内快捷键
    QShortcut *ocrSC = new QShortcut(QKeySequence("Alt+O"), this);
    //截图模式 矩形
    QShortcut *rectSC = new QShortcut(QKeySequence("R"), this);
    //截图模式 圆形
    QShortcut *ovalSC = new QShortcut(QKeySequence("O"), this);
    //截图模式 箭头
    QShortcut *arrowSC = new QShortcut(QKeySequence("L"), this);
    //截图模式 画笔
    QShortcut *lineSC = new QShortcut(QKeySequence("P"), this);
    //截图模式 文本
    QShortcut *textSC = new QShortcut(QKeySequence("T"), this);
    //截图模式 撤销
    QShortcut *undoSC = new QShortcut(QKeySequence("Ctrl+Z"), this);
    //截图模式 全部撤销
    QShortcut *undoAllSC = new QShortcut(QKeySequence("Ctrl+Shift+Z"), this);
    //录屏模式（未做穿透） 监控键盘
    QShortcut *keyBoardSC = new QShortcut(QKeySequence("K"), this);
    //录屏模式（未做穿透） 摄像头
    QShortcut *cameraSC = new QShortcut(QKeySequence("W"), this);
    //截图模式/滚动模式 保存截图 大键盘的enter
    QShortcut *returnSC = new QShortcut(QKeySequence(Qt::Key_Return), this);
    //截图模式/滚动模式 保存截图 小键盘的enter
    QShortcut *enterSC = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    //截图模式/滚动模式 保存截图
    QShortcut *saveShotSC = new QShortcut(QKeySequence("Ctrl+S"), this);
    //截图模式/录屏模式（未做穿透）/滚动模式 退出
    QShortcut *escSC = new QShortcut(QKeySequence("Escape"), this);
    //截图模式/录屏模式（未做穿透）/滚动模式 帮助快捷面板
    QShortcut *shortCutSC = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Slash), this);
    //截图模式/滚动模式 贴图应用内快捷键
    connect(pinScreenshotsSC, &QShortcut::activated, this, [ = ] {
        //滚动截图及普通截图都可以通过快捷键触发贴图
        if (status::shot == m_functionType && Utils::is3rdInterfaceStart == false)
        {
            qDebug() << "shortcut : pinScreenshotsSC (key: alt+p)";
            m_toolBar->shapeClickedFromMain("pinScreenshots");
        }
    });
    //截图模式 滚动截图应用内快捷键
    connect(scrollShotSC, &QShortcut::activated, this, [ = ] {
        // 当第三方接口启动时，不触发快捷键
        if (status::shot == m_functionType && Utils::is3rdInterfaceStart == false)
        {
            qDebug() << "shortcut : scrollShotSC (key: alt+i)";
            m_toolBar->shapeClickedFromMain("scrollShot");
        }
    });
    //截图模式/滚动模式 ocr应用内快捷键
    connect(ocrSC, &QShortcut::activated, this, [ = ] {
        //滚动截图及普通截图都可以通过快捷键触发ocr
        if ((status::shot == m_functionType || status::scrollshot == m_functionType) && Utils::is3rdInterfaceStart == false)
        {
            qDebug() << "shortcut : ocrSC (key: alt+o)";
            m_toolBar->shapeClickedFromMain("ocr");
        }
    });
    //截图模式 矩形
    connect(rectSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            qDebug() << "shortcut : rectSC (key: r)";
            m_toolBar->shapeClickedFromMain("rect");
        }
        if (status::record == m_functionType && Utils::isWaylandMode)
            m_showButtons->showContentButtons(KEY_R);
    });
    //截图模式 圆形
    connect(ovalSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            qDebug() << "shortcut : ovalSC (key: o)";
            m_toolBar->shapeClickedFromMain("circ");
        }
        if (status::record == m_functionType && Utils::isWaylandMode)
            m_showButtons->showContentButtons(KEY_O);
    });
    //截图模式 箭头
    connect(arrowSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            qDebug() << "shortcut : arrowSC (key: l)";
            m_toolBar->shapeClickedFromMain("line");
        }
        if (status::record == m_functionType && Utils::isWaylandMode)
            m_showButtons->showContentButtons(KEY_L);
    });
    //截图模式 画笔
    connect(lineSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            qDebug() << "shortcut : lineSC (key: p)";
            m_toolBar->shapeClickedFromMain("pen");
        }
        if (status::record == m_functionType && Utils::isWaylandMode)
            m_showButtons->showContentButtons(KEY_P);
    });
    //截图模式 文本
    connect(textSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            qDebug() << "shortcut : textSC (key: t)";
            m_toolBar->shapeClickedFromMain("text");
        }
        if (status::record == m_functionType && Utils::isWaylandMode)
            m_showButtons->showContentButtons(KEY_T);
    });
    //截图模式 撤销
    connect(undoSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            qDebug() << "shortcut : undoSC (key: ctrl+z)";
            emit unDo();
        }
    });
    //截图模式 全部撤销
    connect(undoAllSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType)
        {
            qDebug() << "shortcut : undoAllSC (key: ctrl+shift+z)";
            emit unDoAll();
        }
    });
    //录屏模式（未做穿透） 监控键盘
    connect(keyBoardSC, &QShortcut::activated, this, [ = ] {
        if (status::record == m_functionType && RECORD_BUTTON_NORMAL == recordButtonStatus)
        {
            qDebug() << "shortcut : keyBoardSC (key: k)";
            m_toolBar->shapeClickedFromMain("keyBoard");
        }
        if (status::record == m_functionType && Utils::isWaylandMode)
            m_showButtons->showContentButtons(KEY_K);
    });
    //录屏模式（未做穿透） 摄像头
    connect(cameraSC, &QShortcut::activated, this, [ = ] {
        if (status::record == m_functionType && RECORD_BUTTON_NORMAL == recordButtonStatus)
        {
            qDebug() << "shortcut : cameraSC (key: w)";
            m_toolBar->shapeClickedFromMain("camera");
        }
        if (status::record == m_functionType && Utils::isWaylandMode)
            m_showButtons->showContentButtons(KEY_W);
    });
    //截图模式/滚动模式 保存截图 大键盘
    connect(returnSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType || status::scrollshot == m_functionType)
        {
            qDebug() << "shortcut : returnSC (key: enter)";
            saveScreenShot();
        }
    });
    //截图模式/滚动模式 保存截图 小键盘
    connect(enterSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType || status::scrollshot == m_functionType)
        {
            qDebug() << "shortcut : enterSC (key: enter)";
            saveScreenShot();
        }
        if (status::record == m_functionType && Utils::isWaylandMode)
            m_showButtons->showContentButtons(KEY_ENTER);
    });
    //截图模式/滚动模式 保存截图
    connect(saveShotSC, &QShortcut::activated, this, [ = ] {
        if (status::shot == m_functionType || status::scrollshot == m_functionType)
        {
            qDebug() << "shortcut : saveShotSC (key: ctrl+s)";
            saveScreenShot();
        }
    });
    //截图模式/录屏模式（未做穿透）/滚动模式 退出
    connect(escSC, &QShortcut::activated, this, [ = ] {
        //只有正在录屏时，此快捷键无法退出程序
        if (RECORD_BUTTON_RECORDING != recordButtonStatus)
        {
            qDebug() << "shortcut : escSC (key: esc)";
            exitApp();
        }
        if (status::record == m_functionType && Utils::isWaylandMode)
            m_showButtons->showContentButtons(KEY_ESCAPE);
    });
    //截图模式/录屏模式（未做穿透）/滚动模式）帮助面板
    shortCutSC->setAutoRepeat(false);
    connect(shortCutSC, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : helpSC (key: ctrl+shift+?)";
        onViewShortcut();
    });
    if (BaseUtils::isCommandExist("dman")) {
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
        if ((status::scrollshot == m_functionType) || (status::shot == m_functionType) || (status::record == m_functionType && RECORD_BUTTON_RECORDING != recordButtonStatus)) {
            exitApp();
        }
    } else {
        qWarning() << "manual service not available, cannot open manual";
    }
}

void MainWindow::initResource()
{
    m_showButtons = new ShowButtons(this);
    connect(m_showButtons, SIGNAL(keyShowSignal(const QString &)), this, SLOT(showKeyBoardButtons(const QString &)));
    resizeHandleBigImg = DHiDPIHelper::loadNxPixmap(":/newUI/normal/node.svg");
    buttonFeedback = new ButtonFeedback();

    m_initResource = true;
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
        exitScreenCuptureEvent();
    });
    m_functionType = 1;
    m_keyBoardStatus = false;
    m_mouseStatus = false;
    //m_multiKeyButtonsInOnSec = false;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    m_screenWidth = m_backgroundRect.width();
    m_screenHeight = m_backgroundRect.height();

    m_shotStatus = ShotMouseStatus::Normal;


    isPressButton = false;
    isReleaseButton = false;


    recordButtonStatus = RECORD_BUTTON_NORMAL;

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

        //m_recordButton->hide();
        //m_shotButton->hide();
        m_sizeTips->hide();
    }

    else {
        m_toolBar->show();
        m_sideBar->hide();

        //m_recordButton->hide();
        //m_shotButton->show();
        m_sizeTips->show();


        updateToolBarPos();
        //updateShotButtonPos();
        m_sizeTips->setRecorderTipsInfo(false);
        m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
    }

    //初始化ocr
    m_ocrInterface = nullptr;
    m_toolBar->setFocus();
}

//初始化录屏窗口
void MainWindow::initScreenRecorder()
{
    if (!m_pScreenCaptureEvent)
        return;

    m_functionType = status::record;
    m_keyBoardStatus = false;
    m_mouseStatus = false;
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

    if (m_isShapesWidgetExist) {
        m_shapesWidget->hide();
    }

    m_isShapesWidgetExist = false;
    //m_needDrawSelectedPoint = false;


    //构建截屏工具栏按钮 by zyg
    if (m_firstShot == 0) {
        m_toolBar->hide();
        m_sideBar->hide();

        //m_recordButton->hide();
        //m_shotButton->hide();
        m_sizeTips->hide();
    }

    else {
        m_toolBar->show();
        m_sideBar->hide();

        //m_recordButton->show();
        //m_shotButton->hide();
        m_sizeTips->show();

        updateToolBarPos();
        //updateRecordButtonPos();
        m_sizeTips->setRecorderTipsInfo(true);
        m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
    }

    //recordButton->hide();
    //recordOptionPanel->hide();


    m_zoomIndicator->hide();
    //录屏初次进来此字段为false，后面进来此字段为ture故不会改变默认框选区域大小
    if (!m_initScreenRecorder) {
        m_initScreenRecorder = true;
    } else {
        return;
    }

    m_toolBar->setFocus();
}

//滚动截图的初始化函数
void MainWindow::initScrollShot()
{
#ifdef OCR_SCROLL_FLAGE_ON
    if (m_initScroll)
        return;
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
    m_mouseStatus = false;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    m_screenWidth = m_backgroundRect.width();
    m_screenHeight = m_backgroundRect.height();
    isPressButton = false;
    isReleaseButton = false;

    //隐藏工具栏矩形、圆形、箭头、笔画、选项中裁切选项-显示光标
    if (!m_isDirectStartScrollShot) {
        m_toolBar->hideSomeToolBtn();
    }
    update();

    //捕捉区域不能进行拖动
    recordButtonStatus = RECORD_BUTTON_WAIT;

    //重新设置鼠标形状
    resetCursor();

    //先将捕捉区域设置为穿透状态
    setInputEvent();

    m_toolBar->hide();
    //m_shotButton -> hide();
    //隐藏截图模式下左上角提示的图片大小
    m_sizeTips->hide();

    //滚动预览开启初始化
    if (m_previewWidget == nullptr) {
        QRect previewRecordRect {
            static_cast<int>(recordX),
            static_cast<int>(recordY),
            static_cast<int>(recordWidth),
            static_cast<int>(recordHeight)
        };
        m_previewWidget = new PreviewWidget(previewRecordRect, this);
        m_previewWidget->setScreenInfo(m_screenWidth, m_pixelRatio);
        m_previewWidget->initPreviewWidget();
        //此处只是显示预览框的位置及大小，预览框里面还未添加第一张预览图
        m_previewWidget->show();
        //防止预览区域在捕捉区域内部时，遮挡工具栏及保存按钮
        m_previewWidget->lower();
    }
    //获取预览框相对于捕捉区域的位置
    m_previewPostion = m_previewWidget->getPreviewPostion();

    //提示开始滚动截图的方法
    m_scrollShotTip = new ScrollShotTip(this);
    //链接拼接失败提示，点击打开帮助
    connect(m_scrollShotTip, &ScrollShotTip::openScrollShotHelp, this, &MainWindow::onOpenScrollShotHelp);
    //链接拼接失败，点击自动调整捕捉区域
    connect(m_scrollShotTip, &ScrollShotTip::adjustCaptureArea, this, &MainWindow::onAdjustCaptureArea);
    //选择提示类型
    m_scrollShotTip->showTip(TipType::StartScrollShotTip);
    m_scrollShotTip->setBackgroundPixmap(m_backgroundPixmap);
    //根据工具栏获取滚动截图提示框的坐标
    QPoint tipPosition = getScrollShotTipPosition();
    //提示信息移动到指定位置
    m_scrollShotTip->move(tipPosition);
    //滚动截图的处理类
    m_scrollShot = new ScrollScreenshot(this);
    qRegisterMetaType<PixMergeThread::MergeErrorValue>("MergeErrorValue");
    //链接滚动拼接过程显示预览窗口和图片
    connect(m_scrollShot, &ScrollScreenshot::updatePreviewImg, this, &MainWindow::showPreviewWidgetImage);
    //链接自动滚动模式,如果进行模拟了自动滚动鼠标事件将会触发此槽函数
    connect(m_scrollShot, SIGNAL(autoScroll(int)), this, SLOT(onScrollShotCheckScrollType(int)));
    m_autoScrollFlagLast = m_autoScrollFlagNext;
    //链接滚动截图抓取当前捕捉区图片进行图片拼接
    connect(m_scrollShot, &ScrollScreenshot::getOneImg, this, [ = ] {
        //自动滚动截图模式，抓取当前捕捉区域的图片，传递给滚动截图处理类进行图片的拼接
        scrollShotGrabPixmap(m_previewPostion, 5);
    });

    //链接滚动截图拼接过程中返回的错误状态
    connect(m_scrollShot, SIGNAL(merageError(PixMergeThread::MergeErrorValue)), this, SLOT(onScrollShotMerageImgState(PixMergeThread::MergeErrorValue)));
    //滚动截图左上角当前图片的大小及位置
    m_scrollShotSizeTips = new TopTips(this);
    m_scrollShotSizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
    m_scrollShotSizeTips->hide();


#if defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__)
    static int delayTime = 260;
#elif defined (__aarch64__)
    static int delayTime = 220;
#else
    static int delayTime = 100;
#endif

    //工具栏、保存按钮、预览框在捕捉区域内部需对工具栏、保存按钮、预览框及提示延时显示
    if (isToolBarInShotArea()) {
        //延时100ms之后使预览窗口显示第一张预览图，此时为了保证第一张预览图中不包含工具栏、保存按钮及提示
        QTimer::singleShot(delayTime, this, [ = ] {
            showScrollShot();
        });
    }
    //工具栏、保存按钮、预览框不在捕捉区域内部
    else {
        qDebug() << "isToolBarInShotArea false";
        showScrollShot();
    }
    //定时2s后滚动截图的提示消失
    m_tipShowtimer = new QTimer(this);
    connect(m_tipShowtimer, &QTimer::timeout, this, [ = ]() {
        m_tipShowtimer->stop();
        m_scrollShotTip->hide();
        //可调整的捕捉区域消失
        m_isAdjustArea = false;
        //滚动截图：自动调整捕捉区域错误被解决
        m_isErrorWithScrollShot = false;
        update();
        //qDebug() << "提示已消失！" ;

    });
    m_tipShowtimer->setInterval(2000);
    m_initScroll = true;
#endif
}

//根据工具栏获取滚动截图提示框的坐标
QPoint MainWindow::getScrollShotTipPosition()
{
#ifdef OCR_SCROLL_FLAGE_ON
    //const QPoint topLeft = geometry().topLeft();
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    int leftTopX = 0, leftTopY = 0;
    int screenWidth = 0, screenHeight = 0;
//    int toolbarX = static_cast<int>(m_toolBar->x() * m_pixelRatio);
    int toolbarY = static_cast<int>(m_toolBar->y() * m_pixelRatio);
//    int toolbarWidth = static_cast<int>(m_toolBar->width() * m_pixelRatio);
    int toolbarHeight = static_cast<int>(m_toolBar->height() * m_pixelRatio);
    //qDebug() << "toolbarX: " << toolbarX << ",toolbarY: " <<toolbarY << "toolbarWidth: " << toolbarWidth << ",toolbarHeight: " << toolbarHeight;
    //qDebug() << "recordRect.x(): " << recordRect.x() << ",recordRect.y(): " << recordRect.y() << "recordRect.width(): " << recordRect.width() << ",recordRect.height(): " << recordRect.height();

    //获取捕捉区域所在的屏幕
//    screenWidth = static_cast<int>(m_screenWidth * m_pixelRatio) / m_screenCount; //此方法对于复制屏幕的计算会出现问题
    screenWidth = static_cast<int>(m_screenWidth * m_pixelRatio);
    screenHeight = static_cast<int>(m_screenHeight * m_pixelRatio);
    QRect currnetScreenRect;
    if (m_screenCount == 1) {
        screenWidth = static_cast<int>(m_screenWidth * m_pixelRatio);
        screenHeight = static_cast<int>(m_screenHeight * m_pixelRatio);
    } else if (m_screenCount > 1) {
        qDebug() << "m_screenInfo.size(): " << m_screenInfo.size();
        for (int i = 0; i < m_screenInfo.size(); ++i) {
            currnetScreenRect = {
                static_cast<int>(m_screenInfo[i].x),
                static_cast<int>(m_screenInfo[i].y),
                static_cast<int>(m_screenInfo[i].width),
                static_cast<int>(m_screenInfo[i].height)
            };
            if (currnetScreenRect.contains(QPoint(recordRect.x(), recordRect.y()))) {
                screenWidth = static_cast<int>(m_screenInfo[i].width);
                screenHeight = static_cast<int>(m_screenInfo[i].height);
                break;
            }
        }
    }

    //捕捉区域的宽小于300或者高小于100 则提示内容在屏幕中间且与捕捉区域左上角在一个屏幕
    if (recordRect.width() < 300 || recordRect.height() < 100) {
        leftTopX = static_cast<int>(currnetScreenRect.x() + (screenWidth - m_scrollShotTip->width() * m_pixelRatio) / 2);
        leftTopY = static_cast<int>(currnetScreenRect.y() + (screenHeight - m_scrollShotTip->height() * m_pixelRatio) / 2);
    } else {
        leftTopX = static_cast<int>((recordRect.x()  + (recordRect.width()  - m_scrollShotTip->width() * m_pixelRatio) / 2));
        //工具栏在捕捉区域内部 ,判断工具栏的四个点是否在内部
        if (isToolBarInShotArea()) {
            //leftTopY = static_cast<int>((recordRect.y() * m_pixelRatio + (recordRect.height() * m_pixelRatio - m_scrollShotTip->height()) / 100 * 97));
            leftTopY = static_cast<int>(toolbarY + toolbarHeight + 15 * m_pixelRatio);
        } else {
            //工具栏在捕捉区域下,且在捕捉区域外部
            if (toolbarY > recordRect.y()) {
                leftTopY = static_cast<int>(toolbarY - m_scrollShotTip->height() * m_pixelRatio - 15 * m_pixelRatio);
            }
            //工具栏在捕捉区域上,且在捕捉区域外部
            else {
                leftTopY = static_cast<int>(toolbarY + toolbarHeight + 15 * m_pixelRatio);
            }
        }
        //qDebug() << "leftTopX: " << leftTopX << ",leftTopY: " <<leftTopY;
    }

    return QPoint(static_cast<int>(leftTopX / m_pixelRatio), static_cast<int>(leftTopY / m_pixelRatio));
#else
    return QPoint(0, 0);
#endif
}

//初始化滚动截图时，显示滚动截图中的一些公共部件、例如工具栏、提示、图片大小、第一张预览图
void MainWindow::showScrollShot()
{
#ifdef OCR_SCROLL_FLAGE_ON
    bool ok;
    QRect rect(recordX + 1, recordY + 1, recordWidth - 2, recordHeight - 2);
    //滚动截图截取指定区域的第一张图片
    m_firstScrollShotImg = m_screenGrabber.grabEntireDesktop(ok, rect, m_pixelRatio);
    //m_firstScrollShotImg.save("m_firstScrollShotImg1.png");
    //预览区域显示当前指定区域的第一张图片
    m_previewWidget->updateImage(m_firstScrollShotImg.toImage());
    m_previewWidget->show();
    //打开工具栏显示 需放在更新工具栏之前，避免出现工具栏没显示但是已经执行位置更新
    m_toolBar->show();
    //打开截图保存按钮显示
    //m_shotButton->show();
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
            //updateShotButtonPos();
        }
    });
#endif
}

//处理手动滚动截图逻辑
void MainWindow::handleManualScrollShot(int mouseTime, int direction)
{
#ifdef OCR_SCROLL_FLAGE_ON
    qDebug() << "function: " << __func__ << " ,line: " << __LINE__;
    if (m_tipShowtimer != nullptr) {
        m_tipShowtimer->stop();
    }
    m_scrollShotTip->hide();
    m_isAdjustArea = false;
    update();
    static int num = 1;
    ++num;
    if (num % 3 == 0) {
        //滚动截图模式，抓取当前捕捉区域的图片，传递给滚动截图处理类进行图片的拼接
        scrollShotGrabPixmap(m_previewPostion, direction, mouseTime);
        num = 0;
    }
#endif
}

//显示可调整的捕捉区域大小及位置
void MainWindow::showAdjustArea()
{
#ifdef OCR_SCROLL_FLAGE_ON
    //获取可调整的捕捉区域大小及位置
    QRect adjustArea = m_scrollShot->getInvalidArea();
    //根据返回的可调整区域计算出在屏幕中的可调整区域位置
//    m_adjustArea = QRect(
//                       adjustArea.x() + recordX,
//                       adjustArea.y() + recordY,
//                       adjustArea.width(),
//                       adjustArea.height()
//                   );
    m_adjustArea = QRect(
                       static_cast<int>((adjustArea.x() / m_pixelRatio  + recordX)),
                       static_cast<int>((adjustArea.y() / m_pixelRatio + recordY)),
                       static_cast<int>(adjustArea.width() / m_pixelRatio),
                       static_cast<int>(adjustArea.height() / m_pixelRatio)
                   );
    update();
#endif
}
#ifdef OCR_SCROLL_FLAGE_ON
//滚动截图模式，抓取当前捕捉区域的图片，传递给滚动截图处理类进行图片的拼接
void MainWindow::scrollShotGrabPixmap(PreviewWidget::PostionStatus previewPostion, int direction, int mouseTime)
{

//不同的平台延时时间不同
#if defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__)
    static int delayTime = 130;
#elif defined (__aarch64__)
    static int delayTime = 100;
#else
    static int delayTime = 50;
#endif
    qDebug() << QSysInfo::currentCpuArchitecture() << delayTime;
    //滚动截图处理类：设置滚动截图的模式
    if (ScrollShotType::AutoScroll == m_scrollShotType) {
        m_scrollShot->setScrollModel(false);
    } else if (ScrollShotType::ManualScroll == m_scrollShotType) {
        m_scrollShot->setTimeAndCalculateTimeDiff(mouseTime);
        m_scrollShot->setScrollModel(true);
    }
    //判断工具栏是否在捕捉区域内部
    if (isToolBarInShotArea()) {
        qDebug() << __FUNCTION__ << __LINE__;
        //工具栏、保存截图按钮先隐藏在显示，防止出现的预览图中包含工具栏
        m_toolBar->hide();
        //m_shotButton->hide();
        m_scrollShotSizeTips->hide();
    }
    //qDebug() << "function: " << __func__ << " ,line: " << __LINE__;
    //判断预览框是否在捕捉区域内部，如果是在捕捉区域内部，则每次截图前先隐藏预览框，并延时30ms，在进行截图
    if (PreviewWidget::PostionStatus::INSIDE == previewPostion) {
        if (m_previewWidget) {
            m_previewWidget->hide();
        }
        QTimer::singleShot(delayTime, this, [ = ] {
            //只要是滚动模式都会进入此处来处理图片
            bool ok;
            QRect rect(recordX + 1, recordY + 1, recordWidth - 2, recordHeight - 2);
            //抓取捕捉区域图片
            QPixmap img = m_screenGrabber.grabEntireDesktop(ok, rect, m_pixelRatio);
            //滚动截图处理类进行图片的拼接
            m_scrollShot->addPixmap(img, direction);
            if (m_previewWidget)
            {
                m_previewWidget->show();
            }
        });
    } else {
        //之所以此延时需要单独放在这里是由于前面预览图已经做了一个延时，此时在进行延时，会导致，预览图延时的时间太长
        //判断工具栏是否在捕捉区域内部,在捕捉区域内部时需延时，才能保证截图时不将它们截如图像中
        if (isToolBarInShotArea()) {
            QEventLoop eventloop1;
            QTimer::singleShot(delayTime, &eventloop1, SLOT(quit()));
            eventloop1.exec();
        }
        bool ok;
        QRect rect(recordX + 1, recordY + 1, recordWidth - 2, recordHeight - 2);
        //抓取捕捉区域图片
        QPixmap img = m_screenGrabber.grabEntireDesktop(ok, rect, m_pixelRatio);
        //滚动截图处理类进行图片的拼接
        m_scrollShot->addPixmap(img, direction);
    }
    //判断工具栏是否在捕捉区域内部
    if (isToolBarInShotArea()) {
        //工具栏、保存截图按钮显示，防止出现的预览图中包含工具栏
        m_toolBar->show();
        //m_shotButton->show();
        m_scrollShotSizeTips->show();
    }
}
#endif
//判断工具栏是否在在捕捉区域内部
bool MainWindow::isToolBarInShotArea()
{
    const QPoint topLeft = geometry().topLeft();
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio + topLeft.x()),
        static_cast<int>(recordY * m_pixelRatio + topLeft.y()),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    int toolbarY = static_cast<int>(m_toolBar->y() * m_pixelRatio);
    int toolbarHeight = static_cast<int>(m_toolBar->height() * m_pixelRatio);
    if (recordRect.y() <= toolbarY && ((recordRect.y() + recordRect.height()) >= (toolbarY + toolbarHeight)))
        return true;
    return false;
}


//显示预览窗口和图片
void MainWindow::showPreviewWidgetImage(QImage img)
{
#ifdef OCR_SCROLL_FLAGE_ON
    m_scrollShotSizeTips ->updateTips(QPoint(recordX, recordY), QSize(int(img.width() / m_pixelRatio + 2), int(img.height() / m_pixelRatio + 2)));
    m_previewWidget->updateImage(img);
#endif
}

void MainWindow::initLaunchMode(const QString &launchMode)
{
    m_functionType = status::shot;
    if (launchMode == "screenRecord") {
        m_sizeTips->setRecorderTipsInfo(true);
        m_sizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
        m_functionType = status::record;
        initScreenRecorder();
        if (m_sideBar->isVisible()) {
            m_sideBar->hide();
        }
    } else if (launchMode == "screenOcr") {
        m_functionType = status::ocr;
        m_isDirectStartOcr = true;
    } else if (launchMode == "screenScroll") {
        m_functionType = status::scrollshot;
        m_isDirectStartScrollShot = true;
    } else {
        m_functionType = status::shot;
        initScreenShot();
    }
}

void MainWindow::fullScreenshot()
{
    //DDesktopServices::playSystemSoundEffect(DDesktopServices::SEE_Screenshot);
    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();
    repaint();
    qApp->setOverrideCursor(BaseUtils::setCursorShape("start"));
    //    initDBusInterface();
    this->setFocus();
    //    m_configSettings =  ConfigSettings::instance();
    installEventFilter(this);

    // 多屏截取全屏

//    if (m_screenCount == 1) {
//        m_backgroundRect = QApplication::desktop()->screen()->geometry();
//        m_backgroundRect = QRect(m_backgroundRect.topLeft(), m_backgroundRect.size());
//    } else if (m_screenCount > 1) {
    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    m_backgroundRect = t_primaryScreen->virtualGeometry();;
    m_backgroundRect = QRect(m_backgroundRect.topLeft(), m_backgroundRect.size());
//    }
    //
    this->move(m_backgroundRect.x(), m_backgroundRect.y());
    this->setFixedSize(m_backgroundRect.size());
    m_needSaveScreenshot = true;

//    m_toolBar = new ToolBar(this);
//    m_toolBar->hide();

    shotFullScreen(true);
    TempFile::instance()->setFullScreenPixmap(m_resultPixmap);
    const auto r = saveAction(m_resultPixmap);
    save2Clipboard(m_resultPixmap);
    sendNotify(m_saveIndex, m_saveFileName, r);
}
void MainWindow::topWindow()
{
    //DDesktopServices::playSystemSoundEffect(DDesktopServices::SEE_Screenshot);
    //wayland模式下截顶层窗口 需提前设置此属性，保证wayland执行回调函数时可以获取到顶层窗口
    if (Utils::isWaylandMode) {
        m_isFullScreenShot = true;
    }
    this->initAttributes();
    this->initLaunchMode("screenShot");
    this->showFullScreen();
    this->initResource();

    //wayland 模式下不进入以下步骤
    if (Utils::isWaylandMode) {
        return;
    }
    int t_windowCount = DWindowManagerHelper::instance()->allWindowIdList().size();
    DForeignWindow *prewindow = nullptr;
    for (int i = t_windowCount - 1; i >= 0; i--) {
        auto wid = DWindowManagerHelper::instance()->allWindowIdList().at(i);
        if (wid == winId()) continue;
        if (prewindow && Utils::isSysHighVersion1040()) {
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
    if (prewindow && Utils::isSysHighVersion1040()) {
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

    //this->hide();
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
    save2Clipboard(screenShotPix);
    sendNotify(m_saveIndex, m_saveFileName, r);
}

void MainWindow::saveTopWindow()
{
    int topWindowIndex = windowRects.size() - 2;
    if (topWindowIndex < 0) {
        topWindowIndex = 0;
    }
    selectAreaName = windowNames[topWindowIndex];
    recordX = windowRects[topWindowIndex].x();
    recordY = windowRects[topWindowIndex].y();
    recordWidth = windowRects[topWindowIndex].width();
    recordHeight = windowRects[topWindowIndex].height();

    //this->hide();
    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    qDebug() << ratio << recordX << recordY << recordWidth << recordHeight;
    QRect target(static_cast<int>(recordX * ratio),
                 static_cast<int>(recordY * ratio),
                 static_cast<int>(recordWidth * ratio),
                 static_cast<int>(recordHeight * ratio));
//    using namespace utils;
    QPixmap screenShotPix =  m_backgroundPixmap.copy(target);
    qDebug() << "topWindow grabImage is null:" << m_backgroundPixmap.isNull()
             << QRect(recordX, recordY, recordWidth, recordHeight)
             << "\n"
             << "screenShot is null:" << screenShotPix.isNull();
    m_needSaveScreenshot = true;
    //    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Screenshot);
    save2Clipboard(screenShotPix);
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
    QRect target = m_backgroundRect;
    if (Utils::isWaylandMode) {
        target = QRect(m_backgroundRect.x(),
                       m_backgroundRect.y(),
                       static_cast<int>(m_backgroundRect.width() * m_pixelRatio),
                       static_cast<int>(m_backgroundRect.height() *  m_pixelRatio));
    }

    m_backgroundPixmap = getPixmapofRect(target);
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
    if (status::pinscreenshots == m_functionType) return false;
    return pix.save(fileName, format, quality);
}

void MainWindow::save2Clipboard(const QPixmap &pix)
{
    if (pix.isNull()) {
        qDebug() << __FUNCTION__ << "Copy Null Pix To Clipboard!";
        return;
    }
    if (Utils::is3rdInterfaceStart == false) {
        QMimeData *t_imageData = new QMimeData;
        t_imageData->setImageData(pix);
        Q_ASSERT(!pix.isNull());
        QClipboard *cb = qApp->clipboard();
        cb->setMimeData(t_imageData, QClipboard::Clipboard);
    }
}

bool MainWindow::checkSuffix(const QString &str)
{
    int index = str.lastIndexOf(".");
    qDebug() << "index: " << index;
    if (-1 == index) {
        return false;
    }
    QString rightStr = str.right(str.length() - index);
    if (rightStr == ".png" || rightStr == ".jpg" || rightStr == ".bmp" || rightStr == ".jpeg") {
        return true;
    }
    qDebug() << "The rightStr : " << rightStr;
    return false;
}

// waland手动滚动处理逻辑
void MainWindow::wheelEvent(QWheelEvent *event)
{
#ifdef OCR_SCROLL_FLAGE_ON
    qDebug() << __FUNCTION__ << __LINE__ << "m_functionType " << m_functionType;
    if (Utils::isWaylandMode == false || (m_initScroll && status::scrollshot != m_functionType))
        return;
    int x = this->cursor().pos().x();
    int y = this->cursor().pos().y();

    //将当前捕捉区域画为一个矩形
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    //当前鼠标的点
    QPoint mouseMovePoint(x, y);
    //判断当鼠标位置是否在捕捉区域内部,不在捕捉区域内则暂停自动滚动
    if (!recordRect.contains(mouseMovePoint))
        return;
    if (m_scrollShot) {
        int time = int (QDateTime::currentDateTime().toTime_t());
        float len = (event->delta() > 15.0) ? -15.0 : 15.0; // 获取滚轮方向
        int direction = (fabs(double(len) - 15.0) <= EPSILON) ? 5 : 4; // 获取滚轮方向
        scrollShotMouseScrollEvent(time, direction, x, y);
        m_scrollShot->sigalWheelScrolling(len);
        qDebug() << __FUNCTION__ << __LINE__;
    }
#endif
}

void MainWindow::pinScreenshotsLockScreen(bool isLocked)
{
    m_toolBar->setPinScreenshotsEnable(!isLocked);
}

void MainWindow::scrollShotLockScreen(bool isLocked)
{
    //锁屏时暂停自动滚动
    if (isLocked) {
        m_scrollShotStatus = 3;
        //暂停自动滚动截图
        pauseAutoScrollShot();
    }
    //解锁时恢复滚动
    //else {
    //continueScrollShot();
    //}
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

    //判断工具栏位置是否在捕捉区域内部
    if (isToolBarInShotArea()) {
//        qDebug() << "function:" << __func__ << " ,line: " << __LINE__ << " 工具栏位置在捕捉区域内部!";
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

//滚动截图时取消捕捉区域的鼠标穿透
void MainWindow::setCancelInputEvent()
{
    //将当前捕捉区域画为一个矩形
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    //取消捕捉区域穿透
    Utils::cancelInputEvent(static_cast<int>(this->winId()),
                            static_cast<short>(this->x()),
                            static_cast<short>(this->y()),
                            static_cast<unsigned short>(this->width() * m_pixelRatio),
                            static_cast<unsigned short>(this->height() * m_pixelRatio));
}

void MainWindow::showPressFeedback(int x, int y)
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING && m_mouseStatus) {
        buttonFeedback->showPressFeedback(x, y);
    }
}

void MainWindow::showDragFeedback(int x, int y)
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING && m_mouseStatus) {
        buttonFeedback->showDragFeedback(x, y);
    }
}

void MainWindow::showReleaseFeedback(int x, int y)
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING && m_mouseStatus) {
        buttonFeedback->showReleaseFeedback(x, y);
    }
}

void MainWindow::responseEsc()
{
    if (status::record == m_functionType && RECORD_BUTTON_RECORDING != recordButtonStatus) {
        exitApp();
    }
}


void MainWindow::compositeChanged()
{

    // 滚动截图过程中动态切换为2D模式，直接结束
    if (m_functionType == status::shot) {
        m_toolBar->setScrollShotDisabled(!m_wmHelper->hasComposite());
        return;
    }
    if (!m_wmHelper->hasComposite() && m_functionType == status::scrollshot) {
        saveScreenShot();
        return;
    }

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
            exitApp();
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
        m_toolBar->setRecordLaunchMode(m_functionType);
        //m_toolBar->setIsZhaoxinPlatform(m_isZhaoxin);
        m_toolBar->setScrollShotDisabled(!m_wmHelper->hasComposite());

        m_pVoiceVolumeWatcher = new voiceVolumeWatcher(this);
        m_pVoiceVolumeWatcher->setWatch(true); //取消之前的线程方式，采用定时器监测
        connect(m_pVoiceVolumeWatcher, SIGNAL(sigRecodeState(bool)), m_toolBar, SLOT(setMicroPhoneEnable(bool)));
        m_toolBar->setSystemAudioEnable(m_pVoiceVolumeWatcher->getystemAudioState());
        m_toolBarInit = true;

        m_pCameraWatcher = new CameraWatcher(this);
        m_pCameraWatcher->setWatch(true); //取消之前的线程方式，采用定时器监测
        connect(m_pCameraWatcher, SIGNAL(sigCameraState(bool)), this, SLOT(on_CheckVideoCouldUse(bool)));

        //检测是否是锁频状态下再打开截图
        checkIsLockScreen();
    }

    QPoint toolbarPoint;
    m_repaintMainButton = false;
    m_repaintSideBar = false;
    toolbarPoint = QPoint(recordX + recordWidth - m_toolBar->width(),
                          std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));

    if (toolbarPoint.x() <= 0) {
        m_repaintMainButton = true;
        toolbarPoint.setX(recordX);
        if (recordX + m_toolBar->width() > m_backgroundRect.width()) {

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

                int curCurScreenH = m_screenHeight;
                if (m_screenCount > 1) {
                    int i = 0;
                    for (; i < m_screenCount; ++i) {
                        if (m_screenInfo[i].x <= sidebarPoint.x() && m_screenInfo[i].x + m_screenInfo[i].width >= sidebarPoint.x()) {
                            break;
                        }
                    }
                    curCurScreenH = m_screenInfo[i].height;
                }
                if (sidebarPoint.y() + m_sideBar->height() > curCurScreenH) {
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
                    sidebarPoint.setY(recordY - m_sideBar->height() - TOOLBAR_Y_SPACING - m_toolBar->height());
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
/*
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
*/
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
        //m_shotButton->hide();
        //updateRecordButtonPos();
        //m_recordButton->show();
        m_functionType = status::record;
        updateToolBarPos();
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
        //m_recordButton->hide();
        //updateShotButtonPos();
        //m_shotButton->show();
        m_functionType = status::shot;
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
    m_mouseStatus = checked;
}


void MainWindow::changeCameraSelectEvent(bool checked)
{
    //m_recordButton->setEnabled(false);
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
    //m_recordButton->setEnabled(true);
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
        int delayTime = 0;
        //直接通过快捷键启动时需要延时100ms再进行保存，防止未获取到选择区域的名称
        if (m_isDirectStartOcr) {
            delayTime = 100;
        }
        QTimer::singleShot(delayTime, this, [ = ] {
            saveScreenShot();
        });
    } else if (func == "pinScreenshots") {
        m_functionType = status::pinscreenshots;
        m_pinInterface = new PinScreenShotsInterface("com.deepin.PinScreenShots", "/com/deepin/PinScreenShots", QDBusConnection::sessionBus(), this);
        //保存贴图到剪贴板
        saveScreenShot();
        m_pinInterface->openImageAndName(m_resultPixmap.toImage(), m_saveFileName, QPoint(recordX, recordY));
        exitApp();
    } else if (func == "scrollShot") { //点击滚动截图
        //捕捉区域的固件不显示
        drawDragPoint = false;
        repaint();
        //延时100ms防止预览款将捕捉区域的骨架截取到图片中
        QTimer::singleShot(100, this, [ = ] {
            //初始化滚动截图
            initScrollShot();
        });

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
        m_toolBar->setScrollShotDisabled(true);

    }
}

void MainWindow::saveScreenShot()
{

    if (m_pScreenCaptureEvent) {

        if (Utils::isWaylandMode) {
            m_CuresorImageWayland = m_pScreenCaptureEvent->getCursorImageWayland();
        } else {
            m_CursorImage = m_pScreenCaptureEvent->getCursorImage();
        }
    }
    if (status::scrollshot != m_functionType)
        m_shotflag = 1;
    emit saveActionTriggered();
    hideAllWidget();

    m_toolBar->setVisible(false);
    m_sizeTips->setVisible(false);
    m_sideBar->setVisible(false);
    //m_shotButton->setVisible(false);
    //m_recordButton->setVisible(false);
    m_sizeTips->setVisible(false);
#ifdef OCR_SCROLL_FLAGE_ON
    if (m_scrollShotTip) {
        m_scrollShotTip->setVisible(false);
        m_scrollShotTip->hide();
    }
    if(m_previewWidget){
        // 隐藏预览窗口
        m_previewWidget->hide();
    }
    // 延时
#if defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__)
    static int delayTime = 260;
#elif defined (__aarch64__)
    static int delayTime = 220;
#else
    static int delayTime = 100;
#endif
    QEventLoop eventloop;
    QTimer::singleShot(delayTime, &eventloop, SLOT(quit()));
    eventloop.exec();
#endif
    if (m_scrollShotSizeTips) {
        m_scrollShotSizeTips->hide();
    }
    update();

    m_initScroll = false; // 保存时关闭滚动截图
    //滚动截图模式下保存图片
    if (status::scrollshot == m_functionType && m_scrollShotStatus != 0) {
#ifdef OCR_SCROLL_FLAGE_ON
        bool ok;
        QRect rect(recordX + 1, recordY + 1, recordWidth - 2, recordHeight - 2);
        QPixmap img = m_screenGrabber.grabEntireDesktop(ok, rect, m_pixelRatio); // 抓取当前捕捉区域图片
        m_scrollShot->addLastPixmap(img);
        m_resultPixmap = QPixmap::fromImage(m_scrollShot->savePixmap());
        if (m_resultPixmap.isNull()) {
            //普通截图保存图片
            shotCurrentImg();
        }
#endif
    } else {
        //普通截图保存图片
        shotCurrentImg();
    }
    const bool r = saveAction(m_resultPixmap);
    save2Clipboard(m_resultPixmap);
    this->hide();
    if (status::pinscreenshots == m_functionType) return;
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
    if (m_noNotify || Utils::isRootUser) {
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

    QDBusInterface remote_dde_notify_obj("com.deepin.dde.Notification", "/com/deepin/dde/Notification", "com.deepin.dde.Notification");

    const bool remote_dde_notify_obj_exist = remote_dde_notify_obj.isValid();

    QDBusInterface notification("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications",
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
        exitApp();
    });
}

bool MainWindow::saveAction(const QPixmap &pix)
{
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
    m_saveIndex = ConfigSettings::instance()->value("save", "save_op").value<SaveAction>();
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
        // 贴图不用保存
        if (status::pinscreenshots == m_functionType) {
            break;
        }
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
        } else if (!BaseUtils::isValidFormat(fileSuffix)) {

            //检查后缀是以.png|.jpg|.jpeg|.bmp中的一种进行结尾。false:否 true：是
            bool flag = checkSuffix(fileSuffix);
            if (!flag) {
                qWarning() << "The fileName has invalid suffix! fileSuffix: " << fileSuffix;
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
            }
        } else {
            qDebug() << "The fileSuffix is right!  " << fileSuffix;
        }

        qDebug() << "The fileName is: " << m_saveFileName;
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
        if (m_shotWithPath == true) {
            savePath = m_shotSavePath;
        } else if (m_saveIndex == SaveToImage) {
            savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first() + QDir::separator() + "Screenshots";
        } else {
            savePath = QStandardPaths::writableLocation(saveOption);
        }

        // 判断目录是否存在
        if ((!QDir(savePath).exists() && QDir().mkdir(savePath) == false) ||  // 文件不存在，且创建失败
                (QDir(savePath).exists() && !QFileInfo(savePath).isWritable())) {  // 文件存在，且不能写
            savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
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

//    qDebug() << "====== function: " << __func__ << " start ======";
    if (m_shotflag == 1 && status::scrollshot != m_functionType) {
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
//        qDebug() << "function: " << __func__ << " ,line: " << __LINE__;
        painter.setRenderHint(QPainter::Antialiasing, true);
        QRect backgroundRect;

        backgroundRect = QRect(0, 0, rootWindowRect.width(), rootWindowRect.height());
        // FIXME: Under the magnifying glass, it seems to be magnified two times.
        m_backgroundPixmap.setDevicePixelRatio(m_pixelRatio);
        painter.drawPixmap(backgroundRect, m_backgroundPixmap);
    }

    if (recordWidth > 0 && recordHeight > 0) {
//        qDebug() << "function: " << __func__ << " ,line: " << __LINE__;
        if (Utils::isTabletEnvironment && (status::record == m_functionType || status::scrollshot == m_functionType)) {
            // 平板环境屏蔽录屏和滚动截图， 不绘制线框
            return;
        }
        m_firstShot = 1;
        QRect backgroundRect = QRect(0, 0, rootWindowRect.width(), rootWindowRect.height());
        QRect frameRect = QRect(recordX, recordY, recordWidth, recordHeight);

        //只有在滚动截图中,且触发了可以调整捕捉区域时才会显示捕捉区域
        if (status::scrollshot == m_functionType && m_isAdjustArea) {
//            qDebug() << "m_adjustArea.x(): " << m_adjustArea.x()
//                     << "m_adjustArea.y(): " << m_adjustArea.y()
//                     << "m_adjustArea.width(): " << m_adjustArea.width()
//                     << "m_adjustArea.height(): " << m_adjustArea.height();
            //画可调整的捕捉区域位置及大小
            painter.setRenderHint(QPainter::Antialiasing, false);
            QPen framePen(QColor("#01bdff"));
            framePen.setStyle(Qt::SolidLine);
            framePen.setDashOffset(0);
            framePen.setWidth(3);
            painter.setOpacity(1);
            painter.setBrush(QBrush());  // clear brush
            painter.setPen(framePen);
            painter.drawRect(QRect(
                                 std::max(static_cast<int>(m_adjustArea.x()), 1),
                                 std::max(static_cast<int>(m_adjustArea.y()) + 3, 1),
                                 std::min(static_cast<int>(m_adjustArea.width()) - 1, rootWindowRect.width() - 2),
                                 std::min(static_cast<int>(m_adjustArea.height()) - 1, rootWindowRect.height() - 2)));
            painter.setRenderHint(QPainter::Antialiasing, true);
        }

        // Draw background. 画背景
        painter.setBrush(QBrush("#000000"));
        painter.setOpacity(0.2);
        //启用剪辑模式
        painter.setClipping(true);
        //使用指定的剪辑操作将剪辑区域设置为给定区域
        painter.setClipRegion(QRegion(backgroundRect).subtracted(QRegion(frameRect)));
        //画出当前背景
        painter.drawRect(backgroundRect);
        // Reset clip. 重设剪辑区域
        painter.setClipRegion(QRegion(backgroundRect));

        //捕捉区域
        frameRect = QRect(recordX, recordY, recordWidth, recordHeight);
        // Draw frame. 画捕捉区域的虚线框
        if (recordButtonStatus != RECORD_BUTTON_RECORDING) {
//            qDebug() << "function: " << __func__ << " ,line: " << __LINE__;
            painter.setRenderHint(QPainter::Antialiasing, false);
            //QPen framePen(QColor("#01bdff"));
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
        //画虚线框上的骨架点一共8个
        if (recordButtonStatus == RECORD_BUTTON_NORMAL && drawDragPoint) {
            //qDebug() << "function: " << __func__ << " ,line: " << __LINE__;
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
//    qDebug() << "====== function: " << __func__ << " end ======";

}
bool MainWindow::eventFilter(QObject *, QEvent *event)
{
    bool needRepaint = false;

#undef KeyPress
#undef KeyRelease
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        qDebug() << "key press:" << keyEvent->key();

        //截图模式下键盘按键操作
        if (status::shot == m_functionType) {
            bool needRepaint = false;

            //截图编辑界面存在时
            if (m_isShapesWidgetExist) {

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
                    //if (m_recordButton->isVisible()) {
                    //updateRecordButtonPos();
                    //}

                    if (m_sideBar->isVisible()) {
                        updateSideBarPos();
                    }

                    //if (m_shotButton->isVisible()) {
                    //updateShotButtonPos();
                    //}

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
        //录屏模式下键盘按键操作
        else {
            if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                //wayland录屏暂时需要通过eventfilter来处理，后期可能会移至onKeyboardPressWayland进行处理
                if (Utils::isWaylandMode) {
//                    if (keyEvent->key() == Qt::Key_S && RECORD_BUTTON_NORMAL == recordButtonStatus) {
//                        m_toolBar->shapeClickedFromMain("audio");
//                    } else if (keyEvent->key() == Qt::Key_M && RECORD_BUTTON_NORMAL == recordButtonStatus) {
//                        m_toolBar->shapeClickedFromMain("mouse");
//                    } else if (keyEvent->key() == Qt::Key_F3 && RECORD_BUTTON_NORMAL == recordButtonStatus) {
//                        m_toolBar->shapeClickedFromMain("record_option");
//                    }
                }
                //调整捕捉区域快捷键 shift+ctrl+up/down/left/right
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
                }
                //ctrl+up/down/left/right
                else if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
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
                }
                //快捷键 up/down/left/right
                else {
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
                //if (m_recordButton->isVisible()) {
                //updateRecordButtonPos();
                //}

                if (m_sideBar->isVisible()) {
                    updateSideBarPos();
                }

                //if (m_shotButton->isVisible()) {
                //updateShotButtonPos();
                //}

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

            //截图编辑界面存在时，按住shift可以对相应的图形进行等比放大或缩小
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
                    //updateRecordButtonPos();
                    //updateShotButtonPos();
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
                //滚动截图的图片大小提示更新，不会使用此方法
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
                    //updateRecordButtonPos();
                    //updateShotButtonPos();
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
                    //updateRecordButtonPos();
                    //updateShotButtonPos();

                    needRepaint = true;
                } else {
                    if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                        //showRecordButton();
                        updateToolBarPos();
                        if (status::shot == m_functionType && m_sideBar->isVisible()) {
                            updateSideBarPos();
                        }
                        //updateRecordButtonPos();
                        //updateShotButtonPos();
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
        //没打开截图的编辑模式
        if (!m_isShapesWidgetExist) {
            if (m_toolBar->isVisible()) {
                updateToolBarPos();
                m_zoomIndicator->hide();
            }

            if (!isFirstMove) {
                isFirstMove = true;
            } else {
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
                        if (!qFuzzyCompare(1.0, m_pixelRatio) && m_screenCount > 1) {
                            int beforeWidth = 0;
                            for (int index = 0; index < m_screenCount; ++index) {
                                if (tmpPos.x() >= m_screenInfo[index].x && tmpPos.x() < (m_screenInfo[index].x + m_screenInfo[index].width)) {
                                    tmpPos.setX(static_cast<int>((tmpPos.x() - m_screenInfo[index].x) + beforeWidth / m_pixelRatio));
                                    break;
                                }
                                beforeWidth += m_screenInfo[index].width;
                            }
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

                //获取鼠标放到捕捉区边框的动作
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
                                int beforeWidth = 0;
                                for (int index = 0; index < m_screenCount; ++index) {
                                    if (x >= m_screenInfo[index].x && x < (m_screenInfo[index].x + m_screenInfo[index].width)) {
                                        recordX = static_cast<int>((x - m_screenInfo[index].x) + beforeWidth / m_pixelRatio);
                                        break;
                                    }
                                    beforeWidth += m_screenInfo[index].width;
                                }
                                int beforeHeight = 0;
                                for (int index = 0; index < m_screenCount; ++index) {
                                    if (y >= m_screenInfo[index].y && y < (m_screenInfo[index].y + m_screenInfo[index].height)) {
                                        recordY = static_cast<int>((y - m_screenInfo[index].y) + beforeHeight / m_pixelRatio);
                                        break;
                                    }
                                    beforeHeight += m_screenInfo[index].height;
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

            //将当前捕捉区域画为一个矩形
            QRect rect {
                static_cast<int>(recordX * m_pixelRatio),
                static_cast<int>(recordY * m_pixelRatio),
                static_cast<int>(recordWidth * m_pixelRatio),
                static_cast<int>(recordHeight + 1 * m_pixelRatio)
            };

            //如果鼠标位置移出捕捉区域则不显示捕捉区域的骨架节点
            if (!rect.contains(QPoint(static_cast<int>(mouseEvent->x()*m_pixelRatio), static_cast<int>(mouseEvent->y()*m_pixelRatio)))) {
                drawDragPoint = false;
                needRepaint = true;
            }

        }

        //打开了截图的编辑模式
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

    } else if (event->type() == QEvent::Wheel) {
        //qDebug() << "event->type():" << event->type();
        //未进行区域穿透的效果可以由此方式获取相应的鼠标滚轮事件。
        if (status::scrollshot == m_functionType) {
            //滚动截图出现自动调整捕捉区域异常时屏蔽鼠标滚轮事件
            if (m_isErrorWithScrollShot) return false;
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            //qDebug() << "wheelEvent->x(),wheelEvent->y():" << wheelEvent->x() << "," << wheelEvent->y();
            QRect recordRect {
                static_cast<int>(recordX * m_pixelRatio),
                static_cast<int>(recordY * m_pixelRatio),
                static_cast<int>(recordWidth * m_pixelRatio),
                static_cast<int>(recordHeight * m_pixelRatio)
            };
            //当前鼠标滚动的点
            QPoint mouseScrollPoint(wheelEvent->x(), wheelEvent->y());
            //判断鼠标滚动的位置是否是在捕捉区域内部，滚动位置在捕捉区域内部
            if (recordRect.contains(mouseScrollPoint)) {
                m_scrollShotType = ScrollShotType::ManualScroll;
                //qDebug() << "function: " << __func__ << " ,line: " << __LINE__ << " ,m_scrollShotStatus: " << m_scrollShotStatus;
                //当且仅当出现调整捕捉区域的异常情况下，此属性才会为true,防止用户继续滚动鼠标滚轮
                if (!m_isErrorWithScrollShot) {
                    //捕捉区域设置为穿透状态
                    setInputEvent();
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

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (Utils::isWaylandMode) {
        onKeyboardPressWayland(static_cast<int>(event->nativeScanCode()));
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (Utils::isWaylandMode) {
        onKeyboardReleaseWayland(static_cast<int>(event->nativeScanCode()));
    }
}
void MainWindow::tableRecordSet()
{
    m_tabletRecorderHandle = new RecorderTablet(nullptr);

    recordX = 0;
    recordY = 0;
    recordWidth = m_screenSize.width();
    recordHeight = m_screenSize.height();
    // 鼠标点击状态录制
    m_mouseStatus = true;
    startCountdown();
}

//切换为活动窗口
void MainWindow::onActivateWindow()
{
    if (!m_initResource) {
        return;
    }
    if (status::shot == m_functionType || status::scrollshot == m_functionType) {
        activateWindow();
    }
}

//通过x11从底层获取鼠标拖动事件
void MainWindow::onMouseDrag(int x, int y)
{
    if (!m_initResource) {
        return;
    }
    if (status::record == m_functionType) {
        showDragFeedback(x, y);
    }
}

//通过x11从底层获取鼠标按压事件
void MainWindow::onMousePress(int x, int y)
{
    if (!m_initResource) {
        return;
    }
    if (status::record == m_functionType) {
        showPressFeedback(x, y);
    }  else if (m_initScroll && status::scrollshot == m_functionType) {
        scrollShotMouseClickEvent(x, y);
    }
}

//通过x11从底层获取鼠标释放事件
void MainWindow::onMouseRelease(int x, int y)
{
    if (!m_initResource) {
        return;
    }
    if (status::record == m_functionType) {
        showReleaseFeedback(x, y);
    }
}

//通过x11从底层获取鼠标移动事件
void MainWindow::onMouseMove(int x, int y)
{
    if (!m_initResource) {
        return;
    }
    if (m_initScroll && status::scrollshot == m_functionType) {
        scrollShotMouseMoveEvent(x, y);
        this->activateWindow();
    }
}

//通过x11从底层滚动鼠标滚轮
void MainWindow::onMouseScroll(int mouseTime, int direction, int x, int y)
{
    if (!m_initResource) {
        return;
    }
    if (m_initScroll && status::scrollshot == m_functionType) {
        scrollShotMouseScrollEvent(mouseTime, direction, x, y);
    }

}

// Wayland接收键盘按键事件
void MainWindow::onKeyboardPressWayland(const int key)
{
    if (status::record == m_functionType) {
        if (RECORD_BUTTON_NORMAL == recordButtonStatus) {
            if (KEY_ESCAPE == key) {
                responseEsc();
            } else if (KEY_S == key) {
                m_toolBar->shapeClickedFromMain("audio");
            } else if (KEY_M == key) {//录屏快捷键 鼠标 m
                m_toolBar->shapeClickedFromMain("mouse");
            } else if (KEY_F3 == key) { //录屏快捷键 选项 f3
                m_toolBar->shapeClickedFromMain("record_option");
            }
        }
        m_showButtons->showContentButtons(key);
    } else {
        if ((KEY_F3 == key)) {
            m_toolBar->shapeClickedFromMain("option");
        }
    }
}
void MainWindow::onKeyboardReleaseWayland(const int key)
{
    if (status::record == m_functionType) {
        m_showButtons->releaseContentButtons(key);
    }
}

//通过x11从底层获取键盘按下事件
void MainWindow::onKeyboardPress(unsigned char keyCode)
{
    if (!m_initResource) {
        return;
    }
    if (status::record == m_functionType) {
        m_showButtons->showContentButtons(keyCode);
        recordKeyPressEvent(keyCode);
        if (RECORD_BUTTON_RECORDING != recordButtonStatus && keyCode == KEY_ESCAPE) {
            exitApp();
        }
    } else if (status::shot == m_functionType || status::scrollshot == m_functionType) {
        shotKeyPressEvent(keyCode);
    }
}

//键盘按键释放
void MainWindow::onKeyboardRelease(unsigned char keyCode)
{
    if (!m_initResource) {
        return;
    }
    if (status::record == m_functionType) {
        m_showButtons->releaseContentButtons(keyCode);
    }
}

//滚动截图鼠标按钮事件
void MainWindow::scrollShotMouseClickEvent(int x, int y)
{
#ifdef OCR_SCROLL_FLAGE_ON
    //将当前捕捉区域画为一个矩形
    QRect scrollShotRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };

    //当前鼠标点击的点
    QPoint mouseClickPoint(x, y);
    //滚动拼接提示无法继续截图或调整捕捉区域时，鼠标无法点击文字按钮
    if (m_scrollShotTip != nullptr &&
            m_scrollShotTip->isVisible() &&
            (m_scrollShotTip->getTipType() == TipType::ErrorScrollShotTip ||
             m_scrollShotTip->getTipType() == TipType::InvalidAreaShotTip)) {
        //滚动截图的提示
        QRect scrollShotTipRect {
            static_cast<int>(m_scrollShotTip->x() * m_pixelRatio),
            static_cast<int>(m_scrollShotTip->y() * m_pixelRatio),
            static_cast<int>(m_scrollShotTip->width() * m_pixelRatio),
            static_cast<int>(m_scrollShotTip->height() * m_pixelRatio)
        };
        //点击的位置在滚动截图的提示框内部，滚动截图不响应此时的点击事件
        if (scrollShotTipRect.contains(mouseClickPoint)) {
            return;
        }
    }
    //将当前工具栏画为一个矩形
    QRect toolBarRect {
        static_cast<int>(m_toolBar->x() * m_pixelRatio),
        static_cast<int>(m_toolBar->y() * m_pixelRatio),
        static_cast<int>(m_toolBar->width() * m_pixelRatio),
        static_cast<int>(m_toolBar->height() * m_pixelRatio)
    };
    //判断当前点击的点是否在工具栏或截图保存按钮上（当工具栏或截图保存按钮在捕捉区域内部时会进入）,滚动截图不响应此时的点击事件
    if (toolBarRect.contains(mouseClickPoint)) {
        return;
    }

    //鼠标点击次数
    m_scrollShotMouseClick += 1;
    if (m_scrollShotMouseClick > 2) {
        m_scrollShotMouseClick = 2;
    }

    qDebug() << "==============================鼠标按键 m_scrollShotMouseClick :  " << m_scrollShotMouseClick ;
    //qDebug() << "mouseClickPoint x,y :  " << mouseClickPoint.x() << " , " << mouseClickPoint.y();
    //判断当前点击的点是否在捕捉区域内部,不在捕捉区域内则不响应点击事件
    if (!scrollShotRect.contains(mouseClickPoint)) {
        return;
    }

    //滚动截图出现自动调整捕捉区域异常时，屏蔽整个捕捉区域的鼠标点击事件
    if (m_isErrorWithScrollShot) return;
    //qDebug() << "m_scrollShotMouseClick: " << m_scrollShotMouseClick;

    //通过以上所有情况后，只要鼠标进行点击则切换为自动滚动
    if (m_scrollShotType != ScrollShotType::AutoScroll) {
        m_scrollShotType = ScrollShotType::AutoScroll;
    }
    qDebug() << "function: " << __func__ << " ,line: " << __LINE__ << " ,m_scrollShotStatus: " << m_scrollShotStatus;

    //鼠标单击
    if (m_scrollShotMouseClick == 1) {
        qDebug() << "鼠标单击!";
        //第一次进入自动滚动截图，开始自动滚动截图
        if (m_scrollShotStatus == 0 || m_scrollShotStatus == 5) {
            m_scrollShotTip->hide();
            update();
            startAutoScrollShot();
            m_scrollShotStatus = 1;
        }
        //第n次进入 n不等于1，暂停滚动截图
        else if (1 == m_scrollShotStatus || 2 == m_scrollShotStatus) {
            m_scrollShotStatus = 3;
            //暂停自动滚动截图
            pauseAutoScrollShot();
            //取消捕捉区域穿透
            setCancelInputEvent();
        }
        //第n次进入 n不等于1,继续滚动截图
        else if (3 == m_scrollShotStatus || 4 == m_scrollShotStatus || 6 == m_scrollShotStatus) {
            //此处用来处理,当一开始使用手动滚动截图时出现错误的情况下切换自动滚动,自动滚动不会被启动
            if (!m_isAutoScrollShotStart) {
                startAutoScrollShot();
                m_scrollShotStatus = 1;
            } else {
                //设置穿透
                setInputEvent();
                continueAutoScrollShot();
                m_scrollShotStatus = 2;
            }

        }
    }
    //鼠标双击
    else if (m_scrollShotMouseClick == 2) {
        qDebug() << "鼠标双击!";
        //不是第一次进入滚动截图，则保存当前滚动截图
        //saveScreenShot();

    }
#endif
}

//滚动截图鼠标移动事件处理
void MainWindow::scrollShotMouseMoveEvent(int x, int y)
{
#ifdef OCR_SCROLL_FLAGE_ON
    //滚动截图出现异常时屏蔽鼠标移动事件
    //if (m_isErrorWithScrollShot) return;

    //将当前捕捉区域画为一个矩形
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    //当前鼠标的点
    QPoint mouseMovePoint(x, y);
    //判断当鼠标位置是否在捕捉区域内部,不在捕捉区域内则暂停自动滚动，并取消穿透，此时取消穿透对捕捉区域外的操作不构成影响
    if (!recordRect.contains(mouseMovePoint)) {
        if (1 == m_scrollShotStatus || 2 == m_scrollShotStatus || 3 == m_scrollShotStatus) {
            m_scrollShotStatus = 4;
            //暂停自动滚动截图
            pauseAutoScrollShot();
            //取消捕捉区域穿透
            setCancelInputEvent();
        }
        //qDebug() << "111 >> function: " << __func__ << " , line: " << __LINE__;
        //不在捕捉区域内部，则取消屏蔽，使操作者可以点击工具栏、保存、退出等按钮
        Utils::disableXGrabButton();
    }
    //当前的点在捕捉区域内部
    else {
        //将当前工具栏画为一个矩形
        QRect toolBarRect {
            static_cast<int>(m_toolBar->x() * m_pixelRatio),
            static_cast<int>(m_toolBar->y() * m_pixelRatio),
            static_cast<int>(m_toolBar->width() * m_pixelRatio),
            static_cast<int>(m_toolBar->height() * m_pixelRatio)
        };

        //滚动截图的提示
        QRect scrollShotTipRect {
            static_cast<int>(m_scrollShotTip->x() * m_pixelRatio),
            static_cast<int>(m_scrollShotTip->y() * m_pixelRatio),
            static_cast<int>(m_scrollShotTip->width() * m_pixelRatio),
            static_cast<int>(m_scrollShotTip->height() * m_pixelRatio)
        };
        //判断当前鼠标是否在工具栏或截图保存按钮或滚动截图提示上（此时工具栏或截图保存按钮或滚动截图的提示框在捕捉区域内部）
        if (toolBarRect.contains(mouseMovePoint)) {
            //滚动截图启动后，鼠标移动到工具栏或保存按钮时，需暂停自动滚动，并取消捕捉区域穿透
            if (0 != m_scrollShotStatus) {
                m_scrollShotStatus = 4;
                //暂停自动滚动截图
                pauseAutoScrollShot();
                //取消捕捉区域穿透
                setCancelInputEvent();
                //qDebug() << "222 >> function: " << __func__ << " , line: " << __LINE__;
            }
            //在捕捉区域内且在工具栏或截图保存按钮，则取消屏蔽，使操作者可以点击工具栏、保存、退出等按钮
            Utils::disableXGrabButton();
        }
        //判断当前鼠标是否在滚动截图异常提示上（此时滚动截图的提示框在捕捉区域内部且滚动截图一定启动了且自动滚动处于暂停状态）
        else if (scrollShotTipRect.contains(mouseMovePoint)) {
            //滚动截图一定已经启动，鼠标移动到异常提示时，取消捕捉区域穿透
            if (0 != m_scrollShotStatus) {
                //取消捕捉区域穿透
                setCancelInputEvent();
                //qDebug() << "222 >> function: " << __func__ << " , line: " << __LINE__;
                //在捕捉区域内且在异常提示按钮上，则取消屏蔽，使操作者可以点击查看帮助、调整捕捉区域等按钮
                Utils::disableXGrabButton();
            }
        } else {
            //在捕捉区域内部，则打开屏蔽
            Utils::enableXGrabButton();
            //qDebug() << "444 >> function: " << __func__ << " , line: " << __LINE__;
        }
    }
    //判断当前点是否在捕捉区域内部,在捕捉区域内则继续滚动（鼠标移出捕捉区域，在移入捕捉区域时，自动启动滚动截图）
    //else {
    //    //鼠标点击触发的暂停，不论鼠标在捕捉区域内如何移动都不继续
    //    if (4 == m_scrollShotStatus) {
    //        m_scrollShotStatus = 2;
    //        continueScrollShot();
    //    }
    //}
#endif
}

/**
 * @brief 滚动截图时处理鼠标滚轮滚动,手动滚动截图和自动滚动截图都会触发当前的槽函数
 * @param direction 鼠标滚动的方向： 1：向上滚动； 0：向下滚动
 * @param x 当前的x坐标
 * @param y 当前的y坐标
 */
void MainWindow::scrollShotMouseScrollEvent(int mouseTime, int direction, int x, int y)
{
#ifdef OCR_SCROLL_FLAGE_ON
    QRect recordRect {
        static_cast<int>(recordX * m_pixelRatio),
        static_cast<int>(recordY * m_pixelRatio),
        static_cast<int>(recordWidth * m_pixelRatio),
        static_cast<int>(recordHeight * m_pixelRatio)
    };
    //当前鼠标滚动的点
    QPoint mouseScrollPoint(x, y);
    //判断鼠标滚动的位置是否是在捕捉区域内部，不在捕捉区域内部不进行处理
    if (!recordRect.contains(mouseScrollPoint)) return;

    if (!Utils::isWaylandMode) {
        //对比监听自动滚动事件是否正在进行触发, wayland模式下不做此判断
        if (m_autoScrollFlagNext > m_autoScrollFlagLast) {
            m_scrollShotType = ScrollShotType::AutoScroll;
            m_autoScrollFlagLast = m_autoScrollFlagNext;
        } else {
            m_scrollShotType = ScrollShotType::ManualScroll;
        }
    }
    qDebug() << "function: " << __func__ << " ,line: " << __LINE__ << " ,m_scrollShotType: " << m_scrollShotType;
    //当前状态为手动滚动模式时,会先暂停自动滚动
    if (m_scrollShotType == ScrollShotType::ManualScroll) {
        //qDebug() << "function: " << __func__ << " ,line: " << __LINE__ << " ,m_scrollShotStatus: " << m_scrollShotStatus;
        //滚动截图出现自动调整捕捉区域异常时屏蔽鼠标滚轮事件
        if (m_isErrorWithScrollShot) return;

        //滚动截图通过手动滚动截图方式启动，第一次通过手动滚动截图
        if (m_scrollShotStatus == 0) {
            m_scrollShotTip->hide();
            update();
            m_scrollShotStatus = 5;
            //开始手动滚动截图
            startManualScrollShot();
        }
        //这种处理方式适用于捕捉区域为穿透状态，非初次进入滚动图及滚动截图暂停状态
        else {
            //如果滚动截图的异常提示显示则隐藏显示
            if (m_tipShowtimer != nullptr) {
                m_tipShowtimer->stop();
            }
            m_scrollShotTip->hide();

            if (m_scrollShotStatus == 5) {
                m_scrollShotStatus = 5;
            } else {
                qDebug() << "function: " << __func__ << " ,line: " << __LINE__ << " ,m_scrollShotStatus: " << m_scrollShotStatus;
                //更改滚动状态为6,暂停自动滚动
                m_scrollShotStatus = 6;
                //暂停自动滚动截图
                pauseAutoScrollShot();
            }
            //qDebug() << "function: " << __func__ << " ,line: " << __LINE__ << " ,m_scrollShotStatus: " << m_scrollShotStatus;
            //处理手动滚动截图
            setInputEvent();
            handleManualScrollShot(mouseTime, direction);
        }
    }
#endif
}

/**
 * @brief 监听是否正在进行自动滚动
 * @param autoScrollFlag 进行自动滚动时,模拟滚动的操作会,进行次数加1
 */
void MainWindow::onScrollShotCheckScrollType(int autoScrollFlag)
{
    m_autoScrollFlagNext = autoScrollFlag;
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
    qDebug() << ">>>>>>>>> isLocked: " << isLocked;
    if (status::scrollshot == m_functionType) {
        scrollShotLockScreen(isLocked);
    } else if (status::shot == m_functionType) {
        pinScreenshotsLockScreen(isLocked);
    }
}

//打开截图录屏帮助文档并定位到滚动截图
void MainWindow::onOpenScrollShotHelp()
{


    QDBusInterface interFace("com.deepin.Manual.Open",
                             "/com/deepin/Manual/Open",
                             "com.deepin.Manual.Open",
                             QDBusConnection::sessionBus());
    // 帮助手册跳转到子标题，传入子标题标签(标签文档内唯一，固定为英文，与翻译无关)
    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName())                  // 应用名称
        << QString("scrollshot");                         // 帮助文案中的标题名称
    interFace.callWithArgumentList(QDBus::AutoDetect, "OpenTitle", arg);

    exitApp();
}

//自动调整捕捉区域的大小及位置
void MainWindow::onAdjustCaptureArea()
{
#ifdef OCR_SCROLL_FLAGE_ON
    qDebug() << "function: " << __func__ << " ,line: " << __LINE__;
    if (m_tipShowtimer != nullptr) {
        m_tipShowtimer->stop();
    }
    //隐藏提示
    m_scrollShotTip->hide();

    //如果自动捕捉区域为空则返回
    if (m_adjustArea.isNull()) return;
    //可自动调整区域不显示
    m_isAdjustArea = false;
    repaint();

    //重设捕捉区域大小及位置
    recordX = m_adjustArea.x();
    recordY = m_adjustArea.y();
    recordWidth = m_adjustArea.width();
    recordHeight = m_adjustArea.height();
    //更新滚动截图左上角当前图片的大小及位置
    m_scrollShotSizeTips->updateTips(QPoint(recordX, recordY), QSize(recordWidth, recordHeight));
    //更新工具栏位置
    updateToolBarPos();
    //截图保存按钮位置
    //updateShotButtonPos();
    //工具栏、保存截图按钮先隐藏在显示，防止出现的预览图中包含工具栏
    m_toolBar->hide();
    //m_shotButton->hide();
    m_scrollShotSizeTips->hide();
    m_previewWidget->hide();

    //延时时间

#if defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__)
    static int delayTime = 260;
#elif defined (__aarch64__)
    static int delayTime = 220;
#else
    static int delayTime = 100;
#endif
    QTimer::singleShot(delayTime, this, [ = ] {
        //更新预览图的位置及大小
        bool ok;
        QRect previewRecordRect(recordX + 1, recordY + 1, recordWidth - 2, recordHeight - 2);
        m_previewWidget->updatePreviewSize(previewRecordRect);
        m_firstScrollShotImg = m_screenGrabber.grabEntireDesktop(ok, previewRecordRect, m_pixelRatio);
        m_previewWidget->updateImage(m_firstScrollShotImg.toImage());
        m_previewWidget->show();
        //打开工具栏显示
        m_toolBar->show();
        //打开截图保存按钮显示
        //m_shotButton->show();
        //打开滚动截图左上角当前图片的大小显示
        m_scrollShotSizeTips->show();

        //获取预览框相对于捕捉区域的位置
        m_previewPostion = m_previewWidget->getPreviewPostion();
    });
    //清除滚动截图已经保存的图片数据
    m_scrollShot->clearPixmap();

    //自动滚动截图模式是否曾经被启动过
    if (m_isAutoScrollShotStart) {
        //启动过：滚动截图状态为3
        m_scrollShotStatus = 3;
    } else {
        //没有启动过：滚动截图状态恢复为初始状态
        m_scrollShotStatus = 0;
    }

    //滚动截图：自动调整捕捉区域错误已经解决，此方法就是用来解决这个错误
    m_isErrorWithScrollShot = false;
    update();
#endif
}


#ifdef OCR_SCROLL_FLAGE_ON
//滚动截图时，获取拼接时的状态
void MainWindow::onScrollShotMerageImgState(PixMergeThread::MergeErrorValue state)
{
    //暂停滚动截图,可以通过点击继续进行截图
    m_scrollShotStatus = 3;
    //暂停自动滚动截图
    pauseAutoScrollShot();
    if(m_tipShowtimer->isActive()){
        return;
    }
    qDebug() << "function:" << __func__ << " ,line: " << __LINE__ << " , 拼接时的状态: " << state;
    //state = 1：拼接失败
    if (state == PixMergeThread::MergeErrorValue::Failed) {
        //提示滚动截图拼接失败的方法
        m_scrollShotTip->showTip(TipType::ErrorScrollShotTip);
        //qDebug() << "1：拼接失败" ;
        //拼接失败立即保存当前的截图
        //saveScreenShot();
    }
    //state = 2：滚动到底部
    else if (state == PixMergeThread::MergeErrorValue::ReachBottom) {
        m_scrollShotTip->showTip(TipType::EndScrollShotTip);
        //qDebug() << "2：滚动到底部" ;
    }
    //state = 3：拼接截图到截图最大限度
    else if (state == PixMergeThread::MergeErrorValue::MaxHeight) {
        m_scrollShotTip->showTip(TipType::MaxLengthScrollShotTip);
        //qDebug() << "3：拼接截图到截图最大限度" ;
    }
    //state = 4:调整捕捉区域 出现此异常后停止自动滚动截图停止鼠标点击继续滚动截图，手动滚动停止鼠标滚动继续滚动截图
    else if (state == PixMergeThread::MergeErrorValue::InvalidArea) {
        m_isAdjustArea = true;
        //取消捕捉区域穿透，防止用户继续滚动鼠标滚轮
        setCancelInputEvent();
        //显示可调整的捕捉区域大小及位置
        showAdjustArea();
        m_scrollShotTip->showTip(TipType::InvalidAreaShotTip);

        //滚动截图出现异常
        m_isErrorWithScrollShot = true;

    }
    //state = 5: 滚动速度过快
    else if (state == PixMergeThread::MergeErrorValue::RoollingTooFast) {
        m_scrollShotTip->showTip(TipType::QuickScrollShotTip);
    } else {
        return;
    }
    //qDebug() << "function:" << __func__ << " ,line: " << __LINE__ <<"state: " << state;
    //根据工具栏获取滚动截图提示框的坐标
    QPoint tipPosition = getScrollShotTipPosition();
    //提示信息移动到指定位置
    m_scrollShotTip->move(tipPosition);
    //抓取当前提示的背景图
    QPixmap currentBackgroundPixmap = getPixmapofRect(m_backgroundRect);
    m_scrollShotTip->setBackgroundPixmap(currentBackgroundPixmap);
    //显示提示
    m_scrollShotTip->show();
    //m_scrollShotTip->setVisible(true);

    //qDebug() << "提示将在2s后消失！" ;
    //滚动截图异常提示的定时器开始计时
    m_tipShowtimer->start();
}
#endif
void MainWindow::initPadShot()
{
    recordX = 0;
    recordY = 0;
    recordWidth = QApplication::desktop()->width();
    recordHeight = QApplication::desktop()->height();
    updateToolBarPos();
    //updateShotButtonPos();
    QPoint toolbarPoint;
    toolbarPoint = QPoint(recordX + recordWidth - m_toolBar->width() - TOOLBAR_X_SPACING, std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));
    toolbarPoint.setY(recordY + TOOLBAR_Y_SPACING);
    m_toolBar->showAt(toolbarPoint);
}

void MainWindow::exitScreenCuptureEvent()
{
    qDebug() << "line: " << __LINE__ << " >>> function: " << __func__;
#if !(defined (__mips__) || defined (__loongarch_64__))
    if (!m_isZhaoxin && m_pScreenCaptureEvent) {
        m_pScreenCaptureEvent->releaseRes();
        //m_pScreenCaptureEvent->terminate();
        m_pScreenCaptureEvent->wait();
        delete m_pScreenCaptureEvent;
        m_pScreenCaptureEvent = nullptr;
    }
#endif
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
    if (Utils::isWaylandMode) {
        shortcutString << param1 << param2;
    } else {
        shortcutString << "-b" << param1 << param2;
    }

    QProcess *shortcutViewProc = new QProcess(this);
    shortcutViewProc->startDetached("deepin-shortcut-viewer", shortcutString);
    //QProcess::startDetached();
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

void MainWindow::on_CheckVideoCouldUse(bool canUse)
{
    if (!canUse) {
        if (m_cameraWidget) {
            // 监测设备文件是否存在
            if (m_cameraWidget->getcameraStatus() == false) {
                qDebug() << "camera canuse" << canUse;
                m_cameraWidget->cameraStop();
                m_cameraWidget->setCameraStop(canUse);
                m_cameraOffFlag = true;
                m_cameraWidget->hide();
                m_toolBar->setCameraDeviceEnable(canUse);
            } else {
                // 设置Coulduse的值，使m_watchTimer每次都能监测设备文件是否存在
                if (m_pCameraWatcher)
                    m_pCameraWatcher->setCoulduseValue(true);
            }
        } else {
            m_toolBar->setCameraDeviceEnable(canUse);
        }
    } else if (canUse) {
        m_toolBar->setCameraDeviceEnable(canUse);
        if (m_cameraOffFlag) {
            // 重新加载设备
            m_cameraWidget->cameraResume();
        }
    }
}



//截图模式及滚动截图模式键盘按下执行的操作 如果快捷键需要打开下拉列表，则不能使用全局快捷键处理，需使用此方法处理
void MainWindow::shotKeyPressEvent(const unsigned char &keyCode)
{
    //滚动截图及普通截图都可以通过快捷键触发F3
    if (KEY_F3 == keyCode) {
        m_toolBar->shapeClickedFromMain("option");
    }

}

//x11 录屏模式下键盘按下执行的操作 如果快捷键需要打开下拉列表，则不能使用全局快捷键处理，需使用此方法处理
void MainWindow::recordKeyPressEvent(const unsigned char &keyCode)
{
    //录屏快捷键 录音 s
    if (KEY_S == keyCode && RECORD_BUTTON_NORMAL == recordButtonStatus) {
        m_toolBar->shapeClickedFromMain("audio");
    }
    //录屏快捷键 鼠标 m
    else if (KEY_M == keyCode && RECORD_BUTTON_NORMAL == recordButtonStatus) {
        m_toolBar->shapeClickedFromMain("mouse");
    }
    //录屏快捷键 选项 f3
    else if (KEY_F3 == keyCode  && RECORD_BUTTON_NORMAL == recordButtonStatus) {
        m_toolBar->shapeClickedFromMain("record_option");
    }
}
//启动录屏
void MainWindow::startRecord()
{
    recordButtonStatus = RECORD_BUTTON_RECORDING;
    resetCursor();
    repaint();
    //启动录屏
    //兼容性 1040以下的系统，系统托盘图标通过此方式实现
    if (Utils::isSysHighVersion1040() == false) {
        QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon1.svg"))));
        trayIcon->setToolTip(tr("Screen Capture"));
        QTimer *flashTrayIconTimer = new QTimer(this);
        connect(trayIcon, &QSystemTrayIcon::activated, this, [ = ] {
            //先停止系统托盘图标的闪烁
            flashTrayIconTimer->stop();
            stopRecord();
        });
        connect(flashTrayIconTimer, &QTimer::timeout, this, [ = ] {
            static int flashTrayIconCounter = 0;
            QString iconIndex = QString("trayicon%1.svg").arg(flashTrayIconCounter % 2 + 1);
            trayIcon->setIcon(QIcon((Utils::getQrcPath(iconIndex))));
            flashTrayIconCounter++;
        });
        flashTrayIconTimer->start(800);
        trayIcon->show();
    }
    // 平板环境状态栏闪烁
    if (Utils::isTabletEnvironment && m_tabletRecorderHandle) {
        m_tabletRecorderHandle->startStatusBar();
    }
#ifdef KF5_WAYLAND_FLAGE_ON
#if defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__) ||  defined (__aarch64__)
    //wayland下走此方法
    connect(this, &MainWindow::stopRecordArm, this, &MainWindow::stopRecord);
    checkTempFileArm();
#endif
#endif

    qDebug() << "录屏！";
    recordProcess.startRecord();
    // 录屏开始后，隐藏窗口。（2D窗管下支持录屏, 但是会导致摄像头录制不到）
    if (m_hasComposite == false) {
        hide();
        // 显示录屏框区域 和 摄像头。
        m_pRecorderRegion->setCameraShow();
        m_pRecorderRegion->show();
    }
}

/**
 * @brief 开始滚动截图的方式：鼠标左键点击捕捉区域
 */
void MainWindow::startAutoScrollShot()
{
#ifdef OCR_SCROLL_FLAGE_ON
    //自动滚动模式已启动
    m_isAutoScrollShotStart = true;
    //自动调整捕捉区域不显示
    m_isAdjustArea = false;
    qDebug() << "开始自动滚动截图！";
    //设置拼接线程为自动滚动模式
    m_scrollShot->setScrollModel(false);
    if (m_scrollShotStatus != 0) {
        qDebug() << "function: " << __func__ << " ,line: " << __LINE__ << " ,m_scrollShotStatus: " << m_scrollShotStatus;
        //滚动截图已经启动过
        bool ok;
        QRect rect(recordX + 1, recordY + 1, recordWidth - 2, recordHeight - 2);
        //抓取捕捉区域图片
        QPixmap img = m_screenGrabber.grabEntireDesktop(ok, rect, m_pixelRatio);
        //滚动截图处理类进行图片的拼接
        m_scrollShot->addPixmap(img);
    } else {
        //qDebug() << "function: " << __func__ << " ,line: " << __LINE__ << " ,m_scrollShotStatus: " << m_scrollShotStatus;
        //滚动截图从未启动过，滚动截图添加第一张图片并启动
        m_scrollShot->addPixmap(m_firstScrollShotImg);
    }
#endif
}

//暂停滚动截图
void MainWindow::pauseAutoScrollShot()
{
#ifdef OCR_SCROLL_FLAGE_ON
    qDebug() << "function:" << __func__ << " ,line: " << __LINE__ << " 暂停自动滚动截图!";
    //自动滚动截图改变状态，暂停自动滚动
    m_scrollShot->changeState(true);
#endif
}

//继续自动滚动截图
void MainWindow::continueAutoScrollShot()
{
#ifdef OCR_SCROLL_FLAGE_ON
    qDebug() << "function:" << __func__ << " ,line: " << __LINE__ << " 继续自动滚动截图!";
    if (m_tipShowtimer != nullptr) {
        m_tipShowtimer->stop();
    }
    m_scrollShotTip->hide();
    m_isAdjustArea = false;
    update();
    //设置拼接线程为自动滚动模式
    m_scrollShot->setScrollModel(false);
    //滚动截图改变状态，继续滚动
    m_scrollShot->changeState(false);
#endif
}

//开始手动滚动截图，只进入一次
void MainWindow::startManualScrollShot()
{
#ifdef OCR_SCROLL_FLAGE_ON
    //自动调整捕捉区域不显示
    m_isAdjustArea = false;
    qDebug() << "开始手动滚动截图！";
    //设置拼接线程为自动滚动模式
    m_scrollShot->setScrollModel(true);
    //滚动截图添加第一张图片并启动
    m_scrollShot->addPixmap(m_firstScrollShotImg);
#endif
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
    QPainter painter(&m_resultPixmap);

    //wayland模式下截取光标
    if (Utils::isWaylandMode) {
        painter.drawImage(QRect(x - recordX - m_CuresorImageWayland.width() / 2, y - recordY - m_CuresorImageWayland.height() / 2, m_CuresorImageWayland.width(), m_CuresorImageWayland.height()), m_CuresorImageWayland);
    }
    //x11模式下截取光标
    else {
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
        painter.drawImage(QRect(x - recordX - m_CursorImage->width / 2, y - recordY - m_CursorImage->height / 2, m_CursorImage->width, m_CursorImage->height), cursorImage);
        delete[] pixels;
        XFree(m_CursorImage);
    }
    return;
}

void MainWindow::shotFullScreen(bool isFull)
{
    QRect target = m_backgroundRect;
    qDebug() << "m_backgroundRect" << m_backgroundRect;
    if (Utils::isWaylandMode) {
        target = QRect(m_backgroundRect.x(),
                       m_backgroundRect.y(),
                       static_cast<int>(m_backgroundRect.width() * m_pixelRatio),
                       static_cast<int>(m_backgroundRect.height() * m_pixelRatio));
    }

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

        //QRect t_rectbuttonRect = m_recordButton->geometry();

        //t_rectbuttonRect.setX(t_rectbuttonRect.x() - 5);
        //t_rectbuttonRect.setY(t_rectbuttonRect.y() - 2);
        //t_rectbuttonRect.setWidth(t_rectbuttonRect.width() + 6);
        //t_rectbuttonRect.setHeight(t_rectbuttonRect.height() + 2);

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

            //}

            //else if (t_rectbuttonRect.contains(cursorX, cursorY)) {
            // Record button.
            //QApplication::setOverrideCursor(Qt::ArrowCursor);
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
    this->setCursor(Qt::CrossCursor);
}

void MainWindow::resetCursor()
{
    this->setCursor(Qt::ArrowCursor);
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
        qDebug() << "MainWindow::stopRecord()!";
        if (Utils::isWaylandMode) {
            if (m_isStopWaylandRecord) {
                return;
            }
            m_isStopWaylandRecord = true;
        }
        hide();
        emit releaseEvent();
        //正在保存录屏文件通知
        sendSavingNotify();
        // 状态栏闪烁停止
        if (Utils::isTabletEnvironment && m_tabletRecorderHandle) {
            m_tabletRecorderHandle->stop();
        }
        if (m_pRecorderRegion) {
            m_pRecorderRegion->hide();
            m_pRecorderRegion->setCameraShow(false);
        }
        recordButtonStatus = RECORD_BUTTON_SAVEING;
        recordProcess.stopRecord();
    }
}

void MainWindow::startCountdown()
{
    recordButtonStatus = RECORD_BUTTON_WAIT;
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
    if (m_hasComposite == false) {
        // 设置录屏框区域。
        m_pRecorderRegion =  new RecorderRegionShow();
        m_pRecorderRegion->resize(recordWidth + 2, recordHeight + 2);
        m_pRecorderRegion->move(std::max(recordX - 1, 0), std::max(recordY - 1, 0));
        if (m_cameraWidget && m_selectedCamera) {
            m_cameraWidget->hide();
            m_cameraWidget->cameraStop();
            m_cameraWidget->setCameraStop(true);
            m_pRecorderRegion->initCameraInfo(m_cameraWidget->postion(), m_cameraWidget->geometry().size());
        }
    }
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
    //m_recordButton->hide();
    //m_shotButton->hide();
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
    //m_shotButton->raise();
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
    m_initScroll = false; // 保存时关闭滚动截图
    emit releaseEvent();
    qApp->quit();
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
    update();
}
