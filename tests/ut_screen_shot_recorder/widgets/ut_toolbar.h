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
//ACCESS_PRIVATE_FUN(ToolBarWidget, void(QPaintEvent *), paintEvent);
//ACCESS_PRIVATE_FUN(ToolBarWidget, void(QShowEvent *), showEvent);
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


class ToolBarTest:public testing::Test{

public:
    Stub stub;
    ToolBar *m_toolBar;
    MainWindow *m_mainWindow;
    virtual void SetUp() override{
        m_mainWindow = new MainWindow;
        m_toolBar = new ToolBar(m_mainWindow);
    }

    virtual void TearDown() override{
        if(nullptr != m_toolBar)
            m_toolBar->deleteLater();
        if(nullptr != m_mainWindow)
            m_mainWindow->deleteLater();
    }
};

//TEST_F(ToolBarTest, paintEvent)
//{
//    QPaintEvent *paintEvent = new QPaintEvent(QRect());
//    call_private_fun::ToolBarWidgetpaintEvent(*m_toolBarWidget,paintEvent);
//}

//TEST_F(ToolBarTest, showEvent)
//{
//    QShowEvent *showEvent = new QShowEvent();
//    call_private_fun::ToolBarWidgetshowEvent(*m_toolBarWidget,showEvent);
//}

TEST_F(ToolBarTest, setExpand)
{
    m_toolBar->setExpand(true,QString("rect"));
}

TEST_F(ToolBarTest, showAt)
{
    m_toolBar->showAt(QPoint(380,268));
}

TEST_F(ToolBarTest, currentFunctionMode)
{
    m_toolBar->currentFunctionMode(QString("rect"));
}

TEST_F(ToolBarTest, keyBoardCheckedToMainSlot)
{
    m_toolBar->keyBoardCheckedToMainSlot(true);
}

TEST_F(ToolBarTest, microphoneActionCheckedToMainSlot)
{
    m_toolBar->microphoneActionCheckedToMainSlot(true);
}

TEST_F(ToolBarTest, systemAudioActionCheckedToMainSlot)
{
    m_toolBar->systemAudioActionCheckedToMainSlot(true);
}

//TEST_F(ToolBarTest, changeArrowAndLineFromMain)
//{
//    m_toolBar->changeArrowAndLineFromMain(1);
//}

TEST_F(ToolBarTest, initToolBar)
{
    m_toolBar->initToolBar();
}

//TEST_F(ToolBarTest, setRecordButtonDisable)
//{
//    m_toolBar->setRecordButtonDisable();
//}

//TEST_F(ToolBarTest, setRecordLaunchMode)
//{
//    m_toolBar->setRecordLaunchMode(true);
//}

//TEST_F(ToolBarTest, setVideoButtonInit)
//{
//    m_toolBar->setVideoButtonInit();
//}

TEST_F(ToolBarTest, shapeClickedFromMain)
{
    m_toolBar->shapeClickedFromMain(QString("rect"));
}

//TEST_F(ToolBarTest, setMicroPhoneEnable)
//{
//    m_toolBar->setMicroPhoneEnable(true);
//}

//TEST_F(ToolBarTest, setSystemAudioEnable)
//{
//    m_toolBar->setSystemAudioEnable(true);
//}

//TEST_F(ToolBarTest, setCameraDeviceEnable)
//{
//    m_toolBar->setCameraDeviceEnable(true);
//}






