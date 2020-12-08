#pragma once
#include <QDebug>
#include <gtest/gtest.h>
#include "../../src/widgets/keybuttonwidget.h"


using namespace testing;

class KeyButtonWidgetTest:public testing::Test, public QObject{

public:
    KeyButtonWidget *keyWidget;
    virtual void SetUp() override{
        std::cout << "start KeyButtonWidgetTest" << std::endl;
        keyWidget  = new KeyButtonWidget(nullptr);
    }

    virtual void TearDown() override{
        delete keyWidget;
        std::cout << "end KeyButtonWidgetTest" << std::endl;
    }
};
TEST_F(KeyButtonWidgetTest, setKeyLabelWord)
{
    ASSERT_NO_THROW(keyWidget->setKeyLabelWord("Del"););
}
