#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QPoint>

#include "../../../src/main_window.h"
#include "stub.h"
#include "addr_pri.h"


using namespace testing;


ACCESS_PRIVATE_FIELD(MainWindow, ToolBar*, m_toolBar);
ACCESS_PRIVATE_FIELD(MainWindow, int,  m_screenCount);
ACCESS_PRIVATE_FIELD(MainWindow, ShapesWidget*, m_shapesWidget);
class MainWindowTest:public testing::Test{

public:
    Stub stub;
    virtual void SetUp() override{
        std::cout << "start MainWindowTest" << std::endl;
        ConfigSettings::instance()->setValue("save", "saveCursor", true);
    }

    virtual void TearDown() override{
        //delete m_window;
        std::cout << "end MainWindowTest" << std::endl;
        system("killall deepin-shortcut-viewer");

    }
};

TEST_F(MainWindowTest, screenShotShapes)
{
    MainWindow *m_window = new MainWindow;;

    m_window->initAttributes();
    m_window->initResource();
    m_window->initLaunchMode("screenShot");
    m_window->showFullScreen();

    access_private_field::MainWindowm_screenCount(*m_window) = 1;

    QEventLoop loop;

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10,10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(800,600));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,600));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,600));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(900,700));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(900,700));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(900,10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(800,20));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,20));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800,300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(900,300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(900,300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400,700));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(400,900));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400,900));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();




    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10,10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(20,30));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(20,30));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(20,50));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(30,50));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(30,50));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_window, Qt::Key_R);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    ShapesWidget *m_shapesWidget = access_private_field::MainWindowm_shapesWidget(*m_window);


    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400,200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(600,280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(600,280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(600,280));
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

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400,200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(600,280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(600,280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::keyClick(m_window, Qt::Key_O);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    ConfigSettings::instance()->setValue("oval", "is_blur", false);
    ConfigSettings::instance()->setValue("oval", "is_mosaic", true);

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(300,200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(500,280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(500,180));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_window, Qt::Key_L);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->changeArrowAndLineEvent(0);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100,200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(200,280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(200,280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->changeArrowAndLineEvent(1);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(120,210));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(250,360));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(250,360));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_window, Qt::Key_P);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(50,200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(320,280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(120,180));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(280,280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_T);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(60,60));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Z, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    m_window->saveScreenShot();

    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();

    //delete m_window;
}
TEST_F(MainWindowTest, screenShot)
{
    MainWindow *m_window = new MainWindow;;

    m_window->initAttributes();
    m_window->initResource();
    m_window->initLaunchMode("screenShot");
    m_window->showFullScreen();


    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mouseMove(m_window, QPoint(960,480));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(100,100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100,100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(250,200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(250,200));
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

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(200,200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(300,300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(300,300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    m_window->changeFunctionButton("shot");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_R);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(350,300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(400,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_O);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(355,300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(400,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_L);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(350,300));
    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(360,300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_P);

    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(365,300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(400,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400,400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_window, Qt::Key_T);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(370,300));
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

    m_window->saveScreenShot();


    QTest::mouseClick(m_window, Qt::MouseButton::RightButton, Qt::KeyboardModifier::NoModifier, QPoint(400,200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400,250));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();

    //delete m_window;
}

TEST_F(MainWindowTest, screenRecord)
{
    MainWindow *m_window = new MainWindow;

    m_window->initAttributes();
    m_window->initResource();
    m_window->initLaunchMode("screenRecord");
    m_window->showFullScreen();


    ToolBar *m_toolBar = access_private_field::MainWindowm_toolBar(*m_window);

    QEventLoop loop;


    QTest::mouseMove(m_window, QPoint(0,0));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0,0));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_window, QPoint(1400,1050));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1400,1050));
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
    loop.exec();

    //delete m_window;
}

static bool hasComposite_stub(void * obj) {
    return false;
}


static QString CpuArchitecture_stub(void* obj)
{
    return  "mips";
}
TEST_F(MainWindowTest, onHelp)
{
    MainWindow *m_window = new MainWindow;
    m_window->onHelp();
/*
    stub.set(ADDR(DWindowManagerHelper, hasComposite), hasComposite_stub);
    stub.set(ADDR(QSysInfo, currentCpuArchitecture), CpuArchitecture_stub);
    m_window->compositeChanged();
    stub.reset(ADDR(DWindowManagerHelper, hasComposite));
    stub.reset(ADDR(QSysInfo, currentCpuArchitecture));
*/
   delete m_window;
}
