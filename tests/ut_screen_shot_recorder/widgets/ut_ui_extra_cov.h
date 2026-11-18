// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>

#include <QApplication>
#include <QAbstractButton>
#include <QImage>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

#include "addr_pri.h"
#include "stub.h"
#include "ut_toolbarhost_stub.h"
#include "../../src/main_window.h"
#include "../../src/widgets/fineadjusttips.h"
#include "../../src/widgets/roundedcornerwidget.h"
#include "../../src/widgets/sidebar.h"
#include "../../src/widgets/subtoolwidget.h"
#include "../../src/widgets/toolbar.h"
#include "../../src/widgets/toolbarhost.h"

namespace {

void noop0() {}
void noopBool(bool) {}
void noopInt(int) {}
void noopString(const QString &) {}
void noopPointPair(QPoint, QPoint) {}

} // namespace

class MainWindowToolBarHostCovTest : public Test
{
public:
    void SetUp() override
    {
        m_stub.set(ADDR(MainWindow, confirm), noop0);
        m_stub.set(ADDR(MainWindow, exitApp), noop0);
        m_stub.set(ADDR(MainWindow, saveScreenShotToFile), noop0);
        m_stub.set(ADDR(MainWindow, unDo), noop0);
        m_stub.set(ADDR(MainWindow, isInUndoBtn), noopBool);
        m_stub.set(ADDR(MainWindow, changeShotToolEvent), noopString);
        m_stub.set(ADDR(MainWindow, handleOptionMenuShown), noop0);
        m_stub.set(ADDR(MainWindow, changeKeyBoardShowEvent), noopBool);
        m_stub.set(ADDR(MainWindow, changeMouseShowEvent), noopBool);
        m_stub.set(ADDR(MainWindow, changeCameraSelectEvent), noopBool);
        m_stub.set(ADDR(MainWindow, onAiAssistantSelected), noopInt);
        m_stub.set(ADDR(MainWindow, getToolBarPoint), noop0);
        m_stub.set(ADDR(MainWindow, onTreelandSwitchToShotUI), noop0);
        m_stub.set(ADDR(MainWindow, onTreelandSwitchToRecordUI), noop0);
        m_stub.set(ADDR(MainWindow, getSideBarStartPressPoint), noop0);
        m_stub.set(ADDR(MainWindow, getToolBarStartPressPoint), noop0);
        m_stub.set(ADDR(MainWindow, moveToolBars), noopPointPair);
        m_host = new MainWindowToolBarHost(&m_window, &m_window);
    }

    void TearDown() override { delete m_host; }

    Stub m_stub;
    MainWindow m_window;
    MainWindowToolBarHost *m_host = nullptr;
};

TEST_F(MainWindowToolBarHostCovTest, delegatesAllActions)
{
    EXPECT_EQ(m_host->anchorWidget(), &m_window);
    EXPECT_NO_FATAL_FAILURE(m_host->confirm());
    EXPECT_NO_FATAL_FAILURE(m_host->close(false));
    EXPECT_NO_FATAL_FAILURE(m_host->close(true));
    EXPECT_NO_FATAL_FAILURE(m_host->save());
    EXPECT_NO_FATAL_FAILURE(m_host->undo());
    EXPECT_NO_FATAL_FAILURE(m_host->undoHover(true));
    EXPECT_NO_FATAL_FAILURE(m_host->toolChanged(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_host->optionMenuShown());
    EXPECT_NO_FATAL_FAILURE(m_host->keyboardToggled(true));
    EXPECT_NO_FATAL_FAILURE(m_host->mouseToggled(false));
    EXPECT_NO_FATAL_FAILURE(m_host->cameraToggled(true));
    EXPECT_NO_FATAL_FAILURE(m_host->aiAssistantSelected(2));
    EXPECT_NO_FATAL_FAILURE(m_host->toolBarPointRequested());
    EXPECT_NO_FATAL_FAILURE(m_host->switchToShotUI());
    EXPECT_NO_FATAL_FAILURE(m_host->switchToRecordUI());
    EXPECT_NO_FATAL_FAILURE(m_host->sideBarStartPressPoint());
    EXPECT_NO_FATAL_FAILURE(m_host->toolBarStartPressPoint());
    EXPECT_TRUE(m_host->moveToolBars(QPoint(1, 2), QPoint(3, 4)));
}

class RoundedCornerWidgetExtraCovTest : public Test
{
public:
    void SetUp() override
    {
        m_widget = new RoundedCornerWidget;
        m_widget->resize(360, 160);
        m_widget->show();
        QApplication::processEvents();
    }

    void TearDown() override { delete m_widget; }

    RoundedCornerWidget *m_widget = nullptr;
};

TEST_F(RoundedCornerWidgetExtraCovTest, publicStateAndPainting)
{
    m_widget->setRadius(18);
    EXPECT_EQ(m_widget->radius(), 18);
    EXPECT_TRUE(m_widget->isCornerEnabled());
    m_widget->setRadius(0);
    EXPECT_FALSE(m_widget->isCornerEnabled());
    EXPECT_FALSE(m_widget->isRatioLocked());
    EXPECT_DOUBLE_EQ(m_widget->lockedRatio(), 0.0);
    EXPECT_EQ(m_widget->currentPanel(), RoundedCornerWidget::NoPanel);

    m_widget->followSelection(QRect(120, 100, 500, 300), QRect(0, 0, 1000, 700));
    EXPECT_FALSE(m_widget->groupPlateRect(QRect(120, 100, 500, 300),
                                         QRect(0, 0, 1000, 700)).isEmpty());
    EXPECT_FALSE(m_widget->currentGroupPlateRect().isEmpty());
    m_widget->setSelectionSize(500, 300);
    EXPECT_NO_FATAL_FAILURE(m_widget->cursorShapeAt(QPoint(10, 10)));
    m_widget->collapsePanels();

    QImage rendered(m_widget->size(), QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::transparent);
    m_widget->render(&rendered);

    QPixmap source(40, 30);
    source.fill(QColor(100, 120, 140, 128));
    EXPECT_FALSE(RoundedCornerWidget::applyRoundedCorners(source, 8).isNull());
    EXPECT_FALSE(RoundedCornerWidget::compositeOnWhite(source).isNull());
}

TEST_F(RoundedCornerWidgetExtraCovTest, dragAndEventFilters)
{
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(30, 30),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent move(QEvent::MouseMove, QPointF(60, 50),
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(60, 50),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &press));
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &move));
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &release));
    QEvent leave(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_widget, &leave));
}

TEST_F(RoundedCornerWidgetExtraCovTest, privateStateAndPersistencePaths)
{
    access_private_field::RoundedCornerWidgetCovAliasm_selectionWidth(*m_widget) = 640;
    access_private_field::RoundedCornerWidgetCovAliasm_selectionHeight(*m_widget) = 480;
    access_private_field::RoundedCornerWidgetCovAliasm_ratioWidth(*m_widget) = 4;
    access_private_field::RoundedCornerWidgetCovAliasm_ratioHeight(*m_widget) = 3;
    access_private_field::RoundedCornerWidgetCovAliasm_ratioLocked(*m_widget) = true;
    access_private_field::RoundedCornerWidgetCovAliasm_rememberSetting(*m_widget) = true;
    access_private_field::RoundedCornerWidgetCovAliasm_pendingRememberedSetting(*m_widget) = true;
    access_private_field::RoundedCornerWidgetCovAliasm_rememberedIsRatio(*m_widget) = true;
    access_private_field::RoundedCornerWidgetCovAliasm_fixedWidth(*m_widget) = 800;
    access_private_field::RoundedCornerWidgetCovAliasm_fixedHeight(*m_widget) = 600;

    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliassyncRatioFromSelection(
            *m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliascaptureCurrentSetting(
            *m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliaspersistRememberSetting(
            *m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliaspersistFixedSize(
            *m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliaspersistRatioType(
            *m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliasupdateLayout(*m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliasreposition(*m_widget));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliasapplySizeMode(
            *m_widget, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliasapplySizeMode(
            *m_widget, false));
    EXPECT_FALSE(call_private_fun::RoundedCornerWidgetCovAliasrememberedSetting(
                         *m_widget).isEmpty());
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RoundedCornerWidgetCovAliasconsumeRememberedSetting(
            *m_widget));
    EXPECT_FALSE(m_widget->isRememberedSettingRatio());

    QLineEdit lineEdit(m_widget);
    QPushButton button(m_widget);
    EXPECT_TRUE(call_private_fun::RoundedCornerWidgetCovAliasisTextInputWidget(
            *m_widget, &lineEdit));
    EXPECT_FALSE(call_private_fun::RoundedCornerWidgetCovAliasisTextInputWidget(
            *m_widget, &button));
}

TEST_F(RoundedCornerWidgetExtraCovTest, childControlInteractions)
{
    m_widget->show();
    QApplication::processEvents();

    const auto buttons = m_widget->findChildren<QAbstractButton *>();
    for (QAbstractButton *button : buttons) {
        ASSERT_NE(button, nullptr);
        EXPECT_NO_FATAL_FAILURE(button->click());
        QApplication::processEvents();
    }

    const auto lineEdits = m_widget->findChildren<QLineEdit *>();
    for (QLineEdit *edit : lineEdits) {
        EXPECT_NO_FATAL_FAILURE(edit->setText(QStringLiteral("640")));
    }
    const auto spinBoxes = m_widget->findChildren<QSpinBox *>();
    for (QSpinBox *spinBox : spinBoxes) {
        EXPECT_NO_FATAL_FAILURE(spinBox->setValue(spinBox->maximum()));
        EXPECT_NO_FATAL_FAILURE(spinBox->setValue(spinBox->minimum()));
    }
    const auto sliders = m_widget->findChildren<QSlider *>();
    for (QSlider *slider : sliders) {
        EXPECT_NO_FATAL_FAILURE(slider->setValue(slider->maximum()));
        EXPECT_NO_FATAL_FAILURE(slider->setValue(slider->minimum()));
    }
    QApplication::processEvents();
}

class FineAdjustTipsExtraCovTest : public Test
{
public:
    void SetUp() override
    {
        m_parent = new QWidget;
        m_parent->resize(800, 600);
        m_tips = new FineAdjustTips(m_parent);
        m_parent->show();
    }

    void TearDown() override { delete m_parent; }

    QWidget *m_parent = nullptr;
    FineAdjustTips *m_tips = nullptr;
};

TEST_F(FineAdjustTipsExtraCovTest, modesSizingAndPainting)
{
    m_tips->setMode(FineAdjustTips::CursorMode);
    EXPECT_EQ(m_tips->mode(), FineAdjustTips::CursorMode);
    m_tips->setResizeEnabled(true);
    EXPECT_TRUE(m_tips->resizeEnabled());
    m_tips->dockToBottomLeft();

    m_tips->setMode(FineAdjustTips::SelectionMode);
    EXPECT_EQ(m_tips->mode(), FineAdjustTips::SelectionMode);
    m_tips->setResizeEnabled(false);
    EXPECT_FALSE(m_tips->resizeEnabled());
    m_tips->dockToBottomLeft();
    m_tips->show();
    QApplication::processEvents();

    QImage image(m_tips->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    m_tips->render(&image);
}

class ToolWidgetsExtraCovTest : public Test
{
public:
    void SetUp() override
    {
        m_sub = new SubToolWidget(&m_host);
        m_toolBar = new ToolBar;
        m_toolBar->initToolBar(&m_host);
        m_sideBar = new SideBar;
        m_sideBar->initSideBar(&m_host);
    }

    void TearDown() override
    {
        delete m_sub;
        delete m_toolBar;
        delete m_sideBar;
    }

    TestToolBarHost m_host;
    SubToolWidget *m_sub = nullptr;
    ToolBar *m_toolBar = nullptr;
    SideBar *m_sideBar = nullptr;
};

TEST_F(ToolWidgetsExtraCovTest, settersAndStateTransitions)
{
    EXPECT_NO_FATAL_FAILURE(m_sub->setRecordingActive(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setRecordingActive(false));
    EXPECT_NO_FATAL_FAILURE(m_sub->setTableScreenshotEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setTableScreenshotEnable(false));
    EXPECT_NO_FATAL_FAILURE(m_sub->clearShotToolSelection());
    EXPECT_NO_FATAL_FAILURE(m_sub->setScrollShotDisabled(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setPinScreenshotsEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setOcrScreenshotEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setButEnableOnLockScreen(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setMicroPhoneEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_sub->setCameraDeviceEnable(true));

    EXPECT_NO_FATAL_FAILURE(m_toolBar->setRecordingActive(true));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->setRecordingActive(false));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->setScrollActive(true));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->setScrollActive(false));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->setTableScreenshotsEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->setTableScreenshotsEnable(false));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->setScrollShotDisabled(true));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->setPinScreenshotsEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->setOcrScreenshotsEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->setButEnableOnLockScreen(true));
    EXPECT_NO_FATAL_FAILURE(m_toolBar->clearShotToolSelection());
    EXPECT_NO_FATAL_FAILURE(m_toolBar->showWidget());
    EXPECT_NO_FATAL_FAILURE(m_toolBar->hideWidget());
    QEvent leave(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_toolBar, &leave));

    m_sideBar->showWidget();
    m_sideBar->hideWidget();
    QImage sidebarImage(100, 100, QImage::Format_ARGB32_Premultiplied);
    sidebarImage.fill(Qt::transparent);
    m_sideBar->render(&sidebarImage);
    QEvent sidebarLeave(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(m_sideBar, &sidebarLeave));
}
