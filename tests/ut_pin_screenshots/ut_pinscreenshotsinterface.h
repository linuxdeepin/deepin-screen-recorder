// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PINSCREENSHOTSINTERFACE_H
#define UT_PINSCREENSHOTSINTERFACE_H
#include <gtest/gtest.h>
#include <QDBusConnection>
#include <QImage>
#include <QPoint>
#include "service/pinscreenshotsinterface.h"

TEST_F(PinScreenShotsTest, interfaceConstruction)
{
    PinScreenShotsInterface *iface = new PinScreenShotsInterface(
        "com.deepin.PinScreenShots", "/com/deepin/PinScreenShots",
        QDBusConnection::sessionBus());
    EXPECT_NE(iface, nullptr);
    delete iface;
}

TEST_F(PinScreenShotsTest, interfaceStaticName)
{
    EXPECT_STREQ(PinScreenShotsInterface::staticInterfaceName(), "com.deepin.PinScreenShots");
}

TEST_F(PinScreenShotsTest, interfaceOpenImage)
{
    PinScreenShotsInterface iface("com.deepin.PinScreenShots",
                                  "/com/deepin/PinScreenShots",
                                  QDBusConnection::sessionBus());
    QImage img(8, 8, QImage::Format_ARGB32);
    img.fill(Qt::red);
    iface.openImage(img);
    SUCCEED();
}

TEST_F(PinScreenShotsTest, interfaceOpenImageNull)
{
    PinScreenShotsInterface iface("com.deepin.PinScreenShots",
                                  "/com/deepin/PinScreenShots",
                                  QDBusConnection::sessionBus());
    QImage img;
    iface.openImage(img);
    SUCCEED();
}

TEST_F(PinScreenShotsTest, interfaceOpenImageAndName)
{
    PinScreenShotsInterface iface("com.deepin.PinScreenShots",
                                  "/com/deepin/PinScreenShots",
                                  QDBusConnection::sessionBus());
    QImage img(16, 16, QImage::Format_ARGB32);
    img.fill(Qt::green);
    iface.openImageAndName(img, QStringLiteral("shot.png"), QPoint(5, 5));
    SUCCEED();
}

TEST_F(PinScreenShotsTest, interfaceOpenImageAndNameNull)
{
    PinScreenShotsInterface iface("com.deepin.PinScreenShots",
                                  "/com/deepin/PinScreenShots",
                                  QDBusConnection::sessionBus());
    QImage img;
    iface.openImageAndName(img, QStringLiteral("empty.png"), QPoint(0, 0));
    SUCCEED();
}

TEST_F(PinScreenShotsTest, interfaceOpenFile)
{
    PinScreenShotsInterface iface("com.deepin.PinScreenShots",
                                  "/com/deepin/PinScreenShots",
                                  QDBusConnection::sessionBus());
    iface.openFile(QStringLiteral("/tmp/test_screenshot.png"));
    SUCCEED();
}

#endif // UT_PINSCREENSHOTSINTERFACE_H
