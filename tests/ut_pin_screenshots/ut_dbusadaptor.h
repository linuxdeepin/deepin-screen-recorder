// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DBUSADAPTOR_H
#define UT_DBUSADAPTOR_H
#include <gtest/gtest.h>
#include <QObject>
#include <QBuffer>
#include <QColor>
#include <QDBusUnixFileDescriptor>
#include <QImage>
#include <QPoint>
#include <QRect>
#include <QString>
#include "ut_pin_screenshots.h"
#include "ut_pin_geometry_receiver.h"
#include "service/dbuspinscreenshotsadaptor.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static inline QByteArray encodeImageForAdaptor(const QImage &img)
{
    QByteArray raw;
    QBuffer stream(&raw);
    img.save(&stream, "PNG");
    return qCompress(raw, 9).toBase64();
}

namespace {

QByteArray pngBytesForAdaptor(const QImage &img)
{
    QByteArray raw;
    QBuffer stream(&raw);
    img.save(&stream, "PNG");
    return raw;
}

// Creates a sealed memfd containing @p data, matching what the sender writes.
// Returns -1 when the platform does not support sealing.
int createSealedMemfdForAdaptor(const QByteArray &data, bool seal)
{
    const int fd = memfd_create("ut-pin-adaptor", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (fd < 0)
        return -1;
    if (!data.isEmpty()) {
        const ssize_t written = write(fd, data.constData(), static_cast<size_t>(data.size()));
        if (written != data.size()) {
            close(fd);
            return -1;
        }
    }
    if (seal && fcntl(fd, F_ADD_SEALS, F_SEAL_WRITE | F_SEAL_GROW | F_SEAL_SHRINK) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

} // namespace

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

TEST_F(PinScreenShotsTest, adaptorAcceptsSealedPngDescriptor)
{
    QImage img(32, 24, QImage::Format_ARGB32);
    img.fill(Qt::red);
    const int fd = createSealedMemfdForAdaptor(pngBytesForAdaptor(img), true);
    ASSERT_GE(fd, 0);

    PinGeometryReceiver receiver;
    DbusPinScreenShotsAdaptor adaptor(&receiver);
    const QRect geometry(120, 80, 64, 48);
    EXPECT_TRUE(adaptor.openImageAtGeometry(QDBusUnixFileDescriptor(fd),
                                            QStringLiteral("pin.png"), geometry));
    EXPECT_EQ(receiver.calls, 1);
    EXPECT_EQ(receiver.receivedName, QStringLiteral("pin.png"));
    EXPECT_EQ(receiver.receivedGeometry, geometry);
    EXPECT_EQ(receiver.receivedImage.size(), img.size());
    EXPECT_EQ(receiver.receivedImage.pixelColor(0, 0), QColor(Qt::red));
}

TEST_F(PinScreenShotsTest, adaptorRejectsUnsealedDescriptor)
{
    QImage img(16, 16, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    const int fd = createSealedMemfdForAdaptor(pngBytesForAdaptor(img), false);
    ASSERT_GE(fd, 0);

    PinGeometryReceiver receiver;
    DbusPinScreenShotsAdaptor adaptor(&receiver);
    EXPECT_FALSE(adaptor.openImageAtGeometry(QDBusUnixFileDescriptor(fd),
                                             QStringLiteral("pin.png"), QRect(0, 0, 16, 16)));
    EXPECT_EQ(receiver.calls, 0);
}

TEST_F(PinScreenShotsTest, adaptorRejectsInvalidDescriptor)
{
    PinGeometryReceiver receiver;
    DbusPinScreenShotsAdaptor adaptor(&receiver);
    EXPECT_FALSE(adaptor.openImageAtGeometry(QDBusUnixFileDescriptor(),
                                             QStringLiteral("pin.png"), QRect(0, 0, 16, 16)));
    EXPECT_EQ(receiver.calls, 0);
}

TEST_F(PinScreenShotsTest, adaptorRejectsEmptyDescriptorFile)
{
    const int fd = createSealedMemfdForAdaptor(QByteArray(), true);
    ASSERT_GE(fd, 0);

    PinGeometryReceiver receiver;
    DbusPinScreenShotsAdaptor adaptor(&receiver);
    EXPECT_FALSE(adaptor.openImageAtGeometry(QDBusUnixFileDescriptor(fd),
                                             QStringLiteral("pin.png"), QRect(0, 0, 16, 16)));
    EXPECT_EQ(receiver.calls, 0);
}

TEST_F(PinScreenShotsTest, adaptorRejectsNonImageDescriptor)
{
    const int fd = createSealedMemfdForAdaptor(QByteArrayLiteral("not a png"), true);
    ASSERT_GE(fd, 0);

    PinGeometryReceiver receiver;
    DbusPinScreenShotsAdaptor adaptor(&receiver);
    EXPECT_FALSE(adaptor.openImageAtGeometry(QDBusUnixFileDescriptor(fd),
                                             QStringLiteral("pin.png"), QRect(0, 0, 16, 16)));
    EXPECT_EQ(receiver.calls, 0);
}

TEST_F(PinScreenShotsTest, adaptorRejectsNonRegularDescriptor)
{
    const int fd = open("/dev/null", O_RDONLY | O_CLOEXEC);
    ASSERT_GE(fd, 0);

    PinGeometryReceiver receiver;
    DbusPinScreenShotsAdaptor adaptor(&receiver);
    EXPECT_FALSE(adaptor.openImageAtGeometry(QDBusUnixFileDescriptor(fd),
                                             QStringLiteral("pin.png"), QRect(0, 0, 16, 16)));
    EXPECT_EQ(receiver.calls, 0);
}

TEST_F(PinScreenShotsTest, adaptorAcceptsLargeImageDescriptor)
{
    // 4K screenshots must not be rejected by any size heuristic.
    QImage img(3840, 2160, QImage::Format_ARGB32);
    img.fill(Qt::darkCyan);
    const int fd = createSealedMemfdForAdaptor(pngBytesForAdaptor(img), true);
    ASSERT_GE(fd, 0);

    PinGeometryReceiver receiver;
    DbusPinScreenShotsAdaptor adaptor(&receiver);
    EXPECT_TRUE(adaptor.openImageAtGeometry(QDBusUnixFileDescriptor(fd),
                                            QStringLiteral("4k.png"), QRect(0, 0, 1920, 1080)));
    EXPECT_EQ(receiver.calls, 1);
    EXPECT_EQ(receiver.receivedImage.size(), img.size());
}

TEST_F(PinScreenShotsTest, adaptorRejectsInvalidGeometry)
{
    QImage img(16, 16, QImage::Format_ARGB32);
    img.fill(Qt::green);
    const int fd = createSealedMemfdForAdaptor(pngBytesForAdaptor(img), true);
    ASSERT_GE(fd, 0);

    PinGeometryReceiver receiver;
    DbusPinScreenShotsAdaptor adaptor(&receiver);
    EXPECT_FALSE(adaptor.openImageAtGeometry(QDBusUnixFileDescriptor(fd),
                                             QStringLiteral("pin.png"), QRect()));
    EXPECT_EQ(receiver.calls, 0);
}

#endif // UT_DBUSADAPTOR_H
