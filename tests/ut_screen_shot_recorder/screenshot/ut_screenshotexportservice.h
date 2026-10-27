// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QPainter>
#include "../../../src/screenshot/core/screenshottypes.h"
#include "../../../src/screenshot/export/screenshotexportservice.h"
#include "../../../src/screenshot/ports/iannotationeditor.h"

using namespace Dsr::Screenshot;
using namespace testing;

class MockAnnotationRenderer : public IAnnotationEditor
{
public:
    explicit MockAnnotationRenderer(QObject *parent = nullptr)
        : IAnnotationEditor(parent) {}
    mutable int renderCount = 0;
    mutable QRect lastSelection;
    mutable qreal lastScale = 0.0;
    QRectF contentBoundsGlobal() const override { return QRectF(); }
    bool isInteracting() const override { return false; }
    void render(QPainter &painter, const QRect &globalSelection, qreal pixelScale) const override
    {
        ++renderCount;
        lastSelection = globalSelection;
        lastScale = pixelScale;
        painter.fillRect(0, 0, 1, 1, Qt::red);
    }
};

class ScreenshotExportServiceTest : public Test
{
protected:
    ScreenshotExportService service;

    OutputFrame makeFrame(const QRect &geometry, const QImage &image, qreal scale = 1.0)
    {
        OutputFrame frame;
        frame.output.outputId = QStringLiteral("out-") + QString::number(geometry.x());
        frame.output.logicalGeometry = geometry;
        frame.output.devicePixelRatio = 1.0;
        frame.image = image;
        frame.pixelScale = scale;
        return frame;
    }
};

TEST_F(ScreenshotExportServiceTest, EmptyFramesReturnsInvalid)
{
    FrozenFrameSet frames;
    Result result = service.render(frames, QRect(0, 0, 100, 100));
    EXPECT_FALSE(result.isValid());
}

TEST_F(ScreenshotExportServiceTest, EmptySelectionReturnsInvalid)
{
    QImage img(100, 100, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::red);
    FrozenFrameSet frames;
    frames.outputs.append(makeFrame(QRect(0, 0, 100, 100), img));
    frames.virtualDesktopRect = QRect(0, 0, 100, 100);
    Result result = service.render(frames, QRect());
    EXPECT_FALSE(result.isValid());
}

TEST_F(ScreenshotExportServiceTest, SelectionOutsideDesktopReturnsInvalid)
{
    QImage img(100, 100, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::red);
    FrozenFrameSet frames;
    frames.outputs.append(makeFrame(QRect(0, 0, 100, 100), img));
    frames.virtualDesktopRect = QRect(0, 0, 100, 100);
    Result result = service.render(frames, QRect(500, 500, 50, 50));
    EXPECT_FALSE(result.isValid());
}

TEST_F(ScreenshotExportServiceTest, ValidSingleOutputRender)
{
    QImage img(100, 100, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::red);
    FrozenFrameSet frames;
    frames.outputs.append(makeFrame(QRect(0, 0, 100, 100), img));
    frames.virtualDesktopRect = QRect(0, 0, 100, 100);
    frames.generation = 42;

    Result result = service.render(frames, QRect(10, 10, 20, 20));
    ASSERT_TRUE(result.isValid());
    EXPECT_EQ(result.image.size(), QSize(20, 20));
    EXPECT_EQ(result.globalSelection, QRect(10, 10, 20, 20));
    EXPECT_EQ(result.frameGeneration, 42u);
    EXPECT_EQ(result.outputs.size(), 1);
}

TEST_F(ScreenshotExportServiceTest, PixelScaleDoublesOutputSize)
{
    QImage img(200, 200, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::blue);
    FrozenFrameSet frames;
    frames.outputs.append(makeFrame(QRect(0, 0, 100, 100), img, 2.0));
    frames.virtualDesktopRect = QRect(0, 0, 100, 100);

    Result result = service.render(frames, QRect(10, 10, 20, 20));
    ASSERT_TRUE(result.isValid());
    EXPECT_EQ(result.image.size(), QSize(40, 40));
}

TEST_F(ScreenshotExportServiceTest, MultiOutputRenderSpansOutputs)
{
    QImage img1(100, 100, QImage::Format_ARGB32_Premultiplied);
    img1.fill(Qt::red);
    QImage img2(100, 100, QImage::Format_ARGB32_Premultiplied);
    img2.fill(Qt::green);
    FrozenFrameSet frames;
    frames.outputs.append(makeFrame(QRect(0, 0, 100, 100), img1));
    frames.outputs.append(makeFrame(QRect(100, 0, 100, 100), img2));
    frames.virtualDesktopRect = QRect(0, 0, 200, 100);

    Result result = service.render(frames, QRect(50, 0, 100, 50));
    ASSERT_TRUE(result.isValid());
    EXPECT_EQ(result.image.size(), QSize(100, 50));
}

TEST_F(ScreenshotExportServiceTest, AnnotationRenderIsCalled)
{
    QImage img(100, 100, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::red);
    FrozenFrameSet frames;
    frames.outputs.append(makeFrame(QRect(0, 0, 100, 100), img));
    frames.virtualDesktopRect = QRect(0, 0, 100, 100);

    MockAnnotationRenderer annotations;
    Result result = service.render(frames, QRect(10, 10, 20, 20), &annotations);
    ASSERT_TRUE(result.isValid());
    EXPECT_EQ(annotations.renderCount, 1);
    EXPECT_EQ(annotations.lastSelection, QRect(10, 10, 20, 20));
}

TEST_F(ScreenshotExportServiceTest, NullImageOutputSkipped)
{
    QImage nullImg;
    QImage img(100, 100, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::red);
    FrozenFrameSet frames;
    frames.outputs.append(makeFrame(QRect(0, 0, 100, 100), nullImg));
    frames.outputs.append(makeFrame(QRect(100, 0, 100, 100), img));
    frames.virtualDesktopRect = QRect(0, 0, 200, 100);

    Result result = service.render(frames, QRect(100, 0, 50, 50));
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.image.size(), QSize(50, 50));
}
