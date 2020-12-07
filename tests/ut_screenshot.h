#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QPoint>
#include "../src/screenshot.h"
#include "stub.h"
#include "addr_pri.h"


using namespace testing;

ACCESS_PRIVATE_FIELD(Screenshot, MainWindow*, m_window);
int quit_stub(void* obj)
{
    qDebug() <<"I am QCoreApplication quit";
    return 0;
}
class ScreenshotTest:public testing::Test{

public:
    Screenshot *shot;
    MainWindow* mwindow;
    Stub stub;
    virtual void SetUp() override{
        qDebug() << "++++++" << __FUNCTION__ << __LINE__;
        shot = new Screenshot;
        shot->setConfigThemeType(0);
        stub.set(ADDR(QCoreApplication, quit), quit_stub);

        //std::cout << "start ScreenshotTest" << std::endl;
    }

    virtual void TearDown() override{
        //shot->stopRecord();
        qDebug() << "++++++" << __FUNCTION__ << __LINE__;
        stub.reset(ADDR(QCoreApplication, quit));
        delete shot;
        //std::cout << "end ScreenshotTest" << std::endl;
    }
};

TEST_F(ScreenshotTest, startScreenshot)
{
    qDebug() << "++++++" << __FUNCTION__ << __LINE__;
    shot->startScreenshot();
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    sleep(1);
    QTest::mouseMove(mwindow, QPoint(100,100));
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100,100));
    sleep(1);
    //QTest::mouseClick(shot.m_window->, Qt::MouseButton::LeftButton);
    mwindow->saveScreenShot();
    sleep(1);
}



TEST_F(ScreenshotTest, delayScreenshot)
{

    shot->delayScreenshot(3);
    sleep(5);
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    sleep(1);
    QTest::mouseMove(mwindow, QPoint(500,100));
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(500,100));
    sleep(1);
    //QTest::mouseClick(shot.m_window->, Qt::MouseButton::LeftButton);
    mwindow->saveScreenShot();
    sleep(1);
}

TEST_F(ScreenshotTest, fullscreenScreenshot)
{
    shot->fullscreenScreenshot();
    sleep(1);
}

TEST_F(ScreenshotTest, topWindowScreenshot)
{
    shot->topWindowScreenshot();
    sleep(1);
}

TEST_F(ScreenshotTest, noNotifyScreenshot)
{
    shot->noNotifyScreenshot();
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10,10));
    QTest::mouseMove(mwindow, QPoint(100,100));
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100,100));
    mwindow->saveScreenShot();
    sleep(1);
}

TEST_F(ScreenshotTest, savePathScreenshot)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    shot->savePathScreenshot(path);
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    QTest::mouseMove(mwindow, QPoint(100,100));
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100,100));
    mwindow->saveScreenShot();
    sleep(1);
}

TEST_F(ScreenshotTest, startScreenshotFor3rd)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    shot->startScreenshotFor3rd(path);
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    QTest::mouseMove(mwindow, QPoint(100,100));
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100,100));
    mwindow->saveScreenShot();
    sleep(1);
}

TEST_F(ScreenshotTest, startScreenshotFor3rd_Esc)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    shot->startScreenshotFor3rd(path);
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    QTest::mouseMove(mwindow, QPoint(100,100));
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100,100));
    QTest::keyClick(mwindow, Qt::Key_Escape);
    sleep(1);
}
