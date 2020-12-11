#pragma once
#include <gtest/gtest.h>
#include <QTest>

#define private public
#define protected public
#include "../../src/widgets/filter.h"

using namespace testing;
class HintFilterTest:public testing::Test{

public:
    HintFilter *m_hintFilter;
    virtual void SetUp() override{
        m_hintFilter = new HintFilter();
    }

    virtual void TearDown() override{
        if(nullptr != m_hintFilter)
            delete m_hintFilter;
    }
};

TEST_F(HintFilterTest, eventFilter)
{
    QWidget *widget = new QWidget();
    QEvent *enterEvent = new QEvent(QEvent::Enter);
    m_hintFilter->eventFilter(widget,enterEvent);
    QEvent *enterLeave = new QEvent(QEvent::Leave);
    m_hintFilter->eventFilter(widget,enterLeave);
    QEvent *enterMove = new QEvent(QEvent::Move);
    m_hintFilter->eventFilter(widget,enterMove);
}


