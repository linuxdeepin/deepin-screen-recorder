// Copyright (C) 2020 ~ 2026 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbarwidget.h"
#include "accessibility/acTextDefine.h"
#include "putils.h"
#include "../../utils/log.h"

#include <QActionGroup>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include "mainwindow.h"

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
    } else if (PUtils::isTreelandMode) {
        // Treeland：作为主贴图窗口的 subsurface；背景改为自绘，避免 DBlur 在 subsurface 下失效。
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
        qCDebug(dsrApp) << "Window flags set for Treeland mode.";
    } else if (sysIsV23) {
        //v23上ToolTip会导致工具栏的圆角失效
        setWindowFlags(Qt::FramelessWindowHint /*| Qt::WindowStaysOnTopHint*/ | Qt::ToolTip);
        qCDebug(dsrApp) << "Window flags set for System V23 (non-Wayland).";
    } else {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        qCDebug(dsrApp) << "Window flags set for default mode.";
    }
    if (!PUtils::isTreelandMode) {
        this->setWindowFlags(windowFlags() | Qt::BypassWindowManagerHint);
    }

    this->setRadius(30);
    this->setBlurEnabled(!PUtils::isTreelandMode);
    this->setMode(DBlurEffectWidget::GaussianBlur);
    this->setBlendMode(DBlurEffectWidget::InWidgetBlend);
    if (PUtils::isTreelandMode) {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAutoFillBackground(false);
    }
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
    
    // 确保几何信息更新
    this->updateGeometry();
    this->layout()->activate();
    
    if (this->isHidden()) {
        this->show();
        qCDebug(dsrApp) << "Toolbar was hidden, now shown.";
    }
    
    // 再次确保几何信息正确
    QRect geom = this->geometry();
    qCDebug(dsrApp) << "Toolbar geometry after showing:" << geom;
    
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
    connect(m_subTool, &SubToolWidget::signalSaveToLocalButtonClicked, this, &ToolBarWidget::signalSaveToLocalButtonClicked);
    qCDebug(dsrApp) << "Connected signalOcrButtonClicked.";

    m_mainTool = new MainToolWidget(this);
    qCDebug(dsrApp) << "MainToolWidget created.";
    connect(m_mainTool, SIGNAL(signalCloseButtonClicked()), this, SIGNAL(sendCloseButtonClicked()));
    qCDebug(dsrApp) << "Connected signalCloseButtonClicked.";
    connect(m_mainTool, SIGNAL(signalSaveButtonClicked()), this, SIGNAL(sendSaveButtonClicked()));
    qCDebug(dsrApp) << "Connected signalSaveButtonClicked.";
    connect(m_mainTool, SIGNAL(signalSaveToLocalButtonClicked()), this, SIGNAL(sendSaveToLocalButtonClicked()));
    qCDebug(dsrApp) << "Connected signalSaveToLocalButtonClicked.";

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    qCDebug(dsrApp) << "Horizontal layout created.";
    hLayout->setContentsMargins(10, 0, 10, 0);
    hLayout->addWidget(m_subTool, 0, Qt::AlignCenter);
    hLayout->addWidget(m_mainTool, 0,  Qt::AlignCenter);
    setLayout(hLayout);
    qCDebug(dsrApp) << "Layout set for toolbar widget.";
    setMouseTracking(true);
}

void ToolBarWidget::paintEvent(QPaintEvent *event)
{
    if (!PUtils::isTreelandMode) {
        DBlurEffectWidget::paintEvent(event);
        return;
    }

    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 铺满客户区的不透明圆角底（单色、无描边）
    const QRectF backgroundRect = QRectF(rect());
    const qreal radius = 16.0;

    const QColor backgroundColor =
        (DGuiApplicationHelper::instance()->themeType() == THEMETYPE)
            ? QColor(237, 237, 237, 255)
            : QColor(48, 48, 48, 255);

    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor);
    painter.drawRoundedRect(backgroundRect, radius, radius);
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

    MainWindow* parentWindow = qobject_cast<MainWindow*>(this->parent());
    if (parentWindow) {
        parentWindow->checkToolbarVisibility();
    }

    QWidget::enterEvent(event);
}

// 处理鼠标离开工具栏的情况
void ToolBarWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    qCDebug(dsrApp) << "Mouse left ToolBarWidget.";

    MainWindow* parentWindow = qobject_cast<MainWindow*>(this->parent());
    if (parentWindow) {
        // 当鼠标离开时，我们不直接隐藏，而是让主窗口启动隐藏定时器
        parentWindow->startToolbarHideTimer();
    }

    QWidget::leaveEvent(event);
}
