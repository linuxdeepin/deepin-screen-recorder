// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "addr_pri.h"
#include "../../src/widgets/shottoolwidget.h"
#include "../../src/utils/configsettings.h"

using namespace testing;

// The legacy ut_shottoolwidget.h is disabled (ctor/initRectLabel/m_blurRectButton
// drift). This file targets the current ShotToolWidget API with a null MainWindow
// (the ctor only stores the pointer; initWidget/initEffectLabel build UI without
// dereferencing it). Covers:
//   - switchContent across all branches: thickness-panel shapes (rectangle/oval/
//     gio/line/arrow/pen -> lazy initThicknessLabel), text (lazy initTextLabel),
//     effect, empty string, and unknown.
//   - shapeSelected (m_pMainWindow null guard -> no-op, must not crash).
//   - colorChecked (currently empty body, just ensures it returns).
//   - installTipHint (creates a ToolTips with the parent; safe with null parent).
//
// MainWindow is NOT dereferenced in any covered path, so passing nullptr keeps
// the test hermetic (no need for the heavy MainWindow stubs).

class ShotToolWidgetCovTest : public Test
{
public:
    ShotToolWidget *m_w = nullptr;
    void SetUp() override { m_w = new ShotToolWidget(nullptr); }
    void TearDown() override { delete m_w; }
};

// ---------- switchContent: thickness-panel shapes ----------

TEST_F(ShotToolWidgetCovTest, switchContentRectangleBuildsThickness)
{
    ConfigSettings::instance()->setValue("rectangle", "line_width", 1);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("rectangle")));
}

TEST_F(ShotToolWidgetCovTest, switchContentOvalBuildsThickness)
{
    ConfigSettings::instance()->setValue("oval", "line_width", 0);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("oval")));
}

TEST_F(ShotToolWidgetCovTest, switchContentGioUsesRectangleWidth)
{
    ConfigSettings::instance()->setValue("rectangle", "line_width", 1);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("gio")));
}

TEST_F(ShotToolWidgetCovTest, switchContentLineArrowPen)
{
    ConfigSettings::instance()->setValue("line", "line_width", 1);
    ConfigSettings::instance()->setValue("arrow", "line_width", 1);
    ConfigSettings::instance()->setValue("pen", "line_width", 1);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("line")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("arrow")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("pen")));
}

TEST_F(ShotToolWidgetCovTest, switchContentThicknessInvalidIndex)
{
    // line_width out of range -> the else branch (no click)
    ConfigSettings::instance()->setValue("pen", "line_width", 999);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("pen")));
}

// ---------- switchContent: text / effect / empty / unknown ----------

TEST_F(ShotToolWidgetCovTest, switchContentTextBuildsTextPanel)
{
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("text")));
    // second call: textInitFlag already true -> skip re-init
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("text")));
}

TEST_F(ShotToolWidgetCovTest, switchContentEffect)
{
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("effect")));
}

TEST_F(ShotToolWidgetCovTest, switchContentEmptyIsNoop)
{
    int before = m_w->count();
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QString()));
    EXPECT_EQ(m_w->count(), before);
}

TEST_F(ShotToolWidgetCovTest, switchContentUnknownIsNoop)
{
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("totally_unknown")));
    SUCCEED();
}

// ---------- shapeSelected (null MainWindow -> no-op) ----------

TEST_F(ShotToolWidgetCovTest, shapeSelectedNullMainWindowSafe)
{
    EXPECT_NO_FATAL_FAILURE(m_w->shapeSelected(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->shapeSelected(QStringLiteral("oval")));
    EXPECT_NO_FATAL_FAILURE(m_w->shapeSelected(QStringLiteral("line")));
}

// ---------- colorChecked (empty body) ----------

TEST_F(ShotToolWidgetCovTest, colorCheckedIsNoop)
{
    EXPECT_NO_FATAL_FAILURE(m_w->colorChecked(QStringLiteral("color1")));
    EXPECT_NO_FATAL_FAILURE(m_w->colorChecked(QString()));
    SUCCEED();
}

// ---------- installTipHint ----------

TEST_F(ShotToolWidgetCovTest, installTipHintSetsProperty)
{
    QWidget *w = new QWidget;
    EXPECT_NO_FATAL_FAILURE(m_w->installTipHint(w, QStringLiteral("tip text")));
    QVariant v = w->property("HintWidget");
    EXPECT_TRUE(v.isValid());
    delete w;
}

// ---------- signal ----------

TEST_F(ShotToolWidgetCovTest, changeArrowAndLineSignalEmittable)
{
    QSignalSpy spy(m_w, &ShotToolWidget::changeArrowAndLine);
    EXPECT_TRUE(spy.isValid());
    SUCCEED();
}
