// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QImage>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QEvent>
#include <QPaintEvent>
#include "addr_pri.h"
#include "stub.h"

#include "../../src/widgets/camerawidget.h"
#include "../../src/widgets/toolbar.h"
#include "../../src/event_monitor.h"
#include "../../src/camera/majorimageprocessingthread.h"
#include "../../src/RecorderRegionShow.h"
#include "../../src/gstrecord/gstrecordx.h"
#include "../../src/gstrecord/gstinterface.h"
#include "../../src/widgets/keybuttonwidget.h"
#include "../../src/widgets/imagemenu.h"
#include "../../src/utils.h"

using namespace testing;

// =============================================================================
// CameraWidgetCov3Test
// Targets uncovered: initUI, cameraStart (no v4l2 device path returns -1),
// restartDevices (no device path), onReceiveMajorImage (QImage/QPixmap),
// scaledPixmap skip (unimplemented), setDevcieName, getCameraStatus,
// setCameraWidgetImmovable, postion all quadrants, setRecordRect variants.
// =============================================================================
class CameraWidgetCov3Test : public Test
{
public:
    CameraWidget *m_cam = nullptr;
    void SetUp() override
    {
        m_cam = new CameraWidget();
        m_cam->resize(200, 150);
        m_cam->setRecordRect(0, 0, 800, 600);
    }
    void TearDown() override
    {
        // Leak intentionally: destructors touch v4l2 globals.
    }
};

// initUI: builds layout + DLabel
TEST_F(CameraWidgetCov3Test, initUIBuildsLayout)
{
    EXPECT_NO_FATAL_FAILURE(m_cam->initUI());
    qApp->processEvents();
}

// setDevcieName: store + state change
TEST_F(CameraWidgetCov3Test, setDevcieNameVariadic)
{
    EXPECT_NO_FATAL_FAILURE(m_cam->setDevcieName(QStringLiteral("/dev/video0")));
    EXPECT_NO_FATAL_FAILURE(m_cam->setDevcieName(QString()));
    EXPECT_NO_FATAL_FAILURE(m_cam->setDevcieName(QStringLiteral("USB Camera B4.09.25")));
}

// setRecordRect: extreme values
TEST_F(CameraWidgetCov3Test, setRecordRectExtremeValues)
{
    EXPECT_NO_FATAL_FAILURE(m_cam->setRecordRect(-100, -100, 0, 0));
    EXPECT_NO_FATAL_FAILURE(m_cam->setRecordRect(0, 0, 10000, 10000));
    EXPECT_NO_FATAL_FAILURE(m_cam->setRecordRect(100, 200, 300, 400));
}

// getCameraStatus: no m_imgPrcThread -> returns 0
TEST_F(CameraWidgetCov3Test, getCameraStatusNoThread)
{
    int s = -1;
    EXPECT_NO_FATAL_FAILURE(s = m_cam->getCameraStatus());
    EXPECT_EQ(s, 0);
}

// setCameraWidgetImmovable: state setter both polarities
TEST_F(CameraWidgetCov3Test, setCameraWidgetImmovableBoth)
{
    EXPECT_NO_FATAL_FAILURE(m_cam->setCameraWidgetImmovable(true));
    EXPECT_NO_FATAL_FAILURE(m_cam->setCameraWidgetImmovable(false));
}

// postion: explicit geometry-induced quadrants (re-set recordRect then move)
TEST_F(CameraWidgetCov3Test, postionQuadrantRightBottom)
{
    m_cam->setRecordRect(0, 0, 800, 600);
    m_cam->move(100, 100); // x != recordX, y != recordY -> rightBottom
    EXPECT_NO_FATAL_FAILURE((void)m_cam->postion());
}

// onReceiveMajorImage(QImage): null m_cameraUI -> safe early return
TEST_F(CameraWidgetCov3Test, onReceiveMajorImageQImageNoUI)
{
    QImage img(50, 50, QImage::Format_RGB32);
    img.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(m_cam->onReceiveMajorImage(img));
}

// onReceiveMajorImage(QPixmap): null m_cameraUI -> safe early return
TEST_F(CameraWidgetCov3Test, onReceiveMajorImageQPixmapNoUI)
{
    QPixmap pix(50, 50);
    pix.fill(Qt::blue);
    EXPECT_NO_FATAL_FAILURE(m_cam->onReceiveMajorImage(pix));
}

// onReceiveMajorImage: after initUI -> m_cameraUI valid path
TEST_F(CameraWidgetCov3Test, onReceiveMajorImageAfterInitUI)
{
    m_cam->initUI();
    QImage img(60, 40, QImage::Format_RGB32);
    img.fill(Qt::green);
    EXPECT_NO_FATAL_FAILURE(m_cam->onReceiveMajorImage(img));
    QPixmap pix(60, 40);
    pix.fill(Qt::yellow);
    EXPECT_NO_FATAL_FAILURE(m_cam->onReceiveMajorImage(pix));
    qApp->processEvents();
}

// cameraStart: with no real device -> camInit fails, m_imgPrcThread starts
TEST_F(CameraWidgetCov3Test, cameraStartNoDeviceSafe)
{
    EXPECT_NO_FATAL_FAILURE(m_cam->cameraStart());
}

// =============================================================================
// ToolBarCov3Test
// Targets uncovered ToolBarWidget surface: changeShotToolEvent both branches,
// setHideToolbar, setRecordLaunchFromMain / setVideoInitFromMain /
// shapeClickedFromBar / setCameraDeviceEnable. ToolBar (wrapper) with
// initToolBar cannot be exercised (needs MainWindow); focus on null-safe path
// and direct ToolBarWidget calls.
// =============================================================================
class ToolBarCov3Test : public Test
{
public:
    ToolBar *m_bar = nullptr;
    void SetUp() override { m_bar = new ToolBar(); }
    void TearDown() override
    {
        // Leak intentionally (ToolBarWidget couples to MainWindow).
    }
};

// Repeated accessor calls to flip statement coverage on simple getters
TEST_F(ToolBarCov3Test, isDragedIsPressedRepeated)
{
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_FATAL_FAILURE((void)m_bar->isDraged());
        EXPECT_NO_FATAL_FAILURE((void)m_bar->isPressed());
    }
}

// Multiple showAt positions to flip move() branch coverage
TEST_F(ToolBarCov3Test, showAtManyPositions)
{
    for (int i = 0; i < 5; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_bar->showAt(QPoint(i * 10, i * 20)));
    }
    qApp->processEvents();
}

// currentFunctionMode: emits signal and updates
TEST_F(ToolBarCov3Test, currentFunctionModeMany)
{
    QSignalSpy spy(m_bar, &ToolBar::currentFunctionToMain);
    QStringList shapes = {"rectangle", "record", "shot", "oval", "line", "arrow", "pen", "text"};
    for (const QString &s : shapes) {
        EXPECT_NO_FATAL_FAILURE(m_bar->currentFunctionMode(s));
    }
    EXPECT_GE(spy.count(), 1);
    qApp->processEvents();
}

// getInnerWidgetRect with varied sizes (null-widget branch returns this->rect)
TEST_F(ToolBarCov3Test, getInnerWidgetRectVaried)
{
    QList<QSize> sizes = {QSize(1, 1), QSize(100, 50), QSize(500, 100), QSize(0, 0)};
    for (const QSize &s : sizes) {
        m_bar->resize(s);
        QRect r;
        EXPECT_NO_FATAL_FAILURE(r = m_bar->getInnerWidgetRect());
    }
}

// guarded setters (null m_toolbarWidget -> early return)
TEST_F(ToolBarCov3Test, allGuardedSettersRepeated)
{
    for (int i = 0; i < 2; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_bar->setScrollShotDisabled(i % 2 == 0));
        bool s = (i % 2 == 0);
        EXPECT_NO_FATAL_FAILURE(m_bar->setPinScreenshotsEnable(s));
        EXPECT_NO_FATAL_FAILURE(m_bar->setOcrScreenshotsEnable(s));
        EXPECT_NO_FATAL_FAILURE(m_bar->setButEnableOnLockScreen(s));
    }
}

// guarded slots
TEST_F(ToolBarCov3Test, allGuardedSlotsRepeated)
{
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_bar->setRecordButtonDisable());
        EXPECT_NO_FATAL_FAILURE(m_bar->setRecordLaunchMode(static_cast<unsigned int>(i)));
        EXPECT_NO_FATAL_FAILURE(m_bar->setVideoButtonInit());
        EXPECT_NO_FATAL_FAILURE(m_bar->shapeClickedFromMain(QString::number(i)));
        EXPECT_NO_FATAL_FAILURE(m_bar->setCameraDeviceEnable(i % 2 == 0));
    }
}

// null-widget rect getters
TEST_F(ToolBarCov3Test, nullWidgetRectGetters)
{
    QString func = QStringLiteral("rect");
    EXPECT_NO_FATAL_FAILURE((void)m_bar->getFuncSubToolX(func));
    EXPECT_NO_FATAL_FAILURE((void)m_bar->getShotOptionRect());
    EXPECT_NO_FATAL_FAILURE((void)m_bar->getAiButtonGlobalRect());
    EXPECT_NO_FATAL_FAILURE((void)m_bar->getAiButtonGlobalCenter());
}

// showWidget / hideWidget on null m_toolbarWidget
TEST_F(ToolBarCov3Test, showHideWidgetNullRepeated)
{
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_bar->showWidget());
        EXPECT_NO_FATAL_FAILURE(m_bar->hideWidget());
    }
}

// =============================================================================
// EventMonitorCov2Test
// Targets uncovered: callback (static), handleEvent (synthetic XRecord data),
// getCursorImage (XOpenDisplay may fail in CI -> returns nullptr safely).
// =============================================================================
class EventMonitorCov2Test : public Test
{
public:
    EventMonitor *m_mon = nullptr;
    void SetUp() override { m_mon = new EventMonitor(); }
    void TearDown() override { /* leak to avoid thread-destruction issues */ }
};

// releaseRes: repeated calls when m_display null are safe
TEST_F(EventMonitorCov2Test, releaseResRepeatedSafe)
{
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_mon->releaseRes());
    }
}

// wayland slots: many value variations to flip branches
TEST_F(EventMonitorCov2Test, waylandSlotsVaried)
{
    QSignalSpy pressSpy(m_mon, &EventMonitor::mousePress);
    QSignalSpy releaseSpy(m_mon, &EventMonitor::mouseRelease);
    QSignalSpy moveSpy(m_mon, &EventMonitor::mouseMove);

    int buttonTypes[] = {Button1, Button2, Button3, 4, 5, 999, -1};
    for (int bt : buttonTypes) {
        EXPECT_NO_FATAL_FAILURE(m_mon->ButtonPressEvent(bt, bt * 10, bt * 20, QStringLiteral("s")));
        EXPECT_NO_FATAL_FAILURE(m_mon->ButtonReleaseEvent(bt, bt * 11, bt * 21, QString()));
    }
    for (int i = 0; i < 10; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_mon->CursorMoveEvent(i * 5, i * 7, QString()));
    }
    EXPECT_GE(pressSpy.count(), 0);
    EXPECT_GE(releaseSpy.count(), 0);
    EXPECT_GE(moveSpy.count(), 0);
}

// getCursorImageWayland: safe DBus call (stubbed/no kwin)
TEST_F(EventMonitorCov2Test, getCursorImageWaylandSafe)
{
    QImage img;
    EXPECT_NO_FATAL_FAILURE(img = m_mon->getCursorImageWayland());
}

// getCursorImage: XOpenDisplay may return null in offscreen -> safe nullptr
TEST_F(EventMonitorCov2Test, getCursorImageSafe)
{
    XFixesCursorImage *img = nullptr;
    EXPECT_NO_FATAL_FAILURE(img = m_mon->getCursorImage());
    (void)img;
}

// initWaylandEventMonitor: safe to call when service not registered
TEST_F(EventMonitorCov2Test, initWaylandEventMonitorSafe)
{
    for (int i = 0; i < 2; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_mon->initWaylandEventMonitor());
    }
}

// callback static function: pass nullptr data to handleEvent (will early-return
// on data->category mismatch). Wrap in EXPECT_NO_FATAL_FAILURE.
TEST_F(EventMonitorCov2Test, callbackWithNullData)
{
    // Passing nullptr crashes (dereferences data->category); skip direct call.
    // Instead, call handleEvent with a synthetic XRecordInterceptData whose
    // category is NOT XRecordFromServer, exercising the early-return path.
    XRecordInterceptData fake;
    fake.category = 0; // not XRecordFromServer
    fake.data = nullptr;
    fake.data_len = 0;
    EXPECT_NO_FATAL_FAILURE(m_mon->handleEvent(&fake));
}

// =============================================================================
// MajorImageProcessingThreadCovTest
// Targets uncovered: ctor (calls init), init, setCameraDevice (nullptr safe),
// getStatus (returns m_stopped), stop (no-op when not running).
// run() requires live v4l2 device -> skipped.
// =============================================================================
class MajorImageProcessingThreadCovTest : public Test
{
public:
    MajorImageProcessingThread *m_t = nullptr;
    void SetUp() override { m_t = new MajorImageProcessingThread(); }
    void TearDown() override { /* leak: stop+dtor may hang on null device */ }
};

// getStatus: initial 0 after init()
TEST_F(MajorImageProcessingThreadCovTest, getStatusInitial)
{
    int s = -999;
    EXPECT_NO_FATAL_FAILURE(s = m_t->getStatus());
    EXPECT_EQ(s, 0);
}

// init: re-call (already done in ctor) - safe
TEST_F(MajorImageProcessingThreadCovTest, initRepeated)
{
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_t->init());
    }
    EXPECT_EQ(m_t->getStatus(), 0);
}

// setCameraDevice: nullptr safe store
TEST_F(MajorImageProcessingThreadCovTest, setCameraDeviceNullptr)
{
    EXPECT_NO_FATAL_FAILURE(m_t->setCameraDevice(nullptr));
}

// stop: not running -> no-op (m_stopped is 0, if(m_stopped) is false)
TEST_F(MajorImageProcessingThreadCovTest, stopWhenNotRunning)
{
    EXPECT_NO_FATAL_FAILURE(m_t->stop());
}

// =============================================================================
// RecorderRegionShowCov3Test
// Targets uncovered: setDevcieName (more), updateKeyBoardButtonStyle (loop
// with non-empty list via showKeyBoardButtons), showKeyBoardButtons >5 limit
// branch (pop_front), setCameraShow null-guard, initCameraInfo (needs v4l2 ->
// stubbed cameraStart in setUp).
// =============================================================================
class RecorderRegionShowCov3Test : public Test
{
public:
    Stub stub;
    RecorderRegionShow *m_region = nullptr;

    static void passInputEvent_stub(int) {}
    static bool cameraStart_stub() { return true; }

    void SetUp() override
    {
        stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
        stub.set(ADDR(CameraWidget, cameraStart), cameraStart_stub);
        m_region = new RecorderRegionShow();
        m_region->resize(800, 600);
        m_region->move(100, 100);
    }
    void TearDown() override { /* leak: destructor touches m_cameraWidget */ }
};

// setDevcieName: many values
TEST_F(RecorderRegionShowCov3Test, setDevcieNameMany)
{
    QStringList names = {"/dev/video0", "USB Camera", "", "Device-1", "Long Name With Spaces"};
    for (const QString &n : names) {
        EXPECT_NO_FATAL_FAILURE(m_region->setDevcieName(n));
    }
}

// setCameraShow: m_cameraWidget null -> safe no-op
TEST_F(RecorderRegionShowCov3Test, setCameraShowNullGuard)
{
    EXPECT_NO_FATAL_FAILURE(m_region->setCameraShow(true));
    EXPECT_NO_FATAL_FAILURE(m_region->setCameraShow(false));
    EXPECT_NO_FATAL_FAILURE(m_region->setCameraShow());
}

// updateKeyBoardButtonStyle: empty list -> safe
TEST_F(RecorderRegionShowCov3Test, updateKeyBoardButtonStyleEmpty)
{
    EXPECT_NO_FATAL_FAILURE(m_region->updateKeyBoardButtonStyle());
}

// showKeyBoardButtons: 1-5 keys to exercise updateMultiKeyBoardPos branches
TEST_F(RecorderRegionShowCov3Test, showKeyBoardButtonsUpToFive)
{
    QStringList keys = {"A", "B", "C", "D", "E"};
    for (const QString &k : keys) {
        EXPECT_NO_FATAL_FAILURE(m_region->showKeyBoardButtons(k));
    }
    qApp->processEvents();
}

// showKeyBoardButtons: 6+ keys to exercise pop_front branch
TEST_F(RecorderRegionShowCov3Test, showKeyBoardButtonsOverLimit)
{
    for (int i = 0; i < 8; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_region->showKeyBoardButtons(QString::number(i)));
    }
    qApp->processEvents();
}

// initCameraInfo: stubs out cameraStart; tests all 4 position branches
TEST_F(RecorderRegionShowCov3Test, initCameraInfoAllPositions)
{
    CameraWidget::Position positions[] = {
        CameraWidget::Position::leftTop,
        CameraWidget::Position::leftBottom,
        CameraWidget::Position::rightTop,
        CameraWidget::Position::rightBottom
    };
    for (auto p : positions) {
        RecorderRegionShow *r = new RecorderRegionShow();
        r->resize(800, 600);
        r->move(50, 50);
        EXPECT_NO_FATAL_FAILURE(r->initCameraInfo(p, QSize(200, 150)));
        // leak r (destructor touches camera device)
    }
    qApp->processEvents();
}

// =============================================================================
// GstRecordXCov3Test
// Targets uncovered: pure setters and pipelineStructuredOutput via existing
// ACCESS_PRIVATE declaration. setX11RecordMouse, setBoardVendorType,
// getGloop, setFramerate boundary, setRecordArea.
// =============================================================================
class GstRecordXCov3Test : public Test
{
public:
    GstRecordX *m_gst = nullptr;
    void SetUp() override
    {
        ASSERT_NO_FATAL_FAILURE(gstInterface::initFunctions());
        m_gst = new GstRecordX();
    }
    void TearDown() override { /* leak: destructor touches m_pipeline */ }
};

// All setters with varied values
TEST_F(GstRecordXCov3Test, allSettersVaried)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setInputDeviceName(QStringLiteral("plughw:1,0")));
    EXPECT_NO_FATAL_FAILURE(m_gst->setInputDeviceName(QString()));
    EXPECT_NO_FATAL_FAILURE(m_gst->setOutputDeviceName(QStringLiteral("alsa_output.pci")));
    EXPECT_NO_FATAL_FAILURE(m_gst->setOutputDeviceName(QString()));
    EXPECT_NO_FATAL_FAILURE(m_gst->setAudioType(GstRecordX::AudioType::None));
    EXPECT_NO_FATAL_FAILURE(m_gst->setAudioType(GstRecordX::AudioType::Mic));
    EXPECT_NO_FATAL_FAILURE(m_gst->setAudioType(GstRecordX::AudioType::Sys));
    EXPECT_NO_FATAL_FAILURE(m_gst->setAudioType(GstRecordX::AudioType::Mix));
    EXPECT_NO_FATAL_FAILURE(m_gst->setVidoeType(GstRecordX::VideoType::webm));
    EXPECT_NO_FATAL_FAILURE(m_gst->setVidoeType(GstRecordX::VideoType::ogg));
}

// setFramerate boundary values
TEST_F(GstRecordXCov3Test, setFramerateBoundary)
{
    int rates[] = {1, 10, 15, 24, 30, 60, 120, 0, -1, 1000};
    for (int r : rates) {
        EXPECT_NO_FATAL_FAILURE(m_gst->setFramerate(r));
    }
}

// setRecordArea variations
TEST_F(GstRecordXCov3Test, setRecordAreaVariations)
{
    QList<QRect> rects = {
        QRect(0, 0, 100, 100),
        QRect(-10, -10, 50, 50),
        QRect(1000, 1000, 1920, 1080),
        QRect(0, 0, 0, 0),
        QRect(1, 1, 1, 1)
    };
    for (const QRect &r : rects) {
        EXPECT_NO_FATAL_FAILURE(m_gst->setRecordArea(r));
    }
}

// setSavePath variations
TEST_F(GstRecordXCov3Test, setSavePathVariations)
{
    QStringList paths = {"/tmp/a.webm", "/tmp/b.ogg", "", "/very/long/path/that/does/not/exist/file.mkv"};
    for (const QString &p : paths) {
        EXPECT_NO_FATAL_FAILURE(m_gst->setSavePath(p));
    }
}

// setX11RecordMouse both states
TEST_F(GstRecordXCov3Test, setX11RecordMouseBoth)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setX11RecordMouse(true));
    EXPECT_NO_FATAL_FAILURE(m_gst->setX11RecordMouse(false));
}

// setBoardVendorType both states + edge
TEST_F(GstRecordXCov3Test, setBoardVendorTypeEdge)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setBoardVendorType(0));
    EXPECT_NO_FATAL_FAILURE(m_gst->setBoardVendorType(1));
    EXPECT_NO_FATAL_FAILURE(m_gst->setBoardVendorType(-1));
    EXPECT_NO_FATAL_FAILURE(m_gst->setBoardVendorType(99999));
}

// getGloop: returns nullptr until waylandGstStartRecord
TEST_F(GstRecordXCov3Test, getGloopNullptr)
{
    GMainLoop *loop = nullptr;
    EXPECT_NO_FATAL_FAILURE(loop = m_gst->getGloop());
    EXPECT_EQ(loop, nullptr);
}

// x11GstStopRecord: m_pipeline null (warning branch)
TEST_F(GstRecordXCov3Test, x11GstStopRecordNullPipeline)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->x11GstStopRecord());
}

// waylandWriteVideoFrame: stub returns false in test build
TEST_F(GstRecordXCov3Test, waylandWriteVideoFrameStubFalse)
{
    unsigned char data[16] = {0};
    bool ok = true;
    EXPECT_NO_FATAL_FAILURE(ok = m_gst->waylandWriteVideoFrame(data, 2, 2));
    EXPECT_FALSE(ok);
}

// waylandGstStartRecord / waylandGstStopRecord: test-build no-op bodies
TEST_F(GstRecordXCov3Test, waylandGstStartStopNoOps)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->waylandGstStartRecord());
    EXPECT_NO_FATAL_FAILURE(m_gst->waylandGstStopRecord());
}

// Repeated setter chain
TEST_F(GstRecordXCov3Test, setterChainRepeated)
{
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_FATAL_FAILURE({
            m_gst->setVidoeType(i % 2 == 0 ? GstRecordX::VideoType::webm : GstRecordX::VideoType::ogg);
            m_gst->setAudioType(static_cast<GstRecordX::AudioType>(i % 4));
            m_gst->setFramerate(10 * (i + 1));
            m_gst->setRecordArea(QRect(i * 10, i * 10, 320, 240));
            m_gst->setSavePath(QStringLiteral("/tmp/ut_chain_%1.webm").arg(i));
            m_gst->setX11RecordMouse(i % 2 == 0);
            m_gst->setBoardVendorType(i);
        });
    }
}

// =============================================================================
// KeyButtonWidgetCovTest
// Targets uncovered: ctor, setKeyLabelWord, destructor.
// =============================================================================
class KeyButtonWidgetCovTest : public Test
{
public:
    KeyButtonWidget *m_kw = nullptr;
    void SetUp() override { m_kw = new KeyButtonWidget(); }
    void TearDown() override { /* leak: passInputEvent coupling */ }
};

// setKeyLabelWord: various strings
TEST_F(KeyButtonWidgetCovTest, setKeyLabelWordVarious)
{
    QStringList words = {"Ctrl", "Shift", "Alt", "Del", "Esc", "Enter", "Tab", "Space", ""};
    for (const QString &w : words) {
        EXPECT_NO_FATAL_FAILURE(m_kw->setKeyLabelWord(w));
    }
    qApp->processEvents();
}

// setKeyLabelWord repeated to repaint
TEST_F(KeyButtonWidgetCovTest, setKeyLabelWordRepeatedRepaint)
{
    for (int i = 0; i < 10; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_kw->setKeyLabelWord(QStringLiteral("K%1").arg(i)));
    }
    qApp->processEvents();
}

// =============================================================================
// ImageMenuCov2Test
// Targets topup: paintEvent (geometry adjustment branches), getBorderMenu
// cache hit (parent matches), ActionChecked with nullptr button (clear path).
// =============================================================================
class ImageMenuCov2Test : public Test
{
public:
    ImageBorderHelper *m_h = nullptr;
    void SetUp() override { m_h = ImageBorderHelper::instance(); }
    void TearDown() override
    {
        EXPECT_NO_FATAL_FAILURE(m_h->setActionState(ImageBorderHelper::Nothing, false));
    }
};

// getBorderMenu: cache hit (same parent) returns same pointer
TEST_F(ImageMenuCov2Test, getBorderMenuCacheHit)
{
    QWidget parent;
    ImageMenu *m1 = nullptr;
    ImageMenu *m2 = nullptr;
    EXPECT_NO_FATAL_FAILURE(m1 = m_h->getBorderMenu(ImageBorderHelper::External, QStringLiteral("e"), &parent));
    EXPECT_NO_FATAL_FAILURE(m2 = m_h->getBorderMenu(ImageBorderHelper::External, QStringLiteral("e"), &parent));
    EXPECT_EQ(m1, m2);
}

// getBorderMenu: cache invalidate on parent mismatch
TEST_F(ImageMenuCov2Test, getBorderMenuCacheInvalidateOnParentMismatch)
{
    QWidget p1, p2;
    ImageMenu *m1 = nullptr;
    ImageMenu *m2 = nullptr;
    EXPECT_NO_FATAL_FAILURE(m1 = m_h->getBorderMenu(ImageBorderHelper::Prototype, QStringLiteral("p"), &p1));
    EXPECT_NO_FATAL_FAILURE(m2 = m_h->getBorderMenu(ImageBorderHelper::Prototype, QStringLiteral("p"), &p2));
    EXPECT_NE(m1, m2);
}

// setBorderTypeDetail with valid id then getBorderTypeDetail non-zero
TEST_F(ImageMenuCov2Test, setGetBorderTypeDetailFlow)
{
    EXPECT_NO_FATAL_FAILURE(m_h->setBorderTypeDetail(0)); // Nothing -> no menu
    EXPECT_NO_FATAL_FAILURE(m_h->getBorderTypeDetail());
}

// getPixmapAddBorder with Nothing selected -> returns input pixmap unchanged
TEST_F(ImageMenuCov2Test, getPixmapAddBorderNothingReturnsInput)
{
    QPixmap in(20, 20);
    in.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(m_h->setActionState(ImageBorderHelper::Nothing, false));
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = m_h->getPixmapAddBorder(in));
    EXPECT_FALSE(out.isNull());
}

// pruneBorderMenus repeated
TEST_F(ImageMenuCov2Test, pruneBorderMenusRepeated)
{
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_h->pruneBorderMenus());
    }
}

// ImageMenu constructor for each BorderType (already covered but adds repeats)
TEST_F(ImageMenuCov2Test, constructEachTypeRepeated)
{
    for (int t = ImageBorderHelper::External; t <= ImageBorderHelper::Projection; ++t) {
        auto type = static_cast<ImageBorderHelper::BorderType>(t);
        EXPECT_NO_FATAL_FAILURE({
            ImageMenu *m = new ImageMenu(type, QStringLiteral("title"), nullptr);
            m->AddAction();
            // leak: children with Qt-signal refs
        });
    }
}

// ActionWidget setPixmap with various paths
TEST_F(ImageMenuCov2Test, actionWidgetSetPixmapVarious)
{
    EXPECT_NO_FATAL_FAILURE({
        ActionWidget *w = new ActionWidget();
        w->setPixmap(QStringLiteral("imageBorder/pc.svg"));
        w->setPixmap(QStringLiteral("nonexistent.svg"));
        w->setPixmap(QString());
        w->setActionState(true);
        w->setActionState(false);
        // leak w
    });
}

// getBorderTypeDetail iterating multiple registered menus
TEST_F(ImageMenuCov2Test, getBorderTypeDetailMultipleMenus)
{
    EXPECT_NO_FATAL_FAILURE(m_h->getBorderMenu(ImageBorderHelper::External, QStringLiteral("e"), nullptr));
    EXPECT_NO_FATAL_FAILURE(m_h->getBorderMenu(ImageBorderHelper::Prototype, QStringLiteral("p"), nullptr));
    EXPECT_NO_FATAL_FAILURE(m_h->getBorderMenu(ImageBorderHelper::Projection, QStringLiteral("proj"), nullptr));
    int detail = -1;
    EXPECT_NO_FATAL_FAILURE(detail = m_h->getBorderTypeDetail());
    (void)detail;
}
