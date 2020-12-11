#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>

#define private public
#define protected public
#include "../../src/widgets/shottoolwidget.h"

using namespace testing;
class ShotToolWidgetTest:public testing::Test{

public:
    ShotToolWidget *m_shotToolWidget;
    QString m_tips = QString("tips");
    virtual void SetUp() override{
        m_shotToolWidget = new ShotToolWidget();
    }

    virtual void TearDown() override{
        if(nullptr != m_shotToolWidget)
            delete m_shotToolWidget;
    }
};

TEST_F(ShotToolWidgetTest, initWidget)
{
    m_shotToolWidget->initWidget();
}

TEST_F(ShotToolWidgetTest, initRectLabel)
{
    m_shotToolWidget->initRectLabel();
}

TEST_F(ShotToolWidgetTest, initCircLabel)
{
    m_shotToolWidget->initCircLabel();
}

TEST_F(ShotToolWidgetTest, initLineLabel)
{
    m_shotToolWidget->initLineLabel();
}

TEST_F(ShotToolWidgetTest, initPenLabel)
{
    m_shotToolWidget->initPenLabel();
}

TEST_F(ShotToolWidgetTest, initTextLabel)
{
    m_shotToolWidget->initTextLabel();
}



