#pragma once
#include <gtest/gtest.h>
#include "../utils/desktopinfo.h"


using namespace testing;

class DesktopInfoTest:public testing::Test{

public:
    DesktopInfo deskInfo;
    virtual void SetUp() override{
        std::cout << "start DesktopInfoTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DesktopInfoTest" << std::endl;
    }
};

TEST_F(DesktopInfoTest, waylandDectected)
{
    EXPECT_FALSE(deskInfo.waylandDectected());
}

TEST_F(DesktopInfoTest, windowManager)
{
    EXPECT_NE(DesktopInfo::OTHER, deskInfo.windowManager());
}
