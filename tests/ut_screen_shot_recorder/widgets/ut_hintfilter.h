#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include "stub.h"
#include "addr_pri.h"
#define private public
#define protected public
#include "../../src/widgets/filter.h"

using namespace testing;
bool toBool_stub_true()
{
    return true;
}
bool toBool_stub_false()
{
    return false;
}
class HintFilterTest:public testing::Test{

public:
    Stub stub;
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

    stub.set(ADDR(QVariant,toBool),toBool_stub_true);
    QEvent *enterEvent1 = new QEvent(QEvent::Enter);
    m_hintFilter->eventFilter(widget,enterEvent1);
    stub.reset(ADDR(QVariant,toBool));
    QEvent *enterEvent2 = new QEvent(QEvent::Enter);
    m_hintFilter->eventFilter(widget,enterEvent2);
    stub.set(ADDR(QVariant,toBool),toBool_stub_false);
    stub.reset(ADDR(QVariant,toBool));

}


