#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/widgets/subtoolwidget.h"

using namespace testing;
class SubToolWidgetTest:public testing::Test{

public:
    SubToolWidget *m_subToolWidget;
    virtual void SetUp() override{
        m_subToolWidget = new SubToolWidget();
        m_subToolWidget->initWidget();
        m_subToolWidget->initRecordLabel();
        m_subToolWidget->initShotLabel();
    }

    virtual void TearDown() override{
        if(nullptr != m_subToolWidget)
            delete m_subToolWidget;
    }
};

TEST_F(SubToolWidgetTest, installTipHint)
{
    QWidget *w1 = new QWidget();
    m_subToolWidget->installTipHint(w1,QString("ut"));

    QWidget *w2 = new QWidget(w1);
    QWidget *w3 = new QWidget(w2);
    m_subToolWidget->installTipHint(w3,QString("ut"));
}


TEST_F(SubToolWidgetTest, installHint)
{
    QWidget *w = new QWidget();
    QWidget *hint = new QWidget();
    m_subToolWidget->installHint(w,hint);
}

TEST_F(SubToolWidgetTest, switchContent)
{
    m_subToolWidget->switchContent(QString("record"));
    m_subToolWidget->switchContent(QString("shot"));
}

TEST_F(SubToolWidgetTest, changeArrowAndLineFromSideBar)
{
    m_subToolWidget->changeArrowAndLineFromSideBar(0);
    m_subToolWidget->changeArrowAndLineFromSideBar(1);
}

TEST_F(SubToolWidgetTest, setRecordLaunchMode)
{
    m_subToolWidget->setRecordLaunchMode(true);
}

TEST_F(SubToolWidgetTest, setVideoButtonInitFromSub)
{
    m_subToolWidget->setVideoButtonInitFromSub();
}

TEST_F(SubToolWidgetTest, shapeClickedFromWidget)
{
    m_subToolWidget->shapeClickedFromWidget(QString("rect"));
    m_subToolWidget->shapeClickedFromWidget(QString("circ"));
    m_subToolWidget->shapeClickedFromWidget(QString("line"));
    m_subToolWidget->shapeClickedFromWidget(QString("pen"));
    m_subToolWidget->shapeClickedFromWidget(QString("text"));
    m_subToolWidget->shapeClickedFromWidget(QString("option"));
}

TEST_F(SubToolWidgetTest, setMicroPhoneEnable)
{
    m_subToolWidget->setMicroPhoneEnable(false);
    m_subToolWidget->setMicroPhoneEnable(true);
}

TEST_F(SubToolWidgetTest, setCameraDeviceEnable)
{
    m_subToolWidget->setCameraDeviceEnable(false);
    m_subToolWidget->setCameraDeviceEnable(true);
}

TEST_F(SubToolWidgetTest, setSystemAudioEnable)
{
    m_subToolWidget->setSystemAudioEnable(false);
    m_subToolWidget->setSystemAudioEnable(true);
}

//TEST_F(SubToolWidgetTest, installHint)
//{
//    QWidget *w = new QWidget();
//    QWidget *hint = new QWidget();
//    m_subToolWidget->installHint(w,hint);
//}

//TEST_F(SubToolWidgetTest, switchContent)
//{
//    m_subToolWidget->switchContent(QString("record"));
//    m_subToolWidget->switchContent(QString("shot"));
//}




