#pragma once
#include <gtest/gtest.h>
#include "../utils/shortcut.h"


using namespace testing;

class ShortcutTest:public testing::Test{

public:
    Shortcut shortcut;
    virtual void SetUp() override{
        std::cout << "start ShortcutTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end ShortcutTest" << std::endl;
    }
};

TEST_F(ShortcutTest, toStr)
{
    EXPECT_FALSE(shortcut.toStr().isEmpty());
}

