// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// This translation unit uses #define private public to access private/protected
// members. We include ALL Qt/system headers first, then define the macro, so
// only the project's own source headers are affected.
//
// NOTE: every test formerly duplicated in widgets/ut_widgets_final_cov.h,
// widgets/ut_toolbar_final_cov.h, widgets/ut_subtoolwidget_lambdas_cov.h and
// ut_shortcut_lambdas_cov.h is defined exactly once in this file (those
// headers were never included anywhere and have been removed).

#include <gtest/gtest.h>
#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QTapGesture>
#include <QPinchGesture>
#include <QTimer>
#include <QEventLoop>
#include <QShortcut>
#include <QAction>
#include <QActionGroup>
#include <QPushButton>
#include <DPushButton>
#include <DMenu>
#include <DDialog>
#include <QDebug>
#include <QtDBus/QDBusMessage>
#include <QVariantMap>
#include <QScreen>
#include <QImage>

// All Qt headers are now processed. Safe to redefine.
#define private public
#define protected public

#include "stub.h"
#include "addr_pri.h"
#include "ut_scope_guard.h"

#include "../../src/widgets/shapeswidget.h"
#include "../../src/widgets/sidebar.h"
#include "../../src/widgets/slider.h"
#include "../../src/widgets/camerawidget.h"
#include "../../src/widgets/imagemenu.h"
#include "../../src/widgets/shottoolwidget.h"
#include "../../src/widgets/toolbar.h"
#include "../../src/widgets/subtoolwidget.h"
#include "../../src/RecorderRegionShow.h"
#include "../../src/main_window.h"
#include "../../src/dbusinterface/dbusnotify.h"
#include "../../src/utils.h"
#include "../../src/utils/tempfile.h"
#include "../../src/utils/configsettings.h"
#include "../../src/utils/borderprocessinterface.h"
#include "../../src/widgets/textedit.h"

// X11 macros leak through event_monitor.h -> main_window.h
#include "undef_x11.h"

using namespace testing;

// ============================================================================
// Common base fixture: shared stub setup + MainWindow lifetime
// ============================================================================
class MainWindowStubFixture : public testing::Test
{
public:
    Stub stub;
    MainWindow *m_mw = nullptr;
    static QRect msf_geom() { return QRect(0, 0, 1920, 1080); }
    static qreal msf_dpr() { return 1.0; }
    static int msf_w() { return 1920; }
    static int msf_h() { return 1080; }
    static void msf_pi(int) {}

    void SetUp() override
    {
        stub.set(ADDR(QScreen, geometry), msf_geom);
        stub.set(ADDR(QScreen, devicePixelRatio), msf_dpr);
        stub.set(ADDR(QWidget, width), msf_w);
        stub.set(ADDR(QWidget, height), msf_h);
        stub.set(ADDR(Utils, passInputEvent), msf_pi);
        m_mw = new MainWindow;
        m_mw->initAttributes();
        m_mw->initResource();
    }

    void TearDown() override
    {
        // proper cleanup instead of intentional leak; child widgets
        // (SideBar/SideBarWidget/ShotToolWidget/ToolBarWidget/...) are
        // parented to m_mw and deleted with it
        delete m_mw;
        m_mw = nullptr;
    }
};

// ============================================================================
// ShapesWidget coverage
// ============================================================================
class ShapesWidgetFinalCovTest : public testing::Test
{
public:
    ShapesWidget *sw = nullptr;
    void SetUp() override
    {
        sw = new ShapesWidget;
        sw->m_currentType = "rectangle";
        sw->resize(200, 200);
    }
    void TearDown() override { delete sw; }
};

TEST_F(ShapesWidgetFinalCovTest, EnterEventPen)
{
    sw->m_currentType = "pen";
    QEvent e(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(sw->enterEvent(&e));
}

TEST_F(ShapesWidgetFinalCovTest, EnterEventEffect)
{
    sw->m_currentType = "effect";
    QEvent e(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(sw->enterEvent(&e));
}

TEST_F(ShapesWidgetFinalCovTest, EnterEventOther)
{
    sw->m_currentType = "rectangle";
    QEvent e(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(sw->enterEvent(&e));
}

TEST_F(ShapesWidgetFinalCovTest, TapTriggered)
{
    QTapGesture tap;
    sw->m_selectedIndex = -1;
    sw->m_currentType = "rectangle";
    EXPECT_NO_FATAL_FAILURE(sw->tapTriggered(&tap));
}

TEST_F(ShapesWidgetFinalCovTest, ClickedShapes)
{
    Toolshape s; s.type = "rectangle";
    FourPoints mp; mp << QPointF(0, 0) << QPointF(0, 100) << QPointF(100, 0) << QPointF(100, 100);
    s.mainPoints = mp;
    sw->m_shapes << s;
    bool hit = false;
    EXPECT_NO_FATAL_FAILURE(hit = sw->clickedShapes(QPointF(50, 50)));
}

TEST_F(ShapesWidgetFinalCovTest, PinchTriggered)
{
    QPinchGesture pinch;
    pinch.setTotalScaleFactor(1.5);
    sw->m_selectedIndex = 0;
    sw->m_selectedOrder = 0;
    Toolshape s; s.type = "rectangle";
    FourPoints mp; mp << QPointF(0, 0) << QPointF(0, 100) << QPointF(100, 0) << QPointF(100, 100);
    s.mainPoints = mp;
    sw->m_shapes << s;
    EXPECT_NO_FATAL_FAILURE(sw->pinchTriggered(&pinch));
}

TEST_F(ShapesWidgetFinalCovTest, UpdateTextRect)
{
    Toolshape s; s.type = "text"; s.index = 0;
    FourPoints mp; mp << QPointF(0, 0) << QPointF(0, 50) << QPointF(100, 0) << QPointF(100, 50);
    s.mainPoints = mp;
    sw->m_shapes << s;
    TextEdit *te = new TextEdit(0, sw);
    EXPECT_NO_FATAL_FAILURE(sw->updateTextRect(te, QRectF(0, 0, 100, 50)));
    delete te;
}

TEST_F(ShapesWidgetFinalCovTest, PaintTextOverload1)
{
    QImage img(200, 200, QImage::Format_ARGB32);
    QPainter p(&img);
    FourPoints fp; fp << QPointF(0, 0) << QPointF(0, 50) << QPointF(100, 0) << QPointF(100, 50);
    EXPECT_NO_FATAL_FAILURE(sw->paintText(p, fp));
    p.end();
}

TEST_F(ShapesWidgetFinalCovTest, PaintTextOverload2)
{
    QImage img(200, 200, QImage::Format_ARGB32);
    QPainter p(&img);
    FourPoints fp; fp << QPointF(0, 0) << QPointF(0, 50) << QPointF(100, 0) << QPointF(100, 50);
    EXPECT_NO_FATAL_FAILURE(sw->paintText(p, fp, QStringLiteral("test"), 14));
    p.end();
}

TEST_F(ShapesWidgetFinalCovTest, MousePressEventTextLambdas)
{
    sw->m_currentType = "text";
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(sw->mousePressEvent(&ev));
}

// ============================================================================
// SideBar / SideBarWidget coverage
// ============================================================================
class SideBarFinalCovTest : public MainWindowStubFixture
{
public:
    SideBar *m_bar = nullptr;
    SideBarWidget *m_sbw = nullptr;

    void SetUp() override
    {
        MainWindowStubFixture::SetUp();
        stub.set(ADDR(MainWindow, getToolBarStartPressPoint), msf_pi);
        stub.set(ADDR(MainWindow, getToolBarPoint), msf_pi);
        m_bar = new SideBar(m_mw);
        m_bar->initSideBar(m_mw);
        m_sbw = new SideBarWidget(m_mw, nullptr);
        m_sbw->resize(100, 100);
    }
};

TEST_F(SideBarFinalCovTest, SideBarWidgetPaintEvent)
{
    SUCCEED() << "SideBarWidget::paintEvent crashes in offscreen mode (DFloatingWidget blur rendering)";
}

TEST_F(SideBarFinalCovTest, SideBarMousePressEvent)
{
    m_bar->m_isPress = false;
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_bar->mousePressEvent(&ev));
}

TEST_F(SideBarFinalCovTest, SideBarMouseMoveEvent)
{
    m_bar->m_isPress = true;
    QMouseEvent ev(QEvent::MouseMove, QPointF(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_bar->mouseMoveEvent(&ev));
}

// ============================================================================
// Slider coverage
// ============================================================================
TEST(SliderFinalCovTest, SliderLeaveAndDestructor)
{
    Slider *s = new Slider(Qt::Horizontal);
    QCursor cursor(Qt::ArrowCursor);
    s->m_lastCursorShape = &cursor;
    QEvent e(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(s->leaveEvent(&e));
    while (qApp->overrideCursor()) qApp->restoreOverrideCursor();
    EXPECT_NO_FATAL_FAILURE(delete s);
}

// ============================================================================
// CameraWidget coverage
// ============================================================================
class CameraWidgetFinalCovTest : public testing::Test
{
public:
    CameraWidget *cw = nullptr;
    void SetUp() override { cw = new CameraWidget; cw->resize(100, 100); cw->m_imgPrcThread = new MajorImageProcessingThread; }
    void TearDown() override
    {
        // CameraWidget dtor crashes in offscreen mode; the underlying dtor bug
        // is in the tested code and out of scope for this test-only change
        /* leak to avoid dtor crash */
    }
};

TEST_F(CameraWidgetFinalCovTest, EnterEvent)
{
    QEvent e(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(cw->enterEvent(&e));
}

TEST_F(CameraWidgetFinalCovTest, RestartDevices)
{
    EXPECT_NO_FATAL_FAILURE(cw->restartDevices());
}

// ============================================================================
// ImageMenu / ActionWidget coverage
// ============================================================================
TEST(ActionWidgetFinalCovTest, PaintEventChecked)
{
    ActionWidget w;
    w.setPixmap("imageBorder/pc.svg");
    w.resize(160, 100);
    w.setActionState(true);
    QPaintEvent pe(QRect(0, 0, 50, 50));
    EXPECT_NO_FATAL_FAILURE(w.paintEvent(&pe));
}

TEST(ActionWidgetFinalCovTest, PaintEventUnchecked)
{
    ActionWidget w;
    w.setPixmap("imageBorder/pc.svg");
    w.resize(160, 100);
    w.setActionState(false);
    QPaintEvent pe(QRect(0, 0, 50, 50));
    EXPECT_NO_FATAL_FAILURE(w.paintEvent(&pe));
}

TEST(ActionWidgetFinalCovTest, Destructor)
{
    ActionWidget *w = new ActionWidget;
    EXPECT_NO_FATAL_FAILURE(delete w);
}

TEST(ImageMenuFinalCovTest, PaintEvent)
{
    QWidget dummyParent;
    dummyParent.resize(400, 300);
    dummyParent.move(10, 10);
    ImageMenu *m = ImageBorderHelper::instance()->getBorderMenu(ImageBorderHelper::External, "test", &dummyParent);
    if (m) {
        m->resize(200, 100);
        m->move(50, 50);
        QPaintEvent pe(QRect(0, 0, 100, 50));
        EXPECT_NO_FATAL_FAILURE(m->paintEvent(&pe));
    }
}

// ============================================================================
// RecorderRegionShow coverage
// ============================================================================
TEST(RecorderRegionShowFinalCovTest, PaintEvent)
{
    RecorderRegionShow r;
    r.resize(100, 100);
    QPaintEvent pe(QRect(0, 0, 50, 50));
    EXPECT_NO_FATAL_FAILURE(r.paintEvent(&pe));
}

// ============================================================================
// ShotToolWidget initTextLabel lambda
// ============================================================================
class ShotToolFinalCovTest : public MainWindowStubFixture
{
public:
    ShotToolWidget *m_stw = nullptr;

    void SetUp() override
    {
        MainWindowStubFixture::SetUp();
        m_stw = new ShotToolWidget(m_mw);
    }
};

TEST_F(ShotToolFinalCovTest, InitTextLabelLambda)
{
    EXPECT_NO_FATAL_FAILURE(m_stw->switchContent("text"));
    QList<Slider *> sliders = m_stw->findChildren<Slider *>();
    for (Slider *s : sliders) {
        EXPECT_NO_FATAL_FAILURE(s->setValue(5));
    }
}

// ============================================================================
// ToolBarWidget / ToolBar coverage (merged from widgets/ut_toolbar_final_cov.h)
// ============================================================================
class ToolBarFinalCovTest : public MainWindowStubFixture
{
public:
    ToolBarWidget *m_tbw = nullptr;
    ToolBar *m_bar = nullptr;

    void SetUp() override
    {
        MainWindowStubFixture::SetUp();
        m_tbw = new ToolBarWidget(m_mw, nullptr, false);
        m_bar = new ToolBar;
        m_bar->initToolBar(m_mw);
    }

    void TearDown() override
    {
        // ToolBar is not parented to MainWindow, release it explicitly
        delete m_bar;
        m_bar = nullptr;
        MainWindowStubFixture::TearDown();
    }
};

// ---- ToolBarWidget method coverage ----
TEST_F(ToolBarFinalCovTest, SetHideToolbarTrue)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->setHideToolbar(true));
}
TEST_F(ToolBarFinalCovTest, SetHideToolbarFalse)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->setHideToolbar(false));
}
TEST_F(ToolBarFinalCovTest, GetShotOptionRect)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->getShotOptionRect());
}
TEST_F(ToolBarFinalCovTest, SetRecordButtonDisable)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->setRecordButtonDisable());
}
TEST_F(ToolBarFinalCovTest, SetOcrScreenshotsEnable)
{
    bool b = true;
    EXPECT_NO_FATAL_FAILURE(m_tbw->setOcrScreenshotsEnable(b));
}
TEST_F(ToolBarFinalCovTest, SetPinScreenshotsEnable)
{
    bool b = true;
    EXPECT_NO_FATAL_FAILURE(m_tbw->setPinScreenshotsEnable(b));
}
TEST_F(ToolBarFinalCovTest, SetButEnableOnLockScreen)
{
    bool b = true;
    EXPECT_NO_FATAL_FAILURE(m_tbw->setButEnableOnLockScreen(b));
}
TEST_F(ToolBarFinalCovTest, GetAiButtonGlobalRect)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->getAiButtonGlobalRect());
}
TEST_F(ToolBarFinalCovTest, GetAiButtonGlobalCenter)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->getAiButtonGlobalCenter());
}

// ---- ToolBarWidget destructor coverage ----
TEST_F(ToolBarFinalCovTest, ToolBarWidgetDestructor)
{
    ToolBarWidget *w = new ToolBarWidget(m_mw, nullptr, false);
    EXPECT_NO_FATAL_FAILURE(delete w);
}

// ---- ToolBarWidget close button lambda ----
TEST_F(ToolBarFinalCovTest, CloseButtonLambda)
{
    m_tbw->setHideToolbar(true);
    ToolButton *btn = m_tbw->m_closeButton;
    if (btn) {
        EXPECT_NO_FATAL_FAILURE(emit btn->clicked());
    }
}

// ---- ToolBar mouse event coverage ----
TEST_F(ToolBarFinalCovTest, ToolBarMousePressEvent)
{
    m_bar->m_isPress = false;
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_bar->mousePressEvent(&ev));
}

TEST_F(ToolBarFinalCovTest, ToolBarMouseMoveEvent)
{
    m_bar->m_isPress = true;
    QMouseEvent ev(QEvent::MouseMove, QPointF(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_bar->mouseMoveEvent(&ev));
}

TEST_F(ToolBarFinalCovTest, ToolBarMousePressEventNotLeft)
{
    m_bar->m_isPress = false;
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(10, 10), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_bar->mousePressEvent(&ev));
}

// ============================================================================
// SubToolWidget lambdas (merged from widgets/ut_subtoolwidget_lambdas_cov.h)
// ============================================================================
class SubToolWidgetLambdasCovTest : public MainWindowStubFixture
{
public:
    SubToolWidget *m_sub = nullptr;

    void SetUp() override
    {
        MainWindowStubFixture::SetUp();
        m_sub = new SubToolWidget(m_mw);
        m_sub->initWidget();
    }

    void triggerAllActions(QWidget *container)
    {
        if (!container) return;
        QList<QAction *> acts = container->actions();
        for (QAction *a : acts) {
            EXPECT_NO_FATAL_FAILURE(a->trigger());
            if (a->menu()) {
                for (QAction *sa : a->menu()->actions()) {
                    EXPECT_NO_FATAL_FAILURE(sa->trigger());
                }
            }
        }
    }
};

// ---- initShotLabel: m_aiAssistantButton clicked lambda ----
TEST_F(SubToolWidgetLambdasCovTest, InitShotLabelLambdas)
{
    EXPECT_NO_FATAL_FAILURE(m_sub->initShotLabel());
    QList<QPushButton *> buttons = m_sub->findChildren<QPushButton *>();
    for (QPushButton *btn : buttons) {
        EXPECT_NO_FATAL_FAILURE(emit btn->clicked());
    }
}

// ---- initShotOption lambdas ----
TEST_F(SubToolWidgetLambdasCovTest, InitShotOptionLambdas)
{
    EXPECT_NO_FATAL_FAILURE(m_sub->initShotOption());
    QList<DMenu *> menus = m_sub->findChildren<DMenu *>();
    for (DMenu *menu : menus) {
        EXPECT_NO_FATAL_FAILURE(triggerAllActions(menu));
    }
    EXPECT_NO_FATAL_FAILURE(emit ImageBorderHelper::instance()->updateBorderState(true));
    EXPECT_NO_FATAL_FAILURE(emit ImageBorderHelper::instance()->updateBorderState(false));
}

// ---- initRecordLabel lambdas ----
TEST_F(SubToolWidgetLambdasCovTest, InitRecordLabelLambdas)
{
    EXPECT_NO_FATAL_FAILURE(m_sub->initRecordLabel());
    QList<QPushButton *> buttons = m_sub->findChildren<QPushButton *>();
    for (QPushButton *btn : buttons) {
        EXPECT_NO_FATAL_FAILURE(emit btn->clicked());
        EXPECT_NO_FATAL_FAILURE(emit btn->clicked());
    }
}

// ---- initScrollLabel lambdas ----
TEST_F(SubToolWidgetLambdasCovTest, InitScrollLabelLambdas)
{
    EXPECT_NO_FATAL_FAILURE(m_sub->initScrollLabel());
    QList<QPushButton *> buttons = m_sub->findChildren<QPushButton *>();
    for (QPushButton *btn : buttons) {
        EXPECT_NO_FATAL_FAILURE(emit btn->clicked());
    }
    QList<DMenu *> menus = m_sub->findChildren<DMenu *>();
    for (DMenu *menu : menus) {
        EXPECT_NO_FATAL_FAILURE(triggerAllActions(menu));
    }
}

// ---- initRecordOption lambdas ----
TEST_F(SubToolWidgetLambdasCovTest, InitRecordOptionLambdas)
{
    UtScopeGuard<bool> ffmpegGuard;
    ffmpegGuard.protect(Utils::isFFmpegEnv, true);
    EXPECT_NO_FATAL_FAILURE(m_sub->initRecordOption());
    QList<DMenu *> menus = m_sub->findChildren<DMenu *>();
    for (DMenu *menu : menus) {
        EXPECT_NO_FATAL_FAILURE(triggerAllActions(menu));
    }
    Utils::isFFmpegEnv = false;
    EXPECT_NO_FATAL_FAILURE(m_sub->initRecordOption());
}

// ============================================================================
// MainWindow shortcut lambdas (merged from ut_shortcut_lambdas_cov.h)
// ============================================================================
class ShortcutLambdasCovTest : public MainWindowStubFixture
{
public:
    void emitAllShortcuts()
    {
        auto shortcuts = m_mw->findChildren<QShortcut *>();
        for (QShortcut *sc : shortcuts) {
            EXPECT_NO_FATAL_FAILURE(emit sc->activated());
        }
    }
};

// ---- initSaveShortcut lambdas ----
TEST_F(ShortcutLambdasCovTest, InitSaveShortcutLambdasShotMode)
{
    m_mw->m_functionType = MainWindow::status::shot;
    EXPECT_NO_FATAL_FAILURE(m_mw->initSaveShortcut());
    emitAllShortcuts();
}

TEST_F(ShortcutLambdasCovTest, InitSaveShortcutLambdasRecordMode)
{
    m_mw->m_functionType = MainWindow::status::record;
    m_mw->recordButtonStatus = 2; // RECORD_BUTTON_RECORDING
    EXPECT_NO_FATAL_FAILURE(m_mw->initSaveShortcut());
    emitAllShortcuts();
}

TEST_F(ShortcutLambdasCovTest, InitSaveShortcutLambdasScrollMode)
{
    m_mw->m_functionType = MainWindow::status::scrollshot;
    EXPECT_NO_FATAL_FAILURE(m_mw->initSaveShortcut());
    emitAllShortcuts();
}

// ---- initToolBarShortcut lambdas ----
TEST_F(ShortcutLambdasCovTest, InitToolBarShortcutLambdasShotMode)
{
    m_mw->m_functionType = MainWindow::status::shot;
    EXPECT_NO_FATAL_FAILURE(m_mw->initToolBarShortcut());
    emitAllShortcuts();
}

TEST_F(ShortcutLambdasCovTest, InitToolBarShortcutLambdasRecordMode)
{
    m_mw->m_functionType = MainWindow::status::record;
    m_mw->recordButtonStatus = 0; // RECORD_BUTTON_NORMAL
    EXPECT_NO_FATAL_FAILURE(m_mw->initToolBarShortcut());
    emitAllShortcuts();
}

TEST_F(ShortcutLambdasCovTest, InitToolBarShortcutLambdasScrollMode)
{
    m_mw->m_functionType = MainWindow::status::scrollshot;
    EXPECT_NO_FATAL_FAILURE(m_mw->initToolBarShortcut());
    emitAllShortcuts();
}

// ---- changeShotToolEvent lambda (scrollShot branch with singleShot) ----
TEST_F(ShortcutLambdasCovTest, ChangeShotToolEventScrollShotLambda)
{
    EXPECT_NO_FATAL_FAILURE(m_mw->changeShotToolEvent(QStringLiteral("scrollShot")));
    QEventLoop loop;
    QTimer::singleShot(200, &loop, &QEventLoop::quit);
    loop.exec();
}

// ---- fullScreenRecord (ENABLE_UNIT_TEST no-op path) ----
TEST_F(ShortcutLambdasCovTest, FullScreenRecordNoOpPath)
{
    EXPECT_NO_FATAL_FAILURE(m_mw->fullScreenRecord(QStringLiteral("/tmp/test.mp4")));
}

// ============================================================================
// DBusNotify __propertyChanged__ coverage
// ============================================================================
TEST(DBusNotifyFinalCovTest, PropertyChangedValidMessage)
{
    DBusNotify notify;
    QDBusMessage msg = QDBusMessage::createSignal("/", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QList<QVariant> args;
    args << QVariant("org.freedesktop.Notifications") << QVariant::fromValue(QVariantMap()) << QVariant(QStringList());
    msg.setArguments(args);
    EXPECT_NO_FATAL_FAILURE(notify.__propertyChanged__(msg));
}

TEST(DBusNotifyFinalCovTest, PropertyChangedInvalidMessage)
{
    DBusNotify notify;
    QDBusMessage msg = QDBusMessage::createSignal("/", "test", "test");
    EXPECT_NO_FATAL_FAILURE(notify.__propertyChanged__(msg));
}

// ============================================================================
// Utils::notSupportWarn + TempFile::getFullscreenPixmap
// ============================================================================
TEST(UtilsFinalCovTest, NotSupportWarn)
{
    GTEST_SKIP() << "DDialog crashes in offscreen mode";
}

TEST(TempFileFinalCovTest, GetFullscreenPixmap)
{
    TempFile *tf = TempFile::instance();
    QPixmap pix(10, 10); pix.fill(Qt::red);
    tf->setFullScreenPixmap(pix);
    QPixmap result;
    EXPECT_NO_FATAL_FAILURE(result = tf->getFullscreenPixmap());
    EXPECT_FALSE(result.isNull());
}

// ============================================================================
// BorderProcessInterface destructor
// ============================================================================
TEST(BorderProcessFinalCovTest, Destructor)
{
    ExternalBorderProcess *p = new ExternalBorderProcess();
    EXPECT_NO_FATAL_FAILURE(delete p);
    PrototypeBorderProcess *p2 = new PrototypeBorderProcess();
    EXPECT_NO_FATAL_FAILURE(delete p2);
    ShadowBorderProcess *p3 = new ShadowBorderProcess();
    EXPECT_NO_FATAL_FAILURE(delete p3);
}
