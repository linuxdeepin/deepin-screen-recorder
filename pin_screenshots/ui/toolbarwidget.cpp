// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbarwidget.h"
#include "accessibility/acTextDefine.h"
#include "putils.h"

#include <QActionGroup>
#include <QMouseEvent>
#include <DFontSizeManager>

#define THEMETYPE 1 // 主题颜色为浅色

const QSize MIN_TOOLBAR_WIDGET_SIZE = QSize(194, 60);

ToolBarWidget::ToolBarWidget(DWidget *parent): DBlurEffectWidget(parent)
{
    if (PUtils::isWaylandMode) {
        setWindowFlags(Qt::Sheet | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    } else {
        setWindowFlags(Qt::ToolTip);
    }

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
    setMinimumWidth(196);
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

//重写鼠标移动事件：解决工具栏可以被拖动的问题
void ToolBarWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    //qDebug() << event->button() << event->x() << event->y();
    //QWidget::mouseMoveEvent(event);
}

