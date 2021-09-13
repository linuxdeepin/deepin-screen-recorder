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
    bool flag = call_private_fun::MainWindowisToolBarInShotArea(*m_window);

    EXPECT_EQ(flag, true);


}
