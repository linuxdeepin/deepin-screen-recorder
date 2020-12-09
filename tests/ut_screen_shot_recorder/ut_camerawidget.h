#pragma once
#include <gtest/gtest.h>
#include <QTest>

#define private public
#define protected public
#include "../../src/widgets/camerawidget.h"

using namespace testing;
class CameraWidgetTest:public testing::Test{

public:
    CameraWidget *m_cameraWidget;
    virtual void SetUp() override{
        m_cameraWidget = new CameraWidget();
    }

    virtual void TearDown() override{
        if(nullptr != m_cameraWidget)
            delete m_cameraWidget;
    }
};

TEST_F(CameraWidgetTest, setRecordRect)
{
    m_cameraWidget->setRecordRect(10,10,100,100);
}

TEST_F(CameraWidgetTest, showAt)
{
    m_cameraWidget->showAt(QPoint(10,10));
}


TEST_F(CameraWidgetTest, initCamera)
{
    m_cameraWidget->initCamera();
}

TEST_F(CameraWidgetTest, postion)
{
    m_cameraWidget->postion();
}

