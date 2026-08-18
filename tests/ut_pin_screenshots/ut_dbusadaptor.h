// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DBUSADAPTOR_H
#define UT_DBUSADAPTOR_H
#include <gtest/gtest.h>
#include <QObject>
#include <QBuffer>
#include <QImage>
#include <QPoint>
#include "service/dbuspinscreenshotsadaptor.h"

static inline QByteArray encodeImageForAdaptor(const QImage &img)
{
    QByteArray raw;
    QBuffer stream(&raw);
    img.save(&stream, "PNG");
    return qCompress(raw, 9).toBase64();
}

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

TEST_F(PinScreenShotsTest, adaptorOpenImage)
{
    QObject parent;
    DbusPinScreenShotsAdaptor adaptor(&parent);
    QImage img(8, 8, QImage::Format_ARGB32);
    img.fill(Qt::red);
    adaptor.openImage(encodeImageForAdaptor(img));
    SUCCEED();
}

TEST_F(PinScreenShotsTest, adaptorOpenImageEmptyData)
{
    QObject parent;
    DbusPinScreenShotsAdaptor adaptor(&parent);
    adaptor.openImage(QByteArray());
    SUCCEED();
}

TEST_F(PinScreenShotsTest, adaptorOpenImageAndName)
{
    QObject parent;
    DbusPinScreenShotsAdaptor adaptor(&parent);
    QImage img(16, 16, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    adaptor.openImageAndName(encodeImageForAdaptor(img), QStringLiteral("test.png"), QPoint(10, 20));
    SUCCEED();
}

TEST_F(PinScreenShotsTest, adaptorOpenImageAndNameEmptyData)
{
    QObject parent;
    DbusPinScreenShotsAdaptor adaptor(&parent);
    adaptor.openImageAndName(QByteArray(), QStringLiteral("empty.png"), QPoint(0, 0));
    SUCCEED();
}

TEST_F(PinScreenShotsTest, adaptorOpenFile)
{
    QObject parent;
    DbusPinScreenShotsAdaptor adaptor(&parent);
    bool ret = adaptor.openFile(QStringLiteral("/tmp/test_screenshot.png"));
    EXPECT_TRUE(ret);
}

#endif // UT_DBUSADAPTOR_H
