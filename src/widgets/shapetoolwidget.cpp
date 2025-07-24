// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shapetoolwidget.h"
#include "../utils/configsettings.h"
#include "../utils.h"
#include "../utils/log.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"
#include "tooltips.h"

#include <DBlurEffectWidget>
#include <DPalette>

#include <QHBoxLayout>
#include <QDebug>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace {
const QSize TOOL_ICON_SIZE = QSize(30, 30);
const QSize TOOL_BUTTON_SIZE = QSize(36, 36);
}

ShapeToolWidget::ShapeToolWidget(DWidget *parent) :
    DLabel(parent)
{
    qCDebug(dsrApp) << "ShapeToolWidget constructor called.";
    initWidget();
}

ShapeToolWidget::~ShapeToolWidget()
{
    qCDebug(dsrApp) << "ShapeToolWidget destructor called.";
}

void ShapeToolWidget::initWidget()
{
    qCDebug(dsrApp) << "ShapeToolWidget::initWidget called.";
    setMouseTracking(true);
    setAcceptDrops(true);
    
    // 初始化形状按钮
    initShapeButtons();
    
    // 创建布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);  // 设置合适的间距
    mainLayout->addWidget(m_rectButton);
    mainLayout->addWidget(m_ovalButton);
    mainLayout->addStretch();  // 添加弹性空间，确保按钮靠左对齐
    
    setLayout(mainLayout);
    
    qCDebug(dsrApp) << "ShapeToolWidget initialized with size:" << size();
}

void ShapeToolWidget::initShapeButtons()
{
    qCDebug(dsrApp) << "ShapeToolWidget::initShapeButtons called.";
    setFixedSize(96,50);
    // 创建按钮组
    m_shapeBtnGroup = new QButtonGroup(this);
    m_shapeBtnGroup->setExclusive(true);
    
    // 创建矩形按钮
    m_rectButton = new ToolButton(this);
    m_rectButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_rectButton->setIconSize(TOOL_ICON_SIZE);
    m_rectButton->setIcon(QIcon::fromTheme(QString("rectangle-normal")));
    m_rectButton->setCheckable(true);
    Utils::setAccessibility(m_rectButton, "rectangle_button");
    
    // 创建椭圆按钮
    m_ovalButton = new ToolButton(this);
    m_ovalButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_ovalButton->setIconSize(TOOL_ICON_SIZE);
    m_ovalButton->setIcon(QIcon::fromTheme(QString("oval-normal")));
    m_ovalButton->setCheckable(true);
    Utils::setAccessibility(m_ovalButton, "oval_button");
    
    // 添加到按钮组
    m_shapeBtnGroup->addButton(m_rectButton);
    m_shapeBtnGroup->addButton(m_ovalButton);
    
    // 连接信号槽
    connect(m_shapeBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
    [=](QAbstractButton *button) {
        if (button == m_rectButton) {
            qCDebug(dsrApp) << "Rectangle button clicked.";
            emit shapeSelected("rectangle");
            ConfigSettings::instance()->setValue("shape", "current", "rectangle");
        } else if (button == m_ovalButton) {
            qCDebug(dsrApp) << "Oval button clicked.";
            emit shapeSelected("oval");
            ConfigSettings::instance()->setValue("shape", "current", "oval");
        }
    });
    
    // 根据配置设置默认选中状态
    QString currentShape = ConfigSettings::instance()->getValue("shape", "current").toString();
    if (currentShape == "oval") {
        m_ovalButton->setChecked(true);
    } else {
        // 默认选中矩形
        m_rectButton->setChecked(true);
    }
}

void ShapeToolWidget::selectShape(const QString &shape)
{
    qCDebug(dsrApp) << "ShapeToolWidget::selectShape called with shape:" << shape;
    if (shape == "rectangle") {
        m_rectButton->setChecked(true);
        emit shapeSelected("rectangle");
    } else if (shape == "oval") {
        m_ovalButton->setChecked(true);
        emit shapeSelected("oval");
    }
}
