// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include "../../src/utils/borderprocessinterface.h"

using namespace testing;

// Helper: create a fully-opaque pixmap (no alpha → shaped=false path).
static QPixmap makeOpaquePixmap(int w, int h)
{
    QPixmap pix(w, h);
    pix.fill(QColor(100, 150, 200, 255));
    return pix;
}

// Helper: create a pixmap with transparent corners (rounded rect cut-out).
// This exercises the alpha-channel detection → shaped=true path.
static QPixmap makeRoundedPixmap(int w, int h, int radius)
{
    QPixmap src(w, h);
    src.fill(Qt::transparent);
    QPainter p(&src);
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, w, h), radius, radius);
    p.fillPath(path, QColor(100, 150, 200, 255));
    p.end();
    return src;
}

// ── getPixmapAddBorder with opaque pixmap → non-shaped path (commit 487c41cd) ──

TEST(BorderProcessFixCovTest, getPixmapAddBorderOpaqueNonShaped)
{
    ShadowBorderProcess p;
    p.initBorderInfo(0);
    QPixmap out = p.getPixmapAddBorder(makeOpaquePixmap(100, 80));
    EXPECT_FALSE(out.isNull());
}

// ── getPixmapAddBorder with rounded (alpha) pixmap → shaped path (commit 487c41cd) ──

TEST(BorderProcessFixCovTest, getPixmapAddBorderRoundedShaped)
{
    ShadowBorderProcess p;
    p.initBorderInfo(0);
    QPixmap out = p.getPixmapAddBorder(makeRoundedPixmap(100, 80, 15));
    EXPECT_FALSE(out.isNull());
}

// ── getPixmapAddBorder with rounded pixmap, small size → drawShadow early return ──

TEST(BorderProcessFixCovTest, getPixmapAddBorderRoundedSmallSize)
{
    ShadowBorderProcess p;
    p.initBorderInfo(0);
    QPixmap out = p.getPixmapAddBorder(makeRoundedPixmap(4, 4, 2));
    EXPECT_FALSE(out.isNull());
}

// ── getPixmapAddBorder with opaque pixmap, small size → drawShadow early return ──

TEST(BorderProcessFixCovTest, getPixmapAddBorderOpaqueSmallSize)
{
    ShadowBorderProcess p;
    p.initBorderInfo(0);
    QPixmap out = p.getPixmapAddBorder(makeOpaquePixmap(4, 4));
    EXPECT_FALSE(out.isNull());
}

// ── getPixmapAddBorder with larger rounded pixmap (offset < radius path) ──

TEST(BorderProcessFixCovTest, getPixmapAddBorderRoundedLarge)
{
    ShadowBorderProcess p;
    p.initBorderInfo(0);
    QPixmap out = p.getPixmapAddBorder(makeRoundedPixmap(200, 150, 20));
    EXPECT_FALSE(out.isNull());
    EXPECT_GT(out.width(), 200);
    EXPECT_GT(out.height(), 150);
}

// ── getPixmapAddBorder with fully transparent pixmap (alpha but no opaque pixels) ──

TEST(BorderProcessFixCovTest, getPixmapAddBorderFullyTransparent)
{
    ShadowBorderProcess p;
    p.initBorderInfo(0);
    QPixmap pix(50, 50);
    pix.fill(Qt::transparent);
    QPixmap out = p.getPixmapAddBorder(pix);
    EXPECT_FALSE(out.isNull());
}
