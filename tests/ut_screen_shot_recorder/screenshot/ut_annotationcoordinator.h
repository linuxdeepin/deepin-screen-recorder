// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QRectF>
#include "../../../src/screenshot/core/selectionmodel.h"
#include "../../../src/screenshot/session/annotationcoordinator.h"
#include "../../../src/screenshot/ports/iannotationeditor.h"

using namespace Dsr::Screenshot;
using namespace testing;

class MockAnnotationEditor : public IAnnotationEditor
{
public:
    explicit MockAnnotationEditor(QObject *parent = nullptr)
        : IAnnotationEditor(parent) {}
    QRectF m_bounds;
    bool m_interacting = false;
    mutable int renderCallCount = 0;
    QRectF contentBoundsGlobal() const override { return m_bounds; }
    bool isInteracting() const override { return m_interacting; }
    void render(QPainter &, const QRect &, qreal) const override { ++renderCallCount; }
};

class AnnotationCoordinatorTest : public Test
{
protected:
    SelectionModel selection;
};

TEST_F(AnnotationCoordinatorTest, NullEditorIsSafe)
{
    AnnotationCoordinator coord(&selection, nullptr);
    EXPECT_EQ(coord.editor(), nullptr);
    EXPECT_NO_FATAL_FAILURE(coord.syncBounds());
}

TEST_F(AnnotationCoordinatorTest, ConstructorSyncsBounds)
{
    selection.setDesktopBounds(QRect(0, 0, 1000, 1000));
    selection.setGeometry(QRect(100, 100, 200, 200));
    ASSERT_EQ(selection.state(), InteractionState::Ready);

    MockAnnotationEditor editor;
    editor.m_bounds = QRectF(50, 50, 80, 80);

    AnnotationCoordinator coord(&selection, &editor);
    EXPECT_EQ(coord.editor(), &editor);

    QRect geom = selection.geometry();
    EXPECT_TRUE(geom.contains(QRect(50, 50, 80, 80)));
    EXPECT_TRUE(geom.contains(QRect(100, 100, 200, 200)));
}

TEST_F(AnnotationCoordinatorTest, SyncBoundsAppliesEditorBounds)
{
    selection.setDesktopBounds(QRect(0, 0, 1000, 1000));
    selection.setGeometry(QRect(200, 200, 100, 100));

    MockAnnotationEditor editor;
    editor.m_bounds = QRectF();

    AnnotationCoordinator coord(&selection, &editor);
    QRect before = selection.geometry();

    editor.m_bounds = QRectF(180, 180, 60, 60);
    coord.syncBounds();

    QRect after = selection.geometry();
    EXPECT_TRUE(after.contains(QRect(180, 180, 60, 60)));
    EXPECT_TRUE(after.contains(QRect(200, 200, 100, 100)));
}

TEST_F(AnnotationCoordinatorTest, ContentBoundsChangedSignalUpdatesSelection)
{
    selection.setDesktopBounds(QRect(0, 0, 1000, 1000));
    selection.setGeometry(QRect(300, 300, 100, 100));

    MockAnnotationEditor editor;
    editor.m_bounds = QRectF();
    AnnotationCoordinator coord(&selection, &editor);

    editor.m_bounds = QRectF(280, 280, 40, 40);
    emit editor.contentBoundsChanged(editor.m_bounds);

    QRect geom = selection.geometry();
    EXPECT_TRUE(geom.contains(QRect(280, 280, 40, 40)));
    EXPECT_TRUE(geom.contains(QRect(300, 300, 100, 100)));
}
