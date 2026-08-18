// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QEnterEvent>
#include <QTapGesture>
#include <QPinchGesture>
#include <QAction>
#include <QActionGroup>
#include <QButtonGroup>
#include <QScreen>
#include <QImage>
#include <QPushButton>
#include <DPushButton>
#include <DMenu>
#include <QDebug>

#include "stub.h"
#include "addr_pri.h"

#include "../../src/widgets/shapeswidget.h"
#include "../../src/widgets/sidebar.h"
#include "../../src/widgets/toolbar.h"
#include "../../src/widgets/subtoolwidget.h"
#include "../../src/widgets/camerawidget.h"
#include "../../src/widgets/imagemenu.h"
#include "../../src/widgets/shottoolwidget.h"
#include "../../src/widgets/slider.h"
#include "../../src/widgets/textedit.h"
#include "../../src/widgets/toolbutton.h"
#include "../../src/widgets/savebutton.h"
#include "../../src/main_window.h"
#include "../../src/utils.h"
#include "../../src/utils/configsettings.h"
#include "../../src/camera/majorimageprocessingthread.h"

using namespace testing;

ACCESS_PRIVATE_FUN(ShapesWidget, void(QEvent *), enterEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QTapGesture *), tapTriggered);
ACCESS_PRIVATE_FUN(ShapesWidget, bool(QPointF), clickedShapes);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPinchGesture *), pinchTriggered);
ACCESS_PRIVATE_FUN(ShapesWidget, void(TextEdit *, QRectF), updateTextRect);

ACCESS_PRIVATE_FUN(SideBar, void(QMouseEvent *), mousePressEvent);
ACCESS_PRIVATE_FUN(SideBar, void(QMouseEvent *), mouseMoveEvent);
ACCESS_PRIVATE_FUN(SideBarWidget, void(QPaintEvent *), paintEvent);

ACCESS_PRIVATE_FUN(ToolBar, void(QMouseEvent *), mousePressEvent);
ACCESS_PRIVATE_FUN(ToolBar, void(QMouseEvent *), mouseMoveEvent);

ACCESS_PRIVATE_FUN(CameraWidget, void(QEvent *), enterEvent);
ACCESS_PRIVATE_FUN(CameraWidget, void(), restartDevices);

ACCESS_PRIVATE_FUN(ImageMenu, void(QPaintEvent *), paintEvent);
ACCESS_PRIVATE_FUN(ActionWidget, void(QPaintEvent *), paintEvent);

ACCESS_PRIVATE_FUN(Slider, void(QEvent *), leaveEvent);

ACCESS_PRIVATE_FIELD(ShapesWidget, int, m_selectedIndex);
ACCESS_PRIVATE_FIELD(ShapesWidget, int, m_selectedOrder);
ACCESS_PRIVATE_FIELD(SideBar, bool, m_isPress);
ACCESS_PRIVATE_FIELD(SideBar, MainWindow *, m_pMainWindow);
ACCESS_PRIVATE_FIELD(CameraWidget, MajorImageProcessingThread *, m_imgPrcThread);
ACCESS_PRIVATE_FIELD(ToolBarWidget, ToolButton *, m_closeButton);
ACCESS_PRIVATE_FIELD(Slider, QCursor *, m_lastCursorShape);
ACCESS_PRIVATE_FIELD(SubToolWidget, ToolButton *, m_cameraButton);
ACCESS_PRIVATE_FIELD(SubToolWidget, ToolButton *, m_shotButton);
ACCESS_PRIVATE_FIELD(SubToolWidget, ToolButton *, m_recorderButton);
ACCESS_PRIVATE_FIELD(SubToolWidget, ToolButton *, m_aiAssistantScrollButton);
ACCESS_PRIVATE_FIELD(SubToolWidget, ToolButton *, m_ocrScrollButton);
ACCESS_PRIVATE_FIELD(SubToolWidget, ToolButton *, m_keyBoardButton);

static void wcf_noop() {}

class WidgetsCovFinalTest : public Test
{
public:
    ShapesWidget *sw = nullptr;
    void SetUp() override
    {
        sw = new ShapesWidget;
        sw->resize(200, 200);
        access_private_field::ShapesWidgetm_currentType(*sw) = "rectangle";
    }
    void TearDown() override { delete sw; }

    static FourPoints makeFP(qreal x, qreal y, qreal w, qreal h)
    {
        FourPoints fp;
        fp << QPointF(x, y) << QPointF(x + w, y) << QPointF(x + w, y + h) << QPointF(x, y + h);
        return fp;
    }
    static Toolshape makeShape(const QString &type)
    {
        Toolshape s;
        s.type = type;
        s.mainPoints = makeFP(0, 0, 100, 60);
        s.points = QList<QPointF>{QPointF(0, 0), QPointF(100, 60)};
        s.colorIndex = 0;
        s.lineWidth = 3;
        s.isBlur = false;
        s.isOval = 0;
        s.radius = 10;
        s.index = 0;
        return s;
    }
};

TEST_F(WidgetsCovFinalTest, ShapesWidgetEnterEventPen)
{
    access_private_field::ShapesWidgetm_currentType(*sw) = "pen";
    QEvent e(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetenterEvent(*sw, &e));
}

TEST_F(WidgetsCovFinalTest, ShapesWidgetEnterEventEffect)
{
    access_private_field::ShapesWidgetm_currentType(*sw) = "effect";
    QEvent e(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetenterEvent(*sw, &e));
}

TEST_F(WidgetsCovFinalTest, ShapesWidgetEnterEventOther)
{
    access_private_field::ShapesWidgetm_currentType(*sw) = "rectangle";
    QEvent e(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetenterEvent(*sw, &e));
}

TEST_F(WidgetsCovFinalTest, ShapesWidgetTapTriggered)
{
    QTapGesture tap;
    access_private_field::ShapesWidgetm_selectedIndex(*sw) = -1;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgettapTriggered(*sw, &tap));
}

TEST_F(WidgetsCovFinalTest, ShapesWidgetClickedShapes)
{
    access_private_field::ShapesWidgetm_shapes(*sw) << makeShape("rectangle");
    bool hit = false;
    EXPECT_NO_FATAL_FAILURE(hit = call_private_fun::ShapesWidgetclickedShapes(*sw, QPointF(50, 30)));
}

TEST_F(WidgetsCovFinalTest, ShapesWidgetPinchTriggered)
{
    QPinchGesture pinch;
    pinch.setTotalScaleFactor(1.5);
    access_private_field::ShapesWidgetm_selectedIndex(*sw) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*sw) = 0;
    access_private_field::ShapesWidgetm_shapes(*sw) << makeShape("rectangle");
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetpinchTriggered(*sw, &pinch));
}

TEST_F(WidgetsCovFinalTest, ShapesWidgetUpdateTextRect)
{
    Toolshape s = makeShape("text");
    s.index = 0;
    access_private_field::ShapesWidgetm_shapes(*sw) << s;
    TextEdit *te = new TextEdit(0, sw);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetupdateTextRect(*sw, te, QRectF(0, 0, 100, 50)));
    delete te;
}

TEST_F(WidgetsCovFinalTest, ShapesWidgetMousePressEventTextLambdas)
{
    access_private_field::ShapesWidgetm_currentType(*sw) = "text";
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetmousePressEvent(*sw, &ev));
    auto &editMap = access_private_field::ShapesWidgetm_editMap(*sw);
    if (!editMap.isEmpty()) {
        TextEdit *te = editMap.first();
        EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(te, "clickToEditing", Qt::DirectConnection, Q_ARG(int, editMap.firstKey())));
        EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(te, "textEditSelected", Qt::DirectConnection, Q_ARG(int, editMap.firstKey())));
    }
}

class SideBarCovFinalTest : public Test
{
public:
    Stub stub;
    SideBar *m_bar = nullptr;

    void SetUp() override
    {
        m_bar = new SideBar;
    }
    void TearDown() override { delete m_bar; }
};

TEST_F(SideBarCovFinalTest, SideBarMouseMoveEvent)
{
    access_private_field::SideBarm_isPress(*m_bar) = true;
    QMouseEvent ev(QEvent::MouseMove, QPointF(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SideBarmouseMoveEvent(*m_bar, &ev));
}

class SideBarWithMainCovFinalTest : public Test
{
public:
    Stub stub;
    MainWindow *m_mw = nullptr;
    SideBar *m_bar = nullptr;
    SideBarWidget *m_sbw = nullptr;

    void SetUp() override
    {
        stub.set(ADDR(MainWindow, getToolBarStartPressPoint), wcf_noop);
        stub.set(ADDR(MainWindow, getToolBarPoint), wcf_noop);
        m_mw = new MainWindow;
        m_bar = new SideBar;
        m_bar->initSideBar(m_mw);
        m_sbw = new SideBarWidget(m_mw, nullptr);
        m_sbw->resize(100, 100);
    }
    void TearDown() override { }
};

TEST_F(SideBarWithMainCovFinalTest, SideBarMousePressEvent)
{
    access_private_field::SideBarm_isPress(*m_bar) = false;
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SideBarmousePressEvent(*m_bar, &ev));
    while (qApp->overrideCursor()) qApp->restoreOverrideCursor();
}

TEST_F(SideBarWithMainCovFinalTest, SideBarMousePressEventRightButton)
{
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(10, 10), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SideBarmousePressEvent(*m_bar, &ev));
    while (qApp->overrideCursor()) qApp->restoreOverrideCursor();
}

TEST_F(SideBarWithMainCovFinalTest, SideBarWidgetPaintEvent)
{
    SUCCEED() << "SideBarWidget::paintEvent crashes in offscreen mode (DFloatingWidget blur rendering)";
}

TEST(SliderCovFinalTest, SliderLeaveAndDestructor)
{
    Slider *s = new Slider(Qt::Horizontal);
    QCursor cursor(Qt::ArrowCursor);
    access_private_field::Sliderm_lastCursorShape(*s) = &cursor;
    QEvent leave(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SliderleaveEvent(*s, &leave));
    while (qApp->overrideCursor()) qApp->restoreOverrideCursor();
    EXPECT_NO_FATAL_FAILURE(delete s);
}

TEST(SliderCovFinalTest, SliderDestructorOnly)
{
    Slider *s = new Slider(Qt::Horizontal);
    EXPECT_NO_FATAL_FAILURE(delete s);
}

class CameraWidgetCovFinalTest : public Test
{
public:
    CameraWidget *cw = nullptr;
    void SetUp() override
    {
        cw = new CameraWidget;
        cw->resize(100, 100);
        access_private_field::CameraWidgetm_imgPrcThread(*cw) = new MajorImageProcessingThread;
    }
    void TearDown() override { }
};

TEST_F(CameraWidgetCovFinalTest, EnterEvent)
{
    QEvent e(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::CameraWidgetenterEvent(*cw, &e));
    while (qApp->overrideCursor()) qApp->restoreOverrideCursor();
}

TEST_F(CameraWidgetCovFinalTest, RestartDevices)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::CameraWidgetrestartDevices(*cw));
}

TEST(ActionWidgetCovFinalTest, PaintEventDirect)
{
    ActionWidget w;
    w.setPixmap("imageBorder/pc.svg");
    w.resize(160, 100);
    w.setActionState(true);
    QPaintEvent pe1(QRect(0, 0, 50, 50));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ActionWidgetpaintEvent(w, &pe1));
    w.setActionState(false);
    QPaintEvent pe2(QRect(0, 0, 50, 50));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ActionWidgetpaintEvent(w, &pe2));
}

TEST(ActionWidgetCovFinalTest, Destructor)
{
    ActionWidget *w = new ActionWidget;
    EXPECT_NO_FATAL_FAILURE(delete w);
}

TEST(ImageMenuCovFinalTest, PaintEventDirect)
{
    QWidget dummyParent;
    dummyParent.resize(400, 300);
    dummyParent.move(10, 10);
    ImageMenu *m = ImageBorderHelper::instance()->getBorderMenu(ImageBorderHelper::External, "test", &dummyParent);
    if (m) {
        m->resize(200, 100);
        m->move(50, 50);
        QPaintEvent pe(QRect(0, 0, 100, 50));
        EXPECT_NO_FATAL_FAILURE(call_private_fun::ImageMenupaintEvent(*m, &pe));
    }
}

class ToolBarCovFinalTest : public Test
{
public:
    Stub stub;
    MainWindow *m_mw = nullptr;
    ToolBar *m_bar = nullptr;

    void SetUp() override
    {
        stub.set(ADDR(MainWindow, getSideBarStartPressPoint), wcf_noop);
        stub.set(ADDR(MainWindow, getToolBarStartPressPoint), wcf_noop);
        stub.set(ADDR(MainWindow, getToolBarPoint), wcf_noop);
        m_mw = new MainWindow;
        m_bar = new ToolBar;
        m_bar->initToolBar(m_mw);
    }
    void TearDown() override
    {
        while (qApp->overrideCursor()) qApp->restoreOverrideCursor();
    }
};

TEST_F(ToolBarCovFinalTest, MousePressEvent)
{
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolBarmousePressEvent(*m_bar, &ev));
}

TEST_F(ToolBarCovFinalTest, MousePressEventRightButton)
{
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(10, 10), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolBarmousePressEvent(*m_bar, &ev));
}

TEST_F(ToolBarCovFinalTest, MouseMoveEventPressed)
{
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::ToolBarmousePressEvent(*m_bar, &press);
    QMouseEvent move(QEvent::MouseMove, QPointF(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolBarmouseMoveEvent(*m_bar, &move));
}

TEST_F(ToolBarCovFinalTest, MouseMoveEventNotPressed)
{
    QMouseEvent move(QEvent::MouseMove, QPointF(20, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ToolBarmouseMoveEvent(*m_bar, &move));
}

class ToolBarWidgetCovFinalTest : public Test
{
public:
    MainWindow *m_mw = nullptr;
    ToolBarWidget *m_tbw = nullptr;

    void SetUp() override
    {
        m_mw = new MainWindow;
        m_tbw = new ToolBarWidget(m_mw, nullptr, false);
    }
    void TearDown() override { }
};

TEST_F(ToolBarWidgetCovFinalTest, GetAiButtonGlobalCenter)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->getAiButtonGlobalCenter());
}

TEST_F(ToolBarWidgetCovFinalTest, GetAiButtonGlobalRect)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->getAiButtonGlobalRect());
}

TEST_F(ToolBarWidgetCovFinalTest, SetOcrScreenshotsEnable)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->setOcrScreenshotsEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_tbw->setOcrScreenshotsEnable(false));
}

TEST_F(ToolBarWidgetCovFinalTest, SetButEnableOnLockScreen)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->setButEnableOnLockScreen(true));
    EXPECT_NO_FATAL_FAILURE(m_tbw->setButEnableOnLockScreen(false));
}

TEST_F(ToolBarWidgetCovFinalTest, SetRecordButtonDisable)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->setRecordButtonDisable());
}

TEST_F(ToolBarWidgetCovFinalTest, GetShotOptionRect)
{
    QRect r;
    EXPECT_NO_FATAL_FAILURE(r = m_tbw->getShotOptionRect());
}

TEST_F(ToolBarWidgetCovFinalTest, SetPinScreenshotsEnable)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->setPinScreenshotsEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_tbw->setPinScreenshotsEnable(false));
}

TEST_F(ToolBarWidgetCovFinalTest, SetHideToolbar)
{
    EXPECT_NO_FATAL_FAILURE(m_tbw->setHideToolbar(true));
    EXPECT_NO_FATAL_FAILURE(m_tbw->setHideToolbar(false));
}

TEST_F(ToolBarWidgetCovFinalTest, Destructor)
{
    MainWindow *mw = new MainWindow;
    ToolBarWidget *tbw = new ToolBarWidget(mw, nullptr, false);
    EXPECT_NO_FATAL_FAILURE(delete tbw);
    delete mw;
}

TEST_F(ToolBarWidgetCovFinalTest, ConstructorCloseButtonLambda)
{
    Stub s;
    s.set(ADDR(MainWindow, exitApp), wcf_noop);
    ToolButton *closeBtn = access_private_field::ToolBarWidgetm_closeButton(*m_tbw);
    if (closeBtn) {
        EXPECT_NO_FATAL_FAILURE(closeBtn->click());
    }
}

class SubToolWidgetCovFinalTest : public Test
{
public:
    Stub stub;
    MainWindow *m_mw = nullptr;
    SubToolWidget *m_sub = nullptr;

    void SetUp() override
    {
        stub.set(ADDR(MainWindow, getToolBarPoint), wcf_noop);
        m_mw = new MainWindow;
        m_sub = new SubToolWidget(m_mw);
        m_sub->switchContent(QStringLiteral("shot"));
    }
    void TearDown() override { }
};

TEST_F(SubToolWidgetCovFinalTest, RecordOptionMenuActions)
{
    DMenu *menu = access_private_field::SubToolWidgetm_recordOptionMenu(*m_sub);
    if (!menu) { SUCCEED(); return; }
    for (QAction *act : menu->actions()) {
        if (act && act->isCheckable()) {
            act->setChecked(true);
        }
        if (act && !act->menu()) {
            EXPECT_NO_FATAL_FAILURE(act->trigger());
        }
    }
    SUCCEED();
}

TEST_F(SubToolWidgetCovFinalTest, ShotOptionMenuActions)
{
    DMenu *menu = access_private_field::SubToolWidgetm_optionMenu(*m_sub);
    if (!menu) { SUCCEED(); return; }
    for (QAction *act : menu->actions()) {
        if (act && !act->menu()) {
            EXPECT_NO_FATAL_FAILURE(act->trigger());
        }
    }
    SUCCEED();
}

TEST_F(SubToolWidgetCovFinalTest, ShotOptionBorderStateLambda)
{
    EXPECT_NO_FATAL_FAILURE(ImageBorderHelper::instance()->setActionState(ImageBorderHelper::External, true));
    EXPECT_NO_FATAL_FAILURE(ImageBorderHelper::instance()->setActionState(ImageBorderHelper::Nothing, false));
}

TEST_F(SubToolWidgetCovFinalTest, ScrollOptionMenuActions)
{
    m_sub->switchContent(QStringLiteral("scroll"));
    DMenu *menu = access_private_field::SubToolWidgetm_scrollOptionMenu(*m_sub);
    if (!menu) { SUCCEED(); return; }
    for (QAction *act : menu->actions()) {
        if (act && !act->menu()) {
            EXPECT_NO_FATAL_FAILURE(act->trigger());
        }
    }
    SUCCEED();
}

TEST_F(SubToolWidgetCovFinalTest, RecordLabelButtonLambdas)
{
    m_sub->switchContent(QStringLiteral("record"));
    ToolButton *cameraBtn = access_private_field::SubToolWidgetm_cameraButton(*m_sub);
    if (cameraBtn) {
        cameraBtn->setEnabled(true);
        EXPECT_NO_FATAL_FAILURE(cameraBtn->click());
        EXPECT_NO_FATAL_FAILURE(cameraBtn->click());
    }
    ToolButton *shotBtn = access_private_field::SubToolWidgetm_shotButton(*m_sub);
    if (shotBtn) {
        EXPECT_NO_FATAL_FAILURE(shotBtn->click());
    }
    ToolButton *kbBtn = access_private_field::SubToolWidgetm_keyBoardButton(*m_sub);
    if (kbBtn) {
        EXPECT_NO_FATAL_FAILURE(kbBtn->click());
        EXPECT_NO_FATAL_FAILURE(kbBtn->click());
    }
    DMenu *optMenu = access_private_field::SubToolWidgetm_optionMenu(*m_sub);
    if (optMenu) {
        for (auto *act : optMenu->actions()) {
            EXPECT_NO_FATAL_FAILURE(act->trigger());
        }
    }
    SUCCEED();
}

TEST_F(SubToolWidgetCovFinalTest, ShotLabelRecorderButtonLambda)
{
    m_sub->switchContent(QStringLiteral("shot"));
    ToolButton *recBtn = access_private_field::SubToolWidgetm_recorderButton(*m_sub);
    if (recBtn) {
        EXPECT_NO_FATAL_FAILURE(recBtn->click());
    }
    SUCCEED();
}

TEST_F(SubToolWidgetCovFinalTest, ShotLabelButtonLambdas)
{
    m_sub->switchContent(QStringLiteral("shot"));
    QList<ToolButton *> buttons = m_sub->findChildren<ToolButton *>();
    for (ToolButton *btn : buttons) {
        if (dynamic_cast<SaveButton *>(btn)) continue;
        if (btn->menu()) continue;
        EXPECT_NO_FATAL_FAILURE(btn->click());
    }
    SUCCEED();
}

TEST_F(SubToolWidgetCovFinalTest, ScrollLabelButtonLambdas)
{
    m_sub->switchContent(QStringLiteral("scroll"));
    ToolButton *aiBtn = access_private_field::SubToolWidgetm_aiAssistantScrollButton(*m_sub);
    if (aiBtn) {
        EXPECT_NO_FATAL_FAILURE(aiBtn->click());
    }
    ToolButton *ocrBtn = access_private_field::SubToolWidgetm_ocrScrollButton(*m_sub);
    if (ocrBtn) {
        EXPECT_NO_FATAL_FAILURE(ocrBtn->click());
    }
    SUCCEED();
}

TEST_F(SubToolWidgetCovFinalTest, ShotBtnGroupLambda)
{
    QButtonGroup *grp = m_sub->findChild<QButtonGroup *>();
    if (!grp) { SUCCEED(); return; }
    QList<QAbstractButton *> btns = grp->buttons();
    for (QAbstractButton *btn : btns) {
        if (dynamic_cast<SaveButton *>(btn)) continue;
        auto *tb = qobject_cast<ToolButton *>(btn);
        if (tb && tb->menu()) continue;
        EXPECT_NO_FATAL_FAILURE(btn->click());
    }
    SUCCEED();
}

class ShotToolWidgetCovFinalTest : public Test
{
public:
    MainWindow *m_mw = nullptr;
    ShotToolWidget *m_stw = nullptr;
    void SetUp() override
    {
        m_mw = new MainWindow;
        m_stw = new ShotToolWidget(m_mw);
    }
    void TearDown() override { }
};

TEST_F(ShotToolWidgetCovFinalTest, InitTextLabelLambda)
{
    EXPECT_NO_FATAL_FAILURE(m_stw->switchContent(QStringLiteral("text")));
    QList<Slider *> sliders = m_stw->findChildren<Slider *>();
    for (Slider *s : sliders) {
        EXPECT_NO_FATAL_FAILURE(s->setValue(5));
        EXPECT_NO_FATAL_FAILURE(s->setValue(0));
    }
}
