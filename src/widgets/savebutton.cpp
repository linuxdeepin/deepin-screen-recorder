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

SaveButton::SaveButton(QWidget *parent)
    : ToolButton(parent)
{
    qCDebug(dsrApp) << "SaveButton constructor entered";
    setFixedSize(kSaveButtonWidth, kSaveButtonHeight);
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
    
    setSaveIcon(QIcon::fromTheme("save"));
    setListIcon(QIcon::fromTheme("dropdown"));
}

SaveButton::~SaveButton()
{

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
        
        // 判断是浅色主题还是暗黑主题
        QColor windowColor = palette().color(QPalette::Window);
        bool isLightTheme = windowColor.lightness() > 128;
        
        // 判断当前hover/press的区域
        bool highlightRightArea = (m_optionsMenu && m_optionsMenu->isVisible()) || (currentMouseX > kSaveAreaWidth);
        
        // 定义颜色：基础色(0.1)、hover色(0.15)、press色(0.2)
        // 浅色模式: rgba(0,0,0,alpha)  深色模式: rgba(255,255,255,alpha)
        QColor baseColor = isLightTheme ? QColor(0, 0, 0, 25) : QColor(255, 255, 255, 25);      // 0.1 * 255 ≈ 25
        QColor hoverColor = isLightTheme ? QColor(0, 0, 0, 38) : QColor(255, 255, 255, 38);     // 0.15 * 255 ≈ 38
        QColor pressColor = isLightTheme ? QColor(0, 0, 0, 51) : QColor(255, 255, 255, 51);     // 0.2 * 255 ≈ 51
        
        QColor leftColor, rightColor;
        
        if (m_saveClicked) {
            leftColor = pressColor;
            rightColor = baseColor;
        } else if (m_listClicked) {
            leftColor = baseColor;
            rightColor = pressColor;
        } else if (highlightRightArea) {
            leftColor = baseColor;
            rightColor = hoverColor;
        } else {
            leftColor = hoverColor;
            rightColor = baseColor;
        }
        
        // 绘制左边保存区域
        painter.setBrush(leftColor);
        painter.setPen(Qt::NoPen);
        QRectF leftRect(0, 0, kSaveAreaWidth + kCornerRadius, rect.height());
        QPainterPath leftPath;
        leftPath.addRoundedRect(leftRect, kCornerRadius, kCornerRadius);
        painter.setClipRect(0, 0, kSaveAreaWidth, rect.height());
        painter.fillPath(leftPath, leftColor);
        painter.setClipping(false);
        
        // 绘制右边下拉区域
        painter.setBrush(rightColor);
        QRectF rightRect(kSaveAreaWidth - kCornerRadius, 0, kListAreaWidth + kCornerRadius, rect.height());
        QPainterPath rightPath;
        rightPath.addRoundedRect(rightRect, kCornerRadius, kCornerRadius);
        painter.setClipRect(kSaveAreaWidth, 0, kListAreaWidth, rect.height());
        painter.fillPath(rightPath, rightColor);
        painter.setClipping(false);
    }
    
    QPainter painter(this);
    QRect rect = this->rect();
    
    if (!m_saveIcon.isNull()) {
        int iconSize = 24;
        int iconX = kLeftPadding + (kSaveAreaWidth - kLeftPadding - iconSize) / 2;
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
    if (!m_hoverFlag) {
        m_hoverFlag = true;
        update();
    }
    
    if (event->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const bool clickOnList = event->position().x() > kSaveAreaWidth;
#else
        const bool clickOnList = event->x() > kSaveAreaWidth;
#endif
        if (clickOnList) {
            m_saveClicked = false;
            m_listClicked = true;
            update();
            event->accept();
            return;
        }
        m_saveClicked = true;
        m_listClicked = false;
        update();
    }
    ToolButton::mousePressEvent(event);
}

void SaveButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_listClicked && m_optionsMenu) {
            emit expandSaveOption(true);
            QPoint menuPos = mapToGlobal(rect().bottomLeft());
            m_optionsMenu->popup(menuPos);
            m_saveClicked = false;
            m_listClicked = false;
            update();
            event->accept();
            return;
        } else if (m_saveClicked) {
            emit saveAction();
            emit clicked(); 
        }
    }
    
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
