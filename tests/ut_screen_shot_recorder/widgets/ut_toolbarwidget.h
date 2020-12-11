#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include "stub.h"
#include "addr_pri.h"
#include <DApplicationHelper>

#include "../../src/main_window.h"
#include "../../src/widgets/toolbar.h"

using namespace testing;
//ACCESS_PRIVATE_FUN(ToolTips, void(QResizeEvent *ev), resizeEvent);
//ACCESS_PRIVATE_FUN(ToolButton, void(QEvent *e), enterEvent);
//ACCESS_PRIVATE_FUN(ToolButton, void(QEvent *e), leaveEvent);
//ACCESS_PRIVATE_FUN(ToolButton, void(QMouseEvent *e), mousePressEvent);
ACCESS_PRIVATE_FUN(ToolBarWidget, void(QPaintEvent *), paintEvent);
ACCESS_PRIVATE_FUN(ToolBarWidget, void(QShowEvent *), showEvent);
//DGuiApplicationHelper::ColorType themeType_default_stub()
//{
//    return DGuiApplicationHelper::ColorType::UnknownType;
//}
//DGuiApplicationHelper::ColorType themeType_Light_stub()
//{
//    return DGuiApplicationHelper::ColorType::LightType;
//}
//DGuiApplicationHelper::ColorType themeType_Dark_stub()
//{
//    return DGuiApplicationHelper::ColorType::DarkType;
//}

//void paintEvent_stub(QPaintEvent* e)
//{
//    Q_UNUSED(e);
//};

class ToolBarWidgetTest:public testing::Test{

public:
    Stub stub;
    ToolBar *m_toolBar;
    MainWindow *m_mainWindow;
    ToolBarWidget *m_toolBarWidget;
    virtual void SetUp() override{
        m_mainWindow = new MainWindow;
        m_toolBar = new ToolBar(m_mainWindow);
        m_toolBarWidget = new ToolBarWidget(m_toolBar);
    }

    virtual void TearDown() override{
        if(nullptr != m_toolBarWidget)
            m_toolBarWidget->deleteLater();
        if(nullptr != m_toolBar)
            m_toolBar->deleteLater();
        if(nullptr != m_mainWindow)
            m_mainWindow->deleteLater();
    }
};

//TEST_F(ToolBarWidgetTest, paintEvent)
//{
//    QPaintEvent *paintEvent = new QPaintEvent(QRect());
//    call_private_fun::ToolBarWidgetpaintEvent(*m_toolBarWidget,paintEvent);
//}

TEST_F(ToolBarWidgetTest, showEvent)
{
    QShowEvent *showEvent = new QShowEvent();
    call_private_fun::ToolBarWidgetshowEvent(*m_toolBarWidget,showEvent);
}

TEST_F(ToolBarWidgetTest, setExpand)
{
    m_toolBarWidget->setExpand(true,QString("record"));
}

TEST_F(ToolBarWidgetTest, keyBoardCheckedSlot)
{
    m_toolBarWidget->keyBoardCheckedSlot(true);
}

TEST_F(ToolBarWidgetTest, changeArrowAndLineFromBar)
{
    m_toolBarWidget->changeArrowAndLineFromBar(1);
}

TEST_F(ToolBarWidgetTest, setRecordButtonDisableFromMain)
{
    m_toolBarWidget->setRecordButtonDisableFromMain();
}

TEST_F(ToolBarWidgetTest, shapeClickedFromBar)
{
    m_toolBarWidget->shapeClickedFromBar(QString("circ"));
}

TEST_F(ToolBarWidgetTest, setRecordLaunchFromMain)
{
    m_toolBarWidget->setRecordLaunchFromMain(true);
}

TEST_F(ToolBarWidgetTest, setVideoInitFromMain)
{
    m_toolBarWidget->setVideoInitFromMain();
}

TEST_F(ToolBarWidgetTest, setMicroPhoneEnable)
{
    m_toolBarWidget->setMicroPhoneEnable(true);
}

TEST_F(ToolBarWidgetTest, setSystemAudioEnable)
{
    m_toolBarWidget->setSystemAudioEnable(true);
}

TEST_F(ToolBarWidgetTest, setCameraDeviceEnable)
{
    m_toolBarWidget->setCameraDeviceEnable(true);
}






