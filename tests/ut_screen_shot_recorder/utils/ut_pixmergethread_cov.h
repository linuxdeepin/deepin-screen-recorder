// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QPixmap>
#include <gtest/gtest.h>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/utils/pixmergethread.h"

using namespace testing;

// Coverage tests for PixMergeThread. The existing ut_pixmergethread.h covers
// the splicePictureUp/Down error branches, isOneWay, calculateTimeDiff,
// getMerageResult and the scroll-down/up queue flows. Here we cover the
// remaining safe surface: setIsLastImg, clearCurImg, addShotImg early-return,
// mergeImageWork (empty + default status), getTopFixedHigh/getBottomFixedHigh
// invalid-image guards, and getScrollChangeRectArea empty/mismatch guards.
//
// NOTE on ACCESS_PRIVATE_FUN: this file is compiled into the same translation
// unit as ut_pixmergethread.h (via test_all_interfaces.h). That file already
// declares getScrollChangeRectArea / qPixmapToCvMat / splicePictureDown /
// splicePictureUp. We therefore declare ONLY the accessors that are NOT yet
// declared anywhere: mergeImageWork, getTopFixedHigh, getBottomFixedHigh. For
// getScrollChangeRectArea we reuse the accessor already provided by
// ut_pixmergethread.h (the orchestrator includes it before this file, mirroring
// the ut_screengrabber.h -> ut_screengrabber_ext.h ordering).
ACCESS_PRIVATE_FUN(PixMergeThread, bool(const cv::Mat &, int), mergeImageWork);
ACCESS_PRIVATE_FUN(PixMergeThread, int(cv::Mat &, cv::Mat &), getTopFixedHigh);
ACCESS_PRIVATE_FUN(PixMergeThread, int(cv::Mat &, cv::Mat &), getBottomFixedHigh);

class PixMergeCovTest : public testing::Test
{
public:
    PixMergeThread *t = nullptr;
    void SetUp() override { t = new PixMergeThread(); }
    void TearDown() override { delete t; }
};

// setIsLastImg simply toggles a flag; safe to call.
TEST_F(PixMergeCovTest, setIsLastImgTrueFalse)
{
    EXPECT_NO_FATAL_FAILURE(t->setIsLastImg(true));
    EXPECT_NO_FATAL_FAILURE(t->setIsLastImg(false));
}

// clearCurImg releases the internal cv::Mat; calling twice is a no-op.
TEST_F(PixMergeCovTest, clearCurImgTwice)
{
    EXPECT_NO_FATAL_FAILURE(t->clearCurImg());
    EXPECT_NO_FATAL_FAILURE(t->clearCurImg());
}

// setScrollModel toggles the manual-scroll flag.
TEST_F(PixMergeCovTest, setScrollModelToggle)
{
    EXPECT_NO_FATAL_FAILURE(t->setScrollModel(true));
    EXPECT_NO_FATAL_FAILURE(t->setScrollModel(false));
}

// stopTask flips m_loopTask to false.
TEST_F(PixMergeCovTest, stopTaskSetsLoopFalse)
{
    EXPECT_NO_FATAL_FAILURE(t->stopTask());
}

// addShotImg when m_loopTask == false returns early without enqueueing.
TEST_F(PixMergeCovTest, addShotImgAfterStopIsEarlyReturn)
{
    t->stopTask();
    QPixmap pix(10, 10);
    pix.fill(Qt::blue);
    EXPECT_NO_FATAL_FAILURE(t->addShotImg(pix));
}

// addShotImg with an empty m_curImg seeds it from the first image (the
// "first image" branch) and returns early.
TEST_F(PixMergeCovTest, addShotImgFirstImageSeedsCurImg)
{
    QPixmap pix(8, 8);
    pix.fill(Qt::green);
    EXPECT_NO_FATAL_FAILURE(t->addShotImg(pix));
}

// mergeImageWork on an empty image returns false (guard branch).
TEST_F(PixMergeCovTest, mergeImageWorkEmptyReturnsFalse)
{
    cv::Mat empty;
    bool ok = true;
    EXPECT_NO_FATAL_FAILURE(ok = call_private_fun::PixMergeThreadmergeImageWork(*t, empty, PixMergeThread::ScrollDown));
    EXPECT_FALSE(ok);
}

// mergeImageWork with an invalid status falls into the default branch and
// returns false without splicing.
TEST_F(PixMergeCovTest, mergeImageWorkInvalidStatusReturnsFalse)
{
    cv::Mat m(60, 60, CV_8UC4, cv::Scalar(10, 20, 30, 255));
    bool ok = true;
    EXPECT_NO_FATAL_FAILURE(ok = call_private_fun::PixMergeThreadmergeImageWork(*t, m, 9999));
    EXPECT_FALSE(ok);
}

// getTopFixedHigh / getBottomFixedHigh: empty images -> guard returns 0.
TEST_F(PixMergeCovTest, getTopFixedHighEmptyImagesReturnsZero)
{
    cv::Mat a, b;
    int h = -1;
    EXPECT_NO_FATAL_FAILURE(h = call_private_fun::PixMergeThreadgetTopFixedHigh(*t, a, b));
    EXPECT_EQ(0, h);
}

TEST_F(PixMergeCovTest, getBottomFixedHighEmptyImagesReturnsZero)
{
    cv::Mat a, b;
    int h = -1;
    EXPECT_NO_FATAL_FAILURE(h = call_private_fun::PixMergeThreadgetBottomFixedHigh(*t, a, b));
    EXPECT_EQ(0, h);
}

// Mismatched dimensions -> guard returns 0.
TEST_F(PixMergeCovTest, getTopFixedHighMismatchedDimsReturnsZero)
{
    cv::Mat a(10, 10, CV_8UC4, cv::Scalar(0));
    cv::Mat b(20, 20, CV_8UC4, cv::Scalar(0));
    int h = -1;
    EXPECT_NO_FATAL_FAILURE(h = call_private_fun::PixMergeThreadgetTopFixedHigh(*t, a, b));
    EXPECT_EQ(0, h);
}

TEST_F(PixMergeCovTest, getBottomFixedHighMismatchedDimsReturnsZero)
{
    cv::Mat a(10, 10, CV_8UC4, cv::Scalar(0));
    cv::Mat b(20, 20, CV_8UC4, cv::Scalar(0));
    int h = -1;
    EXPECT_NO_FATAL_FAILURE(h = call_private_fun::PixMergeThreadgetBottomFixedHigh(*t, a, b));
    EXPECT_EQ(0, h);
}

// Identical images -> no change found -> returns 0 (the final return).
TEST_F(PixMergeCovTest, getTopFixedHighIdenticalImagesReturnsZero)
{
    cv::Mat a(10, 10, CV_8UC4, cv::Scalar(1, 2, 3, 4));
    cv::Mat b = a.clone();
    int h = -1;
    EXPECT_NO_FATAL_FAILURE(h = call_private_fun::PixMergeThreadgetTopFixedHigh(*t, a, b));
    EXPECT_EQ(0, h);
}

TEST_F(PixMergeCovTest, getBottomFixedHighIdenticalImagesReturnsZero)
{
    cv::Mat a(10, 10, CV_8UC4, cv::Scalar(1, 2, 3, 4));
    cv::Mat b = a.clone();
    int h = -1;
    EXPECT_NO_FATAL_FAILURE(h = call_private_fun::PixMergeThreadgetBottomFixedHigh(*t, a, b));
    EXPECT_EQ(0, h);
}

// A difference at a known row returns that row index (top).
TEST_F(PixMergeCovTest, getTopFixedHighFindsFirstChangedRow)
{
    cv::Mat a(10, 4, CV_8UC4, cv::Scalar(0));
    cv::Mat b(10, 4, CV_8UC4, cv::Scalar(0));
    b.at<cv::Vec4b>(3, 0) = cv::Vec4b(255, 255, 255, 255);
    int h = call_private_fun::PixMergeThreadgetTopFixedHigh(*t, a, b);
    EXPECT_EQ(3, h);
}

// getBottomFixedHigh with a change returns a non-negative index.
TEST_F(PixMergeCovTest, getBottomFixedHighFindsFirstChangedFromBottom)
{
    cv::Mat a(10, 4, CV_8UC4, cv::Scalar(0));
    cv::Mat b(10, 4, CV_8UC4, cv::Scalar(0));
    b.at<cv::Vec4b>(7, 0) = cv::Vec4b(9, 9, 9, 9);
    int h = call_private_fun::PixMergeThreadgetBottomFixedHigh(*t, a, b);
    EXPECT_GE(h, 0);
}

// getScrollChangeRectArea: reuse the accessor declared in ut_pixmergethread.h.
// (If that header is not included first this case will fail to compile; the
// orchestrator is expected to order the base file before _cov files, exactly
// as it does for ut_screengrabber.h -> ut_screengrabber_ext.h.)
TEST_F(PixMergeCovTest, getScrollChangeRectAreaEmptyReturnsInvalid)
{
    cv::Mat a, b;
    QRect r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::PixMergeThreadgetScrollChangeRectArea(*t, a, b));
    EXPECT_LT(r.width(), 0);
}

TEST_F(PixMergeCovTest, getScrollChangeRectAreaMismatchedDimsReturnsInvalid)
{
    cv::Mat a(10, 10, CV_8UC4, cv::Scalar(0));
    cv::Mat b(5, 5, CV_8UC4, cv::Scalar(0));
    QRect r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::PixMergeThreadgetScrollChangeRectArea(*t, a, b));
    EXPECT_LT(r.width(), 0);
}

// Identical images -> no change detected -> returns QRect(-1,-1,-1,-1).
TEST_F(PixMergeCovTest, getScrollChangeRectAreaNoChangeReturnsInvalid)
{
    cv::Mat a(20, 20, CV_8UC4, cv::Scalar(7, 8, 9, 10));
    cv::Mat b = a.clone();
    QRect r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::PixMergeThreadgetScrollChangeRectArea(*t, a, b));
    EXPECT_LT(r.width(), 0);
}

// Different images -> a valid change rect is returned.
TEST_F(PixMergeCovTest, getScrollChangeRectAreaFindsChangeRect)
{
    cv::Mat a(20, 20, CV_8UC4, cv::Scalar(0));
    cv::Mat b(20, 20, CV_8UC4, cv::Scalar(0));
    b(cv::Rect(5, 5, 6, 6)).setTo(cv::Scalar(255, 255, 255, 255));
    QRect r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::PixMergeThreadgetScrollChangeRectArea(*t, a, b));
    EXPECT_GE(r.width(), 0);
}

// === public helpers ===

TEST_F(PixMergeCovTest, calculateTimeDiffUpdatesState)
{
    EXPECT_NO_FATAL_FAILURE(t->calculateTimeDiff(1000));
    EXPECT_NO_FATAL_FAILURE(t->calculateTimeDiff(1000));
}

TEST_F(PixMergeCovTest, isOneWayWithNoDirectionIsTrue)
{
    // fresh thread: upCount == downCount == 0 -> one-way true
    EXPECT_TRUE(t->isOneWay());
}

TEST_F(PixMergeCovTest, getMerageResultEmptyWhenNoCurImg)
{
    t->clearCurImg();
    QImage img;
    EXPECT_NO_FATAL_FAILURE(img = t->getMerageResult());
    EXPECT_TRUE(img.isNull());
}
