/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     Zhang Wenchao <zhangwenchao@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "toolbarwidget.h"
#include "accessibility/acTextDefine.h"

#include <QActionGroup>
#include <DFontSizeManager>

#define THEMETYPE 1 // 主题颜色为浅色

const QSize MIN_TOOLBAR_WIDGET_SIZE = QSize(194, 60);

ToolBarWidget::ToolBarWidget(DWidget *parent): DBlurEffectWidget(parent)
{

    setWindowFlags(Qt::ToolTip);
    this->setRadius(30);
    this->setBlurEnabled(true);
    this->setMode(DBlurEffectWidget::GaussianBlur);
    this->setBlendMode(DBlurEffectWidget::InWindowBlend);
    // 初始化主题样式
    if (DGuiApplicationHelper::instance()->themeType() == THEMETYPE) {
        this->setMaskColor(QColor(255, 255, 255, 76));
    } else {
        this->setMaskColor(QColor(0, 0, 0, 76));
    }

    initToolBarWidget(); // 初始化按钮菜单
    // 工具栏样式跟随系统样式改变
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ToolBarWidget::onThemeTypeChange);
}

// 获取保存信息
QPair<int, int> ToolBarWidget::getSaveInfo()
{
    return m_subTool->getSaveInfo();
}

// 选项按钮被点击
void ToolBarWidget::onOptionButtonClicked()
{
    m_subTool->onOptionButtonClicked();
}

// 主题变化
void ToolBarWidget::onThemeTypeChange(DGuiApplicationHelper::ColorType themeType)
{
    if (themeType == THEMETYPE) {
        this->setMaskColor(QColor(255, 255, 255, 76));
        m_closeButton->setHoverPic(":/newUI/hover/close-hover.svg");
        m_closeButton->setNormalPic(":/newUI/normal/close-normal.svg");

    } else {
        this->setMaskColor(QColor(0, 0, 0, 76));
        m_closeButton->setHoverPic(":/newUI/dark/hover/close-hover_dark.svg");
        m_closeButton->setNormalPic(":/newUI/dark/normal/close-normal_dark.svg");
    }
}

void ToolBarWidget::initToolBarWidget()
{
    m_subTool = new SubToolWidget(this);
    connect(m_subTool, SIGNAL(signalOcrButtonClicked()), this, SIGNAL(signalOcrButtonClicked()));

    m_closeButton = new DImageButton(this);
    m_closeButton->setObjectName(AC_TOOLBARWIDGET_CLOSE_PIN_BUT);
    m_closeButton->setAccessibleName(AC_TOOLBARWIDGET_CLOSE_PIN_BUT);
    // 初始化关闭按钮样式
    if (DGuiApplicationHelper::instance()->themeType() == THEMETYPE) {
        m_closeButton->setHoverPic(":/newUI/hover/close-hover.svg");
        m_closeButton->setNormalPic(":/newUI/normal/close-normal.svg");
    } else {
        m_closeButton->setHoverPic(":/newUI/dark/hover/close-hover_dark.svg");
        m_closeButton->setNormalPic(":/newUI/dark/normal/close-normal_dark.svg");
    }
    // 注册按钮点击事件
    connect(m_closeButton, SIGNAL(clicked()), this, SIGNAL(signalCloseButtonClicked()));
    setMinimumSize(MIN_TOOLBAR_WIDGET_SIZE);
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_subTool, 0, Qt::AlignCenter);
    hLayout->addWidget(m_closeButton, 0,  Qt::AlignCenter);
    setLayout(hLayout);
}


