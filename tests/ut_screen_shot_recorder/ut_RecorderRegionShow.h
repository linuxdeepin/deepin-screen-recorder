// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QPoint>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/RecorderRegionShow.h"
#include "../../src/utils.h"

using namespace testing;

class RecorderRegionShowTest: public testing::Test
{

public:
    Stub stub;
//    RecorderRegionShow *m_pRecorderRegion;
    virtual void SetUp() override
    {
        std::cout << "start RecorderRegionShowTest" << std::endl;
//        RecorderRegionShow *m_pRecorderRegion =  new RecorderRegionShow();
//        m_pRecorderRegion->resize(800, 600);
//        m_pRecorderRegion->move(100, 100);
//        m_pRecorderRegion->show();
    }

    virtual void TearDown() override
    {
        std::cout << "end RecorderRegionShowTest" << std::endl;
//        m_pRecorderRegion->hide();
//        delete m_pRecorderRegion;
    }
};

//TEST_F(RecorderRegionShowTest, showKeyBoardButtons)
//{
//    for(int i = 0; i < 6; ++i) {
//        QString key = QString("%1").arg(i);
//        m_pRecorderRegion->showKeyBoardButtons(key);
//        m_pRecorderRegion->update();
//        QEventLoop loop;
//        QTimer::singleShot(1000, &loop, SLOT(quit()));
//        loop.exec();
//    }
//    QEventLoop loop;
//    QTimer::singleShot(3000, &loop, SLOT(quit()));
//    loop.exec();
//}

void initCamera_stub()
{

}

bool cameraStart_stub()
{
    return true;
}
void passInputEvent_stub1(int wid)
{
    Q_UNUSED(wid);
}
bool getcameraStatus_stub()
{
    return false;
}
ACCESS_PRIVATE_FUN(RecorderRegionShow, void(), updateMultiKeyBoardPos);
void updateMultiKeyBoardPos_stub()
{

}
ACCESS_PRIVATE_FIELD(RecorderRegionShow, QList<KeyButtonWidget *>, m_keyButtonList);
ACCESS_PRIVATE_FIELD(RecorderRegionShow, CameraWidget *, m_cameraWidget);
ACCESS_PRIVATE_FIELD(RecorderRegionShow, QPainter *, m_painter);
ACCESS_PRIVATE_FUN(RecorderRegionShow, void(QPaintEvent *), paintEvent);
