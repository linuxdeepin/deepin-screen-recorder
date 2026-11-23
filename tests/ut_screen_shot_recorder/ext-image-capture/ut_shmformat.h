// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSignalSpy>
#include <QTemporaryFile>
#include <gtest/gtest.h>

#include "ext-image-capture/capturepipelineworker.h"
#include "ext-image-capture/shmformat.h"

TEST(ShmFormatTest, ValidatesProtocolLayout)
{
    using namespace Dsr::ExtImageCapture;
    EXPECT_EQ(strideForShm(WL_SHM_FORMAT_BGR888, 1366), 4104);
    EXPECT_EQ(strideForShm(WL_SHM_FORMAT_BGR888, 3840), 11520);
    EXPECT_EQ(strideForShm(WL_SHM_FORMAT_XRGB8888, 1366), 5464);
    EXPECT_EQ(strideForShm(WL_SHM_FORMAT_BGR888, INT32_MAX), 0);
    EXPECT_EQ(bufferSizeForShm(4104, INT32_MAX), 0);
    EXPECT_EQ(bufferSizeForShm(4104, 0), 0);
}

TEST(ShmFormatTest, ReadsRedAndBlueAcrossPaddedScanlines)
{
    using namespace Dsr::ExtImageCapture;
    const int stride = strideForShm(WL_SHM_FORMAT_BGR888, 5);
    ASSERT_EQ(stride, 24);
    QByteArray pixels(stride * 2, '\0');
    pixels[stride - 1] = '\x7e';
    pixels[0] = '\xff';
    pixels[stride + 2] = '\xff';

    const QImage rgb = shmImageView(pixels.constData(), pixels.size(), 5, 2, stride,
                                   WL_SHM_FORMAT_BGR888);
    ASSERT_FALSE(rgb.isNull());
    EXPECT_EQ(rgb.pixelColor(0, 0), QColor(Qt::red));
    EXPECT_EQ(rgb.pixelColor(0, 1), QColor(Qt::blue));
    const QImage bgr = shmImageView(pixels.constData(), pixels.size(), 5, 2, stride,
                                   WL_SHM_FORMAT_RGB888);
    ASSERT_FALSE(bgr.isNull());
    EXPECT_EQ(bgr.pixelColor(0, 0), QColor(Qt::blue));
    EXPECT_EQ(bgr.pixelColor(0, 1), QColor(Qt::red));
    EXPECT_TRUE(shmImageView(pixels.constData(), pixels.size() - 1, 5, 2, stride,
                             WL_SHM_FORMAT_BGR888).isNull());
}

TEST(ShmFormatTest, RecordingConvertsPaddedBg24ToPackedRgba)
{
    constexpr int stride = 24;
    QByteArray pixels(stride * 2, '\0');
    pixels[0] = '\xff';
    pixels[stride + 2] = '\xff';

    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    ASSERT_EQ(file.write(pixels), pixels.size());
    ASSERT_TRUE(file.flush());

    ShmFrameSourcePtr source;
    QString error;
    ASSERT_TRUE(makeShmFrameSource(file.handle(), pixels.size(), 5, 2, stride,
                                  WL_SHM_FORMAT_BGR888, 123, 1, &source, &error))
            << error.toStdString();
    file.close();
    CapturePipelineWorker worker;
    QSignalSpy ready(&worker, &CapturePipelineWorker::singleFrameReady);
    worker.convertSingleShm(source, 1);

    ASSERT_EQ(ready.size(), 1);
    const auto &frame = ready.first();
    EXPECT_EQ(frame.at(3).toInt(), 20);
    const QByteArray rgba = frame.at(0).toByteArray();
    ASSERT_EQ(rgba.size(), 40);
    EXPECT_EQ(rgba.mid(0, 4).toHex(), QByteArray("ff0000ff"));
    EXPECT_EQ(rgba.mid(20, 4).toHex(), QByteArray("0000ffff"));
}

TEST(ShmFormatTest, RecordingConverts32BitChannelOrderAndOpaqueX)
{
    using namespace Dsr::ExtImageCapture;
    alignas(4) const uchar pixel[] = {0x12, 0x34, 0x56, 0};
    EXPECT_EQ(shmToRgba(pixel, sizeof(pixel), 1, 1, 4, WL_SHM_FORMAT_XRGB8888).toHex(),
              QByteArray("563412ff"));
    EXPECT_EQ(shmToRgba(pixel, sizeof(pixel), 1, 1, 4, WL_SHM_FORMAT_XBGR8888).toHex(),
              QByteArray("123456ff"));
}

TEST(ShmFormatTest, MultiScreenRecordingCombinesDifferentShmFormats)
{
    const QRect left(0, 0, 2, 2);
    const QRect right(2, 0, 2, 2);
    const QByteArray bg24 = QByteArray::fromHex("ff0000ff0000000000000000").repeated(2);
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    ASSERT_EQ(file.write(bg24), bg24.size());
    ASSERT_TRUE(file.flush());

    FrameInput first;
    first.screenName = QStringLiteral("left");
    first.screenGeometry = left;
    first.width = 2;
    first.height = 2;
    first.stride = 12;
    ASSERT_TRUE(makeShmFrameSource(file.handle(), bg24.size(), 2, 2, 12,
                                  WL_SHM_FORMAT_BGR888, 1, 1, &first.shm, nullptr));
    FrameInput second;
    second.screenName = QStringLiteral("right");
    second.screenGeometry = right;
    second.width = 2;
    second.height = 2;
    second.stride = 8;
    second.cpuShmFormat = WL_SHM_FORMAT_XRGB8888;
    second.cpuData = QByteArray::fromHex("ff000000").repeated(4);

    CapturePipelineWorker worker;
    worker.configureMulti(QSize(4, 2), 1, QRect(), false,
                          {{first.screenName, left}, {second.screenName, right}});
    QSignalSpy ready(&worker, &CapturePipelineWorker::multiFrameReady);
    worker.processMultiRound(1, {first, second});
    if (ready.isEmpty())
        ASSERT_TRUE(ready.wait(1000));
    ASSERT_EQ(ready.size(), 1);
    const auto &frame = ready.first();
    EXPECT_EQ(frame.at(3).toInt(), 16);
    EXPECT_EQ(frame.at(0).toByteArray().toHex(),
              QByteArray("ff0000ffff0000ff0000ffff0000ffff").repeated(2));
}
