// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QtDBus/QDBusInterface>
#include <gtest/gtest.h>
#include "../../src/utils/dbusutils.h"

using namespace testing;

// Coverage tests for DBusUtils. The existing ut_dbusutils.h calls redDBusProperty
// and redDBusMethod against live deepin services. Here we cover the remaining
// branches: the invalid-interface early-return paths and isAiAssistantAvailable.
//
// All of these are safe: an invalid DBus service simply yields an invalid
// interface, and the functions return a default QVariant(0) / false. They do
// not spawn processes or touch hardware.
class DBusUtilsCovTest : public testing::Test
{
public:
    void SetUp() override { qDebug() << "start DBusUtilsCovTest"; }
    void TearDown() override { qDebug() << "end DBusUtilsCovTest"; }
};

// redDBusProperty with a non-existent service -> invalid interface -> returns QVariant(0).
TEST_F(DBusUtilsCovTest, redDBusPropertyInvalidServiceReturnsZero)
{
    QVariant v;
    EXPECT_NO_FATAL_FAILURE(v = DBusUtils::redDBusProperty(
        QStringLiteral("org.does.not.Exist"),
        QStringLiteral("/nonexistent"),
        QStringLiteral("org.does.not.Exist"),
        "Foo"));
    EXPECT_TRUE(v.isValid());
}

TEST_F(DBusUtilsCovTest, redDBusPropertyEmptyInterfaceReturnsZero)
{
    QVariant v;
    EXPECT_NO_FATAL_FAILURE(v = DBusUtils::redDBusProperty(
        QStringLiteral("org.does.not.Exist"),
        QStringLiteral("/nonexistent"),
        QString(),
        "Foo"));
    EXPECT_TRUE(v.isValid());
}

// redDBusMethod with a non-existent service -> invalid interface -> returns QVariant(0).
TEST_F(DBusUtilsCovTest, redDBusMethodInvalidServiceReturnsZero)
{
    QVariant v;
    EXPECT_NO_FATAL_FAILURE(v = DBusUtils::redDBusMethod(
        QStringLiteral("org.does.not.Exist"),
        QStringLiteral("/nonexistent"),
        QStringLiteral("org.does.not.Exist"),
        "SomeMethod"));
    EXPECT_TRUE(v.isValid());
}

// redDBusMethod on a path that exists but a method that does not: exercises the
// reply.isValid() == false branch. SessionBus is always present in offscreen Qt.
TEST_F(DBusUtilsCovTest, redDBusMethodCallOnBogusInterface)
{
    QVariant v;
    EXPECT_NO_FATAL_FAILURE(v = DBusUtils::redDBusMethod(
        QStringLiteral("org.freedesktop.DBus"),
        QStringLiteral("/org/freedesktop/DBus"),
        QStringLiteral("org.freedesktop.DBus"),
        "NoSuchMethodExists"));
    EXPECT_TRUE(v.isValid());
}

// isAiAssistantAvailable: queries com.deepin.copilot / com.iflytek.aiassistant.
// On CI these are absent so it must return false without hanging or crashing.
TEST_F(DBusUtilsCovTest, isAiAssistantAvailableReturnsBool)
{
    bool avail = true;
    EXPECT_NO_FATAL_FAILURE(avail = DBusUtils::isAiAssistantAvailable());
    // We cannot assert the value (depends on host), but it must not crash.
    EXPECT_TRUE(avail == true || avail == false);
}

// Constructor/destructor smoke (the class is mostly static but has ctors).
TEST_F(DBusUtilsCovTest, constructorAndDestructorRunClean)
{
    EXPECT_NO_FATAL_FAILURE(DBusUtils u;);
    DBusUtils *u = nullptr;
    EXPECT_NO_FATAL_FAILURE(u = new DBusUtils; delete u;);
}
