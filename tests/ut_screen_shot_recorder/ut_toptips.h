#pragma once
#include <gtest/gtest.h>
#include <QTest>

#define private public
#define protected public
#include "../../src/widgets/toptips.h"

using namespace testing;
class TopTipsTest:public testing::Test{

public:
    TopTips *m_topTips;
    virtual void SetUp() override{
        m_topTips = new TopTips();
    }

    virtual void TearDown() override{
        if(nullptr != m_topTips)
            delete m_topTips;
    }
};

TEST_F(TopTipsTest, setRecorderTipsInfo)
{
    m_topTips->setRecorderTipsInfo(true);
}

TEST_F(TopTipsTest, setContent)
{
    m_topTips->setContent(QSize(100,20));
}

TEST_F(TopTipsTest, updateTips)
{
    m_topTips->updateTips(QPoint(10,10),QSize(100,30));
}
