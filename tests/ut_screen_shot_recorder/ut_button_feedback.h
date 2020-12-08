#pragma once
#include <gtest/gtest.h>
#include <QEventLoop>
#include "../../src/button_feedback.h"

using namespace testing;

class ButtonFeedbackTest:public testing::Test{

public:
    ButtonFeedback *buttonFeedback;
    virtual void SetUp() override{
        buttonFeedback = new ButtonFeedback();
        std::cout << "start ButtonFeedbackTest" << std::endl;
    }

    virtual void TearDown() override{
        delete buttonFeedback;
        std::cout << "end ButtonFeedbackTest" << std::endl;
    }
};

TEST_F(ButtonFeedbackTest, showPressFeedback)
{
    buttonFeedback->showPressFeedback(50, 50);
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}


TEST_F(ButtonFeedbackTest, showDragFeedback)
{
    buttonFeedback->showPressFeedback(50, 100);
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}


TEST_F(ButtonFeedbackTest, showReleaseFeedback)
{

    buttonFeedback->showReleaseFeedback(50, 150);
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}
