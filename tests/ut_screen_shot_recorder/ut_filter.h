#pragma once
#include <gtest/gtest.h>
#include <QTest>

#define private public
#define protected public
#include "../../src/widgets/filter.h"

using namespace testing;
class HoverShadowFilterTest:public testing::Test{

public:
    HoverShadowFilter *m_hoverShadowFilter;
    virtual void SetUp() override{
        m_hoverShadowFilter = new HoverShadowFilter();
    }

    virtual void TearDown() override{
        if(nullptr != m_hoverShadowFilter)
            delete m_hoverShadowFilter;
    }
};




