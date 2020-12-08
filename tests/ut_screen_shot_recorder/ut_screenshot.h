#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QPoint>
#include "../../src/screenshot.h"
#include "stub.h"
#include "addr_pri.h"


using namespace testing;

ACCESS_PRIVATE_FIELD(Screenshot, MainWindow*, m_window);
int quit_stub(void* obj)
{
    qDebug() <<"I am QCoreApplication quit";
    return 0;
}

QDBusMessage callWithArgumentList_stub(void *obj,QDBus::CallMode mode,
                                  const QString &method,
                                       const QList<QVariant> &args){

    qDebug() <<"I am QDBusInterface callWithArgumentList";
    return QDBusMessage();

}
class ScreenshotTest:public testing::Test{

public:
    Screenshot *shot = new Screenshot;
    MainWindow* mwindow;
    QEventLoop loop;
    Stub stub;
    virtual void SetUp() override{
        qDebug() << "++++++" << __FUNCTION__ << __LINE__;
        stub.set(ADDR(QCoreApplication, quit), quit_stub);
        stub.set(ADDR(QDBusInterface, callWithArgumentList), callWithArgumentList_stub);
        //std::cout << "start ScreenshotTest" << std::endl;
    }

    virtual void TearDown() override{
        //shot->stopRecord();
        qDebug() << "++++++" << __FUNCTION__ << __LINE__;
        stub.reset(ADDR(QCoreApplication, quit));
        stub.reset(ADDR(QDBusInterface, callWithArgumentList));
        //mwindow->setEnabled(false);
        //delete mwindow;
        //delete shot;
        //std::cout << "end ScreenshotTest" << std::endl;
    }
};

TEST_F(ScreenshotTest, startScreenshot)
{
    qDebug() << "++++++" << __FUNCTION__ << __LINE__;
    shot->startScreenshot();
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseMove(mwindow, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    mwindow->saveScreenShot();
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ScreenshotTest, delayScreenshot)
{

    shot->delayScreenshot(3);
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseMove(mwindow, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    mwindow->saveScreenShot();
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ScreenshotTest, fullscreenScreenshot)
{
    shot->fullscreenScreenshot();
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ScreenshotTest, topWindowScreenshot)
{
    shot->topWindowScreenshot();
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ScreenshotTest, noNotifyScreenshot)
{
    shot->noNotifyScreenshot();
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10,10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseMove(mwindow, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    mwindow->saveScreenShot();
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ScreenshotTest, savePathScreenshot)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    shot->savePathScreenshot(path);
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseMove(mwindow, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    mwindow->saveScreenShot();
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ScreenshotTest, startScreenshotFor3rd)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    shot->startScreenshotFor3rd(path);
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseMove(mwindow, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    mwindow->saveScreenShot();
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ScreenshotTest, startScreenshotFor3rd_Esc)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    shot->startScreenshotFor3rd(path);
    mwindow = access_private_field::Screenshotm_window(*shot);
    QTest::mousePress(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseMove(mwindow, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::mouseRelease(mwindow, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::keyClick(mwindow, Qt::Key_Escape);
    QTimer::singleShot(4000, &loop, SLOT(quit()));
    loop.exec();
}
