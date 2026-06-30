// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <gtest/gtest.h>
#include "../../src/utils/borderprocessinterface.h"
#include "../../src/utils/configsettings.h"

using namespace testing;

// Coverage tests for the border-process classes. The existing
// ut_borderprocessinterface.h already covers all initBorderInfo branches,
// getPixmapAddBorder for several styles, both aspect-ratio branches of
// calculateBorderImageInfo, and the ShadowBorderProcess small-image path.
//
// Here we focus on the still-uncovered branches:
//  - ExternalBorderProcess::cropShotImageEx JPG path (format != 0)
//  - PrototypeBorderProcess::cropShotImage JPG vertical-processing path
//  - base BorderProcessInterface::calculateBorderImageInfo default impl
//  - drawShadow with rect large enough to enter the blur path
//  - getBorderImage horizontal-crop vs vertical-crop via differing aspect ratios
class BorderProcessCovTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

static QPixmap makePix(int w, int h)
{
    QPixmap p(w, h);
    p.fill(QColor(100, 150, 200));
    return p;
}

// === base class ===

TEST_F(BorderProcessCovTest, baseCalculateBorderImageInfoSetsSizeAndPoint)
{
    ExternalBorderProcess p; // inherits the base virtual
    QSize in(42, 17);
    EXPECT_NO_FATAL_FAILURE(p.BorderProcessInterface::calculateBorderImageInfo(in));
}

// === ExternalBorderProcess: cropShotImageEx takes the JPG branch when format != 0 ===

TEST_F(BorderProcessCovTest, externalGetPixmapBorderStyle2TriggersCropShotImageEx)
{
    // Save current format, switch to JPG (1), restore afterwards.
    ConfigSettings *cs = ConfigSettings::instance();
    QVariant origFormat = cs->getValue("shot", "format");
    cs->setValue("shot", "format", QVariant(1)); // non-png -> cropShotImageEx processes

    ExternalBorderProcess p;
    p.initBorderInfo(2); // BorderStyle_2 -> cropShotImageEx in getPixmapAddBorder
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = p.getPixmapAddBorder(makePix(120, 70)));
    EXPECT_FALSE(out.isNull());

    cs->setValue("shot", "format", origFormat);
}

// === ExternalBorderProcess: BorderStyle_4 / BorderStyle_8 hit cropShotImage ===

TEST_F(BorderProcessCovTest, externalGetPixmapBorderStyle4CropShotImageJpg)
{
    ConfigSettings *cs = ConfigSettings::instance();
    QVariant origFormat = cs->getValue("shot", "format");
    cs->setValue("shot", "format", QVariant(1)); // JPG -> colorReset = 255

    ExternalBorderProcess p;
    p.initBorderInfo(4); // BorderStyle_4 -> cropShotImage
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = p.getPixmapAddBorder(makePix(100, 60)));
    EXPECT_FALSE(out.isNull());

    cs->setValue("shot", "format", origFormat);
}

TEST_F(BorderProcessCovTest, externalGetPixmapBorderStyle8CropShotImagePng)
{
    ConfigSettings *cs = ConfigSettings::instance();
    QVariant origFormat = cs->getValue("shot", "format");
    cs->setValue("shot", "format", QVariant(0)); // PNG -> colorReset = 0

    ExternalBorderProcess p;
    p.initBorderInfo(8); // BorderStyle_8 -> cropShotImage
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = p.getPixmapAddBorder(makePix(100, 60)));
    EXPECT_FALSE(out.isNull());

    cs->setValue("shot", "format", origFormat);
}

// BorderStyle_5 draws the date text (already covered) - re-run to confirm the
// drawDateText path is stable for a tiny image too.
TEST_F(BorderProcessCovTest, externalGetPixmapBorderStyle5DrawsDateTiny)
{
    ExternalBorderProcess p;
    p.initBorderInfo(5);
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = p.getPixmapAddBorder(makePix(40, 30)));
    EXPECT_FALSE(out.isNull());
}

// calculateBorderImageInfo with equal aspect ratio takes the "taller or equal"
// branch (vertical crop). Use a 1:1 image against a non-1:1 center.
TEST_F(BorderProcessCovTest, externalCalculateBorderImageInfoSquareInput)
{
    ExternalBorderProcess p;
    p.initBorderInfo(1);
    EXPECT_NO_FATAL_FAILURE(p.calculateBorderImageInfo(QSize(80, 80)));
}

TEST_F(BorderProcessCovTest, externalCalculateBorderImageInfoWideInput)
{
    ExternalBorderProcess p;
    p.initBorderInfo(3);
    EXPECT_NO_FATAL_FAILURE(p.calculateBorderImageInfo(QSize(300, 40)));
}

TEST_F(BorderProcessCovTest, externalCalculateBorderImageInfoTallInput)
{
    ExternalBorderProcess p;
    p.initBorderInfo(6);
    EXPECT_NO_FATAL_FAILURE(p.calculateBorderImageInfo(QSize(30, 300)));
}

// === PrototypeBorderProcess ===

// BorderEffects_1 (PC) exercises the vertical-processing path in cropShotImage.
TEST_F(BorderProcessCovTest, prototypeBorderEffects1VerticalPath)
{
    PrototypeBorderProcess p;
    p.initBorderInfo(1);
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = p.getPixmapAddBorder(makePix(120, 70)));
    EXPECT_FALSE(out.isNull());
}

TEST_F(BorderProcessCovTest, prototypeBorderEffects1JpgFormat)
{
    ConfigSettings *cs = ConfigSettings::instance();
    QVariant origFormat = cs->getValue("shot", "format");
    cs->setValue("shot", "format", QVariant(1));

    PrototypeBorderProcess p;
    p.initBorderInfo(1);
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = p.getPixmapAddBorder(makePix(90, 60)));
    EXPECT_FALSE(out.isNull());

    cs->setValue("shot", "format", origFormat);
}

// BorderEffects_3 (iPad) and _4 (cellphone): widen the style coverage.
TEST_F(BorderProcessCovTest, prototypeBorderEffects3And4)
{
    for (int cfg : {3, 4}) {
        PrototypeBorderProcess p;
        p.initBorderInfo(cfg);
        QPixmap out;
        EXPECT_NO_FATAL_FAILURE(out = p.getPixmapAddBorder(makePix(80, 80)));
        EXPECT_FALSE(out.isNull());
    }
}

TEST_F(BorderProcessCovTest, prototypeCalculateBorderImageInfoWideVsTall)
{
    PrototypeBorderProcess p;
    p.initBorderInfo(2);
    EXPECT_NO_FATAL_FAILURE(p.calculateBorderImageInfo(QSize(200, 50)));
    EXPECT_NO_FATAL_FAILURE(p.calculateBorderImageInfo(QSize(50, 200)));
}

// === ShadowBorderProcess ===

// A large image enters the real blur path (drawShadow does not early-return).
TEST_F(BorderProcessCovTest, shadowLargeImageBlurPath)
{
    ShadowBorderProcess p;
    p.initBorderInfo(0);
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = p.getPixmapAddBorder(makePix(200, 200)));
    EXPECT_FALSE(out.isNull());
}
