#pragma once
#include <QtTest>
#include <QColor>
#include <QDebug>
#include <gtest/gtest.h>
#include "../widgets/zoomIndicator.h"


using namespace testing;

class ZoomIndicatorTest:public testing::Test, public QObject{

public:
    ZoomIndicator *zoomindicator;
    virtual void SetUp() override{
        std::cout << "start ZoomIndicatorTest" << std::endl;
        zoomindicator = new ZoomIndicator(nullptr);

    }

    virtual void TearDown() override{
        delete zoomindicator;
        std::cout << "end ZoomIndicatorTest" << std::endl;
    }
};
TEST_F(ZoomIndicatorTest, ButtonClicked)
{
    zoomindicator->showMagnifier(QPoint(100, 100));
}
