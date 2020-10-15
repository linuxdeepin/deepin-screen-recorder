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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DWindowManagerHelper>
//#include <dscreenwindowsutil.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QObject>
#include <QPainter>
#include <DWidget>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
//#include <dwindowmanager.h>
#include <QTimer>
#include <DDialog>

#include "record_process.h"
#include "voice_record_process.h"
#include "record_button.h"
#include "record_option_panel.h"
#include "countdown_tooltip.h"
//#include "start_tooltip.h"
#include "button_feedback.h"
#include "show_buttons.h"
#include "widgets/shapeswidget.h"
#include "widgets/toptips.h"
#include "widgets/toolbar.h"
#include "widgets/sidebar.h"
#include "widgets/keybuttonwidget.h"
#include "widgets/zoomIndicator.h"
#include "widgets/camerawidget.h"
#include "widgets/filter.h"
#include "utils/saveutils.h"
#include "utils/voicevolumewatcher.h"
#include "utils/camerawatcher.h"
#include "utils/screengrabber.h"

#include "dbusinterface/dbuscontrolcenter.h"
#include "dbusinterface/dbusnotify.h"
#include "dbusinterface/dbuszone.h"

// Make this include at last, otherwise QtX11 will conflict with x11 lib to make compile failed.
#include "event_monitor.h"
#include "screen_shot_event.h"


#include <KF5/KWayland/Client/connection_thread.h>
#include <KF5/KWayland/Client/clientmanagement.h>
#include <KF5/KWayland/Client/event_queue.h>
#include <KF5/KWayland/Client/registry.h>

#undef Bool

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
//DWM_USE_NAMESPACE

using namespace KWayland::Client;
class MainWindow : public DWidget
{
    Q_OBJECT

//    Q_CLASSINFO("D-Bus Interface", "com.deepin.ScreenRecorder")

    static const int CURSOR_BOUND;
    static const int RECORD_MIN_SIZE;
    static const int RECORD_MIN_HEIGHT;
    static const int RECORD_MIN_SHOT_SIZE;
    static const int DRAG_POINT_RADIUS;

    static const int RECORD_BUTTON_NORMAL;
    static const int RECORD_BUTTON_WAIT;
    static const int RECORD_BUTTON_RECORDING;

    static const int ACTION_MOVE;
    static const int ACTION_RESIZE_TOP_LEFT;
    static const int ACTION_RESIZE_TOP_RIGHT;
    static const int ACTION_RESIZE_BOTTOM_LEFT;
    static const int ACTION_RESIZE_BOTTOM_RIGHT;
    static const int ACTION_RESIZE_TOP;
    static const int ACTION_RESIZE_BOTTOM;
    static const int ACTION_RESIZE_LEFT;
    static const int ACTION_RESIZE_RIGHT;

    static const int RECORD_OPTIONAL_PADDING;

    static const int CAMERA_WIDGET_MAX_WIDTH;
    static const int CAMERA_WIDGET_MAX_HEIGHT;
    static const int CAMERA_WIDGET_MIN_WIDTH;
    static const int CAMERA_WIDGET_MIN_HEIGHT;

public:
    MainWindow(DWidget *parent = nullptr);
    ~MainWindow()
    {
        m_pVoiceVolumeWatcher->stopWatch();

        m_pCameraWatcher->stopWatch();
        QThread::currentThread()->msleep(500);
    }

    enum ShotMouseStatus {
        Normal,
        Wait,
        Shoting,
    };
    struct ScreenInfo {
        int x;
        int y;
        int width;
        int height;
        QString name;
    };
    // Split attributes and resource for speed up start.
    void initAttributes();
    void initResource();
    void initScreenShot();
    void initScreenRecorder();
    void initShortcut();
    void initLaunchMode(const QString &launchMode);
    void delayScreenshot(double num);
    void fullScreenshot();
    void testScreenshot();

    void topWindow();
    void saveTopWindow();
    void savePath(const QString &path);
    void noNotify();
    void setConfigThemeType(int themeType);
    void sendSavingNotify();

signals:
    void releaseEvent();
    void hideScreenshotUI();
    void saveActionTriggered();
    void unDo();
    void unDoAll();
    void deleteShapes();

public slots:
    void startRecord();
    void flashTrayIcon();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void shotCurrentImg();
    void addCursorToImage();
    void shotFullScreen(bool isFull = false);
    void onHelp();

    Q_SCRIPTABLE void stopRecord();
    void startCountdown();
    void showPressFeedback(int x, int y);
    void showDragFeedback(int x, int y);
    void showReleaseFeedback(int x, int y);
    void responseEsc();
    void onActivateWindow();
    void compositeChanged();
    void updateToolBarPos();
    void updateSideBarPos();
    void updateRecordButtonPos();
    void updateShotButtonPos();
    void updateCameraWidgetPos();
    void changeFunctionButton(QString type);
    void showKeyBoardButtons(const QString &key);
    void changeKeyBoardShowEvent(bool checked);
    void changeMouseShowEvent(bool checked);
    void changeMicrophoneSelectEvent(bool checked);
    void changeSystemAudioSelectEvent(bool checked);
    void changeGifSelectEvent(bool checked);
    void changeMp4SelectEvent(bool checked);
    void changeFrameRateEvent(int frameRate);
    void changeCameraSelectEvent(bool checked);
    void showMultiKeyBoardButtons();
    void updateMultiKeyBoardPos();
    void changeShotToolEvent(const QString &func);
    void saveScreenShot();
    bool saveAction(const QPixmap &pix);
    void sendNotify(SaveAction saveAction, QString saveFilePath, const bool succeed);
    void reloadImage(QString effect);
    void shotImgWidthEffect();
    void changeArrowAndLineEvent(int line);
    void exitApp();
//    void initVirtualCard();
    void onViewShortcut();
    void shapeClickedSlot(QString shape);
    void on_CheckRecodeCouldUse(bool canUse);
    void on_CheckVideoCouldUse(bool canUse);
    void checkCpuIsZhaoxin();
    void waylandwindowinfo(const QVector<ClientManagement::WindowState> &m_windowStates);

protected:
    bool eventFilter(QObject *object, QEvent *event);
    int getAction(QEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeBottom(QMouseEvent *event);
    void resizeLeft(QMouseEvent *event);
    void resizeRight(QMouseEvent *event);
    void resizeTop(QMouseEvent *event);
    void updateCursor(QEvent *event);
    void setDragCursor();
    void resetCursor();
    void setFontSize(QPainter &painter, int textSize);
    void showRecordButton();
    void hideRecordButton();
    void hideAllWidget();
    void hideCameraWidget();
    void adjustLayout(QVBoxLayout *layout, int layoutWidth, int layoutHeight);
    void initShapeWidget(QString type);
    int getRecordInputType(bool selectedMic, bool selectedSystemAudio);
    void initBackground();
    QPixmap getPixmapofRect(const QRect &rect);
    void installTipHint(QWidget *w, const QString &hintstr);
    void installHint(QWidget *w, QWidget *hint);
    void setupRegistry(Registry *registry);
private:
//    QList<WindowRect> windowRects;
    QList<QRect> windowRects;
    QList<QString> windowNames;
    ShowButtons *m_showButtons;

    QTimer *flashTrayIconTimer;

    QRect screenRect;

    RecordProcess recordProcess;
    voiceVolumeWatcher *m_pVoiceVolumeWatcher;
    CameraWatcher *m_pCameraWatcher;
    ScreenGrabber m_screenGrabber;
//    VoiceRecordProcess voiceRecordProcess;
//    WindowRect rootWindowRect;
    QRect rootWindowRect;

    SaveAction m_saveIndex;
    //m_saveFileName is the storage path of the screenshot image.
    QString m_saveFileName;

    bool drawDragPoint;

    bool isFirstDrag;
    bool isFirstMove;
    bool isFirstPressButton;
    bool isFirstReleaseButton;
    bool isPressButton;
    bool isReleaseButton;

    int dragAction;
    int dragRecordHeight;
    int dragRecordWidth;
    int dragRecordX;
    int dragRecordY;
    int dragStartX;
    int dragStartY;

    int m_shotStatus;
    int recordButtonStatus;
    int recordHeight;
    int recordWidth;
    int recordX;
    int recordY;

    int recordButtonState;

    int flashTrayIconCounter;


//    QRect m_backgroundRect;
    //for shot
    QPixmap m_backgroundPixmap;
    QPixmap m_resultPixmap;
    bool m_keyboardGrabbed = false;
    bool m_keyboardReleased = false;
    //for shot

    QPixmap resizeHandleBigImg;
    QPixmap resizeHandleSmallImg;

    QString selectAreaName;

    QSystemTrayIcon *trayIcon;

//    DWindowManager *windowManager;

    QVBoxLayout *recordButtonLayout;
    QVBoxLayout *countdownLayout;
    RecordButton *recordButton;
    RecordOptionPanel *recordOptionPanel;

//    StartTooltip *startTooltip;
    CountdownTooltip *countdownTooltip;

    ButtonFeedback *buttonFeedback;

    EventMonitor *m_pScreenRecordEvent;
    ScreenShotEvent *m_pScreenShotEvent;

    DWindowManagerHelper *m_wmHelper;
    ShapesWidget *m_shapesWidget;
    TopTips *m_sizeTips;
    ToolBar *m_toolBar;
//    DScreenWindowsUtil *m_swUtil;
    QRect m_backgroundRect;
    //添加截屏和录屏的按钮
    DPushButton *m_recordButton;
    DPushButton *m_shotButton;
    QList<KeyButtonWidget *> m_keyButtonList;
    QList<KeyButtonWidget *> m_tempkeyButtonList;

    int m_functionType;  //0: record, 1: shot
    int m_keyBoardStatus; //0: keyBoard off, 1:keyBoard On
    int m_mouseStatus; //0: keyBoard off, 1:keyBoard On
    bool m_repaintMainButton;//false: no need to repaint record button or shot button, true:...
    bool m_repaintSideBar;   //false: no need to repaint sidebar, true:...
    QTimer *m_keyBoardTimer;
    bool m_multiKeyButtonsInOnSec;

    bool m_selectedMic;
    bool m_selectedSystemAudio;
    bool m_gifMode;
    bool m_mp4Mode;
    int m_frameRate;
    int m_screenWidth;  //屏幕宽度
    int m_screenHeight; //屏幕高度
    SideBar *m_sideBar; //截图功能侧边栏功能
    ZoomIndicator *m_zoomIndicator;
    bool m_isShapesWidgetExist = false;
    bool m_isSideBarInside = false;
    bool m_isToolBarInside = false;


    //截图功能使用的变量初始化
    DBusZone *m_hotZoneInterface;
    DBusNotify *m_notifyDBInterface;
    MenuController *m_menuController;
    bool m_noNotify = false;
    bool m_isShiftPressed = false;
    bool m_drawNothing = false;
    bool m_needDrawSelectedPoint;
    bool m_needSaveScreenshot = false;
    bool m_interfaceExist = false;
    bool m_toolBarInit = false;
    bool m_sideBarInit = false;
    // Just use for debug.
    // int repaintCounter;
    CameraWidget *m_cameraWidget;
    bool m_selectedCamera = false;
    bool m_cameraOffFlag = false;
    bool m_initCamera = false;
    bool m_launchWithRecordFunc = false;
    bool m_shotWithPath = false;
    int m_screenNum;
    QString m_shotSavePath;
    //bool m_copyToClipboard = false;
    QString m_savePicturePath;
    int m_shotflag = 0;
    int m_firstShot = 0;
    bool m_isZhaoxin = false;
    HintFilter *hintFilter         = nullptr;
    DesktopInfo m_desktopInfo;
    QList<ScreenInfo> m_screenInfo;
    XFixesCursorImage *m_CursorImage = nullptr;
    QSize m_screenSize;

    // 获取wayland窗口信息相关。
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue = nullptr;
    Compositor *m_compositor = nullptr;
    PlasmaWindowManagement *m_windowManagement = nullptr;
    ClientManagement *m_clientManagement = nullptr;
    QVector<ClientManagement::WindowState> m_windowStates;
    bool m_isFullScreenShot = false;
};

#endif //MAINWINDOW_H
