#pragma once
#include <gtest/gtest.h>
#include <QRect>
#include <QPixmap>
#include <QDebug>
#include "../src/utils/screengrabber.h"
#include "stub.h"


using namespace testing;

class ScreenGrabberTest:public testing::Test{

public:
    Stub stub;
    ScreenGrabber screenGrabber;
    virtual void SetUp() override{
        std::cout << "start ScreenGrabberTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end ScreenGrabberTest" << std::endl;
    }
};

bool waylandDectected_stub(void *obj)
{
    return true;
}

TEST_F(ScreenGrabberTest, grabEntireDesktop)
{
    bool ok = false;
    QRect rect(0, 0, 600, 400);
    QPixmap pix = screenGrabber.grabEntireDesktop(ok, rect, 0);
    qDebug() << pix.rect();
    EXPECT_EQ(true, ok);
}
TEST_F(ScreenGrabberTest, grabEntireDesktop_wayland)
{
    stub.set(ADDR( DesktopInfo, waylandDectected), waylandDectected_stub);
    bool ok = false;
    QRect rect;
    QPixmap pix = screenGrabber.grabEntireDesktop(ok, rect, 0);
    qDebug() << pix.rect();
    EXPECT_EQ(true, ok);
}
