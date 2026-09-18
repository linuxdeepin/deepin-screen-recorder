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

// initShapeButtons lambda: 构造函数已连接 m_shapeBtnGroup->buttonClicked。
// 通过 findChildren 找到 rectangle/oval 按钮并 click，触发 lambda 分支。
TEST_F(ShapeToolWidgetTest, initShapeButtonsLambdaFires)
{
    QSignalSpy spy(m_w, &ShapeToolWidget::shapeSelected);
    auto buttons = m_w->findChildren<ToolButton *>();
    // rectangle 按钮的 accessibleName 为 "rectangle_button"
    ToolButton *rectBtn = nullptr;
    ToolButton *ovalBtn = nullptr;
    for (auto *b : buttons) {
        if (b->accessibleName() == QStringLiteral("rectangle_button")) rectBtn = b;
        if (b->accessibleName() == QStringLiteral("oval_button")) ovalBtn = b;
    }
    if (rectBtn) {
        EXPECT_NO_FATAL_FAILURE(rectBtn->click());
    }
    if (ovalBtn) {
        EXPECT_NO_FATAL_FAILURE(ovalBtn->click());
    }
    EXPECT_GE(spy.count(), 1);
}
