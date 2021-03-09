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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "record_process.h"
#include "countdown_tooltip.h"
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
#include "RecorderRegionShow.h"
#include "event_monitor.h"
#include "screen_shot_event.h"
#include "recordertablet.h"

#include <DWindowManagerHelper>
#include <DDialog>
#include <DWidget>

#include <QApplication>
#include <QDesktopWidget>
#include <QObject>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QTimer>

#undef Bool

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
//DWM_USE_NAMESPACE

class MainWindow : public DWidget
{
    Q_OBJECT

//    Q_CLASSINFO("D-Bus Interface", "com.deepin.ScreenRecorder")

    //static const int CURSOR_BOUND;
    static const int RECORD_MIN_SIZE;
    static const int RECORD_MIN_HEIGHT;
    static const int RECORD_MIN_SHOT_SIZE;
    static const int DRAG_POINT_RADIUS;

    static const int RECORD_BUTTON_NORMAL;
    static const int RECORD_BUTTON_WAIT;
    static const int RECORD_BUTTON_RECORDING;
    static const int RECORD_BUTTON_SAVEING;

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

    enum status{
        record = 0,
        shot
    };

public:
    explicit MainWindow(DWidget *parent = nullptr);
    ~MainWindow()
    {
        if (m_pVoiceVolumeWatcher) {
            m_pVoiceVolumeWatcher->setWatch(false);
            //run函数里面有sleep,所以此处加terminate
            m_pVoiceVolumeWatcher->terminate();
            if (!QSysInfo::currentCpuArchitecture().startsWith("mips")) {
                m_pVoiceVolumeWatcher->wait();
                m_pVoiceVolumeWatcher = nullptr;
            }
        }
        if (m_pCameraWatcher) {
            m_pCameraWatcher->setWatch(false);
            m_pCameraWatcher->terminate();
            if (!QSysInfo::currentCpuArchitecture().startsWith("mips")) {
                m_pCameraWatcher->wait();
                m_pCameraWatcher = nullptr;
            }
        }
        if (m_pScreenRecordEvent) {
            m_pScreenRecordEvent->terminate();
            if (!QSysInfo::currentCpuArchitecture().startsWith("mips")) {
                m_pScreenRecordEvent->wait();
                m_pScreenRecordEvent = nullptr;
            }
        }
        if (m_pScreenShotEvent) {
            m_pScreenShotEvent->terminate();
            if (!QSysInfo::currentCpuArchitecture().startsWith("mips")) {
                m_pScreenShotEvent->wait();
                m_pScreenShotEvent = nullptr;
            }
        }
        if (m_showButtons) {
            //退出进程，最好不用deleteLater，因为有可能等不到下一次事件循环，导致资源不能释放
            delete m_showButtons;
            m_showButtons = nullptr;
        }
        if (countdownTooltip) {
            delete countdownTooltip;
            countdownTooltip = nullptr;
        }
        if (buttonFeedback) {
            delete buttonFeedback;
            buttonFeedback = nullptr;
        }
        if (m_shapesWidget) {
            delete m_shapesWidget;
            m_shapesWidget = nullptr;
        }
        if (m_sizeTips) {
            delete m_sizeTips;
            m_sizeTips = nullptr;
        }
        if (m_toolBar) {
            delete m_toolBar;
            m_toolBar = nullptr;
        }
        if (m_recordButton) {
            delete m_recordButton;
            m_recordButton = nullptr;
        }
        if (m_shotButton) {
            delete m_shotButton;
            m_shotButton = nullptr;
        }
        if (m_zoomIndicator) {
            delete m_zoomIndicator;
            m_zoomIndicator = nullptr;
        }
        if (m_menuController) {
            delete m_menuController;
            m_menuController = nullptr;
        }
        if (m_cameraWidget) {
            delete m_cameraWidget;
            m_cameraWidget = nullptr;
        }
        if (m_tabletRecorderHandle) {
            delete m_tabletRecorderHandle;
            m_tabletRecorderHandle = nullptr;
        }
        if (m_sideBar) {
            delete m_sideBar;
            m_sideBar = nullptr;
        }
        if (m_pRecorderRegion) {
            delete m_pRecorderRegion;
            m_pRecorderRegion = nullptr;
        }
        for (int i = 0; i < m_keyButtonList.count(); i++) {
            delete m_keyButtonList.at(i);
        }
        m_keyButtonList.clear();

        //以前的流程没执行到此处，没暴露延迟500ms的问题，以前的无用代码
        //QThread::currentThread()->msleep(500);
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
    //void delayScreenshot(double num);
    void fullScreenshot();

    void topWindow();
    void savePath(const QString &path);
    void startScreenshotFor3rd(const QString &path);
    void noNotify();
    void sendSavingNotify();
    // 强制录屏保存退出通知,3D->2D模式
    void forciblySavingNotify();

signals:
    void releaseEvent();
    void hideScreenshotUI();
    void saveActionTriggered();
    void unDo();
    void unDoAll();
    void deleteShapes();

public slots:
    void onExit();
    void startRecord();
    //void flashTrayIcon();
    //void iconActivated(QSystemTrayIcon::ActivationReason reason);
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
    void changeShowMouseShowEvent(bool checked);
    void changeMicrophoneSelectEvent(bool checked);
    void changeSystemAudioSelectEvent(bool checked);
    void changeCameraSelectEvent(bool checked);
    void updateMultiKeyBoardPos();
    void changeShotToolEvent(const QString &func);
    void saveScreenShot();
    bool saveAction(const QPixmap &pix);
    void sendNotify(SaveAction saveAction, QString saveFilePath, const bool succeed);
    void reloadImage(QString effect);
    void shotImgWidthEffect();
    void changeArrowAndLineEvent(int line);
    void exitApp();
    void onViewShortcut();
    void shapeClickedSlot(QString shape);
    void on_CheckRecodeCouldUse(bool canUse);
    void on_CheckVideoCouldUse(bool canUse);
    void checkCpuIsZhaoxin();
    void onShotKeyPressEvent(const unsigned char &keyCode);
    void onRecordKeyPressEvent(const unsigned char &keyCode);
    void tableRecordSet();
    /**
     * @brief initPadShot:初始化平板截图
     */
    void initPadShot();

    void exitScreenRecordEvent();
    void exitScreenShotEvent();

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
    void hideAllWidget();
    void adjustLayout(QVBoxLayout *layout, int layoutWidth, int layoutHeight);
    void initShapeWidget(QString type);
    int getRecordInputType(bool selectedMic, bool selectedSystemAudio);
    void initBackground();
    QPixmap getPixmapofRect(const QRect &rect);
    bool saveImg(const QPixmap &pix, const QString& fileName, const char* format = nullptr);
private:
//    QList<WindowRect> windowRects;
    QList<QRect> windowRects;
    QList<QString> windowNames;
    ShowButtons *m_showButtons = nullptr;
    //QTimer *flashTrayIconTimer = nullptr;
    QRect screenRect;
    RecordProcess recordProcess;
    voiceVolumeWatcher *m_pVoiceVolumeWatcher = nullptr;
    CameraWatcher *m_pCameraWatcher = nullptr;
    ScreenGrabber m_screenGrabber;
//    VoiceRecordProcess voiceRecordProcess;
//    WindowRect rootWindowRect;
    QRect rootWindowRect;
    SaveAction m_saveIndex = SaveAction::SaveToClipboard;
    //m_saveFileName is the storage path of the screenshot image.
    QString m_saveFileName;

    bool drawDragPoint = false;
    bool isFirstDrag = false;
    bool isFirstMove = false;
    bool isFirstPressButton = false;
    bool isFirstReleaseButton = false;
    bool isPressButton = false;
    bool isReleaseButton = false;

    int dragAction = 0;
    int dragRecordHeight = 0;
    int dragRecordWidth = 0;
    int dragRecordX = -1;
    int dragRecordY = -1;
    int dragStartX = 0;
    int dragStartY = 0;

    int m_shotStatus = ShotMouseStatus::Normal; //
    int recordButtonStatus = RECORD_BUTTON_NORMAL;
    int recordHeight = 0;
    int recordWidth = 0;
    int recordX = 0;
    int recordY = 0;

    int flashTrayIconCounter = 0;
    QPixmap m_backgroundPixmap;
    QPixmap m_resultPixmap;
    QPixmap resizeHandleBigImg;
    //QPixmap resizeHandleSmallImg;

    QString selectAreaName = "";
    //dde-dock显示时长插件代替系统托盘
    //QSystemTrayIcon *trayIcon = nullptr;
    CountdownTooltip *countdownTooltip = nullptr;
    ButtonFeedback *buttonFeedback = nullptr;
    EventMonitor *m_pScreenRecordEvent = nullptr;
    ScreenShotEvent *m_pScreenShotEvent = nullptr;

    DWindowManagerHelper *m_wmHelper;
    ShapesWidget *m_shapesWidget = nullptr;
    TopTips *m_sizeTips = nullptr;
    ToolBar *m_toolBar = nullptr;
    QRect m_backgroundRect;
    //添加截屏和录屏的按钮
    DPushButton *m_recordButton = nullptr;
    DPushButton *m_shotButton = nullptr;
    QList<KeyButtonWidget *> m_keyButtonList;

    int m_functionType = status::record;  //0: record, 1: shot
    bool m_keyBoardStatus = false; //false: keyBoard off, true:keyBoard On
    int m_mouseStatus = 0; //0: mouse check off, 1:mouse check On
    bool m_mouseShowStatus = true; //0: show mouse off, 1:show mouse On
    bool m_repaintMainButton = false;//false: no need to repaint record button or shot button, true:...
    bool m_repaintSideBar = false;   //false: no need to repaint sidebar, true:...

    bool m_selectedMic = true;
    bool m_selectedSystemAudio = true;
    int m_screenWidth = 0;  //屏幕宽度
    int m_screenHeight = 0; //屏幕高度
    SideBar *m_sideBar = nullptr; //截图功能侧边栏功能
    ZoomIndicator *m_zoomIndicator = nullptr;
    bool m_isShapesWidgetExist = false;
    bool m_isSideBarInside = false;
    bool m_isToolBarInside = false;


    //截图功能使用的变量初始化
    MenuController *m_menuController = nullptr;
    bool m_noNotify = false;
    bool m_isShiftPressed = false;
    bool m_needSaveScreenshot = false;
    bool m_toolBarInit = false;
    bool m_sideBarInit = false;
    CameraWidget *m_cameraWidget = nullptr;
    bool m_selectedCamera = false;
    bool m_cameraOffFlag = false;
    bool m_launchWithRecordFunc = false;
    bool m_shotWithPath = false;
    int m_screenCount;
    QString m_shotSavePath;
    int m_shotflag = 0;
    int m_firstShot = 0;
    bool m_isZhaoxin = false;
    QList<ScreenInfo> m_screenInfo;
    XFixesCursorImage *m_CursorImage = nullptr;
    QSize m_screenSize;
    RecorderRegionShow *m_pRecorderRegion = nullptr;
    qreal m_pixelRatio = 1.0;
    bool m_hasComposite = true;
    bool m_initScreenShot;
    bool m_initScreenRecorder;
    RecorderTablet *m_tabletRecorderHandle = nullptr;
    int m_cursorBound;
};

#endif //MAINWINDOW_H
