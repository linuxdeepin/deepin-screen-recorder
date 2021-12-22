#ifndef UT_PIN_SCREENSHOTS_H
#define UT_PIN_SCREENSHOTS_H
#include <gtest/gtest.h>


class PinScreenShotsTest: public testing::Test
{

public:
    virtual void SetUp() override
    {
        std::cout << "start PinScreenShotsTest" << std::endl;

    }

    virtual void TearDown() override
    {
        std::cout << "end PinScreenShotsTest" << std::endl;
    }
};

#endif // UT_PIN_SCREENSHOTS_H
