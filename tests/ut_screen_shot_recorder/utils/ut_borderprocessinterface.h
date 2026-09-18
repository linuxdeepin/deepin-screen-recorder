// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QPixmap>
#include <QPainter>
#include "../../src/utils/borderprocessinterface.h"

using namespace testing;

static QPixmap makeTestPixmap(int w, int h)
{
    QPixmap pix(w, h);
    pix.fill(QColor(100, 150, 200));
    return pix;
}

TEST(ExternalBorderProcessTest, allBorderStyles)
{
    ExternalBorderProcess p;
    // 遍历各 borderConfig，覆盖 switch 全部分支
    for (int cfg = 1; cfg <= 9; ++cfg) {
        EXPECT_NO_FATAL_FAILURE(p.initBorderInfo(cfg));
    }
    p.initBorderInfo(99); // default 分支

    // 关键样式触发不同绘制路径（含 crop/drawDate）
    for (int cfg : {1, 2, 3, 4, 5, 8}) {
        ExternalBorderProcess ep;
        ep.initBorderInfo(cfg);
        QPixmap out = ep.getPixmapAddBorder(makeTestPixmap(120, 70));
        EXPECT_FALSE(out.isNull());
    }
}

TEST(ExternalBorderProcessTest, calculateBorderImageInfoBothBranches)
{
    ExternalBorderProcess p;
    p.initBorderInfo(1);
    // 宽图 -> 横向；高图 -> 纵向，两个 ratio 分支都覆盖
    p.calculateBorderImageInfo(QSize(200, 50)); // wider
    p.calculateBorderImageInfo(QSize(50, 200)); // taller
}

TEST(PrototypeBorderProcessTest, allEffects)
{
    for (int cfg : {1, 2, 3, 4}) {
        PrototypeBorderProcess p;
        EXPECT_NO_FATAL_FAILURE(p.initBorderInfo(cfg));
        QPixmap out = p.getPixmapAddBorder(makeTestPixmap(100, 60));
        EXPECT_FALSE(out.isNull());
    }
    PrototypeBorderProcess p5;
    p5.initBorderInfo(99); // 无匹配分支
    p5.calculateBorderImageInfo(QSize(80, 80));
}

TEST(ShadowBorderProcessTest, addBorder)
{
    ShadowBorderProcess p;
    p.initBorderInfo(0);
    QPixmap out = p.getPixmapAddBorder(makeTestPixmap(100, 100));
    EXPECT_FALSE(out.isNull());
    // 小图触发 drawShadow 提前返回分支
    QPixmap small = makeTestPixmap(2, 2);
    QPixmap out2 = p.getPixmapAddBorder(small);
    EXPECT_FALSE(out2.isNull());
}

TEST(BorderProcessInterfaceTest, baseCalculateBorderImageInfo)
{
    ExternalBorderProcess p; // 基类默认实现
    EXPECT_NO_FATAL_FAILURE(p.BorderProcessInterface::calculateBorderImageInfo(QSize(10, 10)));
}
