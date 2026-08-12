// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DBUSADAPTOR_H
#define UT_DBUSADAPTOR_H
#include <gtest/gtest.h>
#include <QObject>
#include "service/dbuspinscreenshotsadaptor.h"

TEST_F(PinScreenShotsTest, adaptorConstruction)
{
    QObject parent;
    DbusPinScreenShotsAdaptor *adaptor = new DbusPinScreenShotsAdaptor(&parent);
    EXPECT_NE(adaptor, nullptr);
    delete adaptor;
}

TEST_F(PinScreenShotsTest, adaptorParentSet)
{
    QObject *parent = new QObject();
    DbusPinScreenShotsAdaptor *adaptor = new DbusPinScreenShotsAdaptor(parent);
    EXPECT_EQ(adaptor->parent(), parent);
    delete parent;
}

#endif // UT_DBUSADAPTOR_H
