#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QObject>
#include <QDebug>
#include "../countdown_tooltip.h"


using namespace testing;

class CountdownTooltipTest:public testing::Test{

public:
    CountdownTooltip *countTip = new CountdownTooltip;
    virtual void SetUp() override{
        std::cout << "start CountdownTooltipTest" << std::endl;

    }

    virtual void TearDown() override{
        std::cout << "end CountdownTooltipTest" << std::endl;
    }
public slots:
    void startRecord();
};

TEST_F(CountdownTooltipTest, start)
{
    countTip->show();
    countTip->start();
    sleep(5);
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
}
void CountdownTooltipTest::startRecord()
{
    qDebug() << "startRecord";
}
