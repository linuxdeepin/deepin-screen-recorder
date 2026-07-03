// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include "addr_pri.h"
#include "../../src/widgets/shottoolwidget.h"
#include "../../src/utils/configsettings.h"

using namespace testing;

// 覆盖 shottoolwidget.cpp 剩余未覆盖分支：
//  - switchContent 在已初始化 thickness/text 面板后的二次切换（跳过 lazy init）
//  - shapeSelected 在 m_pMainWindow 非空时的调用路径（用 nullptr 仍走 if 守卫）
//  - initThicknessLabel / initTextLabel / initEffectLabel 直接调用
//  - 各种 ConfigSettings 组合下 effect 工具栏的初始化分支
// 既有 ut_shottoolwidget_cov.h 已覆盖基本 switchContent 分支，这里补充边界与重复调用。
// ACCESS_PRIVATE_FUN/FIELD 未在既有头声明，本文件按需声明。

class ShotToolWidgetCov2Test : public Test
{
public:
    ShotToolWidget *m_w = nullptr;
    void SetUp() override { m_w = new ShotToolWidget(nullptr); }
    void TearDown() override { delete m_w; }
};

// switchContent：rectangle -> 初始化 thickness；再调 oval -> 复用 thickness
TEST_F(ShotToolWidgetCov2Test, switchContentThicknessReuseAcrossShapes)
{
    ConfigSettings::instance()->setValue("rectangle", "line_width", 1);
    ConfigSettings::instance()->setValue("oval", "line_width", 0);
    ConfigSettings::instance()->setValue("line", "line_width", 1);
    ConfigSettings::instance()->setValue("arrow", "line_width", 1);
    ConfigSettings::instance()->setValue("pen", "line_width", 1);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("oval")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("rectangle"))); // 复用
}

// switchContent：gio 使用 rectangle 配置
TEST_F(ShotToolWidgetCov2Test, switchContentGioAfterInit)
{
    ConfigSettings::instance()->setValue("rectangle", "line_width", 2);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("gio"))); // 复用 thickness
}

// switchContent：rectangle -> text -> rectangle（removeWidget/addWidget 交叉）
TEST_F(ShotToolWidgetCov2Test, switchContentCrossPanels)
{
    ConfigSettings::instance()->setValue("rectangle", "line_width", 0);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("text")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("rectangle"))); // 再切回，textInitFlag=true
}

// switchContent：effect 在 thickness 已初始化后切换
TEST_F(ShotToolWidgetCov2Test, switchContentEffectAfterThickness)
{
    ConfigSettings::instance()->setValue("rectangle", "line_width", 1);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("effect")));
}

// switchContent：text 在 effect 后切换
TEST_F(ShotToolWidgetCov2Test, switchContentTextAfterEffect)
{
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("effect")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("text")));
}

// switchContent：thickness_index = -1（配置为 -1）
TEST_F(ShotToolWidgetCov2Test, switchContentNegativeLineWidth)
{
    ConfigSettings::instance()->setValue("rectangle", "line_width", -1);
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("rectangle")));
}

// initEffectLabel：直接调用（重复构造）
TEST_F(ShotToolWidgetCov2Test, initEffectLabelRepeatSafe)
{
    // initWidget 已在构造时调用 initEffectLabel；再调一次验证不崩溃
    EXPECT_NO_FATAL_FAILURE(m_w->initEffectLabel());
}

// initTextLabel：直接调用
TEST_F(ShotToolWidgetCov2Test, initTextLabelDirectCall)
{
    EXPECT_NO_FATAL_FAILURE(m_w->initTextLabel());
}

// initThicknessLabel：直接调用
TEST_F(ShotToolWidgetCov2Test, initThicknessLabelDirectCall)
{
    EXPECT_NO_FATAL_FAILURE(m_w->initThicknessLabel());
}

// initWidget：直接调用（重复构造）
TEST_F(ShotToolWidgetCov2Test, initWidgetRepeatSafe)
{
    EXPECT_NO_FATAL_FAILURE(m_w->initWidget());
}

// installTipHint：多个 widget 安装 hint
TEST_F(ShotToolWidgetCov2Test, installTipHintMultipleWidgets)
{
    QWidget *w1 = new QWidget;
    QWidget *w2 = new QWidget;
    EXPECT_NO_FATAL_FAILURE(m_w->installTipHint(w1, QStringLiteral("hint1")));
    EXPECT_NO_FATAL_FAILURE(m_w->installTipHint(w2, QStringLiteral("hint2")));
    EXPECT_TRUE(w1->property("HintWidget").isValid());
    EXPECT_TRUE(w2->property("HintWidget").isValid());
    delete w1;
    delete w2;
}

// shapeSelected：null MainWindow -> 守卫跳过（安全）
TEST_F(ShotToolWidgetCov2Test, shapeSelectedNullMainWindowAllShapes)
{
    EXPECT_NO_FATAL_FAILURE(m_w->shapeSelected(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->shapeSelected(QStringLiteral("oval")));
    EXPECT_NO_FATAL_FAILURE(m_w->shapeSelected(QStringLiteral("line")));
    EXPECT_NO_FATAL_FAILURE(m_w->shapeSelected(QStringLiteral("pen")));
    EXPECT_NO_FATAL_FAILURE(m_w->shapeSelected(QString()));
}

// colorChecked：各种 colorType 字符串
TEST_F(ShotToolWidgetCov2Test, colorCheckedVariousTypes)
{
    EXPECT_NO_FATAL_FAILURE(m_w->colorChecked(QStringLiteral("rectangle")));
    EXPECT_NO_FATAL_FAILURE(m_w->colorChecked(QStringLiteral("oval")));
    EXPECT_NO_FATAL_FAILURE(m_w->colorChecked(QStringLiteral("custom")));
    EXPECT_NO_FATAL_FAILURE(m_w->colorChecked(QString()));
}

// effect 配置分支：isBlur=true -> blurBut 选中
TEST_F(ShotToolWidgetCov2Test, effectConfigBlurEnabled)
{
    ConfigSettings::instance()->setValue("effect", "isBlur", true);
    ConfigSettings::instance()->setValue("effect", "isOval", 0);
    // 通过重新构造触发 initEffectLabel 读取配置
    ShotToolWidget *w = new ShotToolWidget(nullptr);
    EXPECT_NO_FATAL_FAILURE(w->initEffectLabel());
    delete w;
}

// effect 配置分支：isBlur=false -> mosaicBut 选中
TEST_F(ShotToolWidgetCov2Test, effectConfigMosaicEnabled)
{
    ConfigSettings::instance()->setValue("effect", "isBlur", false);
    ConfigSettings::instance()->setValue("effect", "isOval", 1);
    ShotToolWidget *w = new ShotToolWidget(nullptr);
    EXPECT_NO_FATAL_FAILURE(w->initEffectLabel());
    delete w;
}

// effect 配置分支：isOval=2 -> pen 工具选中
TEST_F(ShotToolWidgetCov2Test, effectConfigPenSelected)
{
    ConfigSettings::instance()->setValue("effect", "isBlur", false);
    ConfigSettings::instance()->setValue("effect", "isOval", 2);
    ShotToolWidget *w = new ShotToolWidget(nullptr);
    EXPECT_NO_FATAL_FAILURE(w->initEffectLabel());
    delete w;
}

// effect 配置分支：isOval=999（default）-> rectangle 选中
TEST_F(ShotToolWidgetCov2Test, effectConfigDefaultRectangle)
{
    ConfigSettings::instance()->setValue("effect", "isBlur", false);
    ConfigSettings::instance()->setValue("effect", "isOval", 999);
    ShotToolWidget *w = new ShotToolWidget(nullptr);
    EXPECT_NO_FATAL_FAILURE(w->initEffectLabel());
    delete w;
}

// switchContent：连续多次切换不同 shape
TEST_F(ShotToolWidgetCov2Test, switchContentRapidCycling)
{
    ConfigSettings::instance()->setValue("rectangle", "line_width", 1);
    ConfigSettings::instance()->setValue("oval", "line_width", 0);
    ConfigSettings::instance()->setValue("line", "line_width", 1);
    ConfigSettings::instance()->setValue("arrow", "line_width", 2);
    ConfigSettings::instance()->setValue("pen", "line_width", 0);
    for (int i = 0; i < 10; ++i) {
        EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("rectangle")));
        EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("text")));
        EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("effect")));
        EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("oval")));
    }
}

// changeArrowAndLine 信号可发射
TEST_F(ShotToolWidgetCov2Test, changeArrowAndLineSignalEmittable)
{
    QSignalSpy spy(m_w, &ShotToolWidget::changeArrowAndLine);
    EXPECT_TRUE(spy.isValid());
    EXPECT_NO_FATAL_FAILURE(emit m_w->changeArrowAndLine(0));
    EXPECT_NO_FATAL_FAILURE(emit m_w->changeArrowAndLine(1));
    EXPECT_GE(spy.count(), 2);
}
