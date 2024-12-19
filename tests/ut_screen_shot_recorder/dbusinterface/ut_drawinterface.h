// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDBusConnection>
#include <QScreen>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <gtest/gtest.h>
#include "../dbusinterface/drawinterface.h"


using namespace testing;

class DrawInterfaceTest:public testing::Test{

public:
    DrawInterface *m_draw;
    QList<QImage> list;
    virtual void SetUp() override{
        std::cout << "start DrawInterfaceTest" << std::endl;
        m_draw = new DrawInterface("com.deepin.Draw", "/com/deepin/Draw", QDBusConnection::sessionBus(), nullptr);
    }

    virtual void TearDown() override{
        delete m_draw;
        system("killall deepin-draw");
        std::cout << "end DrawInterfaceTest" << std::endl;
    }
};

TEST_F(DrawInterfaceTest, openFiles)
{

    QList<QString> list;
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
            + "/Wallpapers/desktop.jpg";
    qDebug() << desktopPath;
    list.append(desktopPath);
    m_draw->openFiles(list);
    sleep(1);

}

TEST_F(DrawInterfaceTest, openImages)
{
    QList<QImage> list;
    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 使用 0 作为窗口ID来抓取整个屏幕
    QPixmap pix = t_primaryScreen->grabWindow(0);
    list.append(pix.toImage());
    m_draw->openImages(list);
}

