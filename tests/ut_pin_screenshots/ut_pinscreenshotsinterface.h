// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PINSCREENSHOTSINTERFACE_H
#define UT_PINSCREENSHOTSINTERFACE_H
#include <gtest/gtest.h>
#include <QDBusConnection>
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

#endif // UT_PINSCREENSHOTSINTERFACE_H
