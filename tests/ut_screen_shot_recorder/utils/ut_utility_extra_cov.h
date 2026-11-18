// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>

#include <QImage>
#include <QPainter>

#include "../../src/utils.h"
#include "../../src/utils/pixmergethread.h"
#include "../../src/utils/shapesutils.h"

using namespace testing;

class TestPixMergeThread : public PixMergeThread
{
public:
    using PixMergeThread::detectDirection;
};

TEST(UtilsExtraCovTest, effectAnnotationImageVariants)
{
    QImage image(64, 48, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(20, 40, 60, 128));
    EXPECT_NO_FATAL_FAILURE((void)effectAnnotationImage(image, true, 10));
    EXPECT_NO_FATAL_FAILURE((void)effectAnnotationImage(image, false, 10));
    EXPECT_NO_FATAL_FAILURE((void)effectAnnotationImage(QImage(), true, 10));
}

TEST(PixMergeThreadExtraCovTest, directionAndPendingQueue)
{
    TestPixMergeThread merge;
    EXPECT_EQ(merge.pendingImageCount(), 0);
    EXPECT_NO_FATAL_FAILURE(merge.dropOldestPending());

    cv::Mat first(20, 20, CV_8UC4, cv::Scalar(10, 20, 30, 255));
    PixMergeThread::PictureDirection direction = PixMergeThread::ScrollDown;
    EXPECT_NO_FATAL_FAILURE((void)merge.detectDirection(first, &direction));
    EXPECT_NO_FATAL_FAILURE((void)merge.detectDirection(first, nullptr));
}
