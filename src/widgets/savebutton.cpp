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
    if (m_optionsMenu) {
        disconnect(m_optionsMenu, &QMenu::aboutToHide, this, &SaveButton::onMenuAboutToHide);
    }
    
    m_optionsMenu = menu;
    
    if (m_optionsMenu) {
        connect(m_optionsMenu, &QMenu::aboutToHide, this, &SaveButton::onMenuAboutToHide);
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
    
    if (m_hoverFlag || (m_optionsMenu && m_optionsMenu->isVisible())) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QRect rect = this->rect();
        
        QColor hoverColor(0, 0, 0, 26); 
        painter.setBrush(hoverColor);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect, kCornerRadius, kCornerRadius);
        
        QColor deepenColor(0, 0, 0, 13);
        painter.setBrush(deepenColor);
        painter.setPen(Qt::NoPen);
        
        if (currentMouseX <= kSaveAreaWidth) {
            QRectF leftRect(0, 0, kSaveAreaWidth + kCornerRadius, rect.height());
            QPainterPath leftPath;
            leftPath.addRoundedRect(leftRect, kCornerRadius, kCornerRadius);
            
            painter.setClipRect(0, 0, kSaveAreaWidth, rect.height());
            painter.fillPath(leftPath, deepenColor);
            painter.setClipping(false);
        } else {
            QRectF rightRect(kSaveAreaWidth - kCornerRadius, 0, kListAreaWidth + kCornerRadius, rect.height());
            QPainterPath rightPath;
            rightPath.addRoundedRect(rightRect, kCornerRadius, kCornerRadius);
            
            painter.setClipRect(kSaveAreaWidth, 0, kListAreaWidth, rect.height());
            painter.fillPath(rightPath, deepenColor);
            painter.setClipping(false);
        }
    }
    
    QPainter painter(this);
    QRect rect = this->rect();
    
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
    
    if (!m_listIcon.isNull()) {
        int iconSize = 12;
        int iconX = kSaveAreaWidth + (kListAreaWidth - iconSize) / 2;
        int iconY = (rect.height() - iconSize) / 2;
        QRect iconRect(iconX, iconY, iconSize, iconSize);
        
        painter.save();
        if (m_listClicked || (m_optionsMenu && m_optionsMenu->isVisible())) {
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_saveClicked = event->position().x() <= kSaveAreaWidth;
        m_listClicked = event->position().x() > kSaveAreaWidth;
#else
        m_saveClicked = event->x() <= kSaveAreaWidth;
        m_listClicked = event->x() > kSaveAreaWidth;
#endif
        
        if (m_saveClicked) {
            emit saveAction();
            emit clicked(); 
        } else if (m_listClicked && m_optionsMenu) {
            emit expandSaveOption(true);
            // 显示保存菜单
            QPoint menuPos = mapToGlobal(rect().bottomLeft());
            m_optionsMenu->exec(menuPos);
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
        currentMouseX = 0;  
        update();
    }
}

void SaveButton::mouseMoveEvent(QMouseEvent *event)
{
    ToolButton::mouseMoveEvent(event);
    int oldMouseX = currentMouseX;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    currentMouseX = event->position().x();
#else
    currentMouseX = event->x();
#endif
    
    if (!m_hoverFlag) {
        m_hoverFlag = true;
        update();
    } else if (oldMouseX != currentMouseX) {
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