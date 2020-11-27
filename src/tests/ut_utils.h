#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPainter>
#include  <QFont>
#include "../utils.h"


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

TEST_F(UtilsTest, getRenderSize)
{


}
TEST_F(UtilsTest, getQrcPath)
{


}
TEST_F(UtilsTest, drawTooltipBackground)
{


}
TEST_F(UtilsTest, drawTooltipText)
{


}
TEST_F(UtilsTest, passInputEvent)
{


}
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
TEST_F(UtilsTest, warnNoComposite)
{
    Utils::warnNoComposite();
}
TEST_F(UtilsTest, getRecordingSaveDirectory)
{
    QString subDir = Utils::getRecordingSaveDirectory();
}
