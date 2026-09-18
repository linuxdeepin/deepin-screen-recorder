// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../../src/utils.h"
#include "../../src/main_window.h"

// main_window.h -> event_monitor.h -> <X11/Xlib.h> leaks macros (True, False,
// None, ...) that corrupt Qt6 enums pulled in by <QTest>/<QNetworkCookie>.
#include "undef_x11.h"

#include <QTest>
#include <QPoint>
#include <QScreen>
#include <QDir>
#include "stub.h"
#include "addr_pri.h"
#include <gtest/gtest.h>

using namespace testing;


ACCESS_PRIVATE_FIELD(MainWindow, ToolBar *, m_toolBar);
ACCESS_PRIVATE_FIELD(MainWindow, int,  m_screenCount);
ACCESS_PRIVATE_FIELD(MainWindow, ShapesWidget *, m_shapesWidget);

QRect geometry_stub()
{
    return QRect(0, 0, 1920, 1080);
}

void passInputEvent_stub(int wid)
{
    Q_UNUSED(wid);
}

qreal devicePixelRatio_stub_2()
{
    return 1;
}

int width_stub_2()
{
    return 1920;
}

int height_stub_2()
{
    return 1080;
}

QList<QScreen*> screens_stub()
{
    return QList<QScreen*>();
}

class MainWindowTest: public testing::Test
{

public:
    Stub stub;
//    MainWindow *m_window = nullptr;
    virtual void SetUp() override
    {
//        m_window = new MainWindow;
        std::cout << "start MainWindowTest" << std::endl;
        ConfigSettings::instance()->setValue("save", "saveCursor", true);
    }

    virtual void TearDown() override
    {
//        if (m_window) {
//            std::cout << "delete  m_window" << std::endl;
//            //m_window->deleteLater();
//            delete  m_window;
//            m_window = nullptr;
//        }
        std::cout << "end MainWindowTest" << std::endl;
        system("killall deepin-shortcut-viewer");
    }
};


static bool hasComposite_stub(void *obj)
{
    Q_UNUSED(obj);
    return false;
}


static QString CpuArchitecture_stub(void *obj)
{
    Q_UNUSED(obj);
    return  "mips";
}

void initMainWindow_stub()
{

    qDebug() << "replace initMainWindow!";
}

ACCESS_PRIVATE_FIELD(MainWindow, int, m_autoScrollFlagNext);
//
ACCESS_PRIVATE_FIELD(MainWindow, int, recordX);
ACCESS_PRIVATE_FIELD(MainWindow, int, recordY);
ACCESS_PRIVATE_FIELD(MainWindow, int, recordWidth);
ACCESS_PRIVATE_FIELD(MainWindow, int, recordHeight);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_screenWidth);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_screenHeight);
ACCESS_PRIVATE_FIELD(MainWindow, qreal, m_pixelRatio);
ACCESS_PRIVATE_FIELD(MainWindow, ScrollShotTip *, m_scrollShotTip);
ACCESS_PRIVATE_FUN(MainWindow, bool(), isToolBarInShotArea);
ACCESS_PRIVATE_FUN(MainWindow, QPoint(), getScrollShotTipPosition);
static bool isToolBarInShotArea_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}


ACCESS_PRIVATE_FIELD(MainWindow, int, m_scrollShotStatus);
ACCESS_PRIVATE_FIELD(MainWindow, ScrollScreenshot *, m_scrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), startAutoScrollShot);
//替换ScrollScreenshot的setScrollModel函数
static bool setScrollModel_stub(void *obj, bool)
{
    Q_UNUSED(obj);
    return true;
}
//替换ScrollScreenshot的addPixmap函数
static bool addPixmap_stub(void *obj, const QPixmap &piximg, int wheelDirection)
{
    Q_UNUSED(obj);
    Q_UNUSED(piximg);
    Q_UNUSED(wheelDirection);
    return true;
}
//开始自动滚动截图测试用例

ACCESS_PRIVATE_FUN(MainWindow, void(), pauseAutoScrollShot);
static bool changeState_stub(void *obj, bool wheelDirection)
{
    Q_UNUSED(obj);
    Q_UNUSED(wheelDirection);
    return true;
}
//暂停自动滚动截图测试用例
ACCESS_PRIVATE_FUN(MainWindow, void(), continueAutoScrollShot);
//继续自动滚动截图测试用例
ACCESS_PRIVATE_FUN(MainWindow, void(), startManualScrollShot);
//开始手动滚动截图测试用例
ACCESS_PRIVATE_FUN(MainWindow, void(int mouseTime, int direction), handleManualScrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(PreviewWidget::PostionStatus previewPostion, int direction, int mouseTime), scrollShotGrabPixmap);

static bool scrollShotGrabPixmap_stub(void *obj, PreviewWidget::PostionStatus previewPostion, int direction, int mouseTime)
{
    Q_UNUSED(obj);
    Q_UNUSED(previewPostion);
    Q_UNUSED(direction);
    Q_UNUSED(mouseTime);
    return true;
}
//处理手动滚动截图逻辑测试用例
void getInputEvent_stub(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    Q_UNUSED(wid);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);
    qDebug() << "getInputEvent_stub!";
}

void setInputEvent_stub()
{
    qDebug() << "setInputEvent_stub!";
}
ACCESS_PRIVATE_FUN(MainWindow, void(), setInputEvent);
//滚动截图设置区域穿透测试用例
ACCESS_PRIVATE_FUN(MainWindow, void(), setCancelInputEvent);
//滚动截图时取消捕捉区域的鼠标穿透测试用例
ACCESS_PRIVATE_FUN(MainWindow, void(), showAdjustArea);
static QRect getInvalidArea_stub(void *obj)
{
    Q_UNUSED(obj);
    return QRect(0, 0, 500, 500);
}
//显示可调整的捕捉区域大小及位置测试用例
//判断工具栏是否在在捕捉区域内部测试用例
void hideSomeToolBtn_stub()
{

}

void resetCursor_stub()
{

}

void showTip_stub(TipType tipType)
{
    Q_UNUSED(tipType);
    qDebug() << "showTip_stub!!!";
}

bool setBackgroundPixmap_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
QPoint getScrollShotTipPosition_stub(void *obj)
{
    Q_UNUSED(obj);
    return QPoint(500, 500);
}
void showScrollShot_stub()
{

}
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_initScroll);
ACCESS_PRIVATE_FIELD(MainWindow, TopTips *, m_sizeTips);
ACCESS_PRIVATE_FUN(MainWindow, void(), initScrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), resetCursor);
ACCESS_PRIVATE_FUN(MainWindow, void(), showScrollShot);
ACCESS_PRIVATE_FIELD(MainWindow, QPixmap, m_firstScrollShotImg);
//ACCESS_PRIVATE_FIELD(MainWindow, DPushButton *, m_shotButton);
ACCESS_PRIVATE_FIELD(MainWindow, TopTips *, m_scrollShotSizeTips);
ACCESS_PRIVATE_FIELD(MainWindow, PreviewWidget *, m_previewWidget);
ACCESS_PRIVATE_FUN(MainWindow, void(), updateToolBarPos);
//ACCESS_PRIVATE_FUN(MainWindow, void(), updateShotButtonPos);
static void updateToolBarPos_stub(void *obj)
{
    Q_UNUSED(obj);
    qDebug() << "调整工具栏位置";

}
static void updateShotButtonPos_stub(void *obj)
{
    Q_UNUSED(obj);
    qDebug() << "调整截图保存按钮位置";
}
//初始化滚动截图，显示滚动截图中的一些公共部件、例如工具栏、提示、图片大小、第一张预览图，单元测试用例
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isZhaoxin);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_scrollShotType);
//滚动截图模式，抓取当前捕捉区域的图片，传递给滚动截图处理类进行图片的拼接，单元测试
static void updateImage_stub(void *obj, QImage img)
{
    Q_UNUSED(obj);
    Q_UNUSED(img);
    qDebug() << "预览窗口更新图片";
}
ACCESS_PRIVATE_FIELD(MainWindow, ScreenGrabber, m_screenGrabber);
ACCESS_PRIVATE_FUN(MainWindow, void(QImage img), showPreviewWidgetImage);
//显示预览窗口和图片单元测试用例
ACCESS_PRIVATE_FIELD(MainWindow, int, m_scrollShotMouseClick);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isErrorWithScrollShot);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isAutoScrollShotStart);
ACCESS_PRIVATE_FUN(MainWindow, void(int x, int y), scrollShotMouseClickEvent);
bool startAutoScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
bool pauseAutoScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
bool setCancelInputEvent_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
bool continueAutoScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
//滚动截图鼠标按钮事件单元测试用例
ACCESS_PRIVATE_FUN(MainWindow, void(int x, int y), scrollShotMouseMoveEvent);
void disableXGrabButton_stub()
{

}
void enableXGrabButton_stub()
{

}
//滚动截图鼠标移动事件处理,单元测试用例
ACCESS_PRIVATE_FUN(MainWindow, void(int mouseTime, int direction, int x, int y), scrollShotMouseScrollEvent);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_autoScrollFlagLast);
bool handleManualScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
bool startManualScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
//滚动截图时处理鼠标滚轮滚动,单元测试用例
ACCESS_PRIVATE_FUN(MainWindow, void(QDBusMessage msg), onLockScreenEvent);
//滚动截图时，锁屏处理事件，单元测试用例
ACCESS_PRIVATE_FUN(MainWindow, void(), onOpenScrollShotHelp);
//打开截图录屏帮助文档并定位到滚动截图 单元测试
ACCESS_PRIVATE_FUN(MainWindow, void(PixMergeThread::MergeErrorValue state), onScrollShotMerageImgState);
ACCESS_PRIVATE_FUN(MainWindow, QPixmap(const QRect &rect), getPixmapofRect);
ACCESS_PRIVATE_FIELD(MainWindow, QTimer *, m_tipShowtimer);
bool showAdjustArea_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}

QPixmap getPixmapofRect_stub(void *obj, const QRect &rect)
{
    Q_UNUSED(obj);
    Q_UNUSED(rect);
    QPixmap pixmap;
    return pixmap;
}



//滚动截图时，获取拼接时的状态  单元测试用例
ACCESS_PRIVATE_FIELD(MainWindow, QRect, m_adjustArea);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isAdjustArea);
ACCESS_PRIVATE_FUN(MainWindow, void(), onAdjustCaptureArea);
bool clearPixmap_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
//自动调整捕捉区域的大小及位置 单元测试用例
void initAttributes_stub()
{

}
void initLaunchMode_stub(const QString &launchMode)
{
    Q_UNUSED(launchMode);
}
void showFullScreen_stub()
{

}
void initResource_stub()
{

}
void shotFullScreen_stub(bool flag)
{
    Q_UNUSED(flag);
}
bool saveAction_stub(const QPixmap &pix)
{
    Q_UNUSED(pix);
    return true;
}
void sendNotify_stub(SaveAction saveAction, QString saveFilePath, const bool succeed)
{
    Q_UNUSED(saveAction);
    Q_UNUSED(saveFilePath);
    Q_UNUSED(succeed);

}
ACCESS_PRIVATE_FIELD(MainWindow, QPixmap, m_backgroundPixmap);
ACCESS_PRIVATE_FIELD(MainWindow, QList<QRect>, windowRects);
ACCESS_PRIVATE_FIELD(MainWindow, QList<QString>, windowNames);
//TEST_F(MainWindowTest, noNotify)
//{

//    MainWindow *window = new MainWindow();
//    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
//    stub.set(ADDR(MainWindow, initAttributes), initAttributes_stub);
//    stub.set(ADDR(MainWindow, initLaunchMode), initLaunchMode_stub);
//    stub.set(ADDR(MainWindow, showFullScreen), showFullScreen_stub);
//    stub.set(ADDR(MainWindow, initResource), initResource_stub);

//    window->noNotify();

//    stub.reset(ADDR(MainWindow, initMainWindow));
//    stub.reset(ADDR(MainWindow, initAttributes));
//    stub.reset(ADDR(MainWindow, initLaunchMode));
//    stub.reset(ADDR(MainWindow, showFullScreen));
//    stub.reset(ADDR(MainWindow, initResource));
//    delete window;

//}
bool isWaylandMode_stub()
{
    return true;
}
ACCESS_PRIVATE_FUN(MainWindow, void(QWheelEvent *), wheelEvent);
ACCESS_PRIVATE_FIELD(MainWindow, CameraWidget *, m_cameraWidget);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_selectedCamera);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_shotflag);
int getRecordWidth_stub()
{
    return 500;
}
int getRecordHeight_stub()
{
    return 500;
}
CameraWidget::Position postion_stub()
{
    return CameraWidget::Position::leftBottom;
}
void startCountdown_stub()
{

}
ACCESS_PRIVATE_FIELD(MainWindow, QSize, m_screenSize);
void showDragFeedback_stub(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}
ACCESS_PRIVATE_FIELD(MainWindow, unsigned int, m_functionType);
void showPressFeedback_stub(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}
void scrollShotMouseClickEvent_stub(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}
void showReleaseFeedback_stub(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}
void showContentButtons_stub(int s)
{
}
void responseEsc_stub()
{
}
ACCESS_PRIVATE_FIELD(MainWindow, int, recordButtonStatus);
ACCESS_PRIVATE_FIELD(MainWindow, ShowButtons *, m_showButtons);
void shapeClickedFromMain_stub(QString s)
{
}

ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isShapesWidgetExist);

bool isSysHighVersion1040_stub()
{

    return true;
}
void startRecord_stub()
{
}
void m_pRecorderRegion_stub()
{
}
void whileCheckTempFileArm_stub()
{
}
ACCESS_PRIVATE_FIELD(MainWindow, RecorderRegionShow *, m_pRecorderRegion);
#ifdef KF5_WAYLAND_FLAGE_ON
// The following tests reference KWayland::Client::ClientManagement,
// ConnectionThread and the KF5-gated MainWindow members (m_connectionThread,
// whileCheckTempFileArm, waylandwindowinfo). They only compile when
// KF5_WAYLAND_FLAGE_ON is defined, which is currently disabled (see .pro).
ACCESS_PRIVATE_FIELD(MainWindow, QThread *, m_connectionThread);
ACCESS_PRIVATE_FUN(MainWindow, void(), whileCheckTempFileArm);
ACCESS_PRIVATE_FIELD(MainWindow, ConnectionThread *, m_connectionThreadObject);
ACCESS_PRIVATE_FIELD(MainWindow, QList<MainWindow::ScreenInfo>, m_screenInfo);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isVertical);
ACCESS_PRIVATE_FUN(MainWindow, void(const QVector<ClientManagement::WindowState> &), waylandwindowinfo);

#endif // KF5_WAYLAND_FLAGE_ON
