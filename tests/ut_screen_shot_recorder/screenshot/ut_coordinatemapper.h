// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include "../../../src/screenshot/core/coordinatemapper.h"

using namespace Dsr::Screenshot;
using namespace Dsr::Screenshot::CoordinateMapper;
using namespace testing;

class CoordinateMapperTest : public Test
{
protected:
    OutputDescriptor output;
    void SetUp() override
    {
        output.outputId = QStringLiteral("out-1");
        output.logicalGeometry = QRect(100, 200, 1920, 1080);
        output.devicePixelRatio = 1.0;
    }
};

TEST_F(CoordinateMapperTest, GlobalToOutputLocalPoint)
{
    QPoint global(150, 250);
    QPoint local = globalToOutputLocal(global, output);
    EXPECT_EQ(local, QPoint(50, 50));
}

TEST_F(CoordinateMapperTest, OutputLocalToGlobalPoint)
{
    QPoint local(50, 50);
    QPoint global = outputLocalToGlobal(local, output);
    EXPECT_EQ(global, QPoint(150, 250));
}

TEST_F(CoordinateMapperTest, GlobalToOutputLocalRect)
{
    QRect globalRect(100, 200, 300, 400);
    QRect localRect = globalToOutputLocal(globalRect, output);
    EXPECT_EQ(localRect, QRect(0, 0, 300, 400));
}

TEST_F(CoordinateMapperTest, OutputLocalToGlobalRect)
{
    QRect localRect(0, 0, 300, 400);
    QRect globalRect = outputLocalToGlobal(localRect, output);
    EXPECT_EQ(globalRect, QRect(100, 200, 300, 400));
}

TEST_F(CoordinateMapperTest, RoundTripRect)
{
    QRect original(150, 250, 300, 400);
    QRect back = outputLocalToGlobal(globalToOutputLocal(original, output), output);
    EXPECT_EQ(back, original);
}

TEST_F(CoordinateMapperTest, GlobalToFramePixelScale1)
{
    QRect globalRect(100, 200, 100, 100);
    QRect pixelRect = globalToFramePixel(globalRect, output, 1.0);
    EXPECT_EQ(pixelRect, QRect(0, 0, 100, 100));
}

TEST_F(CoordinateMapperTest, GlobalToFramePixelScale2)
{
    QRect globalRect(100, 200, 100, 100);
    QRect pixelRect = globalToFramePixel(globalRect, output, 2.0);
    EXPECT_EQ(pixelRect, QRect(0, 0, 200, 200));
}

TEST_F(CoordinateMapperTest, GlobalToFramePixelFractionalScale)
{
    output.logicalGeometry = QRect(0, 0, 1000, 1000);
    QRect globalRect(10, 10, 90, 90);
    QRect pixelRect = globalToFramePixel(globalRect, output, 1.5);
    // left = floor(10*1.5) = 15, top = floor(10*1.5) = 15
    // right = ceil((10+90-1+1)*1.5) = ceil(100*1.5) = ceil(150) = 150
    // bottom = ceil(100*1.5) = 150
    // width = 150 - 15 = 135, height = 150 - 15 = 135
    EXPECT_EQ(pixelRect, QRect(15, 15, 135, 135));
}
