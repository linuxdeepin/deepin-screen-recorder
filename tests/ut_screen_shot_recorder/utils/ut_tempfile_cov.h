// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QPixmap>
#include <gtest/gtest.h>
#include "../../src/utils/tempfile.h"

using namespace testing;

// Coverage tests for TempFile. The existing ut_tempfile.h covers the basic
// set/get for fullscreen/blur/mosaic. Here we cover isContainsPix and the
// early-return ("already contains radius") branches of setBlurPixmap /
// setMosaicPixmap.
class TempFileCovTest : public testing::Test
{
public:
    TempFile *tf = nullptr;
    QPixmap pix;
    void SetUp() override
    {
        tf = TempFile::instance();
        pix = QPixmap(20, 20);
        pix.fill(Qt::red);
    }
    void TearDown() override {}
};

// isContainsPix with a non-existent radius returns false for both effect types.
TEST_F(TempFileCovTest, isContainsPixBlurAbsent)
{
    EXPECT_FALSE(tf->isContainsPix(QStringLiteral("blur"), 777));
}

TEST_F(TempFileCovTest, isContainsPixMosaicAbsent)
{
    EXPECT_FALSE(tf->isContainsPix(QStringLiteral("mosaic"), 778));
}

// After setting a blur pixmap, isContainsPix("blur", r) flips to true.
TEST_F(TempFileCovTest, isContainsPixBlurPresentAfterSet)
{
    const int r = 12345;
    tf->setBlurPixmap(pix, r);
    EXPECT_TRUE(tf->isContainsPix(QStringLiteral("blur"), r));
    EXPECT_NO_FATAL_FAILURE(tf->getBlurPixmap(r));
}

// After setting a mosaic pixmap, isContainsPix for non-"blur" returns true.
TEST_F(TempFileCovTest, isContainsPixMosaicPresentAfterSet)
{
    const int r = 54321;
    tf->setMosaicPixmap(pix, r);
    EXPECT_TRUE(tf->isContainsPix(QStringLiteral("mosaic"), r));
    EXPECT_NO_FATAL_FAILURE(tf->getMosaicPixmap(r));
}

// setBlurPixmap with a radius that already exists is an early no-op (the
// "already contains" branch).
TEST_F(TempFileCovTest, setBlurPixmapDuplicateRadiusIsNoop)
{
    const int r = 222;
    tf->setBlurPixmap(pix, r);                 // insert
    EXPECT_NO_FATAL_FAILURE(tf->setBlurPixmap(pix, r)); // duplicate -> early return
    EXPECT_TRUE(tf->isContainsPix(QStringLiteral("blur"), r));
}

TEST_F(TempFileCovTest, setMosaicPixmapDuplicateRadiusIsNoop)
{
    const int r = 333;
    tf->setMosaicPixmap(pix, r);
    EXPECT_NO_FATAL_FAILURE(tf->setMosaicPixmap(pix, r));
    EXPECT_TRUE(tf->isContainsPix(QStringLiteral("mosaic"), r));
}

// isContainsPix with an unknown effect string falls through to the mosaic branch
// (else-branch of the if). Must not crash.
TEST_F(TempFileCovTest, isContainsPixUnknownEffectFallsToMosaicBranch)
{
    EXPECT_NO_FATAL_FAILURE(tf->isContainsPix(QStringLiteral("unknown"), 444));
}

// instance() is a singleton.
TEST_F(TempFileCovTest, instanceIsSingleton)
{
    EXPECT_EQ(tf, TempFile::instance());
}

// getFullscreenPixmap default is null before any set.
TEST_F(TempFileCovTest, setFullScreenPixmapAndRetrieve)
{
    tf->setFullScreenPixmap(pix);
    QPixmap got = tf->getFullscreenPixmap();
    EXPECT_EQ(pix.size(), got.size());
}
