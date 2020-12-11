#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>

#define private public
#define protected public
#include "../../src/widgets/subtoolwidget.h"

using namespace testing;
class SubToolWidgetTest:public testing::Test{

public:
    SubToolWidget *m_subToolWidget;
    virtual void SetUp() override{
        m_subToolWidget = new SubToolWidget();
    }

    virtual void TearDown() override{
        if(nullptr != m_subToolWidget)
            delete m_subToolWidget;
    }
};






