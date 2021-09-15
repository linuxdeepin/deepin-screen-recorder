#pragma once

#include "../../src/utils.h"
#include "../../src/main_window.h"

#include <QTest>
#include <QPoint>
#include <QScreen>

#include "stub.h"
#include "addr_pri.h"

#include <gtest/gtest.h>

using namespace testing;


ACCESS_PRIVATE_FIELD(MainWindow, ToolBar *, m_toolBar);
ACCESS_PRIVATE_FIELD(MainWindow, int,  m_screenCount);
ACCESS_PRIVATE_FIELD(MainWindow, ShapesWidget *, m_shapesWidget);

QRect geometry_stub()
{
    return QRect(0, 0, 1920, 1080);
}

void passInputEvent_stub(int wid)
{
    Q_UNUSED(wid);
}

qreal devicePixelRatio_stub_2()
{
    return 1;
}

int width_stub_2()
{
    return 1920;
}

int height_stub_2()
{
    return 1080;
}

int screenCount_stub()
{
    return 1;
}

class MainWindowTest: public testing::Test
{

public:
    Stub stub;
    MainWindow *m_window = nullptr;
    virtual void SetUp() override
    {
        m_window = new MainWindow;
        std::cout << "start MainWindowTest" << std::endl;
        ConfigSettings::instance()->setValue("save", "saveCursor", true);
    }

    virtual void TearDown() override
    {
        if (m_window) {
            //m_window->deleteLater();
            delete  m_window;
            m_window = nullptr;
        }
        std::cout << "end MainWindowTest" << std::endl;
        system("killall deepin-shortcut-viewer");
    }
};
/*
TEST_F(MainWindowTest, initAttributes)
{
    stub.set(ADDR(QScreen,geometry),geometry_stub);
    m_window->initAttributes();
    stub.reset(ADDR(QScreen,geometry));
}

TEST_F(MainWindowTest, initResource)
{
    stub.set(ADDR(QScreen,geometry),geometry_stub);
    m_window->initAttributes();
    stub.reset(ADDR(QScreen,geometry));

    stub.set(ADDR(Utils,passInputEvent),passInputEvent_stub);
    m_window->initResource();
    stub.reset(ADDR(Utils,passInputEvent));
}

TEST_F(MainWindowTest, initLaunchMode)
{
    stub.set(ADDR(QScreen,geometry),geometry_stub);
    m_window->initAttributes();
    stub.reset(ADDR(QScreen,geometry));

    stub.set(ADDR(Utils,passInputEvent),passInputEvent_stub);
    m_window->initResource();
    stub.reset(ADDR(Utils,passInputEvent));

    m_window->initLaunchMode("screenShot");
}
*/

TEST_F(MainWindowTest, screenShotShapes)
{
    stub.set(ADDR(QScreen, geometry), geometry_stub);
    m_window->initAttributes();
    stub.reset(ADDR(QScreen, geometry));

    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
    m_window->initResource();
    stub.reset(ADDR(Utils, passInputEvent));

    m_window->initLaunchMode("screenShot");

    m_window->showFullScreen();

    access_private_field::MainWindowm_screenCount(*m_window) = 1;

    QEventLoop loop;

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10, 10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(800, 600));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800, 600));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800, 600));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(900, 700));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(900, 700));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(900, 10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(800, 20));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800, 20));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(900, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(900, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 700));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(400, 900));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 900));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();




    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10, 10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(20, 30));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(20, 30));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(20, 50));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(30, 50));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(30, 50));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_window, Qt::Key_R);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    ShapesWidget *m_shapesWidget = access_private_field::MainWindowm_shapesWidget(*m_window);


    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Left, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Right, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Up, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Down, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_shapesWidget, Qt::Key_Left,  Qt::ShiftModifier | Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Right, Qt::ShiftModifier | Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Up, Qt::ShiftModifier | Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Down, Qt::ShiftModifier | Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Left, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Right, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Up, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Down, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_shapesWidget, Qt::Key_Z, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    ConfigSettings::instance()->setValue("rectangle", "is_blur", true);
    ConfigSettings::instance()->setValue("rectangle", "is_mosaic", false);

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::keyClick(m_window, Qt::Key_O);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    ConfigSettings::instance()->setValue("oval", "is_blur", false);
    ConfigSettings::instance()->setValue("oval", "is_mosaic", true);

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(300, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(500, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(500, 180));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_window, Qt::Key_L);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->changeArrowAndLineEvent(0);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(200, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(200, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->changeArrowAndLineEvent(1);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(120, 210));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(250, 360));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(250, 360));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_window, Qt::Key_P);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(50, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(320, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(120, 180));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(280, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_T);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(60, 60));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Z, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->saveScreenShot();
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(MainWindowTest, screenShot)
{
    stub.set(ADDR(QScreen, geometry), geometry_stub);
    m_window->initAttributes();
    stub.reset(ADDR(QScreen, geometry));

    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
    m_window->initResource();
    stub.reset(ADDR(Utils, passInputEvent));

    m_window->initLaunchMode("screenShot");

    m_window->showFullScreen();


    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mouseMove(m_window, QPoint(960, 480));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(100, 100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100, 100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(250, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(250, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Left, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Right, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Up, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Down, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_window, Qt::Key_Left, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Right, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Up, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Down, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Left, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Right, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Up, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Down, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->changeFunctionButton("record");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_K);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_C);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(200, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(300, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(300, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->changeFunctionButton("shot");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_R);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(350, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_O);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(355, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_L);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(350, 300));
    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(360, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_P);

    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(365, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_T);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(370, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::keyClick(m_window, Qt::Key_Question, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_Escape, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->sendSavingNotify();
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->responseEsc();

//    m_window->saveScreenShot();


    QTest::mouseClick(m_window, Qt::MouseButton::RightButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 250));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();

    //退出截图
    QTest::keyClick(m_window, Qt::Key_Escape, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(MainWindowTest, screenRecord)
{
    stub.set(ADDR(QScreen, devicePixelRatio), devicePixelRatio_stub_2);
    stub.set(ADDR(QWidget, width), width_stub_2);
    stub.set(ADDR(QWidget, height), height_stub_2);
    stub.set(ADDR(QScreen, geometry), geometry_stub);
    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
    stub.set(ADDR(QDesktopWidget, screenCount), screenCount_stub);

    m_window->initAttributes();
    m_window->initResource();
    m_window->initLaunchMode("screenShot");
    m_window->showFullScreen();

    ToolBar *m_toolBar = access_private_field::MainWindowm_toolBar(*m_window);

    QEventLoop loop;


//    QTest::mouseMove(m_window, QPoint(0,0));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

//    QTest::mouseMove(m_window, QPoint(1400,1050));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1400, 1050));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    //m_window->changeCameraSelectEvent(true);
    m_window->changeKeyBoardShowEvent(true);
    m_window->changeMouseShowEvent(true);

    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_R);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::keyClick(m_window, Qt::Key_W);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::keyClick(m_window, Qt::Key_W);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::keyClick(m_window, Qt::Key_Q);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    m_window->showKeyBoardButtons("F1");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_window->showKeyBoardButtons("F2");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_window->showKeyBoardButtons("F3");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->changeSystemAudioSelectEvent(true);
    m_window->changeCameraSelectEvent(true);

    m_window->startCountdown();
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();

    m_window->showKeyBoardButtons("F4");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_window->showKeyBoardButtons("F5");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_window->showKeyBoardButtons("F6");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_window->showKeyBoardButtons("F7");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->showPressFeedback(100, 100);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_window->showDragFeedback(100, 100);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_window->showReleaseFeedback(100, 100);

    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_window->forciblySavingNotify();

    m_window->stopRecord();
    QTimer::singleShot(1000, &loop, SLOT(quit()));


    stub.reset(ADDR(QScreen, devicePixelRatio));
    stub.reset(ADDR(QWidget, width));
    stub.reset(ADDR(QWidget, height));
    stub.reset(ADDR(QScreen, geometry));
    stub.reset(ADDR(Utils, passInputEvent));
    stub.reset(ADDR(QDesktopWidget, screenCount));

    loop.exec();
}

TEST_F(MainWindowTest, scrollShot)
{
    stub.set(ADDR(QScreen, geometry), geometry_stub);
    m_window->initAttributes();
    stub.reset(ADDR(QScreen, geometry));

    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
    m_window->initResource();
    stub.reset(ADDR(Utils, passInputEvent));

    m_window->initLaunchMode("screenShot");

    m_window->showFullScreen();

    QEventLoop loop;
    QTest::mouseMove(m_window, QPoint(700, 100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(700, 100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(1400, 800));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1400, 800));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    //启动滚动截图
    QTest::keyClick(m_window, Qt::Key_I, Qt::AltModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(1000, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1000, 400));

    QTimer::singleShot(10000, &loop, SLOT(quit()));
    loop.exec();
    //退出滚动截图
    QTest::keyClick(m_window, Qt::Key_Escape, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
}


static bool hasComposite_stub(void *obj)
{
    return false;
}


static QString CpuArchitecture_stub(void *obj)
{
    return  "mips";
}
TEST_F(MainWindowTest, onHelp)
{
    m_window->onHelp();

    stub.set(ADDR(DWindowManagerHelper, hasComposite), hasComposite_stub);
    stub.set(ADDR(QSysInfo, currentCpuArchitecture), CpuArchitecture_stub);
    m_window->compositeChanged();
    stub.reset(ADDR(DWindowManagerHelper, hasComposite));
    stub.reset(ADDR(QSysInfo, currentCpuArchitecture));

}


ACCESS_PRIVATE_FIELD(MainWindow, int, m_autoScrollFlagNext);
//
TEST_F(MainWindowTest, onScrollShotCheckScrollType)
{
    m_window->onScrollShotCheckScrollType(10);

    int &MainWindow_m_autoScrollFlagNext =  access_private_field::MainWindowm_autoScrollFlagNext(*m_window);

    EXPECT_EQ(10, MainWindow_m_autoScrollFlagNext);
}

ACCESS_PRIVATE_FIELD(MainWindow, int, recordX);
ACCESS_PRIVATE_FIELD(MainWindow, int, recordY);
ACCESS_PRIVATE_FIELD(MainWindow, int, recordWidth);
ACCESS_PRIVATE_FIELD(MainWindow, int, recordHeight);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_screenWidth);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_screenHeight);
ACCESS_PRIVATE_FIELD(MainWindow, qreal, m_pixelRatio);
ACCESS_PRIVATE_FIELD(MainWindow, ScrollShotTip *, m_scrollShotTip);
ACCESS_PRIVATE_FUN(MainWindow, bool(), isToolBarInShotArea);
ACCESS_PRIVATE_FUN(MainWindow, QPoint(), getScrollShotTipPosition);
static bool isToolBarInShotArea_stub(void *obj)
{
    return true;
}
TEST_F(MainWindowTest, getScrollShotTipPosition)
{
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_m_screenWidth =  access_private_field::MainWindowm_screenWidth(*m_window);
    MainWindow_m_screenWidth = 1920;
    int &MainWindow_m_screenHeight =  access_private_field::MainWindowm_screenHeight(*m_window);
    MainWindow_m_screenHeight = 1080;
    int &MainWindow_m_screenCount =  access_private_field::MainWindowm_screenCount(*m_window);
    MainWindow_m_screenCount = 1;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;
    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*m_window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*m_window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    auto MainWindow_isToolBarInShotArea = get_private_fun::MainWindowisToolBarInShotArea();
    stub.set(MainWindow_isToolBarInShotArea, isToolBarInShotArea_stub);

    auto point = call_private_fun::MainWindowgetScrollShotTipPosition(*m_window);
    qDebug() << "point: " << point;
    qDebug() << "ppppppp: " << QPoint(static_cast<int>((1920  - MainWindow_m_scrollShotTip->width() * MainWindow_m_pixelRatio) / 2),
                                      static_cast<int>(100 + 15 * MainWindow_m_pixelRatio));
    EXPECT_EQ(QPoint(static_cast<int>((1920  - MainWindow_m_scrollShotTip->width() * MainWindow_m_pixelRatio) / 2),
                     static_cast<int>(100 + 15 * MainWindow_m_pixelRatio)), point);

    stub.reset(MainWindow_isToolBarInShotArea);

}

ACCESS_PRIVATE_FIELD(MainWindow, int, m_scrollShotStatus);
ACCESS_PRIVATE_FIELD(MainWindow, ScrollScreenshot *, m_scrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), startAutoScrollShot);
//替换ScrollScreenshot的setScrollModel函数
static bool setScrollModel_stub(void *obj, bool)
{
    return true;
}
//替换ScrollScreenshot的addPixmap函数
static bool addPixmap_stub(void *obj, const QPixmap &piximg, int wheelDirection)
{
    return true;
}
//开始自动滚动截图测试用例
TEST_F(MainWindowTest, startAutoScrollShot)
{
    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*m_window);
    MainWindow_m_scrollShotStatus = 1;
    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*m_window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, setScrollModel), setScrollModel_stub);
    stub.set(ADDR(ScrollScreenshot, addPixmap), addPixmap_stub);
    call_private_fun::MainWindowstartAutoScrollShot(*m_window);
    stub.reset(ADDR(ScrollScreenshot, setScrollModel));
    stub.reset(ADDR(ScrollScreenshot, addPixmap));

    delete MainWindow_m_scrollShot;
}


ACCESS_PRIVATE_FUN(MainWindow, void(), pauseAutoScrollShot);
static bool changeState_stub(void *obj, bool wheelDirection)
{
    return true;
}
//暂停自动滚动截图测试用例
TEST_F(MainWindowTest, pauseAutoScrollShot)
{
    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*m_window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, changeState), changeState_stub);
    call_private_fun::MainWindowpauseAutoScrollShot(*m_window);
    stub.reset(ADDR(ScrollScreenshot, changeState));
    delete MainWindow_m_scrollShot;

}

ACCESS_PRIVATE_FUN(MainWindow, void(), continueAutoScrollShot);
//继续自动滚动截图测试用例
TEST_F(MainWindowTest, continueAutoScrollShot)
{
    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*m_window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*m_window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, setScrollModel), setScrollModel_stub);
    stub.set(ADDR(ScrollScreenshot, changeState), changeState_stub);

    call_private_fun::MainWindowcontinueAutoScrollShot(*m_window);

    stub.reset(ADDR(ScrollScreenshot, setScrollModel));
    stub.reset(ADDR(ScrollScreenshot, changeState));
    delete MainWindow_m_scrollShot;


}

ACCESS_PRIVATE_FUN(MainWindow, void(), startManualScrollShot);
//开始手动滚动截图测试用例
TEST_F(MainWindowTest, startManualScrollShot)
{
    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*m_window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, setScrollModel), setScrollModel_stub);
    stub.set(ADDR(ScrollScreenshot, addPixmap), addPixmap_stub);
    call_private_fun::MainWindowstartManualScrollShot(*m_window);
    stub.reset(ADDR(ScrollScreenshot, setScrollModel));
    stub.reset(ADDR(ScrollScreenshot, addPixmap));
    delete MainWindow_m_scrollShot;

}

ACCESS_PRIVATE_FUN(MainWindow, void(int mouseTime, int direction), handleManualScrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(PreviewWidget::PostionStatus previewPostion, int direction, int mouseTime), scrollShotGrabPixmap);

static bool scrollShotGrabPixmap_stub(void *obj, PreviewWidget::PostionStatus previewPostion, int direction, int mouseTime)
{
    return true;
}
//处理手动滚动截图逻辑测试用例
TEST_F(MainWindowTest, handleManualScrollShot)
{
    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*m_window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    auto MainWindow_scrollShotGrabPixmap = get_private_fun::MainWindowscrollShotGrabPixmap();
    stub.set(MainWindow_scrollShotGrabPixmap, scrollShotGrabPixmap_stub);

    call_private_fun::MainWindowhandleManualScrollShot(*m_window, 1, 4);

    stub.reset(MainWindow_scrollShotGrabPixmap);

}

ACCESS_PRIVATE_FUN(MainWindow, void(), setInputEvent);
//滚动截图设置区域穿透测试用例
TEST_F(MainWindowTest, setInputEvent)
{
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;
    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*m_window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    auto MainWindow_isToolBarInShotArea = get_private_fun::MainWindowisToolBarInShotArea();
    stub.set(MainWindow_isToolBarInShotArea, isToolBarInShotArea_stub);

    call_private_fun::MainWindowsetInputEvent(*m_window);

    stub.reset(MainWindow_isToolBarInShotArea);

}

ACCESS_PRIVATE_FUN(MainWindow, void(), setCancelInputEvent);
//滚动截图时取消捕捉区域的鼠标穿透测试用例
TEST_F(MainWindowTest, setCancelInputEvent)
{
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;
    call_private_fun::MainWindowsetCancelInputEvent(*m_window);
}

ACCESS_PRIVATE_FUN(MainWindow, void(), showAdjustArea);
static QRect getInvalidArea_stub(void *obj)
{
    return QRect(0, 0, 500, 500);
}
//显示可调整的捕捉区域大小及位置测试用例
TEST_F(MainWindowTest, showAdjustArea)
{
    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*m_window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, getInvalidArea), getInvalidArea_stub);


    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;

    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;

    call_private_fun::MainWindowshowAdjustArea(*m_window);

    stub.reset(ADDR(ScrollScreenshot, getInvalidArea));
    delete MainWindow_m_scrollShot;

}

//判断工具栏是否在在捕捉区域内部测试用例
TEST_F(MainWindowTest, isToolBarInShotArea)
{
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*m_window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);
    MainWindow_toolBar->show();
    bool flag = call_private_fun::MainWindowisToolBarInShotArea(*m_window);
    MainWindow_toolBar->hide();

    EXPECT_EQ(flag, true);
}


TEST_F(MainWindowTest, initScrollShot)
{

}

ACCESS_PRIVATE_FIELD(MainWindow, QPixmap, m_firstScrollShotImg);
ACCESS_PRIVATE_FIELD(MainWindow, DPushButton *, m_shotButton);
ACCESS_PRIVATE_FIELD(MainWindow, TopTips *, m_scrollShotSizeTips);
ACCESS_PRIVATE_FIELD(MainWindow, PreviewWidget *, m_previewWidget);
ACCESS_PRIVATE_FUN(MainWindow, void(), showScrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), updateToolBarPos);
ACCESS_PRIVATE_FUN(MainWindow, void(), updateShotButtonPos);
static void updateToolBarPos_stub(void *obj)
{
    qDebug() << "调整工具栏位置";

}
static void updateShotButtonPos_stub(void *obj)
{
    qDebug() << "调整截图保存按钮位置";
}
//初始化滚动截图，显示滚动截图中的一些公共部件、例如工具栏、提示、图片大小、第一张预览图，单元测试用例
TEST_F(MainWindowTest, showScrollShot)
{
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*m_window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    QRect previewRecordRect {
        static_cast<int>(0),
        static_cast<int>(0),
        static_cast<int>(1920),
        static_cast<int>(1080)
    };
    PreviewWidget *&MainWindow_previewWidget = access_private_field::MainWindowm_previewWidget(*m_window);
    MainWindow_previewWidget = new PreviewWidget(previewRecordRect);
    MainWindow_previewWidget->setScreenInfo(MainWindow_screenWidth, MainWindow_m_pixelRatio);
    MainWindow_previewWidget->initPreviewWidget();

    DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*m_window);
    MainWindow_shotButton = new DPushButton();

    TopTips *&MainWindow_scrollShotSizeTips = access_private_field::MainWindowm_scrollShotSizeTips(*m_window);
    MainWindow_scrollShotSizeTips = new TopTips();

    ScrollShotTip *&MainWindow_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*m_window);
    MainWindow_scrollShotTip = new ScrollShotTip();

    auto MainWindow_updateToolBarPos = get_private_fun::MainWindowupdateToolBarPos();
    stub.set(MainWindow_updateToolBarPos, updateToolBarPos_stub);

    auto MainWindow_updateShotButtonPos = get_private_fun::MainWindowupdateShotButtonPos();
    stub.set(MainWindow_updateShotButtonPos, updateShotButtonPos_stub);

    //调用滚动
    call_private_fun::MainWindowshowScrollShot(*m_window);

    stub.reset(MainWindow_updateToolBarPos);
    stub.reset(MainWindow_updateShotButtonPos);


//    delete MainWindow_toolBar;
//    delete MainWindow_previewWidget;
    //delete MainWindow_shotButton;
    delete MainWindow_scrollShotSizeTips;
//    delete MainWindow_scrollShotTip;

}

ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isZhaoxin);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_scrollShotType);
//滚动截图模式，抓取当前捕捉区域的图片，传递给滚动截图处理类进行图片的拼接，单元测试
TEST_F(MainWindowTest, scrollShotGrabPixmap)
{
    bool &MainWindow_isZhaoxin =  access_private_field::MainWindowm_isZhaoxin(*m_window);
    MainWindow_isZhaoxin = true;

    int &MainWindow_scrollShotType =  access_private_field::MainWindowm_scrollShotType(*m_window);
    MainWindow_scrollShotType = 0;

    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*m_window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, setScrollModel), setScrollModel_stub);
    stub.set(ADDR(ScrollScreenshot, addPixmap), addPixmap_stub);

    auto MainWindow_isToolBarInShotArea = get_private_fun::MainWindowisToolBarInShotArea();
    stub.set(MainWindow_isToolBarInShotArea, isToolBarInShotArea_stub);

    DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*m_window);
    MainWindow_shotButton = new DPushButton();

    TopTips *&MainWindow_scrollShotSizeTips = access_private_field::MainWindowm_scrollShotSizeTips(*m_window);
    MainWindow_scrollShotSizeTips = new TopTips();

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*m_window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;

    QRect previewRecordRect {
        static_cast<int>(0),
        static_cast<int>(0),
        static_cast<int>(1920),
        static_cast<int>(1080)
    };
    PreviewWidget *&MainWindow_previewWidget = access_private_field::MainWindowm_previewWidget(*m_window);
    MainWindow_previewWidget = new PreviewWidget(previewRecordRect);
    MainWindow_previewWidget->setScreenInfo(MainWindow_screenWidth, MainWindow_m_pixelRatio);
    MainWindow_previewWidget->initPreviewWidget();

    call_private_fun::MainWindowscrollShotGrabPixmap(*m_window, PreviewWidget::PostionStatus::INSIDE, 5, 1);
    call_private_fun::MainWindowscrollShotGrabPixmap(*m_window, PreviewWidget::PostionStatus::RIGHT, 5, 1);

    stub.reset(ADDR(ScrollScreenshot, setScrollModel));
    stub.reset(ADDR(ScrollScreenshot, addPixmap));
    stub.reset(MainWindow_isToolBarInShotArea);

    delete MainWindow_m_scrollShot;
    //delete MainWindow_shotButton;
    delete MainWindow_scrollShotSizeTips;
//    delete MainWindow_toolBar;
//    delete MainWindow_previewWidget;

}

static void updateImage_stub(void *obj, QImage img)
{
    qDebug() << "预览窗口更新图片";
}
ACCESS_PRIVATE_FIELD(MainWindow, ScreenGrabber, m_screenGrabber);
ACCESS_PRIVATE_FUN(MainWindow, void(QImage img), showPreviewWidgetImage);
//显示预览窗口和图片单元测试用例
TEST_F(MainWindowTest, showPreviewWidgetImage)
{
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_screenWidth = 1920;

    QRect previewRecordRect {
        static_cast<int>(0),
        static_cast<int>(0),
        static_cast<int>(1920),
        static_cast<int>(1080)
    };
    PreviewWidget *&MainWindow_previewWidget = access_private_field::MainWindowm_previewWidget(*m_window);
    MainWindow_previewWidget = new PreviewWidget(previewRecordRect);
    MainWindow_previewWidget->setScreenInfo(MainWindow_screenWidth, MainWindow_m_pixelRatio);
    MainWindow_previewWidget->initPreviewWidget();
    stub.set(ADDR(PreviewWidget, updateImage), updateImage_stub);

    TopTips *&MainWindow_scrollShotSizeTips = access_private_field::MainWindowm_scrollShotSizeTips(*m_window);
    MainWindow_scrollShotSizeTips = new TopTips();

    ScreenGrabber &MainWindow_m_screenGrabber = access_private_field::MainWindowm_screenGrabber(*m_window);
    bool ok;
    QPixmap pixmap = MainWindow_m_screenGrabber.grabEntireDesktop(ok, previewRecordRect, MainWindow_m_pixelRatio);

    QImage img = pixmap.toImage();

    call_private_fun::MainWindowshowPreviewWidgetImage(*m_window, img);
    stub.reset(ADDR(PreviewWidget, updateImage));

    delete MainWindow_scrollShotSizeTips;
//    delete MainWindow_previewWidget;
}

ACCESS_PRIVATE_FIELD(MainWindow, int, m_scrollShotMouseClick);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isErrorWithScrollShot);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isAutoScrollShotStart);
ACCESS_PRIVATE_FUN(MainWindow, void(int x, int y), onScrollShotMouseClickEvent);
bool startAutoScrollShot_stub(void *obj)
{
    return true;
}
bool pauseAutoScrollShot_stub(void *obj)
{
    return true;
}
bool setCancelInputEvent_stub(void *obj)
{
    return true;
}
bool setInputEvent_stub(void *obj)
{
    return true;
}
bool continueAutoScrollShot_stub(void *obj)
{
    return true;
}
//滚动截图鼠标按钮事件单元测试用例
TEST_F(MainWindowTest, onScrollShotMouseClickEvent)
{
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*m_window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*m_window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*m_window);
    MainWindow_shotButton = new DPushButton();

    bool &MainWindow_m_isErrorWithScrollShot = access_private_field::MainWindowm_isErrorWithScrollShot(*m_window);
    MainWindow_m_isErrorWithScrollShot = false;

    bool &MainWindow_m_isAutoScrollShotStart = access_private_field::MainWindowm_isAutoScrollShotStart(*m_window);
    MainWindow_m_isAutoScrollShotStart = false;

    int &MainWindow_scrollShotType =  access_private_field::MainWindowm_scrollShotType(*m_window);
    MainWindow_scrollShotType = 0;

    int &MainWindow_m_scrollShotMouseClick =  access_private_field::MainWindowm_scrollShotMouseClick(*m_window);
    MainWindow_m_scrollShotMouseClick = 0;

    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*m_window);
    MainWindow_m_scrollShotStatus = 1;

    auto MainWindow_startAutoScrollShot = get_private_fun::MainWindowstartAutoScrollShot();
    stub.set(MainWindow_startAutoScrollShot, startAutoScrollShot_stub);

    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    auto MainWindow_setCancelInputEvent = get_private_fun::MainWindowsetCancelInputEvent();
    stub.set(MainWindow_setCancelInputEvent, setCancelInputEvent_stub);

    auto MainWindow_setInputEvent = get_private_fun::MainWindowsetInputEvent();
    stub.set(MainWindow_setInputEvent, setInputEvent_stub);

    auto MainWindow_continueAutoScrollShot = get_private_fun::MainWindowcontinueAutoScrollShot();
    stub.set(MainWindow_continueAutoScrollShot, continueAutoScrollShot_stub);

    call_private_fun::MainWindowonScrollShotMouseClickEvent(*m_window, 500, 500);

    call_private_fun::MainWindowonScrollShotMouseClickEvent(*m_window, 500, 500);

    stub.reset(MainWindow_startAutoScrollShot);
    stub.reset(MainWindow_pauseAutoScrollShot);
    stub.reset(MainWindow_setCancelInputEvent);
    stub.reset(MainWindow_setInputEvent);
    stub.reset(MainWindow_continueAutoScrollShot);

    //delete MainWindow_m_scrollShotTip;
    //delete MainWindow_shotButton;
//    delete MainWindow_toolBar;
}

ACCESS_PRIVATE_FUN(MainWindow, void(int x, int y), onScrollShotMouseMoveEvent);
//滚动截图鼠标移动事件处理,单元测试用例
TEST_F(MainWindowTest, onScrollShotMouseMoveEvent)
{
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*m_window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*m_window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*m_window);
    MainWindow_shotButton = new DPushButton();

    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*m_window);
    MainWindow_m_scrollShotStatus = 1;

    bool &MainWindow_m_isErrorWithScrollShot = access_private_field::MainWindowm_isErrorWithScrollShot(*m_window);
    MainWindow_m_isErrorWithScrollShot = false;

    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    auto MainWindow_setCancelInputEvent = get_private_fun::MainWindowsetCancelInputEvent();
    stub.set(MainWindow_setCancelInputEvent, setCancelInputEvent_stub);

    call_private_fun::MainWindowonScrollShotMouseMoveEvent(*m_window, 500, 500);

    stub.reset(MainWindow_pauseAutoScrollShot);
    stub.reset(MainWindow_setCancelInputEvent);

    //delete MainWindow_m_scrollShotTip;
    //delete MainWindow_shotButton;
//    delete MainWindow_toolBar;
}

ACCESS_PRIVATE_FUN(MainWindow, void(int mouseTime, int direction, int x, int y), onScrollShotMouseScrollEvent);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_autoScrollFlagLast);
bool handleManualScrollShot_stub(void *obj)
{
    return true;
}
bool startManualScrollShot_stub(void *obj)
{
    return true;
}
//滚动截图时处理鼠标滚轮滚动,单元测试用例
TEST_F(MainWindowTest, onScrollShotMouseScrollEvent)
{

    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;

    int &MainWindow_m_autoScrollFlagNext =  access_private_field::MainWindowm_autoScrollFlagNext(*m_window);
    MainWindow_m_autoScrollFlagNext = 1;

    int &MainWindow_m_autoScrollFlagLast =  access_private_field::MainWindowm_autoScrollFlagLast(*m_window);
    MainWindow_m_autoScrollFlagLast = 1;

    int &MainWindow_m_scrollShotType =  access_private_field::MainWindowm_scrollShotType(*m_window);
    MainWindow_m_scrollShotType = 1;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*m_window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*m_window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*m_window);
    MainWindow_shotButton = new DPushButton();

    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    auto MainWindow_startManualScrollShot = get_private_fun::MainWindowstartManualScrollShot();
    stub.set(MainWindow_startManualScrollShot, startManualScrollShot_stub);

    auto MainWindow_setInputEvent = get_private_fun::MainWindowsetInputEvent();
    stub.set(MainWindow_setInputEvent, setInputEvent_stub);

    auto MainWindow_handleManualScrollShot = get_private_fun::MainWindowhandleManualScrollShot();
    stub.set(MainWindow_handleManualScrollShot, handleManualScrollShot_stub);

    call_private_fun::MainWindowonScrollShotMouseScrollEvent(*m_window, 1, 5, 500, 500);

    stub.reset(MainWindow_pauseAutoScrollShot);
    stub.reset(MainWindow_startManualScrollShot);
    stub.reset(MainWindow_setInputEvent);
    stub.reset(MainWindow_handleManualScrollShot);

    //delete MainWindow_m_scrollShotTip;
    //delete MainWindow_shotButton;
//    delete MainWindow_toolBar;
}

ACCESS_PRIVATE_FUN(MainWindow, void(QDBusMessage msg), onLockScreenEvent);
//滚动截图时，锁屏处理事件，单元测试用例
TEST_F(MainWindowTest, onLockScreenEvent)
{
    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*m_window);
    MainWindow_m_scrollShotStatus = 1;

    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    QDBusMessage msg ;
    QList<QVariant> arguments;
    arguments.append("test");
    msg.setArguments(arguments);
    call_private_fun::MainWindowonLockScreenEvent(*m_window, msg);
    stub.reset(MainWindow_pauseAutoScrollShot);

}

ACCESS_PRIVATE_FUN(MainWindow, void(), onOpenScrollShotHelp);
//打开截图录屏帮助文档并定位到滚动截图 单元测试
TEST_F(MainWindowTest, onOpenScrollShotHelp)
{
    call_private_fun::MainWindowonOpenScrollShotHelp(*m_window);
}

ACCESS_PRIVATE_FUN(MainWindow, void(PixMergeThread::MergeErrorValue state), onScrollShotMerageImgState);
ACCESS_PRIVATE_FUN(MainWindow, QPixmap(const QRect &rect), getPixmapofRect);
ACCESS_PRIVATE_FIELD(MainWindow, QTimer *, m_tipShowtimer);
bool showAdjustArea_stub(void *obj)
{
    return true;
}
QPoint getScrollShotTipPosition_stub(void *obj)
{
    return QPoint(500, 500);
}
QPixmap getPixmapofRect_stub(void *obj, const QRect &rect)
{
    QPixmap pixmap;
    return pixmap;
}
bool setBackgroundPixmap_stub(void *obj)
{
    return true;
}
//滚动截图时，获取拼接时的状态  单元测试用例
TEST_F(MainWindowTest, onScrollShotMerageImgState)
{
    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*m_window);
    MainWindow_m_scrollShotStatus = 1;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*m_window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);
    stub.set(ADDR(ScrollShotTip, setBackgroundPixmap), setBackgroundPixmap_stub);

    auto MainWindow_setCancelInputEvent = get_private_fun::MainWindowsetCancelInputEvent();
    stub.set(MainWindow_setCancelInputEvent, setCancelInputEvent_stub);

    auto MainWindow_showAdjustArea = get_private_fun::MainWindowshowAdjustArea();
    stub.set(MainWindow_showAdjustArea, showAdjustArea_stub);

    auto MainWindow_getScrollShotTipPosition = get_private_fun::MainWindowgetScrollShotTipPosition();
    stub.set(MainWindow_getScrollShotTipPosition, getScrollShotTipPosition_stub);


    auto MainWindow_getPixmapofRect = get_private_fun::MainWindowgetPixmapofRect();
    stub.set(MainWindow_getPixmapofRect, getPixmapofRect_stub);

    QTimer *&MainWindow_m_tipShowtimer = access_private_field::MainWindowm_tipShowtimer(*m_window);
    MainWindow_m_tipShowtimer = new QTimer();

    call_private_fun::MainWindowonScrollShotMerageImgState(*m_window, PixMergeThread::MergeErrorValue::Failed);

    stub.reset(MainWindow_pauseAutoScrollShot);
    stub.reset(ADDR(ScrollShotTip, setBackgroundPixmap));
    stub.reset(MainWindow_setCancelInputEvent);
    stub.reset(MainWindow_showAdjustArea);
    stub.reset(MainWindow_getScrollShotTipPosition);
    stub.reset(MainWindow_getPixmapofRect);

    //delete MainWindow_m_scrollShotTip;
    delete MainWindow_m_tipShowtimer;
}

ACCESS_PRIVATE_FIELD(MainWindow, QRect, m_adjustArea);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isAdjustArea);
ACCESS_PRIVATE_FUN(MainWindow, void(), onAdjustCaptureArea);
bool clearPixmap_stub(void *obj)
{
    return true;
}
//自动调整捕捉区域的大小及位置 单元测试用例
TEST_F(MainWindowTest, onAdjustCaptureArea)
{
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*m_window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*m_window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*m_window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*m_window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*m_window);
    MainWindow_m_pixelRatio = 1.0;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*m_window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    QRect &MainWindow_m_adjustArea = access_private_field::MainWindowm_adjustArea(*m_window);
    MainWindow_m_adjustArea = QRect(0, 0, 1920, 1080);

    bool &MainWindow_m_isAdjustArea = access_private_field::MainWindowm_isAdjustArea(*m_window);
    MainWindow_m_isAdjustArea = false;

    TopTips *&MainWindow_scrollShotSizeTips = access_private_field::MainWindowm_scrollShotSizeTips(*m_window);
    MainWindow_scrollShotSizeTips = new TopTips();

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*m_window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*m_window);
    MainWindow_shotButton = new DPushButton();

    QRect previewRecordRect {
        static_cast<int>(0),
        static_cast<int>(0),
        static_cast<int>(1920),
        static_cast<int>(1080)
    };
    PreviewWidget *&MainWindow_previewWidget = access_private_field::MainWindowm_previewWidget(*m_window);
    MainWindow_previewWidget = new PreviewWidget(previewRecordRect);
    MainWindow_previewWidget->setScreenInfo(MainWindow_screenWidth, MainWindow_m_pixelRatio);
    MainWindow_previewWidget->initPreviewWidget();

    bool &MainWindow_m_isAutoScrollShotStart = access_private_field::MainWindowm_isAutoScrollShotStart(*m_window);
    MainWindow_m_isAutoScrollShotStart = false;

    bool &MainWindow_isZhaoxin =  access_private_field::MainWindowm_isZhaoxin(*m_window);
    MainWindow_isZhaoxin = true;

    auto MainWindow_updateToolBarPos = get_private_fun::MainWindowupdateToolBarPos();
    stub.set(MainWindow_updateToolBarPos, updateToolBarPos_stub);

    auto MainWindow_updateShotButtonPos = get_private_fun::MainWindowupdateShotButtonPos();
    stub.set(MainWindow_updateShotButtonPos, updateShotButtonPos_stub);

    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*m_window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, clearPixmap), clearPixmap_stub);

    call_private_fun::MainWindowonAdjustCaptureArea(*m_window);

    stub.reset(MainWindow_updateToolBarPos);
    stub.reset(MainWindow_updateShotButtonPos);
    stub.reset(ADDR(ScrollScreenshot, clearPixmap));

    //delete MainWindow_m_scrollShotTip;
    //delete MainWindow_shotButton;
    delete MainWindow_scrollShotSizeTips;
//    delete MainWindow_toolBar;
//    delete MainWindow_previewWidget;
    delete MainWindow_m_scrollShot;

}
