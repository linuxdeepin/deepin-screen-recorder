#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QPoint>
#include "../../src/screenshot.h"
#include "../../src/main_window.h"
#include "stub.h"
#include "addr_pri.h"


using namespace testing;
ACCESS_PRIVATE_FIELD(Screenshot, MainWindow*, m_window);
static int state = 0;
class ScreenshotTest:public testing::Test{

public:
    Screenshot *shot = new Screenshot;
    MainWindow* mwindow;
    QEventLoop loop;
    Stub stub;
    virtual void SetUp() override{
        qDebug() << "++++++" << __FUNCTION__ << __LINE__;
    }

    virtual void TearDown() override{
        stub.reset(ADDR(ConfigSettings, value));
        qDebug() << "++++++" << __FUNCTION__ << __LINE__;
    }
};

//TEST_F(ScreenshotTest, startScreenshot)
//{
//    qDebug() << "++++++" << __FUNCTION__ << __LINE__;
//    shot->startScreenshot();
//    mwindow = access_private_field::Screenshotm_window(*shot);
//    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseMove(mwindow, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    mwindow->saveScreenShot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, delayScreenshot)
//{

//    shot->delayScreenshot(3);
//    QTimer::singleShot(5000, &loop, SLOT(quit()));
//    loop.exec();
//    mwindow = access_private_field::Screenshotm_window(*shot);
//    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseMove(mwindow, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    mwindow->saveScreenShot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}

QVariant getShotCfg_stub(void* obj, const QString &group, const QString &key)
{
    if(group == "save") {
        if(key == "format") {
            return state % 3;
        }else if(key == "save_op"){
            return  (state + 1)% 3; // 0 desktop 1 image 2autoSave 3 SaveToSpecificDir
        }
    }
    return true;
}


//TEST_F(ScreenshotTest, fullscreenScreenshot)
//{
//    state = 0;
//    stub.set(ADDR(ConfigSettings, value), getShotCfg_stub);
//    shot->fullscreenScreenshot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, topWindowScreenshot)
//{
//    state = 1;
//    stub.set(ADDR(ConfigSettings, value), getShotCfg_stub);
//    shot->topWindowScreenshot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, desktop_fullscreenScreenshot)
//{
//    state = 2;
//    stub.set(ADDR(ConfigSettings, value), getShotCfg_stub);
//    shot->fullscreenScreenshot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, clipboard_topWindowScreenshot)
//{
//    state = 3;
//    stub.set(ADDR(ConfigSettings, value), getShotCfg_stub);
//    shot->topWindowScreenshot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}


//TEST_F(ScreenshotTest, fullscreenScreenshot_)
//{
//    state = 4;
//    stub.set(ADDR(ConfigSettings, value), getShotCfg_stub);
//    shot->fullscreenScreenshot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, topWindowScreenshot_)
//{
//    state = 5;
//    stub.set(ADDR(ConfigSettings, value), getShotCfg_stub);
//    shot->topWindowScreenshot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, desktop_fullscreenScreenshot_)
//{
//    state = 6;
//    stub.set(ADDR(ConfigSettings, value), getShotCfg_stub);
//    shot->fullscreenScreenshot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, clipboard_topWindowScreenshot_)
//{
//    state = 7;
//    stub.set(ADDR(ConfigSettings, value), getShotCfg_stub);
//    shot->topWindowScreenshot();
//    QTimer::singleShot(2000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, noNotifyScreenshot)
//{
//    shot->noNotifyScreenshot();
//    mwindow = access_private_field::Screenshotm_window(*shot);
//    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10,10));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseMove(mwindow, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    mwindow->saveScreenShot();
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, savePathScreenshot)
//{
//    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
//    shot->savePathScreenshot(path);
//    mwindow = access_private_field::Screenshotm_window(*shot);
//    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseMove(mwindow, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    mwindow->saveScreenShot();
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, startScreenshotFor3rd)
//{
//    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
//    shot->startScreenshotFor3rd(path);
//    mwindow = access_private_field::Screenshotm_window(*shot);
//    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseMove(mwindow, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    mwindow->saveScreenShot();
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//}

//TEST_F(ScreenshotTest, startScreenshotFor3rd_Esc)
//{
//    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
//    shot->startScreenshotFor3rd(path);
//    mwindow = access_private_field::Screenshotm_window(*shot);
//    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseMove(mwindow, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();
//    QTest::keyClick(mwindow, Qt::Key_Escape);
//    QTimer::singleShot(4000, &loop, SLOT(quit()));
//    loop.exec();
//}
