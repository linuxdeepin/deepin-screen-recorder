#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QPoint>
#include "../../src/RecorderRegionShow.h"


using namespace testing;

class RecorderRegionShowTest:public testing::Test{

public:
   RecorderRegionShow * m_pRecorderRegion =  new RecorderRegionShow();
    virtual void SetUp() override{
        std::cout << "start RecorderRegionShowTest" << std::endl;
        m_pRecorderRegion->resize(800, 600);
        m_pRecorderRegion->move(100, 100);
        m_pRecorderRegion->show();
    }

    virtual void TearDown() override{
        std::cout << "end RecorderRegionShowTest" << std::endl;
        m_pRecorderRegion->hide();
        delete m_pRecorderRegion;
    }
};

TEST_F(RecorderRegionShowTest, showKeyBoardButtons)
{
    for(int i = 0; i < 10; ++i) {
        QString key = QString("%1").arg(i);
        m_pRecorderRegion->showKeyBoardButtons(key);
        m_pRecorderRegion->update();
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();
    }
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(RecorderRegionShowTest, initCameraInfo)
{
    /*
    m_pRecorderRegion->initCameraInfo(CameraWidget::Position::rightBottom, QSize(420, 250));
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    */
}

