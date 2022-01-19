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

#include <QActionGroup>
#include <DFontSizeManager>

#define THEMETYPE 1 // 主题颜色为浅色

const QSize MIN_TOOLBAR_WIDGET_SIZE = QSize(205, 70);

ToolBarWidget::ToolBarWidget(DWidget *parent): DFloatingWidget(parent)
{

    setBlurBackgroundEnabled(true);
    blurBackground()->setRadius(30);
    blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    blurBackground()->setBlurEnabled(true);
    blurBackground()->setBlendMode(DBlurEffectWidget::InWindowBlend);
    setAttribute(Qt::WA_TranslucentBackground, false); //取消透明
    // 初始化主题样式
    if (DGuiApplicationHelper::instance()->themeType() == THEMETYPE) {
        blurBackground()->setMaskColor(QColor(255, 255, 255, 76));
    } else {
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));
    }

    initToolBarWidget(); // 初始化按钮菜单
    // 工具栏样式跟随系统样式改变
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ToolBarWidget::onThemeTypeChange);
}

// 获取保存信息
QPair<QString, QString> ToolBarWidget::getSaveInfo()
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
        blurBackground()->setMaskColor(QColor(255, 255, 255, 76));
        m_closeButton->setHoverPic(":/newUI/hover/close-hover.svg");
        m_closeButton->setNormalPic(":/newUI/normal/close-normal.svg");

    } else {
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));
        m_closeButton->setHoverPic(":/newUI/dark/hover/close-hover_dark.svg");
        m_closeButton->setNormalPic(":/newUI/dark/normal/close-normal_dark.svg");
    }
}

void ToolBarWidget::initToolBarWidget()
{
    m_subTool = new SubToolWidget(this);
    connect(m_subTool, SIGNAL(signalOcrButtonClicked()), this, SIGNAL(signalOcrButtonClicked()));

    m_closeButton = new DImageButton(this);
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
    hLayout->setMargin(0);
    hLayout->setSpacing(2);
    hLayout->addStretch();
    hLayout->addWidget(m_subTool, 0, Qt::AlignCenter);
    hLayout->addWidget(m_closeButton, 0,  Qt::AlignCenter);
    setLayout(hLayout);
}


