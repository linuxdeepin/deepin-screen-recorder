// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include "addr_pri.h"
#include "../../src/utils/shapesutils.h"
#include "../../src/widgets/shapeswidget.h"

using namespace testing;

// ShapesWidget event handlers (mousePressEvent ~265 lines, mouseMoveEvent ~195
// lines, mouseReleaseEvent) are the largest uncovered block in shapeswidget.cpp.
// We access the protected handlers via ACCESS_PRIVATE_FUN. With an un-initialized
// ShapesWidget (no shapes selected, m_sideBar/m_menuController may be null), the
// early-return branches complete without SEGV.

ACCESS_PRIVATE_FUN(ShapesWidget, bool(QEvent *), event);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *), mousePressEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *), mouseReleaseEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *), mouseMoveEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QKeyEvent *), keyPressEvent);
ACCESS_PRIVATE_FIELD(ShapesWidget, Toolshapes, m_shapes);

class ShapesWidgetEventTest : public Test
{
public:
    ShapesWidget *m_w;
    void SetUp() override { m_w = new ShapesWidget; }
    void TearDown() override { delete m_w; }
};

TEST_F(ShapesWidgetEventTest, eventNone)
{
    QEvent ev(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetevent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, mousePressLeftButton)
{
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(50, 50),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetmousePressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, mousePressRightButton)
{
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(50, 50),
        Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetmousePressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, mouseReleaseLeftButton)
{
    QMouseEvent ev(QEvent::MouseButtonRelease, QPointF(50, 50),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetmouseReleaseEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, mouseMoveNoButton)
{
    QMouseEvent ev(QEvent::MouseMove, QPointF(50, 50),
        Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetmouseMoveEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, keyPressEscape)
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, keyPressDelete)
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, keyPressCtrlZ)
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, keyPressLeftArrow)
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ShapesWidgetkeyPressEvent(*m_w, &ev));
}

TEST_F(ShapesWidgetEventTest, rectangleContentAndHandlesHaveDistinctHits)
{
    Toolshape rectangle;
    rectangle.type = QStringLiteral("rectangle");
    rectangle.mainPoints = {
        QPointF(10, 10), QPointF(10, 110),
        QPointF(110, 10), QPointF(110, 110)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {rectangle};

    const ShapesWidget::InteractionHit content =
            m_w->interactionAt(QPointF(60, 60));
    EXPECT_EQ(content.kind, ShapesWidget::InteractionKind::None);

    const ShapesWidget::InteractionHit topEdge =
            m_w->interactionAt(QPointF(35, 10));
    EXPECT_EQ(topEdge.kind, ShapesWidget::InteractionKind::Move);

    const ShapesWidget::InteractionHit topHandle =
            m_w->interactionAt(QPointF(60, 10));
    EXPECT_EQ(topHandle.kind, ShapesWidget::InteractionKind::Resize);
    EXPECT_EQ(topHandle.handle, ShapesWidget::InteractionHandle::Top);

    const ShapesWidget::InteractionHit outside =
            m_w->interactionAt(QPointF(300, 300));
    EXPECT_EQ(outside.kind, ShapesWidget::InteractionKind::None);
}

TEST_F(ShapesWidgetEventTest, interactionDoesNotSelectRectangleInterior)
{
    Toolshape rectangle;
    rectangle.type = QStringLiteral("rectangle");
    rectangle.mainPoints = {
        QPointF(10, 10), QPointF(10, 110),
        QPointF(110, 10), QPointF(110, 110)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {rectangle};

    EXPECT_FALSE(m_w->clickedOnShapes(QPointF(60, 60)));
    EXPECT_TRUE(m_w->clickedOnShapes(QPointF(35, 10)));
}

TEST_F(ShapesWidgetEventTest, interactionDoesNotSelectOvalInteriorOrBoundingBoxCorner)
{
    Toolshape oval;
    oval.type = QStringLiteral("oval");
    oval.mainPoints = {
        QPointF(10, 10), QPointF(10, 110),
        QPointF(110, 10), QPointF(110, 110)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {oval};

    EXPECT_EQ(m_w->interactionAt(QPointF(60, 60)).kind,
              ShapesWidget::InteractionKind::None);
    EXPECT_EQ(m_w->interactionAt(QPointF(35, 35)).kind,
              ShapesWidget::InteractionKind::None);
    EXPECT_FALSE(m_w->clickedOnShapes(QPointF(60, 60)));
    EXPECT_FALSE(m_w->clickedOnShapes(QPointF(35, 35)));

    EXPECT_EQ(m_w->interactionAt(QPointF(60, 10)).kind,
              ShapesWidget::InteractionKind::Resize);
    EXPECT_TRUE(m_w->clickedOnShapes(QPointF(60, 10)));
}

TEST_F(ShapesWidgetEventTest, committedTextUsesMoveInteractionAndDrawsEditingFrame)
{
    Toolshape text;
    text.type = QStringLiteral("text");
    text.index = 7;
    text.text = QStringLiteral("abc");
    text.mainPoints = {
        QPointF(10, 10), QPointF(10, 46),
        QPointF(90, 10), QPointF(90, 46)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {text};
    m_w->setInteractionPlatform(ShapesWidget::InteractionPlatform::TreeLand);

    const ShapesWidget::InteractionHit hit =
            m_w->interactionAt(QPointF(40, 30));
    EXPECT_EQ(hit.kind, ShapesWidget::InteractionKind::Move);

    m_w->setInlineEditingShape(text.index);
    QImage image(100, 60, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    m_w->paintEditingContent(painter);
    painter.end();

    bool hasEditingFrame = false;
    for (int y = 0; y < image.height() && !hasEditingFrame; ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.alpha() > 0
                    && pixel.red() < 32
                    && pixel.green() > 160
                    && pixel.blue() > 220) {
                hasEditingFrame = true;
                break;
            }
        }
    }
    EXPECT_TRUE(hasEditingFrame);
}

TEST_F(ShapesWidgetEventTest, x11CommittedTextKeepsLegacyTextEditorHit)
{
    Toolshape text;
    text.type = QStringLiteral("text");
    text.index = 8;
    text.text = QStringLiteral("abc");
    text.mainPoints = {
        QPointF(10, 10), QPointF(10, 46),
        QPointF(90, 10), QPointF(90, 46)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {text};

    const ShapesWidget::InteractionHit hit =
            m_w->interactionAt(QPointF(40, 30));
    EXPECT_EQ(hit.kind, ShapesWidget::InteractionKind::TextEditor);
}

TEST_F(ShapesWidgetEventTest, committedTextKeepsConfiguredColor)
{
    Toolshape text;
    text.type = QStringLiteral("text");
    text.index = 9;
    text.text = QStringLiteral("MMM");
    text.colorIndex = 3; // #F82A2A
    text.fontSize = 24;
    text.mainPoints = {
        QPointF(5, 5), QPointF(5, 45),
        QPointF(95, 5), QPointF(95, 45)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {text};
    m_w->setInteractionPlatform(ShapesWidget::InteractionPlatform::TreeLand);

    QImage image(100, 60, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    m_w->paintContent(painter);
    painter.end();

    bool hasConfiguredColor = false;
    for (int y = 0; y < image.height() && !hasConfiguredColor; ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.alpha() > 0
                    && pixel.red() > 150
                    && pixel.green() < 100
                    && pixel.blue() < 100) {
                hasConfiguredColor = true;
                break;
            }
        }
    }
    EXPECT_TRUE(hasConfiguredColor);
}

TEST_F(ShapesWidgetEventTest, selectedTextDrawsFrameAndSurvivesTextToolSync)
{
    Toolshape text;
    text.type = QStringLiteral("text");
    text.index = 11;
    text.text = QStringLiteral("move me");
    text.colorIndex = 3;
    text.fontSize = 16;
    text.mainPoints = {
        QPointF(10, 10), QPointF(10, 46),
        QPointF(90, 10), QPointF(90, 46)
    };
    access_private_field::ShapesWidgetm_shapes(*m_w) = {text};
    m_w->setInteractionPlatform(ShapesWidget::InteractionPlatform::TreeLand);
    m_w->setTextSelectionFrameEnabled(true);

    ASSERT_TRUE(m_w->beginInteraction(QPointF(40, 30)));
    const QRectF before = m_w->contentBounds();

    // The selected-text frame must use the same dashed input frame as the
    // inline editor. A translucent/solid frame regression still contains the
    // expected blue pixels, so verify that the bottom edge also has gaps.
    QImage selectionImage(120, 70, QImage::Format_ARGB32_Premultiplied);
    selectionImage.fill(Qt::transparent);
    QPainter selectionPainter(&selectionImage);
    m_w->paintEditingContent(selectionPainter);
    selectionPainter.end();

    int dashedPixels = 0;
    int gapPixels = 0;
    for (int x = 10; x <= 90; ++x) {
        const QColor pixel = selectionImage.pixelColor(x, 46);
        const bool isFramePixel = pixel.alpha() > 0
                && pixel.red() < 32
                && pixel.green() > 160
                && pixel.blue() > 220;
        if (isFramePixel)
            ++dashedPixels;
        else
            ++gapPixels;
    }
    EXPECT_GT(dashedPixels, 0);
    EXPECT_GT(gapPixels, 0);

    // Selecting a text annotation synchronizes the toolbar to the text tool.
    // That synchronization must not clear the active drag selection.
    m_w->setCurrentShape(QStringLiteral("text"));
    m_w->updateInteraction(QPointF(60, 50));
    const QRectF after = m_w->contentBounds();
    EXPECT_NE(before.topLeft(), after.topLeft());

    QImage image(100, 60, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    m_w->paintEditingContent(painter);
    painter.end();

    bool hasSelectionFrame = false;
    for (int y = 0; y < image.height() && !hasSelectionFrame; ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.alpha() > 0
                    && pixel.red() < 32
                    && pixel.green() > 160
                    && pixel.blue() > 220) {
                hasSelectionFrame = true;
                break;
            }
        }
    }
    EXPECT_TRUE(hasSelectionFrame);
}
