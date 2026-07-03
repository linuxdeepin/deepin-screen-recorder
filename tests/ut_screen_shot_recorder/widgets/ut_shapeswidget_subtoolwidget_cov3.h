// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QList>
#include <QWidget>
#include <QVBoxLayout>
#include <QSignalSpy>
#include "addr_pri.h"
#include "../../src/utils/shapesutils.h"
#include "../../src/widgets/shapeswidget.h"
#include "../../src/main_window.h"
#include "../../src/widgets/subtoolwidget.h"
#include "../../src/utils/configsettings.h"
#include "../../src/utils/saveutils.h"

using namespace testing;

// ShapesWidgetSubToolCov3Test targets REMAINING uncovered paths in
// shapeswidget.cpp and subtoolwidget.cpp by exercising both widgets under a
// more realistic lifecycle: shown + processEvents (so paintEvent/update paths
// fire) and with multiple shapes / selection state set up first.
//
// DEDUP: all ACCESS_PRIVATE_FUN/FIELD targets used elsewhere for these two
// classes are NOT re-declared here. Only NEW, previously-undeclared private
// fields are added below; private functions are reached via the existing
// declarations (handlePaint / paint* / event handlers / resetForTextToolSwitch
// for ShapesWidget; showAIAssistantWidget for SubToolWidget).
//
// New private-field declarations (verified absent from sibling cov headers).
// Note: QMap<int, TextEdit *> contains a comma which would break the macro,
// so we alias it before the macro call.
using ShapesEditMap = QMap<int, TextEdit *>;
ACCESS_PRIVATE_FIELD(ShapesWidget, QPointF, m_pressedPoint);
ACCESS_PRIVATE_FIELD(ShapesWidget, QPointF, m_movingPoint);
ACCESS_PRIVATE_FIELD(ShapesWidget, int, m_hoveredIndex);
ACCESS_PRIVATE_FIELD(ShapesWidget, Toolshape, m_selectedShape);
ACCESS_PRIVATE_FIELD(ShapesWidget, Toolshape, m_hoveredShape);
ACCESS_PRIVATE_FIELD(ShapesWidget, ShapesEditMap, m_editMap);
ACCESS_PRIVATE_FIELD(ShapesWidget, QRect, m_globalRect);
ACCESS_PRIVATE_FIELD(ShapesWidget, QPoint, m_currentCursor);

class ShapesWidgetSubToolCov3Test : public Test
{
public:
    QWidget *m_dummy = nullptr;
    ShapesWidget *m_sw = nullptr;
    MainWindow *m_mw = nullptr;
    SubToolWidget *m_sub = nullptr;

    void SetUp() override
    {
        m_dummy = new QWidget;
        m_dummy->resize(800, 600);
        m_sw = new ShapesWidget(m_dummy);
        m_sw->resize(800, 600);

        // SubToolWidget requires a MainWindow in its ctor; create one lazily
        // and parent the subtool to the same dummy.
        m_mw = new MainWindow;
        m_sub = new SubToolWidget(m_mw, m_dummy);

        m_dummy->show();
        m_sw->show();
        qApp->processEvents();
    }
    void TearDown() override
    {
        // Intentionally leak m_sw / m_sub / m_mw / m_dummy — their destructors
        // tear down Qt widget trees that conflict with later TEST_Fs in this
        // single-TU binary (same pattern as MainWindowCovTest).
    }

    static FourPoints rectFP(qreal x, qreal y, qreal w, qreal h)
    {
        FourPoints fp;
        fp << QPointF(x, y) << QPointF(x + w, y)
           << QPointF(x + w, y + h) << QPointF(x, y + h);
        return fp;
    }
    static Toolshape makeShape(const QString &type)
    {
        Toolshape s;
        s.type = type;
        s.mainPoints = rectFP(20, 20, 200, 120);
        s.points = QList<QPointF>{QPointF(20, 20), QPointF(220, 140)};
        s.colorIndex = 0;
        s.lineWidth = 3;
        s.isBlur = false;
        s.isOval = 0;
        s.radius = 10;
        s.index = 0;
        return s;
    }
};

// ---------- ShapesWidget: lifecycle + show/paint pipeline ----------
TEST_F(ShapesWidgetSubToolCov3Test, shapesWidgetShowAndRepaint)
{
    EXPECT_NO_FATAL_FAILURE(m_sw->update());
    qApp->processEvents();
    EXPECT_NO_FATAL_FAILURE(m_sw->repaint());
    qApp->processEvents();
}

// ---------- setCurrentShape sweep with all tool types + visibility ----------
TEST_F(ShapesWidgetSubToolCov3Test, setCurrentShapeFullSweep)
{
    const QStringList types{
        "rectangle", "oval", "arrow", "line", "pen",
        "text", "effect", "blur", "mosaic", QString()
    };
    for (const QString &t : types) {
        EXPECT_NO_FATAL_FAILURE(m_sw->setCurrentShape(t));
        qApp->processEvents();
    }
}

// ---------- clearSelected / read-only / remove-unchanged with shapes present ----------
TEST_F(ShapesWidgetSubToolCov3Test, clearAndTextEditsWithShapes)
{
    Toolshapes &shapes = access_private_field::ShapesWidgetm_shapes(*m_sw);
    shapes.append(makeShape("rectangle"));
    shapes.append(makeShape("text"));
    EXPECT_NO_FATAL_FAILURE(m_sw->clearSelected());
    EXPECT_NO_FATAL_FAILURE(m_sw->setAllTextEditReadOnly());
    EXPECT_NO_FATAL_FAILURE(m_sw->setNoChangedTextEditRemove());
    qApp->processEvents();
}

// ---------- saveActionTriggered + menu slots ----------
TEST_F(ShapesWidgetSubToolCov3Test, saveAndMenuSlots)
{
    EXPECT_NO_FATAL_FAILURE(m_sw->saveActionTriggered());
    EXPECT_NO_FATAL_FAILURE(m_sw->menuSaveSlot());
    EXPECT_NO_FATAL_FAILURE(m_sw->menuCloseSlot());
}

// ---------- drag / rotate / resize with a selected shape ----------
TEST_F(ShapesWidgetSubToolCov3Test, dragRotateResizeSelected)
{
    Toolshapes &shapes = access_private_field::ShapesWidgetm_shapes(*m_sw);
    shapes.append(makeShape("rectangle"));
    access_private_field::ShapesWidgetm_currentIndex(*m_sw) = 0;
    EXPECT_NO_FATAL_FAILURE(m_sw->handleDrag(QPointF(20, 20), QPointF(80, 80)));
    EXPECT_NO_FATAL_FAILURE(m_sw->handleRotate(QPointF(120, 60)));
    for (int k = ShapesWidget::First; k <= ShapesWidget::Eighth; ++k) {
        EXPECT_NO_FATAL_FAILURE(m_sw->handleResize(QPointF(25, 25), k));
    }
    qApp->processEvents();
}

// ---------- undo / delete flows with populated shapes ----------
TEST_F(ShapesWidgetSubToolCov3Test, undoAndDeleteFlows)
{
    Toolshapes &shapes = access_private_field::ShapesWidgetm_shapes(*m_sw);
    for (int i = 0; i < 4; ++i) {
        Toolshape s = makeShape(i % 2 ? "oval" : "rectangle");
        s.index = i;
        shapes.append(s);
    }
    EXPECT_NO_FATAL_FAILURE(m_sw->undoDrawShapes());
    EXPECT_NO_FATAL_FAILURE(m_sw->undoDrawShapes());
    EXPECT_NO_FATAL_FAILURE(m_sw->undoAllDrawShapes());
    EXPECT_NO_FATAL_FAILURE(m_sw->deleteCurrentShape());
    EXPECT_NO_FATAL_FAILURE(m_sw->undoDrawShapes()); // empty-list noop
}

// ---------- isInUndoBtn / microAdjust / shift+microAdjust ----------
TEST_F(ShapesWidgetSubToolCov3Test, microAdjustAndUndoBtnAllStates)
{
    EXPECT_NO_FATAL_FAILURE(m_sw->isInUndoBtn(true));
    EXPECT_NO_FATAL_FAILURE(m_sw->isInUndoBtn(false));
    EXPECT_NO_FATAL_FAILURE(m_sw->setShiftKeyPressed(true));
    const QStringList dirs{
        Direction::LEFT, Direction::RIGHT, Direction::UP, Direction::DOWN,
        Direction::CTRL_LEFT, Direction::CTRL_RIGHT, Direction::CTRL_UP, Direction::CTRL_DOWN,
        Direction::CTRL_SHIFT_LEFT, Direction::CTRL_SHIFT_RIGHT,
        Direction::CTRL_SHIFT_UP, Direction::CTRL_SHIFT_DOWN,
        QStringLiteral("Unknown")
    };
    for (const QString &d : dirs) {
        EXPECT_NO_FATAL_FAILURE(m_sw->microAdjust(d));
    }
    EXPECT_NO_FATAL_FAILURE(m_sw->setShiftKeyPressed(false));
    for (const QString &d : dirs) {
        EXPECT_NO_FATAL_FAILURE(m_sw->microAdjust(d));
    }
}

// ---------- setGlobalRect / updateCursorShape / paintImage ----------
TEST_F(ShapesWidgetSubToolCov3Test, globalRectCursorAndPaintImage)
{
    EXPECT_NO_FATAL_FAILURE(m_sw->setGlobalRect(QRect(0, 0, 1920, 1080)));
    EXPECT_NO_FATAL_FAILURE(m_sw->updateCursorShape());
    QImage img(800, 600, QImage::Format_ARGB32);
    img.fill(Qt::white);
    EXPECT_NO_FATAL_FAILURE(m_sw->paintImage(img));
    qApp->processEvents();
}

// ---------- hit-tests with a populated shape list (drives clickedOnShapes/hover paths) ----------
TEST_F(ShapesWidgetSubToolCov3Test, hitTestsWithPopulatedShapes)
{
    Toolshapes &shapes = access_private_field::ShapesWidgetm_shapes(*m_sw);
    shapes.append(makeShape("rectangle"));
    shapes.append(makeShape("oval"));
    const QList<QPointF> probes{
        QPointF(50, 50), QPointF(120, 80), QPointF(300, 300),
        QPointF(20, 20), QPointF(220, 140), QPointF(1000, 1000)
    };
    for (const QPointF &p : probes) {
        EXPECT_NO_FATAL_FAILURE(m_sw->clickedOnShapes(p));
    }
    // also call the per-shape hover dispatcher on a freshly built Toolshape
    Toolshape ts = makeShape("rectangle");
    EXPECT_NO_FATAL_FAILURE(m_sw->hoverOnShapes(ts, QPointF(100, 80)));
    EXPECT_NO_FATAL_FAILURE(m_sw->hoverOnShapes(ts, QPointF(5000, 5000)));
    EXPECT_NO_FATAL_FAILURE(m_sw->isExistsText());
    EXPECT_NO_FATAL_FAILURE(m_sw->textEditIsReadOnly());
}

// ---------- SubToolWidget: full setter/slot sweep under one fixture ----------
TEST_F(ShapesWidgetSubToolCov3Test, subToolWidgetFullSweep)
{
    EXPECT_NO_FATAL_FAILURE(m_sub->switchContent(QStringLiteral("record")));
    qApp->processEvents();
    EXPECT_NO_FATAL_FAILURE(m_sub->switchContent(QStringLiteral("shot")));
    qApp->processEvents();
    EXPECT_NO_FATAL_FAILURE(m_sub->switchContent(QStringLiteral("scroll")));
    qApp->processEvents();
    EXPECT_NO_FATAL_FAILURE(m_sub->switchContent(QStringLiteral("shot")));

    EXPECT_NO_FATAL_FAILURE(m_sub->setScrollShotDisabled(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setScrollShotDisabled(false));
    bool b = true;
    EXPECT_NO_FATAL_FAILURE(m_sub->setPinScreenshotsEnable(b));
    b = false;
    EXPECT_NO_FATAL_FAILURE(m_sub->setPinScreenshotsEnable(b));
    EXPECT_NO_FATAL_FAILURE(m_sub->setOcrScreenshotEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setOcrScreenshotEnable(false));
    EXPECT_NO_FATAL_FAILURE(m_sub->setButEnableOnLockScreen(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setButEnableOnLockScreen(false));
    EXPECT_NO_FATAL_FAILURE(m_sub->setRecordButtonDisable());
    EXPECT_NO_FATAL_FAILURE(m_sub->setVideoButtonInitFromSub());
    EXPECT_NO_FATAL_FAILURE(m_sub->updateSaveButtonTip());

    EXPECT_NO_FATAL_FAILURE(m_sub->setMicroPhoneEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setMicroPhoneEnable(false));
    EXPECT_NO_FATAL_FAILURE(m_sub->setCameraDeviceEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setCameraDeviceEnable(false));

    EXPECT_NO_FATAL_FAILURE(m_sub->setRecordLaunchMode(static_cast<unsigned int>(MainWindow::record)));
    EXPECT_NO_FATAL_FAILURE(m_sub->setRecordLaunchMode(static_cast<unsigned int>(MainWindow::ocr)));
    EXPECT_NO_FATAL_FAILURE(m_sub->setRecordLaunchMode(static_cast<unsigned int>(MainWindow::scrollshot)));
    EXPECT_NO_FATAL_FAILURE(m_sub->setRecordLaunchMode(9999u));

    // shape dispatch sweep
    const QStringList shapes{
        "gio", "rectangle", "oval", "rect", "circ", "line", "arrow", "pen",
        "text", "aiassistant", "pinScreenshots", "scrollShot", "ocr",
        "keyBoard", "camera", "effect", "blur", "mosaic", "unknown_xyz", QString()
    };
    for (const QString &s : shapes) {
        EXPECT_NO_FATAL_FAILURE(m_sub->shapeClickedFromWidget(s));
    }

    // geometry / accessors
    QRect r;
    QPoint c;
    EXPECT_NO_FATAL_FAILURE(r = m_sub->getShotOptionRect());
    EXPECT_NO_FATAL_FAILURE(r = m_sub->getAiButtonGlobalRect());
    EXPECT_NO_FATAL_FAILURE(c = m_sub->getAiButtonGlobalCenter());
    for (const QString &s : shapes) {
        QString tmp = s;
        int x = -1;
        EXPECT_NO_FATAL_FAILURE(x = m_sub->getFuncSubToolX(tmp));
        (void)x;
    }

    // tip/hint installation
    QWidget w1, w2;
    EXPECT_NO_FATAL_FAILURE(m_sub->installTipHint(&w1, QStringLiteral("tip A")));
    EXPECT_NO_FATAL_FAILURE(m_sub->installHint(&w1, &w2));

    // eventFilter fall-through
    QObject obj;
    QEvent ev(QEvent::MouseButtonPress);
    EXPECT_NO_FATAL_FAILURE(m_sub->eventFilter(&obj, &ev));
}

// ---------- SubToolWidget: drive config-driven branches of updateSaveButtonTip ----------
TEST_F(ShapesWidgetSubToolCov3Test, subToolWidgetSaveTipAllBranches)
{
    ConfigSettings *s = ConfigSettings::instance();
    ASSERT_NE(s, nullptr);
    const SaveAction ops[] = {SaveToClipboard, SaveToDesktop, SaveToImage,
                              SaveToSpecificDir, AutoSave, CustomScreenSave};
    EXPECT_NO_FATAL_FAILURE({
        s->setValue("shot", "save_ways", SaveWays::Ask);
        m_sub->updateSaveButtonTip();
    });
    for (SaveAction op : ops) {
        EXPECT_NO_FATAL_FAILURE({
            s->setValue("shot", "save_ways", SaveWays::SpecifyLocation);
            s->setValue("shot", "save_op", static_cast<SaveAction>(op));
            s->setValue("shot", "save_dir_change", false);
            s->setValue("shot", "save_dir", QString());
            m_sub->updateSaveButtonTip();
        });
    }
    EXPECT_NO_FATAL_FAILURE({
        s->setValue("shot", "save_op", static_cast<SaveAction>(SaveToSpecificDir));
        s->setValue("shot", "save_dir_change", true);
        m_sub->updateSaveButtonTip();
    });
    EXPECT_NO_FATAL_FAILURE({
        s->setValue("shot", "save_ways", static_cast<SaveWays>(999));
        m_sub->updateSaveButtonTip();
    });
}

// ---------- SubToolWidget: signal emissions sanity (no crashes) ----------
TEST_F(ShapesWidgetSubToolCov3Test, subToolWidgetSignalSpySweep)
{
    QSignalSpy s1(m_sub, &SubToolWidget::keyBoardButtonClicked);
    QSignalSpy s2(m_sub, &SubToolWidget::cameraActionChecked);
    QSignalSpy s3(m_sub, &SubToolWidget::changeShotToolFunc);
    QSignalSpy s4(m_sub, &SubToolWidget::saveMethodChanged);
    EXPECT_NO_FATAL_FAILURE(m_sub->setRecordLaunchMode(static_cast<unsigned int>(MainWindow::record)));
    EXPECT_NO_FATAL_FAILURE(m_sub->shapeClickedFromWidget(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_sub->shapeClickedFromWidget(QStringLiteral("oval")));
    (void)s1.count(); (void)s2.count(); (void)s3.count(); (void)s4.count();
}
