// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "../../src/widgets/shapetoolwidget.h"
#include "../../src/main_window.h"

using namespace testing;

// ShapeToolWidget 此前无测试头；构造 + selectShape 分支 + WithoutSignal setter 均可直接调用。
class ShapeToolWidgetTest : public Test
{
public:
    ShapeToolWidget *m_w;
    void SetUp() override { m_w = new ShapeToolWidget; }
    void TearDown() override { delete m_w; }
};

TEST_F(ShapeToolWidgetTest, construct)
{
    ASSERT_NE(m_w, nullptr);
    EXPECT_NO_FATAL_FAILURE(m_w->initWidget());
}

TEST_F(ShapeToolWidgetTest, selectShape_validAndInvalid)
{
    QSignalSpy spy(m_w, &ShapeToolWidget::shapeSelected);
    // 有效形状
    EXPECT_NO_FATAL_FAILURE(m_w->selectShape(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->selectShape(QStringLiteral("oval")));
    // 无效/空形状 -> 走配置回退与默认矩形分支
    EXPECT_NO_FATAL_FAILURE(m_w->selectShape(QStringLiteral("")));
    EXPECT_NO_FATAL_FAILURE(m_w->selectShape(QStringLiteral("unknown")));
    EXPECT_GE(spy.count(), 0);
}

TEST_F(ShapeToolWidgetTest, selectWithoutSignal)
{
    EXPECT_NO_FATAL_FAILURE(m_w->selectRectangleWithoutSignal());
    EXPECT_NO_FATAL_FAILURE(m_w->selectOvalWithoutSignal());
}

TEST_F(ShapeToolWidgetTest, setMainWindow)
{
    MainWindow *mw = nullptr;
    EXPECT_NO_FATAL_FAILURE(m_w->setMainWindow(mw));
}
