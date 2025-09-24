// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "savebutton.h"
#include "../utils/log.h"
#include <QMenu>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionButton>
#include <QMouseEvent>
#include <QStylePainter>
#include <QPainterPath>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif

SaveButton::SaveButton(DWidget *parent)
    : ToolButton(parent)
{
    qCDebug(dsrApp) << "SaveButton constructor entered";
    setFixedSize(kSaveButtonWidth, kSaveButtonHeight);
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);  // 启用鼠标跟踪以获取精确的悬停区域
    
    // 设置默认图标
    setSaveIcon(QIcon::fromTheme("save"));
    setListIcon(QIcon::fromTheme("dropdown"));
}

SaveButton::~SaveButton()
{
    qCDebug(dsrApp) << "SaveButton destructor entered";
}

void SaveButton::setOptionsMenu(QMenu *menu)
{
    m_menu = menu;
    if (m_menu) {
        connect(m_menu, &QMenu::aboutToHide, this, &SaveButton::onMenuAboutToHide);
    }
}

void SaveButton::setSaveIcon(const QIcon &icon)
{
    m_saveIcon = icon;
    update();
}

void SaveButton::setListIcon(const QIcon &icon)
{
    m_listIcon = icon;
    update();
}

void SaveButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect rect = this->rect();
    
    // 绘制左侧区域背景（左半圆角矩形）
    bool leftHovered = m_hoverFlag && currentMouseX <= kSaveAreaWidth;
    if (leftHovered || m_saveClicked) {
        // 最简单的方法：画一个完整的圆角矩形，然后裁剪右半部分
        QRectF leftRect(0, 0, kSaveAreaWidth + kCornerRadius, rect.height());
        QPainterPath leftPath;
        leftPath.addRoundedRect(leftRect, kCornerRadius, kCornerRadius);
        
        // 裁剪掉超出左侧区域的部分
        QPainterPath clipPath;
        clipPath.addRect(0, 0, kSaveAreaWidth, rect.height());
        leftPath = leftPath.intersected(clipPath);
        
        QColor leftColor;
        if (m_saveClicked) {
            leftColor = QColor(0, 0, 0, 51); // 按下状态 - 20%不透明度
        } else {
            leftColor = QColor(0, 0, 0, 26); // 悬浮状态 - 10%不透明度
        }
        
        painter.fillPath(leftPath, leftColor);
    }
    
    // 绘制右侧区域背景（右半圆角矩形）
    bool rightHovered = m_hoverFlag && currentMouseX > kSaveAreaWidth;
    if (rightHovered || m_listClicked || (m_menu && m_menu->isVisible())) {
        // 最简单的方法：画一个完整的圆角矩形，然后裁剪左半部分
        QRectF rightRect(kSaveAreaWidth - kCornerRadius, 0, kListAreaWidth + kCornerRadius, rect.height());
        QPainterPath rightPath;
        rightPath.addRoundedRect(rightRect, kCornerRadius, kCornerRadius);
        
        // 裁剪掉超出右侧区域的部分
        QPainterPath clipPath;
        clipPath.addRect(kSaveAreaWidth, 0, kListAreaWidth, rect.height());
        rightPath = rightPath.intersected(clipPath);
        
        QColor rightColor;
        if (m_listClicked) {
            rightColor = QColor(0, 0, 0, 51); // 按下状态 - 20%不透明度
        } else {
            rightColor = QColor(0, 0, 0, 26); // 悬浮状态 - 10%不透明度
        }
        
        painter.fillPath(rightPath, rightColor);
    }
    
    // 绘制左侧保存图标
    if (!m_saveIcon.isNull()) {
        int iconSize = 20;
        int iconX = (kSaveAreaWidth - iconSize) / 2;
        int iconY = (rect.height() - iconSize) / 2;
        QRect iconRect(iconX, iconY, iconSize, iconSize);
        
        painter.save();
        if (m_saveClicked) {
            painter.setPen(palette().highlight().color());
        }
        m_saveIcon.paint(&painter, iconRect);
        painter.restore();
    }
    
    // 绘制右侧下拉图标
    if (!m_listIcon.isNull()) {
        int iconSize = 12;
        int iconX = kSaveAreaWidth + (kListAreaWidth - iconSize) / 2;
        int iconY = (rect.height() - iconSize) / 2;
        QRect iconRect(iconX, iconY, iconSize, iconSize);
        
        painter.save();
        if (m_listClicked || (m_menu && m_menu->isVisible())) {
            painter.setPen(palette().highlight().color());
        }
        m_listIcon.paint(&painter, iconRect);
        painter.restore();
    }
}

void SaveButton::mousePressEvent(QMouseEvent *event)
{
    ToolButton::mousePressEvent(event);
    if (!m_hoverFlag) {
        m_hoverFlag = true;
        update();
    }
    
    if (event->button() == Qt::LeftButton) {
        // 判断点击区域
        m_saveClicked = event->x() <= kSaveAreaWidth;
        m_listClicked = event->x() > kSaveAreaWidth;
        
        if (m_saveClicked) {
            emit saveAction();
            emit clicked(); // 兼容原有连接
        } else if (m_listClicked && m_menu) {
            emit expandSaveOption(true);
            m_menu->exec(mapToGlobal(rect().bottomLeft()));
        }
        update();
    }
}

void SaveButton::mouseReleaseEvent(QMouseEvent *event)
{
    ToolButton::mouseReleaseEvent(event);
    m_saveClicked = false;
    m_listClicked = false;
    update();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SaveButton::enterEvent(QEnterEvent *event)
{
    ToolButton::enterEvent(event);
    currentMouseX = event->position().x();
    if (!m_hoverFlag) {
        m_hoverFlag = true;
        update();
    }
}
#else
void SaveButton::enterEvent(QEvent *event)
{
    ToolButton::enterEvent(event);
    // Qt5没有位置信息，使用中点作为默认值
    currentMouseX = kSaveAreaWidth / 2;
    if (!m_hoverFlag) {
        m_hoverFlag = true;
        update();
    }
}
#endif

void SaveButton::leaveEvent(QEvent *event)
{
    ToolButton::leaveEvent(event);
    if (m_hoverFlag) {
        m_hoverFlag = false;
        m_saveClicked = false;
        m_listClicked = false;
        currentMouseX = 0;  // 重置鼠标位置
        update();
    }
}

void SaveButton::mouseMoveEvent(QMouseEvent *event)
{
    ToolButton::mouseMoveEvent(event);
    int oldMouseX = currentMouseX;
    currentMouseX = event->x();
    
    if (!m_hoverFlag) {
        m_hoverFlag = true;
        update();
    } else if (oldMouseX != currentMouseX) {
        // 鼠标在不同区域间移动时重绘
        bool oldLeftHovered = oldMouseX <= kSaveAreaWidth;
        bool newLeftHovered = currentMouseX <= kSaveAreaWidth;
        if (oldLeftHovered != newLeftHovered) {
            update();
        }
    }
}

void SaveButton::onMenuAboutToHide()
{
    m_listClicked = false;
    emit expandSaveOption(false);
    update();
}