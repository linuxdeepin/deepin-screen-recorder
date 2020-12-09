#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>

#define private public
#define protected public
#include "../../src/widgets/tooltips.h"

using namespace testing;
class ToolTipsTest:public testing::Test{

public:
    ToolTips *m_toolTips;
    QString m_tips = QString("tips");
    virtual void SetUp() override{
        m_toolTips = new ToolTips(m_tips);
    }

    virtual void TearDown() override{
        if(nullptr != m_toolTips)
            delete m_toolTips;
    }
};

TEST_F(ToolTipsTest, radius)
{
    m_toolTips->radius();
}

TEST_F(ToolTipsTest, borderColor)
{
    m_toolTips->borderColor();
}

TEST_F(ToolTipsTest, background)
{
    m_toolTips->background();
}

TEST_F(ToolTipsTest, setText)
{
    m_toolTips->setText(QString("tips"));
}

TEST_F(ToolTipsTest, setRadius)
{
    m_toolTips->setRadius(0.3);
}

TEST_F(ToolTipsTest, setVertical)
{
    m_toolTips->setVertical();
}

TEST_F(ToolTipsTest, resetSize)
{
    m_toolTips->resetSize(2);
}




