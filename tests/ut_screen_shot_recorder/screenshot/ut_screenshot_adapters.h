// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>

#include <QBuffer>
#include <QClipboard>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QMimeData>
#include <QPainter>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "addr_pri.h"
#include "../../src/screenshot/adapters/shapeswidgetannotationeditor.h"
#include "../../src/screenshot/export/localscreenshotresultsink.h"
#include "../../src/utils/configsettings.h"
#include "../../src/widgets/shapeswidget.h"

using namespace testing;

using LocalScreenshotResultSinkForTest = Dsr::Screenshot::LocalScreenshotResultSink;
ACCESS_PRIVATE_FIELD(LocalScreenshotResultSinkForTest, bool, m_clipboardSignalConnected);

namespace {

Dsr::Screenshot::Result makeSinkResult()
{
    Dsr::Screenshot::Result result;
    result.image = QImage(8, 8, QImage::Format_ARGB32_Premultiplied);
    result.image.fill(QColor(20, 120, 220, 255));
    result.globalSelection = QRect(0, 0, 8, 8);
    return result;
}

} // namespace

class LocalScreenshotResultSinkTest : public Test
{
public:
    void SetUp() override
    {
        ConfigSettings::instance()->setValue("shot", "format", 0);
        ConfigSettings::instance()->setValue("shot", "border_index", 0);
        m_sink.setNotificationsEnabled(false);
    }

    Dsr::Screenshot::LocalScreenshotResultSink m_sink;
};

TEST_F(LocalScreenshotResultSinkTest, invalidOperationsFailFast)
{
    const Dsr::Screenshot::Result invalid;
    QString error;
    EXPECT_FALSE(m_sink.save(invalid, QStringLiteral("/tmp/unused.png"), nullptr, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("无效")));

    error.clear();
    EXPECT_FALSE(m_sink.copy(invalid, &error));
    EXPECT_FALSE(m_sink.pin(invalid, &error));
    EXPECT_FALSE(m_sink.ocr(invalid, &error));
    EXPECT_FALSE(m_sink.tableRecognize(invalid, &error));
    EXPECT_FALSE(m_sink.aiAssistant(invalid, 0, &error));
    EXPECT_FALSE(m_sink.copyText(QString(), &error));
}

TEST_F(LocalScreenshotResultSinkTest, savesPngAndJpegToExplicitPaths)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const Dsr::Screenshot::Result result = makeSinkResult();
    QString savedPath;
    QString error;
    bool cancelled = true;

    const QString pngPath = directory.filePath(QStringLiteral("capture.png"));
    ASSERT_TRUE(m_sink.save(result, pngPath, &savedPath, &error, &cancelled));
    EXPECT_FALSE(cancelled);
    EXPECT_EQ(savedPath, pngPath);
    EXPECT_TRUE(QFileInfo::exists(pngPath));
    EXPECT_GT(QFileInfo(pngPath).size(), 0);

    ConfigSettings::instance()->setValue("shot", "format", 1);
    const QString jpgPath = directory.filePath(QStringLiteral("capture.jpg"));
    savedPath.clear();
    error.clear();
    ASSERT_TRUE(m_sink.save(result, jpgPath, &savedPath, &error, &cancelled));
    EXPECT_EQ(savedPath, jpgPath);
    EXPECT_TRUE(QFileInfo::exists(jpgPath));
    EXPECT_GT(QFileInfo(jpgPath).size(), 0);
}

TEST_F(LocalScreenshotResultSinkTest, copyAndCopyTextCoverNoLoaderPath)
{
    const Dsr::Screenshot::Result result = makeSinkResult();
    access_private_field::LocalScreenshotResultSinkForTestm_clipboardSignalConnected(m_sink) = false;

    QString error;
    EXPECT_TRUE(m_sink.copy(result, &error));

    QGuiApplication::clipboard()->setText(QStringLiteral("copied text"));
    EXPECT_TRUE(m_sink.copyText(QStringLiteral("copied text"), &error));
}

class ShapesWidgetAnnotationEditorTest : public Test
{
public:
    void SetUp() override
    {
        m_widget.resize(200, 150);
        m_editor = new Dsr::Screenshot::ShapesWidgetAnnotationEditor(&m_widget);
        m_editor->setDesktopBounds(QRect(10, 20, 200, 150));
        m_editor->setGlobalOrigin(QPoint(10, 20));
    }

    void TearDown() override { delete m_editor; }

    ShapesWidget m_widget;
    Dsr::Screenshot::ShapesWidgetAnnotationEditor *m_editor = nullptr;
};

TEST_F(ShapesWidgetAnnotationEditorTest, toolMappingAndGeometryHelpers)
{
    using Dsr::Screenshot::AnnotationTool;
    const AnnotationTool tools[] = {
        AnnotationTool::Pointer,
        AnnotationTool::Rectangle,
        AnnotationTool::Ellipse,
        AnnotationTool::Arrow,
        AnnotationTool::Line,
        AnnotationTool::Pen,
        AnnotationTool::Effect,
        AnnotationTool::Sequence,
        AnnotationTool::Text
    };
    for (AnnotationTool tool : tools) {
        m_editor->setTool(tool);
        EXPECT_EQ(m_editor->tool(), tool);
    }

    m_editor->setEffectSourceImage(QImage(4, 4, QImage::Format_ARGB32));
    m_editor->setInteractionBounds(QRect(0, 0, 100, 100));
    EXPECT_FALSE(m_editor->hasSelectedContent());
    EXPECT_TRUE(m_editor->contentBoundsGlobal().isEmpty());
    EXPECT_FALSE(m_editor->isInteracting());

    m_editor->setTool(AnnotationTool::Rectangle);
    const auto inside = m_editor->hitTest(QPoint(30, 40));
    EXPECT_EQ(inside.kind, Dsr::Screenshot::AnnotationHitKind::CreateContent);
    const auto outside = m_editor->hitTest(QPoint(1000, 1000));
    EXPECT_EQ(outside.kind, Dsr::Screenshot::AnnotationHitKind::None);
}

TEST_F(ShapesWidgetAnnotationEditorTest, interactionInlineEditingAndRenderAreSafe)
{
    using Dsr::Screenshot::AnnotationTool;
    m_editor->setTool(AnnotationTool::Rectangle);
    EXPECT_NO_FATAL_FAILURE(m_editor->beginInteraction(QPoint(30, 40)));
    EXPECT_NO_FATAL_FAILURE(m_editor->updateInteraction(QPoint(80, 90)));
    EXPECT_NO_FATAL_FAILURE(m_editor->endInteraction());
    EXPECT_NO_FATAL_FAILURE(m_editor->cancelInteraction());
    EXPECT_NO_FATAL_FAILURE(m_editor->undo());
    EXPECT_NO_FATAL_FAILURE(m_editor->clearContent());

    ShapesWidget::InlineEditRequest request;
    EXPECT_NO_FATAL_FAILURE(m_editor->takePendingInlineEdit(&request));
    EXPECT_NO_FATAL_FAILURE(m_editor->beginInlineEditAt(QPoint(40, 50), &request));
    EXPECT_NO_FATAL_FAILURE(m_editor->updateInlineText(0, QStringLiteral("text"), QRectF(0, 0, 20, 20)));
    EXPECT_NO_FATAL_FAILURE(m_editor->finishInlineText(0));
    EXPECT_NO_FATAL_FAILURE(m_editor->updateInlineSequence(0, 1));
    EXPECT_NO_FATAL_FAILURE(m_editor->setInlineEditingShape(0));

    QImage canvas(100, 100, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    EXPECT_NO_FATAL_FAILURE(m_editor->render(painter, QRect(10, 20, 100, 100), 1.0));
    EXPECT_NO_FATAL_FAILURE(m_editor->render(painter, QRect(), 1.0));
    EXPECT_NO_FATAL_FAILURE(m_editor->render(painter, QRect(10, 20, 100, 100), 0.0));
}
