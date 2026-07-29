// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QtTest>
#include <QColor>
#include <QDebug>
#include <gtest/gtest.h>
#include "../../src/widgets/colortoolwidget.h"
#include "addr_pri.h"


using namespace testing;

// 原先通过 ACCESS_PRIVATE_FIELD 直接读取 m_redBtn/m_yellowBtn/... 等成员，
// 但这些成员在源码中从未被赋值（已作为死代码移除），读取到的是垃圾指针。
// 改为通过 QButtonGroup 暴露的按钮列表获取真正的 ToolButton 实例。
class ColorToolWidgetTest:public testing::Test, public QObject{

public:
    ColorToolWidget *widget;
    QString curColor;
    virtual void SetUp() override{
        std::cout << "start ColorToolWidgetTest" << std::endl;
        widget  = new ColorToolWidget(nullptr);
        connect(widget, &ColorToolWidget::colorChecked, this, &ColorToolWidgetTest::OnColorChecked);
    }

    virtual void TearDown() override{
        delete widget;
        std::cout << "end ColorToolWidgetTest" << std::endl;
    }
public slots:
    void OnColorChecked(QString color);
};
TEST_F(ColorToolWidgetTest, colorChecked)
{
    // 通过子对象查找真实存在的 ToolButton 实例（由 initColorLabel 创建）
    auto buttons = widget->findChildren<ToolButton *>();
    ASSERT_FALSE(buttons.isEmpty());

    // 依次点击所有颜色按钮，覆盖 colorChecked 信号路径
    for (ToolButton *btn : buttons) {
        ASSERT_NE(btn, nullptr);
        curColor = btn->property("name").toString();
        QTest::mouseClick(btn, Qt::MouseButton::LeftButton);
    }
}
void ColorToolWidgetTest::OnColorChecked(QString color)
{
    EXPECT_EQ(color, curColor);
    //qDebug() << "OnColorChecked" << color;
}
