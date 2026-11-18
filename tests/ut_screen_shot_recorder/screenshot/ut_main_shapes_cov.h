// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>

#include <QApplication>
#include <QCoreApplication>
#include <QImage>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QFileDialog>
#include <QPainter>
#include <QPixmap>
#include <QStringList>
#include <QTemporaryDir>
#include <QWheelEvent>
#include <QRandomGenerator>

#include "addr_pri.h"
#include "stub.h"
#include "../../src/main_window.h"
#include "../../src/utils/configsettings.h"
#include "../../src/utils/saveutils.h"
#include "../../src/utils/shapesutils.h"
#include "../../src/widgets/camerawidget.h"
#include "../../src/widgets/fineadjusttips.h"
#include "../../src/widgets/roundedcornerwidget.h"
#include "../../src/widgets/shapeswidget.h"
#include "../../src/widgets/sidebar.h"
#include "../../src/widgets/textedit.h"
#include "../../src/widgets/toolbar.h"

using MainWindowCovAlias = MainWindow;
using RoundedCornerWidgetCovAlias = RoundedCornerWidget;
using ShapesWidgetCovAlias = ShapesWidget;
using ShapesWidgetPaintAlias = ShapesWidget;

QString g_dialogSavePath;

void mainShapesNoop() {}

QString dialogSavePathStub(QWidget *, const QString &, const QString &,
                           const QString &, QString *, QFileDialog::Options)
{
    return g_dialogSavePath;
}

ACCESS_PRIVATE_FUN(MainWindowCovAlias, bool(QMouseEvent *, const QPoint &),
                   handleEditModeSelectionResize);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(const QPoint &), applyEditModeSelectionResize);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, bool(QKeyEvent *), handleEditModeSelectionResizeKey);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(bool), syncShapesWidgetGeometry);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, int(int) const, resizeCursorShape);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(int, int), onRoundedRatioChanged);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(qreal), applyRatioToCurrentSelection);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(const QPoint &), applyLockedRatioToNewSelection);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(int, int), onRoundedSelectionSizeChanged);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(), applyLockedRatioToSelection);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, bool() const, isRoundedPanelEditorFocused);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, bool(const QPixmap &), saveAction);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(QKeyEvent *), keyPressEvent);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, QPixmap(), paintImage);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(), updateToolBarPos);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(), updateSideBarPos);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(), updateCameraWidgetPos);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(), updateFineAdjustTips);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(), updateMultiKeyBoardPos);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, void(const QString &), showKeyBoardButtons);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, QPoint(QPoint, int), limitToolbarScope);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, int(const QPoint &), getAction);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, QRect() const, backgroundPhysicalRect);
ACCESS_PRIVATE_FUN(MainWindowCovAlias, QRect() const, shotTargetRectPhysical);

ACCESS_PRIVATE_FIELD(MainWindowCovAlias, unsigned int, m_functionType);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, ShapesWidget *, m_shapesWidget);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, bool, m_isShapesWidgetExist);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, recordX);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, recordY);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, recordWidth);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, recordHeight);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, QRect, m_backgroundRect);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, RoundedCornerWidget *, m_roundedCornerWidget);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, qreal, m_lockedRatio);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, dragAction);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, dragStartX);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, dragStartY);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, dragRecordX);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, dragRecordY);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, dragRecordWidth);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, int, dragRecordHeight);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, SaveAction, m_saveIndex);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, QString, m_saveFileName);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, QString, m_shotSavePath);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, QString, m_shotFileName);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, QString, m_shotFileFormat);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, bool, m_shotWithFullPath);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, bool, m_shotWithPath);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, bool, m_noNotify);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, QPixmap, m_backgroundPixmap);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, QPixmap, m_resultPixmap);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, QList<MainWindow::ScreenInfo>, m_screenInfo);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, ToolBar *, m_toolBar);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, SideBar *, m_sideBar);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, CameraWidget *, m_cameraWidget);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, FineAdjustTips *, m_fineAdjustTips);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, bool, isPressMouseLeftButton);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, bool, isReleaseMouseLeftButton);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, bool, drawDragPoint);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, bool, m_editModeResizing);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, bool, m_editModeResizeCursorActive);
ACCESS_PRIVATE_FIELD(MainWindowCovAlias, QString, m_currentToolShape);

ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, bool, m_ratioLocked);
ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, int, m_ratioWidth);
ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, int, m_ratioHeight);
ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, bool, m_rememberSetting);
ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, bool, m_pendingRememberedSetting);
ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, bool, m_rememberedIsRatio);
ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, int, m_fixedWidth);
ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, int, m_fixedHeight);
ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, int, m_selectionWidth);
ACCESS_PRIVATE_FIELD(RoundedCornerWidgetCovAlias, int, m_selectionHeight);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, void(bool), applySizeMode);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, void(), syncRatioFromSelection);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, void(), captureCurrentSetting);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, void(), persistRememberSetting);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, void(), persistFixedSize);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, void(), persistRatioType);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, bool(const QObject *) const,
                   isTextInputWidget);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, QSize() const, rememberedSetting);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, void(), consumeRememberedSetting);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, void(), updateLayout);
ACCESS_PRIVATE_FUN(RoundedCornerWidgetCovAlias, void(), reposition);

ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, int(), sequenceCount);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, int(int), sequenceNumberBeforeOrder);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(const QPointF &), placeSequenceAt);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(int, int), renumberSequenceFrom);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(int), resizeSelectedSequence);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(int) const, isLightSequenceColor);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(QPainter &, FourPoints, int, int) const,
                   paintSequence);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(const QPointF &), moveSequenceLimitTips);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(const QPointF &), updateSequenceLimitTips);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(const QPointF &), showSequenceLimitTips);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(QMouseEvent *), mouseDoubleClickEvent);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(QObject *, QEvent *), eventFilter);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(), commitSequenceEdit);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(), cancelSequenceEdit);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(const QPointF &), updateSequencePreview);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(QPointF, int), handleResize);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(QPointF), handleRotate);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias,
                   void(QPainter &, FourPoints, int, ShapesWidget::ShapeBlurStatus, bool, bool, int) const,
                   paintRect);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias,
                   void(QPainter &, FourPoints, int, ShapesWidget::ShapeBlurStatus, bool, bool, int) const,
                   paintEllipse);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias,
                   void(QPainter &, QList<QPointF>, int, bool) const, paintArrow);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(QPainter &, QList<QPointF>) const, paintLine);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias,
                   void(QPainter &, QList<QPointF>, bool, int, int) const, paintEffectLine);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, QPixmap(bool, int) const, effectPixmap);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(QPainter &, FourPoints) const, paintText);
ACCESS_PRIVATE_FUN(ShapesWidgetPaintAlias,
                   void(QPainter &, FourPoints, const QString &, int) const,
                   paintText);

ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, Toolshapes, m_shapes);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, QString, m_currentType);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, Toolshape, m_currentShape);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, Toolshape, m_selectedShape);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, Toolshape, m_hoveredShape);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, int, m_selectedIndex);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, int, m_selectedOrder);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, bool, m_isSelected);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, bool, m_isResize);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, bool, m_isRotated);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, bool, m_isPressed);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, ResizeDirection, m_resizeDirection);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, QPointF, m_pos1);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, QPointF, m_pos2);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, bool, m_isRecording);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, QLineEdit *, m_sequenceEdit);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, int, m_sequenceEditOrder);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, ToolTips *, m_sequenceLimitTips);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, QWidget *, m_sequencePreview);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, QImage, m_effectSourceImage);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, ShapesEditMap, m_editMap);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, bool, m_editing);
ACCESS_PRIVATE_FIELD(ShapesWidgetCovAlias, int, m_lastEditMapKey);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(), setAllTextEditReadOnly);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(), setNoChangedTextEditRemove);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(), textEditIsReadOnly);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(TextEdit *, QRectF), updateTextRect);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, void(QPinchGesture *), pinchTriggered);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(FourPoints, QPointF, bool), clickedOnRect);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(FourPoints, QPointF, bool), clickedOnEllipse);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(QList<QPointF>, QPointF), clickedOnArrow);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(FourPoints, QList<QPointF>, QPointF), clickedOnLine);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(FourPoints, QPointF), clickedOnText);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(FourPoints, QPointF, bool), hoverOnRect);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(FourPoints, QPointF), hoverOnEllipse);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(QList<QPointF>, QPointF), hoverOnArrow);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(FourPoints, QList<QPointF>, QPointF), hoverOnLine);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(int, FourPoints, QPointF), hoverOnText);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(FourPoints, QPointF), rotateOnPoint);
ACCESS_PRIVATE_FUN(ShapesWidgetCovAlias, bool(FourPoints, QPointF), hoverOnRotatePoint);

namespace {

constexpr int ActionMove = 0;
constexpr int ActionResizeTopLeft = 1;
constexpr int ActionResizeTopRight = 2;
constexpr int ActionResizeBottomLeft = 3;
constexpr int ActionResizeBottomRight = 4;
constexpr int ActionResizeTop = 5;
constexpr int ActionResizeBottom = 6;
constexpr int ActionResizeLeft = 7;
constexpr int ActionResizeRight = 8;

FourPoints shapeRect(qreal x, qreal y, qreal w, qreal h)
{
    return {QPointF(x, y), QPointF(x, y + h),
            QPointF(x + w, y), QPointF(x + w, y + h)};
}

Toolshape makeCovShape(const QString &type, const QRectF &rect, int sequence = 0)
{
    Toolshape shape;
    shape.type = type;
    shape.mainPoints = shapeRect(rect.left(), rect.top(), rect.width(), rect.height());
    shape.points = shape.mainPoints;
    shape.sequenceNumber = sequence;
    shape.lineWidth = 1;
    shape.colorIndex = 2;
    return shape;
}

} // namespace

class MainWindowEditModeCovTest : public Test
{
public:
    void SetUp() override
    {
        Utils::isTreelandMode = false;
        m_window = new MainWindow;
        m_window->resize(1200, 800);
        m_shapes = new ShapesWidget(m_window);
        m_rounded = new RoundedCornerWidget(m_window);

        access_private_field::MainWindowCovAliasm_functionType(*m_window) = MainWindow::shot;
        access_private_field::MainWindowCovAliasm_shapesWidget(*m_window) = m_shapes;
        access_private_field::MainWindowCovAliasm_isShapesWidgetExist(*m_window) = true;
        access_private_field::MainWindowCovAliasm_backgroundRect(*m_window) = QRect(0, 0, 1200, 800);
        access_private_field::MainWindowCovAliasm_roundedCornerWidget(*m_window) = m_rounded;
        access_private_field::MainWindowCovAliasrecordX(*m_window) = 100;
        access_private_field::MainWindowCovAliasrecordY(*m_window) = 80;
        access_private_field::MainWindowCovAliasrecordWidth(*m_window) = 400;
        access_private_field::MainWindowCovAliasrecordHeight(*m_window) = 300;
    }

    void TearDown() override
    {
        while (qApp->overrideCursor())
            qApp->restoreOverrideCursor();
        delete m_window;
        m_window = nullptr;
    }

    MainWindow *m_window = nullptr;
    ShapesWidget *m_shapes = nullptr;
    RoundedCornerWidget *m_rounded = nullptr;
};

TEST_F(MainWindowEditModeCovTest, resizeCursorMappings)
{
    EXPECT_EQ(call_private_fun::MainWindowCovAliasresizeCursorShape(
                      *m_window, ActionResizeTopLeft),
              Qt::SizeFDiagCursor);
    EXPECT_EQ(call_private_fun::MainWindowCovAliasresizeCursorShape(
                      *m_window, ActionResizeTopRight),
              Qt::SizeBDiagCursor);
    EXPECT_EQ(call_private_fun::MainWindowCovAliasresizeCursorShape(
                      *m_window, ActionResizeLeft),
              Qt::SizeHorCursor);
    EXPECT_EQ(call_private_fun::MainWindowCovAliasresizeCursorShape(
                      *m_window, ActionResizeTop),
              Qt::SizeVerCursor);
    EXPECT_EQ(call_private_fun::MainWindowCovAliasresizeCursorShape(
                      *m_window, ActionMove),
              Qt::ArrowCursor);
}

TEST_F(MainWindowEditModeCovTest, syncAndResizeSelection)
{
    access_private_field::ShapesWidgetCovAliasm_shapes(*m_shapes) = {
        makeCovShape(QStringLiteral("rectangle"), QRectF(100, 100, 40, 40))
    };
    m_shapes->move(80, 60);

    call_private_fun::MainWindowCovAliassyncShapesWidgetGeometry(*m_window, true);
    EXPECT_EQ(m_shapes->pos(), QPoint(102, 82));
    EXPECT_EQ(m_shapes->size(), QSize(396, 296));
    EXPECT_EQ(access_private_field::ShapesWidgetCovAliasm_shapes(*m_shapes).first().mainPoints.first(),
              QPointF(78, 78));

    QMouseEvent press(QEvent::MouseButtonPress, QPointF(500, 230),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_TRUE(call_private_fun::MainWindowCovAliashandleEditModeSelectionResize(
            *m_window, &press, QPoint(500, 230)));
    QMouseEvent move(QEvent::MouseMove, QPointF(520, 230),
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_TRUE(call_private_fun::MainWindowCovAliashandleEditModeSelectionResize(
            *m_window, &move, QPoint(520, 230)));
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(520, 230),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_TRUE(call_private_fun::MainWindowCovAliashandleEditModeSelectionResize(
            *m_window, &release, QPoint(520, 230)));

    access_private_field::MainWindowCovAliasrecordWidth(*m_window) = 400;
    QKeyEvent enlarge(QEvent::KeyPress, Qt::Key_Right, Qt::ControlModifier);
    EXPECT_TRUE(call_private_fun::MainWindowCovAliashandleEditModeSelectionResizeKey(
            *m_window, &enlarge));
    QKeyEvent shrink(QEvent::KeyPress, Qt::Key_Left,
                     Qt::ControlModifier | Qt::ShiftModifier);
    EXPECT_TRUE(call_private_fun::MainWindowCovAliashandleEditModeSelectionResizeKey(
            *m_window, &shrink));
}

TEST_F(MainWindowEditModeCovTest, roundedRatioAndSizeCallbacks)
{
    access_private_field::RoundedCornerWidgetCovAliasm_ratioLocked(*m_rounded) = true;
    access_private_field::RoundedCornerWidgetCovAliasm_ratioWidth(*m_rounded) = 16;
    access_private_field::RoundedCornerWidgetCovAliasm_ratioHeight(*m_rounded) = 9;

    call_private_fun::MainWindowCovAliasonRoundedRatioChanged(*m_window, 16, 9);
    EXPECT_DOUBLE_EQ(access_private_field::MainWindowCovAliasm_lockedRatio(*m_window),
                     16.0 / 9.0);

    call_private_fun::MainWindowCovAliasapplyRatioToCurrentSelection(*m_window, 4.0 / 3.0);
    EXPECT_GT(access_private_field::MainWindowCovAliasrecordHeight(*m_window), 0);

    access_private_field::MainWindowCovAliasdragStartX(*m_window) = 100;
    access_private_field::MainWindowCovAliasdragStartY(*m_window) = 80;
    call_private_fun::MainWindowCovAliasapplyLockedRatioToNewSelection(
            *m_window, QPoint(500, 380));
    EXPECT_GT(access_private_field::MainWindowCovAliasrecordWidth(*m_window), 0);

    access_private_field::MainWindowCovAliasdragAction(*m_window) =
            ActionResizeRight;
    access_private_field::MainWindowCovAliasdragRecordX(*m_window) = 100;
    access_private_field::MainWindowCovAliasdragRecordY(*m_window) = 80;
    access_private_field::MainWindowCovAliasdragRecordWidth(*m_window) = 400;
    access_private_field::MainWindowCovAliasdragRecordHeight(*m_window) = 300;
    call_private_fun::MainWindowCovAliasapplyLockedRatioToSelection(*m_window);

    call_private_fun::MainWindowCovAliasonRoundedSelectionSizeChanged(*m_window, 640, 480);
    EXPECT_EQ(access_private_field::MainWindowCovAliasrecordWidth(*m_window), 640);
    EXPECT_EQ(access_private_field::MainWindowCovAliasrecordHeight(*m_window), 480);
}

TEST_F(MainWindowEditModeCovTest, saveActionCoversConfiguredDestinations)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    QPixmap pixmap(120, 80);
    pixmap.fill(Qt::red);
    ConfigSettings::instance()->setValue("shot", "save_ways", SpecifyLocation);
    access_private_field::MainWindowCovAliasm_shotSavePath(*m_window) = directory.path();
    access_private_field::MainWindowCovAliasm_shotFileName(*m_window) = QStringLiteral("saved");
    access_private_field::MainWindowCovAliasm_shotFileFormat(*m_window) = QStringLiteral("png");
    access_private_field::MainWindowCovAliasm_shotWithFullPath(*m_window) = false;
    access_private_field::MainWindowCovAliasm_shotWithPath(*m_window) = false;

    const SaveAction actions[] = {
        SaveToDesktop,
        SaveToImage,
        SaveToSpecificDir,
        AutoSave,
        CustomScreenSave
    };
    for (int format = 0; format < 3; ++format) {
        ConfigSettings::instance()->setValue("shot", "format", format);
        ConfigSettings::instance()->setValue("shot", "save_dir", directory.path());
        ConfigSettings::instance()->setValue("shot", "save_dir_change", false);
        for (SaveAction action : actions) {
            ConfigSettings::instance()->setValue("shot", "save_op",
                                                 static_cast<int>(action));
            EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliassaveAction(
                    *m_window, pixmap));
        }
    }
}

TEST_F(MainWindowEditModeCovTest, saveActionAskAndInvalidSuffixSweep)
{
    Stub stub;
    stub.set(ADDR(QFileDialog, getSaveFileName), dialogSavePathStub);
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    QPixmap pixmap(80, 60);
    pixmap.fill(Qt::green);
    ConfigSettings::instance()->setValue("shot", "save_ways", Ask);
    access_private_field::MainWindowCovAliasm_shotWithPath(*m_window) = false;

    for (int format = 0; format < 4; ++format) {
        ConfigSettings::instance()->setValue("shot", "format", format);
        for (const QString &name : {QStringLiteral("plain"),
                                    QStringLiteral("invalid.data"),
                                    QStringLiteral("valid.png"),
                                    QStringLiteral("valid.jpg"),
                                    QStringLiteral("valid.bmp"),
                                    QString()}) {
            g_dialogSavePath = name.isEmpty() ? QString()
                                              : directory.filePath(name);
            EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliassaveAction(
                    *m_window, pixmap));
        }
    }
    g_dialogSavePath.clear();
}

TEST_F(MainWindowEditModeCovTest, applyAllResizeActions)
{
    access_private_field::MainWindowCovAliasdragStartX(*m_window) = 200;
    access_private_field::MainWindowCovAliasdragStartY(*m_window) = 180;
    access_private_field::MainWindowCovAliasdragRecordX(*m_window) = 100;
    access_private_field::MainWindowCovAliasdragRecordY(*m_window) = 80;
    access_private_field::MainWindowCovAliasdragRecordWidth(*m_window) = 400;
    access_private_field::MainWindowCovAliasdragRecordHeight(*m_window) = 300;
    for (int action = 0; action < 9; ++action) {
        access_private_field::MainWindowCovAliasdragAction(*m_window) = action;
        EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliasapplyEditModeSelectionResize(
                *m_window, QPoint(260 + action * 3, 210 + action * 2)));
    }
}

TEST_F(MainWindowEditModeCovTest, keyEventSweep)
{
    const MainWindow::status modes[] = {
        MainWindow::shot,
        MainWindow::scrollshot,
        MainWindow::record
    };
    const QList<QPair<int, Qt::KeyboardModifiers>> keys = {
        {Qt::Key_Left, Qt::NoModifier},
        {Qt::Key_Right, Qt::ControlModifier},
        {Qt::Key_Up, Qt::ControlModifier | Qt::ShiftModifier},
        {Qt::Key_Down, Qt::ShiftModifier},
        {Qt::Key_A, Qt::NoModifier},
        {Qt::Key_D, Qt::NoModifier},
        {Qt::Key_W, Qt::NoModifier},
        {Qt::Key_S, Qt::NoModifier}
    };

    for (MainWindow::status mode : modes) {
        access_private_field::MainWindowCovAliasm_functionType(*m_window) = mode;
        for (const auto &entry : keys) {
            QKeyEvent event(QEvent::KeyPress, entry.first, entry.second);
            EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliaskeyPressEvent(
                    *m_window, &event));
        }
    }
}

TEST_F(MainWindowEditModeCovTest, mouseEdgeWorkflowSweep)
{
    m_window->show();
    m_shapes->show();
    QApplication::processEvents();

    const QList<QPoint> points = {
        QPoint(100, 80),
        QPoint(500, 80),
        QPoint(100, 380),
        QPoint(500, 380),
        QPoint(300, 80),
        QPoint(300, 380),
        QPoint(100, 230),
        QPoint(500, 230),
        QPoint(300, 230),
        QPoint(900, 700)
    };
    for (const QPoint &point : points) {
        QMouseEvent hover(QEvent::MouseMove, QPointF(point),
                          Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(point),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent drag(QEvent::MouseMove, QPointF(point + QPoint(20, 15)),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(point + QPoint(20, 15)),
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &hover));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &press));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &drag));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &release));
    }
}

TEST_F(MainWindowEditModeCovTest, selectedAnnotationEventSweep)
{
    const QStringList types = {
        QStringLiteral("rectangle"),
        QStringLiteral("oval"),
        QStringLiteral("arrow"),
        QStringLiteral("line"),
        QStringLiteral("pen"),
        QStringLiteral("effect"),
        QStringLiteral("text"),
        QStringLiteral("sequence")
    };
    for (const QString &type : types) {
        Toolshape shape = makeCovShape(type, QRectF(140, 120, 220, 140), 1);
        access_private_field::ShapesWidgetCovAliasm_shapes(*m_shapes) = {shape};
        access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_shapes) = shape;
        access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_shapes) = 0;
        access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_shapes) = 0;
        access_private_field::ShapesWidgetCovAliasm_currentType(*m_shapes) = type;

        const QList<QPair<int, Qt::KeyboardModifiers>> keys = {
            {Qt::Key_Left, Qt::ControlModifier},
            {Qt::Key_Right, Qt::ControlModifier},
            {Qt::Key_Up, Qt::ControlModifier | Qt::ShiftModifier},
            {Qt::Key_Down, Qt::ControlModifier | Qt::ShiftModifier},
            {Qt::Key_A, Qt::NoModifier},
            {Qt::Key_D, Qt::NoModifier},
            {Qt::Key_W, Qt::NoModifier},
            {Qt::Key_S, Qt::NoModifier},
            {Qt::Key_Delete, Qt::NoModifier}
        };
        for (const auto &entry : keys) {
            QKeyEvent event(QEvent::KeyPress, entry.first, entry.second);
            EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliaskeyPressEvent(
                    *m_window, &event));
        }

        const QList<QPoint> points = {
            QPoint(140, 120), QPoint(360, 120), QPoint(140, 260),
            QPoint(360, 260), QPoint(250, 190), QPoint(800, 700)
        };
        for (const QPoint &point : points) {
            QMouseEvent press(QEvent::MouseButtonPress, QPointF(point),
                              Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QMouseEvent drag(QEvent::MouseMove, QPointF(point + QPoint(12, 9)),
                             Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QMouseEvent release(QEvent::MouseButtonRelease, QPointF(point + QPoint(12, 9)),
                                Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &press));
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &drag));
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &release));
        }
    }
}

TEST_F(MainWindowEditModeCovTest, eventTypeStressSweep)
{
    m_window->show();
    QApplication::processEvents();
    const MainWindow::status modes[] = {
        MainWindow::shot,
        MainWindow::scrollshot,
        MainWindow::record
    };
    for (MainWindow::status mode : modes) {
        access_private_field::MainWindowCovAliasm_functionType(*m_window) = mode;
        const QList<QEvent::Type> eventTypes = {
            QEvent::Enter,
            QEvent::Leave,
            QEvent::FocusIn,
            QEvent::FocusOut,
            QEvent::WindowActivate,
            QEvent::WindowDeactivate,
            QEvent::Show,
            QEvent::Hide
        };
        for (QEvent::Type type : eventTypes) {
            QEvent event(type);
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &event));
        }

        QWheelEvent wheel(QPointF(200, 150), QPointF(200, 150), QPoint(0, 0),
                          QPoint(0, 120), Qt::NoButton, Qt::NoModifier,
                          Qt::NoScrollPhase, false);
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &wheel));

        for (Qt::MouseButton button : {Qt::LeftButton, Qt::RightButton, Qt::MiddleButton}) {
            QMouseEvent press(QEvent::MouseButtonPress, QPointF(250, 180), button,
                              button, Qt::NoModifier);
            QMouseEvent move(QEvent::MouseMove, QPointF(290, 220), button,
                             button, Qt::NoModifier);
            QMouseEvent release(QEvent::MouseButtonRelease, QPointF(290, 220), button,
                                Qt::NoButton, Qt::NoModifier);
            QMouseEvent doubleClick(QEvent::MouseButtonDblClick, QPointF(250, 180), button,
                                    button, Qt::NoModifier);
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &press));
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &move));
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &release));
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &doubleClick));
        }

        for (int key : {Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down,
                        Qt::Key_A, Qt::Key_D, Qt::Key_W, Qt::Key_S,
                        Qt::Key_Escape, Qt::Key_Return}) {
            QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier);
            QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier);
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &press));
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &release));
        }
    }
}

TEST_F(MainWindowEditModeCovTest, initializedUiGeometrySweep)
{
    TestToolBarHost host(m_window);
    auto *toolBar = new ToolBar(m_window);
    toolBar->initToolBar(&host);
    auto *sideBar = new SideBar(m_window);
    sideBar->initSideBar(&host);
    auto *camera = new CameraWidget(m_window);
    auto *fineTips = new FineAdjustTips(m_window);

    access_private_field::MainWindowCovAliasm_toolBar(*m_window) = toolBar;
    access_private_field::MainWindowCovAliasm_sideBar(*m_window) = sideBar;
    access_private_field::MainWindowCovAliasm_cameraWidget(*m_window) = camera;
    access_private_field::MainWindowCovAliasm_fineAdjustTips(*m_window) = fineTips;
    toolBar->show();
    sideBar->showWidget();
    camera->show();
    fineTips->show();
    QApplication::processEvents();

    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliasupdateToolBarPos(*m_window));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliasupdateSideBarPos(*m_window));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliasupdateCameraWidgetPos(*m_window));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliasupdateFineAdjustTips(*m_window));

    access_private_field::MainWindowCovAliasm_backgroundPixmap(*m_window) =
            QPixmap(800, 600);
    access_private_field::MainWindowCovAliasm_resultPixmap(*m_window) =
            QPixmap(400, 300);
    m_window->resize(800, 600);
    m_window->show();
    QApplication::processEvents();
    QImage rendered(800, 600, QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::transparent);
    EXPECT_NO_FATAL_FAILURE(m_window->render(&rendered));
}

TEST_F(MainWindowEditModeCovTest, saveFullPathAndClipboardBranches)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    QPixmap pixmap(100, 80);
    pixmap.fill(Qt::blue);
    ConfigSettings::instance()->setValue("shot", "save_ways", SpecifyLocation);
    access_private_field::MainWindowCovAliasm_shotWithPath(*m_window) = true;
    access_private_field::MainWindowCovAliasm_shotWithFullPath(*m_window) = true;
    access_private_field::MainWindowCovAliasm_shotSavePath(*m_window) = directory.path();
    access_private_field::MainWindowCovAliasm_shotFileName(*m_window) = QStringLiteral("full.png");
    access_private_field::MainWindowCovAliasm_shotFileFormat(*m_window) = QStringLiteral("png");
    access_private_field::MainWindowCovAliasm_functionType(*m_window) = MainWindow::shot;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliassaveAction(*m_window, pixmap));

    access_private_field::MainWindowCovAliasm_shotWithPath(*m_window) = false;
    access_private_field::MainWindowCovAliasm_shotWithFullPath(*m_window) = false;
    ConfigSettings::instance()->setValue("shot", "save_op", static_cast<int>(SaveToClipboard));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliassaveAction(*m_window, pixmap));
}

TEST_F(MainWindowEditModeCovTest, mouseStateBranchSweep)
{
    m_window->show();
    m_shapes->show();
    QApplication::processEvents();
    Toolshape shape = makeCovShape(QStringLiteral("rectangle"), QRectF(140, 120, 220, 140));
    access_private_field::ShapesWidgetCovAliasm_shapes(*m_shapes) = {shape};
    access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_shapes) = shape;
    access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_shapes) = 0;
    access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_shapes) = 0;
    access_private_field::MainWindowCovAliasm_currentToolShape(*m_window) =
            QStringLiteral("rectangle");
    access_private_field::MainWindowCovAliasm_functionType(*m_window) = MainWindow::shot;
    access_private_field::MainWindowCovAliasm_isShapesWidgetExist(*m_window) = true;

    for (bool pressed : {false, true}) {
        for (bool released : {false, true}) {
            access_private_field::MainWindowCovAliasisPressMouseLeftButton(*m_window) = pressed;
            access_private_field::MainWindowCovAliasisReleaseMouseLeftButton(*m_window) = released;
            access_private_field::MainWindowCovAliasdrawDragPoint(*m_window) = pressed;
            access_private_field::MainWindowCovAliasm_editModeResizing(*m_window) = pressed;
            access_private_field::MainWindowCovAliasm_editModeResizeCursorActive(*m_window) =
                    pressed;
            for (const QPoint &point : {QPoint(140, 120), QPoint(360, 260),
                                        QPoint(250, 190), QPoint(800, 700)}) {
                QMouseEvent move(QEvent::MouseMove, QPointF(point),
                                 pressed ? Qt::LeftButton : Qt::NoButton,
                                 pressed ? Qt::LeftButton : Qt::NoButton,
                                 pressed ? Qt::ControlModifier : Qt::NoModifier);
                QMouseEvent release(QEvent::MouseButtonRelease, QPointF(point),
                                    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
                EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &move));
                EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &release));
            }
        }
    }
    while (qApp->overrideCursor())
        qApp->restoreOverrideCursor();
}

TEST_F(MainWindowEditModeCovTest, deterministicInputFuzz)
{
    QRandomGenerator random(0x5a17);
    m_window->show();
    m_shapes->show();
    QApplication::processEvents();
    const QStringList types = {
        QStringLiteral("rectangle"), QStringLiteral("oval"), QStringLiteral("arrow"),
        QStringLiteral("line"), QStringLiteral("pen"), QStringLiteral("effect"),
        QStringLiteral("text"), QStringLiteral("sequence")
    };
    const QList<int> keys = {
        Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down,
        Qt::Key_A, Qt::Key_D, Qt::Key_W, Qt::Key_S
    };

    for (int i = 0; i < 400; ++i) {
        const QString type = types.at(random.bounded(types.size()));
        Toolshape shape = makeCovShape(type,
                                       QRectF(random.bounded(300), random.bounded(220),
                                              random.bounded(60, 180),
                                              random.bounded(40, 140)), i + 1);
        access_private_field::ShapesWidgetCovAliasm_shapes(*m_shapes) = {shape};
        access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_shapes) = shape;
        access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_shapes) = 0;
        access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_shapes) = 0;
        access_private_field::ShapesWidgetCovAliasm_currentType(*m_shapes) = type;
        access_private_field::MainWindowCovAliasm_currentToolShape(*m_window) = type;
        access_private_field::MainWindowCovAliasm_functionType(*m_window) =
                (i % 3 == 0) ? MainWindow::scrollshot : MainWindow::shot;
        access_private_field::MainWindowCovAliasm_isShapesWidgetExist(*m_window) = true;
        access_private_field::MainWindowCovAliasisPressMouseLeftButton(*m_window) = (i % 2 == 0);
        access_private_field::MainWindowCovAliasisReleaseMouseLeftButton(*m_window) = (i % 3 == 0);
        access_private_field::MainWindowCovAliasm_editModeResizing(*m_window) = (i % 4 == 0);
        access_private_field::MainWindowCovAliasm_screenInfo(*m_window) = {
            MainWindow::ScreenInfo{0, 0, 1200, 800, QStringLiteral("one")}
        };

        const QPoint point(random.bounded(1000), random.bounded(700));
        const Qt::KeyboardModifiers modifiers =
                static_cast<Qt::KeyboardModifiers>(random.bounded(8));
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(point),
                          Qt::LeftButton, Qt::LeftButton, modifiers);
        QMouseEvent move(QEvent::MouseMove, QPointF(point + QPoint(10, 8)),
                         Qt::LeftButton, Qt::LeftButton, modifiers);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(point + QPoint(10, 8)),
                            Qt::LeftButton, Qt::NoButton, modifiers);
        QKeyEvent key(QEvent::KeyPress, keys.at(random.bounded(keys.size())), modifiers);
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &press));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &move));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &release));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_window, &key));
    }
    while (qApp->overrideCursor())
        qApp->restoreOverrideCursor();
}

TEST_F(MainWindowEditModeCovTest, changeShotToolRecognitionBranches)
{
    Stub stub;
    stub.set(ADDR(MainWindow, saveScreenShot), mainShapesNoop);
    TestToolBarHost host(m_window);
    auto *toolBar = new ToolBar(m_window);
    toolBar->initToolBar(&host);
    auto *sideBar = new SideBar(m_window);
    sideBar->initSideBar(&host);
    access_private_field::MainWindowCovAliasm_toolBar(*m_window) = toolBar;
    access_private_field::MainWindowCovAliasm_sideBar(*m_window) = sideBar;
    access_private_field::MainWindowCovAliasm_roundedCornerWidget(*m_window) = m_rounded;
    access_private_field::MainWindowCovAliasm_isShapesWidgetExist(*m_window) = true;
    access_private_field::MainWindowCovAliasm_shapesWidget(*m_window) = m_shapes;

    for (const QString &tool : {QStringLiteral("ocr"),
                                QStringLiteral("pinScreenshots"),
                                QStringLiteral("tableRecogn")}) {
        EXPECT_NO_FATAL_FAILURE(m_window->changeShotToolEvent(tool));
    }
}

TEST_F(MainWindowEditModeCovTest, roundedPanelFocusAndPaintImage)
{
    m_window->show();
    m_rounded->show();
    QLineEdit *editor = new QLineEdit(m_rounded);
    editor->show();
    editor->setFocus();
    QApplication::processEvents();
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowCovAliasisRoundedPanelEditorFocused(
            *m_window));
    QPixmap painted;
    EXPECT_NO_FATAL_FAILURE(painted = call_private_fun::MainWindowCovAliaspaintImage(*m_window));
    Q_UNUSED(painted);
}

class ShapesWidgetExtendedCovTest : public Test
{
public:
    void SetUp() override
    {
        m_widget = new ShapesWidget;
        m_widget->resize(200, 150);
    }

    void TearDown() override { delete m_widget; }

    ShapesWidget *m_widget = nullptr;
};

TEST_F(ShapesWidgetExtendedCovTest, translateCursorAndLivePaint)
{
    access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {
        makeCovShape(QStringLiteral("rectangle"), QRectF(10, 10, 40, 40)),
        makeCovShape(QStringLiteral("sequence"), QRectF(60, 20, 24, 24), 1)
    };
    m_widget->translateAllShapes(QPointF(5, 7));
    EXPECT_EQ(access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget)
                      .first().mainPoints.first(),
              QPointF(15, 17));
    m_widget->setCursorManagementEnabled(false);
    m_widget->setCursorManagementEnabled(true);

    access_private_field::ShapesWidgetCovAliasm_currentType(*m_widget) =
            QStringLiteral("rectangle");
    access_private_field::ShapesWidgetCovAliasm_currentShape(*m_widget) =
            makeCovShape(QStringLiteral("rectangle"), QRectF());
    access_private_field::ShapesWidgetCovAliasm_pos1(*m_widget) = QPointF(10, 10);
    access_private_field::ShapesWidgetCovAliasm_pos2(*m_widget) = QPointF(80, 70);
    access_private_field::ShapesWidgetCovAliasm_isRecording(*m_widget) = true;

    QImage image(200, 150, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    EXPECT_NO_FATAL_FAILURE(m_widget->paintLiveContent(painter));
}

TEST_F(ShapesWidgetExtendedCovTest, sequenceHelpersAndEditing)
{
    Toolshape first = makeCovShape(QStringLiteral("sequence"), QRectF(10, 10, 24, 24), 1);
    Toolshape second = makeCovShape(QStringLiteral("sequence"), QRectF(50, 10, 24, 24), 2);
    access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {first, second};
    access_private_field::ShapesWidgetCovAliasm_currentType(*m_widget) =
            QStringLiteral("sequence");
    access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_widget) = first;
    access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_widget) = 0;
    access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_widget) = 0;

    EXPECT_EQ(call_private_fun::ShapesWidgetCovAliassequenceCount(*m_widget), 2);
    EXPECT_EQ(call_private_fun::ShapesWidgetCovAliassequenceNumberBeforeOrder(*m_widget, 1), 1);
    EXPECT_TRUE(call_private_fun::ShapesWidgetCovAliasisLightSequenceColor(*m_widget, 2));
    EXPECT_FALSE(call_private_fun::ShapesWidgetCovAliasisLightSequenceColor(*m_widget, 1));

    QImage image(100, 100, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintSequence(
            *m_widget, painter, first.mainPoints, 1, 2));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintSequence(
            *m_widget, painter, FourPoints(), 1, 2));

    call_private_fun::ShapesWidgetCovAliasrenumberSequenceFrom(*m_widget, 0, 10);
    EXPECT_EQ(access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget)
                      .at(0).sequenceNumber, 10);
    EXPECT_EQ(access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget)
                      .at(1).sequenceNumber, 11);

    access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_widget) =
            access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget).first();
    call_private_fun::ShapesWidgetCovAliasresizeSelectedSequence(*m_widget, 2);

    const int before = access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget).size();
    EXPECT_TRUE(call_private_fun::ShapesWidgetCovAliasplaceSequenceAt(
            *m_widget, QPointF(100, 80)));
    EXPECT_EQ(access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget).size(), before + 1);

    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliasshowSequenceLimitTips(
            *m_widget, QPointF(20, 20)));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliasmoveSequenceLimitTips(
            *m_widget, QPointF(30, 30)));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliasupdateSequenceLimitTips(
            *m_widget, QPointF(40, 40)));

    access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {first, second};
    QMouseEvent doubleClick(QEvent::MouseButtonDblClick, QPointF(20, 20),
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::ShapesWidgetCovAliasmouseDoubleClickEvent(*m_widget, &doubleClick);
    QLineEdit *edit = access_private_field::ShapesWidgetCovAliasm_sequenceEdit(*m_widget);
    ASSERT_NE(edit, nullptr);
    edit->setText(QStringLiteral("5"));
    QKeyEvent commitEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    EXPECT_TRUE(call_private_fun::ShapesWidgetCovAliaseventFilter(
            *m_widget, edit, &commitEvent));
    EXPECT_EQ(access_private_field::ShapesWidgetCovAliasm_sequenceEdit(*m_widget), nullptr);

    QMouseEvent doubleClickAgain(QEvent::MouseButtonDblClick, QPointF(20, 20),
                                 Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::ShapesWidgetCovAliasmouseDoubleClickEvent(*m_widget, &doubleClickAgain);
    edit = access_private_field::ShapesWidgetCovAliasm_sequenceEdit(*m_widget);
    ASSERT_NE(edit, nullptr);
    QKeyEvent cancelEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_TRUE(call_private_fun::ShapesWidgetCovAliaseventFilter(
            *m_widget, edit, &cancelEvent));
    EXPECT_EQ(access_private_field::ShapesWidgetCovAliasm_sequenceEdit(*m_widget), nullptr);
}

TEST_F(ShapesWidgetExtendedCovTest, mouseAndKeyWorkflowSweep)
{
    const QStringList types = {
        QStringLiteral("rectangle"),
        QStringLiteral("oval"),
        QStringLiteral("arrow"),
        QStringLiteral("line"),
        QStringLiteral("pen"),
        QStringLiteral("effect"),
        QStringLiteral("text"),
        QStringLiteral("sequence")
    };

    for (const QString &type : types) {
        m_widget->setCurrentShape(type);
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(20, 20),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent move(QEvent::MouseMove, QPointF(120, 90),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(120, 90),
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &press));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &move));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &release));
        EXPECT_NO_FATAL_FAILURE(m_widget->microAdjust(QStringLiteral("left")));
        EXPECT_NO_FATAL_FAILURE(m_widget->microAdjust(QStringLiteral("right")));
        EXPECT_NO_FATAL_FAILURE(m_widget->microAdjust(QStringLiteral("up")));
        EXPECT_NO_FATAL_FAILURE(m_widget->microAdjust(QStringLiteral("down")));
        EXPECT_NO_FATAL_FAILURE(m_widget->deleteCurrentShape());
    }

    const QList<QPair<int, Qt::KeyboardModifiers>> keys = {
        {Qt::Key_Left, Qt::NoModifier},
        {Qt::Key_Right, Qt::ControlModifier},
        {Qt::Key_Up, Qt::ShiftModifier},
        {Qt::Key_Down, Qt::ControlModifier | Qt::ShiftModifier},
        {Qt::Key_A, Qt::NoModifier},
        {Qt::Key_D, Qt::NoModifier},
        {Qt::Key_W, Qt::NoModifier},
        {Qt::Key_S, Qt::NoModifier}
    };
    for (const auto &entry : keys) {
        QKeyEvent event(QEvent::KeyPress, entry.first, entry.second);
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &event));
    }

    QImage image(200, 150, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    EXPECT_NO_FATAL_FAILURE(m_widget->paintImage(image));
}

TEST_F(ShapesWidgetExtendedCovTest, selectedShapeInteractionSweep)
{
    const QStringList types = {
        QStringLiteral("rectangle"),
        QStringLiteral("oval"),
        QStringLiteral("arrow"),
        QStringLiteral("line"),
        QStringLiteral("pen"),
        QStringLiteral("effect"),
        QStringLiteral("text"),
        QStringLiteral("sequence")
    };
    for (const QString &type : types) {
        Toolshape shape = makeCovShape(type, QRectF(30, 30, 100, 70), 1);
        access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {shape};
        access_private_field::ShapesWidgetCovAliasm_currentType(*m_widget) = type;
        access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_widget) = shape;
        access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_widget) = 0;
        access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_widget) = 0;

        QMouseEvent press(QEvent::MouseButtonPress, QPointF(40, 40),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent move(QEvent::MouseMove, QPointF(90, 80),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(90, 80),
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &press));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &move));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &release));

        QImage image(200, 150, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        EXPECT_NO_FATAL_FAILURE(m_widget->paintEditingContent(painter));
        EXPECT_NO_FATAL_FAILURE(m_widget->undoDrawShapes());
    }
}

TEST_F(ShapesWidgetExtendedCovTest, resizeRotateAndKeySweep)
{
    const QStringList types = {
        QStringLiteral("rectangle"),
        QStringLiteral("oval"),
        QStringLiteral("arrow"),
        QStringLiteral("line"),
        QStringLiteral("pen"),
        QStringLiteral("effect"),
        QStringLiteral("text"),
        QStringLiteral("sequence")
    };
    for (const QString &type : types) {
        Toolshape shape = makeCovShape(type, QRectF(20, 20, 100, 80), 1);
        shape.points = {QPointF(20, 20), QPointF(120, 100)};
        access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {shape};
        access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_widget) = shape;
        access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_widget) = 0;
        access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_widget) = 0;
        access_private_field::ShapesWidgetCovAliasm_currentType(*m_widget) = type;

        for (int key = 0; key < 8; ++key) {
            EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliashandleResize(
                    *m_widget, QPointF(70 + key * 3, 60 + key * 2), key));
        }
        EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliashandleRotate(
                *m_widget, QPointF(110, 90)));

        const QList<QPair<int, Qt::KeyboardModifiers>> keys = {
            {Qt::Key_Left, Qt::NoModifier},
            {Qt::Key_Right, Qt::ControlModifier},
            {Qt::Key_Up, Qt::ShiftModifier},
            {Qt::Key_Down, Qt::ControlModifier | Qt::ShiftModifier},
            {Qt::Key_A, Qt::NoModifier},
            {Qt::Key_D, Qt::NoModifier},
            {Qt::Key_W, Qt::NoModifier},
            {Qt::Key_S, Qt::NoModifier}
        };
        for (const auto &entry : keys) {
            QKeyEvent event(QEvent::KeyPress, entry.first, entry.second);
            EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &event));
        }
    }
}

TEST_F(ShapesWidgetExtendedCovTest, selectionDecorationPaintSweep)
{
    m_widget->show();
    QApplication::processEvents();
    const QStringList types = {
        QStringLiteral("rectangle"),
        QStringLiteral("oval"),
        QStringLiteral("arrow"),
        QStringLiteral("line"),
        QStringLiteral("pen"),
        QStringLiteral("effect"),
        QStringLiteral("text"),
        QStringLiteral("sequence")
    };
    for (const QString &type : types) {
        Toolshape shape = makeCovShape(type, QRectF(30, 30, 110, 80), 1);
        shape.points = {QPointF(30, 30), QPointF(140, 110)};
        access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {shape};
        access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_widget) = shape;
        access_private_field::ShapesWidgetCovAliasm_hoveredShape(*m_widget) = shape;
        access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_widget) = 0;
        access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_widget) = 0;
        access_private_field::ShapesWidgetCovAliasm_currentType(*m_widget) = type;
        access_private_field::ShapesWidgetCovAliasm_isSelected(*m_widget) = true;
        access_private_field::ShapesWidgetCovAliasm_isResize(*m_widget) = true;
        access_private_field::ShapesWidgetCovAliasm_isRotated(*m_widget) = true;
        access_private_field::ShapesWidgetCovAliasm_isPressed(*m_widget) = true;

        for (int direction = 0; direction < 9; ++direction) {
            access_private_field::ShapesWidgetCovAliasm_resizeDirection(*m_widget) =
                    static_cast<ResizeDirection>(direction);
            QImage image(200, 150, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            EXPECT_NO_FATAL_FAILURE(m_widget->render(&image));
        }
    }
}

TEST_F(ShapesWidgetExtendedCovTest, primitivePaintVariants)
{
    QImage image(240, 180, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    const FourPoints rect = shapeRect(20, 20, 120, 80);
    const QList<QPointF> points = {QPointF(20, 20), QPointF(100, 60),
                                   QPointF(140, 100), QPointF(180, 140)};

    for (ShapesWidget::ShapeBlurStatus status : {ShapesWidget::Drawing,
                                                 ShapesWidget::Selected,
                                                 ShapesWidget::Hovered,
                                                 ShapesWidget::Normal}) {
        EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintRect(
                *m_widget, painter, rect, 0, status, true, false, 10));
        EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintRect(
                *m_widget, painter, rect, 0, status, false, true, 20));
        EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintEllipse(
                *m_widget, painter, rect, 0, status, true, false, 10));
        EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintEllipse(
                *m_widget, painter, rect, 0, status, false, true, 20));
    }

    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintArrow(
            *m_widget, painter, points.mid(0, 2), 3, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintArrow(
            *m_widget, painter, points.mid(0, 2), 1, false));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintLine(
            *m_widget, painter, points));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintEffectLine(
            *m_widget, painter, points, true, 10, 3));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintEffectLine(
            *m_widget, painter, points, false, 20, 1));
    EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliaseffectPixmap(
            *m_widget, true, 10));
    EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliaseffectPixmap(
            *m_widget, false, 20));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspaintText(
            *m_widget, painter, rect));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetPaintAliaspaintText(
            *m_widget, painter, rect, QStringLiteral("coverage"), 16));
}

TEST_F(ShapesWidgetExtendedCovTest, textEditAndGestureWorkflow)
{
    Toolshape textShape = makeCovShape(QStringLiteral("text"), QRectF(20, 20, 100, 60));
    textShape.index = 0;
    textShape.text = QStringLiteral("hello");
    access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {textShape};
    access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_widget) = textShape;
    access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_widget) = 0;
    access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_widget) = 0;
    access_private_field::ShapesWidgetCovAliasm_currentType(*m_widget) =
            QStringLiteral("text");

    auto *edit = new TextEdit(0, m_widget);
    edit->setPlainText(QStringLiteral("Input text here"));
    access_private_field::ShapesWidgetCovAliasm_editMap(*m_widget).insert(0, edit);
    access_private_field::ShapesWidgetCovAliasm_lastEditMapKey(*m_widget) = 0;
    access_private_field::ShapesWidgetCovAliasm_editing(*m_widget) = true;
    EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliastextEditIsReadOnly(
            *m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliassetAllTextEditReadOnly(
            *m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliassetNoChangedTextEditRemove(
            *m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliasupdateTextRect(
            *m_widget, edit, QRectF(30, 30, 80, 40)));

    QPinchGesture pinch;
    pinch.setCenterPoint(QPointF(70, 60));
    pinch.setLastCenterPoint(QPointF(65, 55));
    pinch.setRotationAngle(15.0);
    pinch.setLastRotationAngle(5.0);
    pinch.setScaleFactor(1.2);
    pinch.setLastScaleFactor(1.0);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliaspinchTriggered(
            *m_widget, &pinch));
    EXPECT_NO_FATAL_FAILURE(m_widget->setGlobalRect(QRect(0, 0, 200, 150)));
    EXPECT_NO_FATAL_FAILURE(m_widget->updateCursorShape());
    EXPECT_NO_FATAL_FAILURE(m_widget->menuSaveSlot());
    EXPECT_NO_FATAL_FAILURE(m_widget->menuCloseSlot());
}

TEST_F(ShapesWidgetExtendedCovTest, updateSelectedShapeBranchSweep)
{
    const QList<QPair<QString, int>> types = {
        {QStringLiteral("rectangle"), 0},
        {QStringLiteral("oval"), 1},
        {QStringLiteral("arrow"), 2},
        {QStringLiteral("line"), 3},
        {QStringLiteral("effect"), 5}
    };
    for (const auto &entry : types) {
        Toolshape shape = makeCovShape(entry.first, QRectF(20, 20, 100, 80), 1);
        access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {shape};
        access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_widget) = shape;
        access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_widget) = 0;
        access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_widget) = 0;
        access_private_field::ShapesWidgetCovAliasm_currentShape(*m_widget) = shape;

        for (const QString &key : {QStringLiteral("line_width"),
                                   QStringLiteral("color_index"),
                                   QStringLiteral("radius")}) {
            EXPECT_NO_FATAL_FAILURE(m_widget->updateSelectedShape(entry.first, key, entry.second));
        }
    }
}

TEST_F(ShapesWidgetExtendedCovTest, hitAndHoverHelperSweep)
{
    const FourPoints rect = shapeRect(20, 20, 120, 80);
    const QList<QPointF> points = {QPointF(20, 20), QPointF(140, 100)};
    const QList<QPointF> positions = {
        QPointF(20, 20), QPointF(80, 60), QPointF(140, 100),
        QPointF(1000, 1000), QPointF(20, 60), QPointF(80, 20)
    };
    for (const QPointF &position : positions) {
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliasclickedOnRect(
                *m_widget, rect, position, true));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliasclickedOnEllipse(
                *m_widget, rect, position, true));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliasclickedOnArrow(
                *m_widget, points, position));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliasclickedOnLine(
                *m_widget, rect, points, position));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliasclickedOnText(
                *m_widget, rect, position));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliashoverOnRect(
                *m_widget, rect, position, true));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliashoverOnEllipse(
                *m_widget, rect, position));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliashoverOnArrow(
                *m_widget, points, position));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliashoverOnLine(
                *m_widget, rect, points, position));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliashoverOnText(
                *m_widget, 0, rect, position));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliasrotateOnPoint(
                *m_widget, rect, position));
        EXPECT_NO_FATAL_FAILURE((void)call_private_fun::ShapesWidgetCovAliashoverOnRotatePoint(
                *m_widget, rect, position));
    }
}

TEST_F(ShapesWidgetExtendedCovTest, drawingInteractionLifecycleSweep)
{
    const QStringList types = {
        QStringLiteral("rectangle"),
        QStringLiteral("oval"),
        QStringLiteral("arrow"),
        QStringLiteral("line"),
        QStringLiteral("pen"),
        QStringLiteral("effect"),
        QStringLiteral("text"),
        QStringLiteral("sequence")
    };
    for (const QString &type : types) {
        access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget).clear();
        access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_widget) = Toolshape();
        access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_widget) = -1;
        access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_widget) = -1;
        access_private_field::ShapesWidgetCovAliasm_currentType(*m_widget) = type;
        access_private_field::ShapesWidgetCovAliasm_isRecording(*m_widget) = false;

        EXPECT_NO_FATAL_FAILURE(m_widget->beginInteraction(QPointF(20, 20)));
        EXPECT_NO_FATAL_FAILURE(m_widget->updateInteraction(QPointF(120, 90)));
        EXPECT_NO_FATAL_FAILURE(m_widget->endInteraction());
        EXPECT_NO_FATAL_FAILURE(m_widget->beginInteraction(QPointF(30, 30)));
        EXPECT_NO_FATAL_FAILURE(m_widget->cancelInteraction());

        ShapesWidget::InlineEditRequest request;
        EXPECT_NO_FATAL_FAILURE((void)m_widget->takePendingInlineEdit(&request));
        EXPECT_NO_FATAL_FAILURE((void)m_widget->beginInlineEditAt(QPointF(40, 40), &request));
        EXPECT_NO_FATAL_FAILURE(m_widget->updateInlineText(0, QStringLiteral("coverage"),
                                                           QRectF(20, 20, 80, 40)));
        EXPECT_NO_FATAL_FAILURE(m_widget->finishInlineText(0));
        EXPECT_NO_FATAL_FAILURE((void)m_widget->updateInlineSequence(0, 5));
        EXPECT_NO_FATAL_FAILURE(m_widget->setInlineEditingShape(0));
    }
}

TEST_F(ShapesWidgetExtendedCovTest, deterministicDrawingFuzz)
{
    QRandomGenerator random(0x3c91);
    m_widget->resize(400, 300);
    m_widget->show();
    QApplication::processEvents();
    const QStringList types = {
        QStringLiteral("rectangle"), QStringLiteral("oval"), QStringLiteral("arrow"),
        QStringLiteral("line"), QStringLiteral("pen"), QStringLiteral("effect"),
        QStringLiteral("text"), QStringLiteral("sequence")
    };
    const QList<int> keys = {
        Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down,
        Qt::Key_A, Qt::Key_D, Qt::Key_W, Qt::Key_S
    };

    for (int i = 0; i < 500; ++i) {
        m_widget->setCurrentShape(types.at(random.bounded(types.size())));
        if ((i % 5) == 0)
            m_widget->setInteractionPlatform((i % 10) == 0
                    ? ShapesWidget::InteractionPlatform::TreeLand
                    : ShapesWidget::InteractionPlatform::X11);

        const QPoint p1(random.bounded(350), random.bounded(250));
        const QPoint p2(random.bounded(350), random.bounded(250));
        const Qt::KeyboardModifiers modifiers =
                static_cast<Qt::KeyboardModifiers>(random.bounded(8));
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(p1),
                          Qt::LeftButton, Qt::LeftButton, modifiers);
        QMouseEvent move(QEvent::MouseMove, QPointF(p2),
                         Qt::LeftButton, Qt::LeftButton, modifiers);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(p2),
                            Qt::LeftButton, Qt::NoButton, modifiers);
        QKeyEvent key(QEvent::KeyPress, keys.at(random.bounded(keys.size())), modifiers);
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &press));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &move));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &release));
        EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &key));
        if ((i % 25) == 0) {
            QImage image(400, 300, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            EXPECT_NO_FATAL_FAILURE(m_widget->render(&image));
        }
    }
}

TEST_F(ShapesWidgetExtendedCovTest, sequenceLimitPreviewAndPainting)
{
    Toolshapes shapes;
    for (int i = 0; i < 100; ++i) {
        Toolshape shape = makeCovShape(QStringLiteral("sequence"),
                                       QRectF(10 + (i % 10) * 18,
                                              10 + (i / 10) * 18, 16, 16),
                                      i + 1);
        shapes.append(shape);
    }
    access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = shapes;
    access_private_field::ShapesWidgetCovAliasm_currentType(*m_widget) =
            QStringLiteral("sequence");
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetCovAliasupdateSequencePreview(
            *m_widget, QPointF(150, 120)));
    QWidget *preview = access_private_field::ShapesWidgetCovAliasm_sequencePreview(*m_widget);
    ASSERT_NE(preview, nullptr);
    preview->show();
    QImage previewImage(40, 40, QImage::Format_ARGB32_Premultiplied);
    previewImage.fill(Qt::transparent);
    EXPECT_NO_FATAL_FAILURE(preview->render(&previewImage));

    QImage image(200, 150, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    EXPECT_NO_FATAL_FAILURE(m_widget->paintContent(painter));
}

TEST_F(ShapesWidgetExtendedCovTest, textPropertyAndEffectSourceBranches)
{
    m_widget->setInteractionPlatform(ShapesWidget::InteractionPlatform::TreeLand);
    Toolshape textShape = makeCovShape(QStringLiteral("text"), QRectF(20, 20, 100, 60));
    textShape.index = 0;
    textShape.text = QStringLiteral("line1\nline2");
    textShape.fontSize = 12;
    access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {textShape};
    access_private_field::ShapesWidgetCovAliasm_selectedShape(*m_widget) = textShape;
    access_private_field::ShapesWidgetCovAliasm_selectedIndex(*m_widget) = 0;
    access_private_field::ShapesWidgetCovAliasm_selectedOrder(*m_widget) = 0;
    access_private_field::ShapesWidgetCovAliasm_currentType(*m_widget) =
            QStringLiteral("text");

    auto *edit = new TextEdit(0, m_widget);
    edit->setPlainText(QStringLiteral("line1\nline2"));
    access_private_field::ShapesWidgetCovAliasm_editMap(*m_widget).insert(0, edit);
    access_private_field::ShapesWidgetCovAliasm_lastEditMapKey(*m_widget) = 0;
    EXPECT_NO_FATAL_FAILURE(m_widget->updateSelectedShape(QStringLiteral("text"),
                                                          QStringLiteral("color_index"), 3));
    EXPECT_NO_FATAL_FAILURE(m_widget->updateSelectedShape(QStringLiteral("text"),
                                                          QStringLiteral("fontsize"), 20));

    QImage source(100, 80, QImage::Format_ARGB32_Premultiplied);
    source.fill(QColor(100, 120, 140, 255));
    access_private_field::ShapesWidgetCovAliasm_effectSourceImage(*m_widget) = source;

    Toolshape effectShape = makeCovShape(QStringLiteral("effect"), QRectF(20, 20, 100, 80));
    effectShape.isBlur = true;
    effectShape.isOval = 0;
    effectShape.radius = 10;
    access_private_field::ShapesWidgetCovAliasm_shapes(*m_widget) = {effectShape};
    QImage image(160, 120, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    EXPECT_NO_FATAL_FAILURE(m_widget->paintContent(painter));
}
