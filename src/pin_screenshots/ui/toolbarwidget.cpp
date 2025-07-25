// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbarwidget.h"
#include "accessibility/acTextDefine.h"
#include "putils.h"
#include "../../utils/log.h"

#include <QActionGroup>
#include <QMouseEvent>
#include <QTimer>

#include <DFontSizeManager>
#include <DSysInfo>

DCORE_USE_NAMESPACE

#define THEMETYPE 1 // 主题颜色为浅色

const QSize MIN_TOOLBAR_WIDGET_SIZE = QSize(210, 56);

ToolBarWidget::ToolBarWidget(DWidget *parent): DBlurEffectWidget(parent)
{
    qCDebug(dsrApp) << "ToolBarWidget constructor called.";
    const bool sysIsV23 = DSysInfo::majorVersion().toInt() == 23;

    if (PUtils::isWaylandMode) {
        setWindowFlags(Qt::Sheet | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
        qCDebug(dsrApp) << "Window flags set for Wayland mode.";
    } else if (sysIsV23) {
        //v23上ToolTip会导致工具栏的圆角失效
        setWindowFlags(Qt::FramelessWindowHint /*| Qt::WindowStaysOnTopHint*/ | Qt::ToolTip);
        qCDebug(dsrApp) << "Window flags set for System V23 (non-Wayland).";
    } else {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        qCDebug(dsrApp) << "Window flags set for default mode.";
    }

    this->setRadius(30);
    this->setBlurEnabled(true);
    this->setMode(DBlurEffectWidget::GaussianBlur);
    this->setBlendMode(DBlurEffectWidget::InWidgetBlend);
    qCDebug(dsrApp) << "Blur effect properties set.";
    // 初始化主题样式
    if (DGuiApplicationHelper::instance()->themeType() == THEMETYPE) {
        this->setMaskColor(QColor(255, 255, 255, 76));
        qCDebug(dsrApp) << "Mask color set to light theme.";
    } else {
        this->setMaskColor(QColor(0, 0, 0, 76));
        qCDebug(dsrApp) << "Mask color set to dark theme.";
    }

    initToolBarWidget(); // 初始化按钮菜单
    qCDebug(dsrApp) << "initToolBarWidget finished.";
    // 工具栏样式跟随系统样式改变
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ToolBarWidget::onThemeTypeChange);
    qCDebug(dsrApp) << "Connected themeTypeChanged signal.";
}

// 获取保存信息
QPair<int, int> ToolBarWidget::getSaveInfo()
{
    qCDebug(dsrApp) << "Getting save information from subtool.";
    return m_subTool->getSaveInfo();
}

void ToolBarWidget::showAt(QPoint pos, bool isfirstTime)
{
    qCDebug(dsrApp) << "Showing toolbar at position:" << pos << ", isFirstTime:" << isfirstTime;
    this->move(pos);
    if (this->isHidden()) {
        this->show();
        qCDebug(dsrApp) << "Toolbar was hidden, now shown.";
    }
    if (isfirstTime) {
        this->hide();
        qCDebug(dsrApp) << "First time showing, hiding toolbar.";
    }
}

// 选项按钮被点击
void ToolBarWidget::onOptionButtonClicked()
{
    qCDebug(dsrApp) << "Option button clicked, calling subTool's onOptionButtonClicked.";
    m_subTool->onOptionButtonClicked();
}

// 主题变化
void ToolBarWidget::onThemeTypeChange(DGuiApplicationHelper::ColorType themeType)
{
    qCDebug(dsrApp) << "Theme type changed to:" << themeType;
    if (themeType == THEMETYPE) {
        this->setMaskColor(QColor(255, 255, 255, 76));
        qCDebug(dsrApp) << "Mask color set to light theme due to theme change.";
    } else {
        this->setMaskColor(QColor(0, 0, 0, 76));
        qCDebug(dsrApp) << "Mask color set to dark theme due to theme change.";
    }
}

void ToolBarWidget::initToolBarWidget()
{
    qCDebug(dsrApp) << "Initializing toolbar widget.";
//    this->setFixedSize(MIN_TOOLBAR_WIDGET_SIZE);
    this->setMinimumSize(MIN_TOOLBAR_WIDGET_SIZE);
    qCDebug(dsrApp) << "Minimum size set for toolbar widget.";
    m_subTool = new SubToolWidget(this);
    qCDebug(dsrApp) << "SubToolWidget created.";
    connect(m_subTool, SIGNAL(signalOcrButtonClicked()), this, SIGNAL(sendOcrButtonClicked()));
    qCDebug(dsrApp) << "Connected signalOcrButtonClicked.";

    m_mainTool = new MainToolWidget(this);
    qCDebug(dsrApp) << "MainToolWidget created.";
    connect(m_mainTool, SIGNAL(signalCloseButtonClicked()), this, SIGNAL(sendCloseButtonClicked()));
    qCDebug(dsrApp) << "Connected signalCloseButtonClicked.";
    connect(m_mainTool, SIGNAL(signalSaveButtonClicked()), this, SIGNAL(sendSaveButtonClicked()));
    qCDebug(dsrApp) << "Connected signalSaveButtonClicked.";

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    qCDebug(dsrApp) << "Horizontal layout created.";
    hLayout->setContentsMargins(10, 0, 10, 0);
    hLayout->addWidget(m_subTool, 0, Qt::AlignCenter);
    hLayout->addWidget(m_mainTool, 0,  Qt::AlignCenter);
    setLayout(hLayout);
    qCDebug(dsrApp) << "Layout set for toolbar widget.";
}

//重写鼠标移动事件：解决工具栏可以被拖动的问题
void ToolBarWidget::mouseMoveEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "Mouse move event received in ToolBarWidget (ignored to prevent dragging).";
    Q_UNUSED(event);
    //qDebug() << event->button() << event->x() << event->y();
    //QWidget::mouseMoveEvent(event);
}

// 确保鼠标进入工具栏时工具栏保持显示
void ToolBarWidget::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    qCDebug(dsrApp) << "Mouse entered ToolBarWidget.";
    // 确保工具栏保持显示
    if (this->isHidden()) {
        this->show();
    }
    QWidget::enterEvent(event);
}

// 处理鼠标离开工具栏的情况
void ToolBarWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    qCDebug(dsrApp) << "Mouse left ToolBarWidget.";
    
    // 获取当前鼠标全局位置
    QPoint globalPos = QCursor::pos();
    
    // 获取父窗口（MainWindow）
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (!parentWidget) {
        QWidget::leaveEvent(event);
        return;
    }
    
    // 检查鼠标是否在父窗口上
    if (!parentWidget->geometry().contains(parentWidget->mapFromGlobal(globalPos))) {
        // 鼠标既不在工具栏上也不在主窗口上，隐藏工具栏
        QTimer::singleShot(100, this, [this]() {
            // 再次检查鼠标位置，避免在移动过程中错误隐藏
            QPoint currentPos = QCursor::pos();
            if (!this->geometry().contains(currentPos) && 
                !qobject_cast<QWidget*>(parent())->geometry().contains(
                    qobject_cast<QWidget*>(parent())->mapFromGlobal(currentPos))) {
                this->hide();
                qCDebug(dsrApp) << "Hiding toolbar after mouse left both toolbar and main window.";
            }
        });
    }
    
    QWidget::leaveEvent(event);
}
