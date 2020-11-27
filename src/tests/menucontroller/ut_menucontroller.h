#pragma once
#include <gtest/gtest.h>
#include <DMenu>
#include <QAction>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include "../menucontroller/menucontroller.h"


using namespace testing;
class MenuControllerTest:public testing::Test, public QObject{

public:
    MenuController *m_menuController;
    QString curAct = "";
    virtual void SetUp() override{
        std::cout << "start MenuControllerTest" << std::endl;
        m_menuController = new MenuController(nullptr);
        connect(m_menuController, &MenuController::saveAction, this, &MenuControllerTest::saveScreenShot);
        connect(m_menuController, &MenuController::closeAction, this, &MenuControllerTest::exitApp);
    }

    virtual void TearDown() override{
        delete  m_menuController;
        std::cout << "end MenuControllerTest" << std::endl;
    }
public slots:
    void exitApp();
    void saveScreenShot();
};

TEST_F(MenuControllerTest, showMenu)
{
    m_menuController->showMenu(QPoint(100, 100));
    //m_menuController->enterEvent(nullptr);
    m_menuController->setUndoEnable(true);
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
    qDebug() << curAct;
}
void MenuControllerTest::saveScreenShot()
{
    EXPECT_EQ(1, 1);
}
void MenuControllerTest::exitApp()
{
    EXPECT_EQ(1, 1);
}

