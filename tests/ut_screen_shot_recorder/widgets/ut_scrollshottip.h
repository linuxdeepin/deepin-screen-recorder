/*

* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.

*

* Author:     wangcong <wangcong@uniontech.com>

*

* Maintainer: wangcong <wangcong@uniontech.com>

*

* This program is free software: you can redistribute it and/or modify

* it under the terms of the GNU General Public License as published by

* the Free Software Foundation, either version 3 of the License, or

* any later version.

*

* This program is distributed in the hope that it will be useful,

* but WITHOUT ANY WARRANTY; without even the implied warranty of

* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the

* GNU General Public License for more details.

*

* You should have received a copy of the GNU General Public License

* along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QObject>
#include <QDebug>
#include "../../src/widgets/scrollshottip.h"
#include "addr_pri.h"

using namespace testing;

class ScrollShotTipTest:public testing::Test{

public:
    ScrollShotTip *scrollShotTip = new ScrollShotTip;
    virtual void SetUp() override{
        std::cout << "start ScrollShotTipTest" << std::endl;

    }

    virtual void TearDown() override{
        scrollShotTip->hide();
        delete scrollShotTip;
        std::cout << "end ScrollShotTipTest" << std::endl;
    }
public slots:
    void startRecord();
};

ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showStartScrollShotTip);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showErrorScrollShotTip);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showEndScrollShotTip);
ACCESS_PRIVATE_FUN(ScrollShotTip, void(), showMaxScrollShotTip);


TEST_F(ScrollShotTipTest, showTip)
{
    scrollShotTip->showTip(TipType::StartScrollShotTip);
    scrollShotTip->move(500,500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();

    scrollShotTip->showTip(TipType::ErrorScrollShotTip);
    scrollShotTip->move(500,500);
    scrollShotTip->show();
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();

    scrollShotTip->showTip(TipType::EndScrollShotTip);
    scrollShotTip->move(500,500);
    scrollShotTip->show();
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();

    scrollShotTip->showTip(TipType::MaxLengthScrollShotTip);
    scrollShotTip->move(500,500);
    scrollShotTip->show();
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();

}
TEST_F(ScrollShotTipTest, getTipType)
{
    scrollShotTip->showTip(TipType::StartScrollShotTip);
    TipType tipType = scrollShotTip->getTipType();
    EXPECT_EQ(tipType, TipType::StartScrollShotTip);
}

TEST_F(ScrollShotTipTest, showStartScrollShotTip)
{
    call_private_fun::ScrollShotTipshowStartScrollShotTip(*scrollShotTip);
    scrollShotTip->move(1000,500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}

TEST_F(ScrollShotTipTest, showErrorScrollShotTip)
{
    call_private_fun::ScrollShotTipshowErrorScrollShotTip(*scrollShotTip);
    scrollShotTip->move(1000,500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}

TEST_F(ScrollShotTipTest, showEndScrollShotTip)
{
    call_private_fun::ScrollShotTipshowEndScrollShotTip(*scrollShotTip);
    scrollShotTip->move(1000,500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}

TEST_F(ScrollShotTipTest, showMaxScrollShotTip)
{
    call_private_fun::ScrollShotTipshowMaxScrollShotTip(*scrollShotTip);
    scrollShotTip->move(1000,500);
    scrollShotTip->show();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    scrollShotTip->hide();
}

