#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPainter>
#include  <QFont>
#include "../../src/utils.h"


using namespace testing;

class UtilsTest:public testing::Test{

public:
    Utils *utils;
    virtual void SetUp() override{
        std::cout << "start UtilsTest" << std::endl;
        utils = new Utils;
    }

    virtual void TearDown() override{
        delete utils;
        std::cout << "end UtilsTest" << std::endl;
    }
};
TEST_F(UtilsTest, setFontSize)
{

    QPainter painter;
    QFont font = painter.font() ;
    int start = font.pointSize();
    qDebug() << start;
    int setsize = 24;
    Utils::setFontSize(painter, setsize);
    //EXPECT_EQ(painter.font().pointSize(), setsize);
}
