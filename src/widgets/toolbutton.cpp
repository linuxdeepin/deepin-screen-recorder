// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbutton.h"
#include <QPainter>
#include <QStyleOptionButton>
#include <QStyleOptionToolButton>
#include <QPaintEvent>
#include <QApplication>
#include <QMenu>
#include <QGuiApplication>
#include <QScreen>
#include <QIcon>

namespace {
// 红点常量定义（基于SVG设计）
constexpr float SVG_WIDTH = 24.0f;
constexpr float SVG_HEIGHT = 24.0f;
constexpr float SVG_DOT_X = 21.0f;
constexpr float SVG_DOT_Y = 3.0f;
constexpr float SVG_DOT_RADIUS = 3.0f;
constexpr int MIN_DOT_RADIUS = 2;
const QColor RED_DOT_COLOR = QColor(0xFF, 0x40, 0x40);
}

ToolButton::ToolButton(QWidget *parent)
    : DToolButton(parent)
    , m_hasHoverState(true)
    , m_isOptionButton(false)
    , m_isUndoButton(false)
    , m_isMousePress(false)
    , m_lastCursorShape(nullptr)
{
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
}

ToolButton::~ToolButton()
{
}

void ToolButton::setHoverState(const bool hasHover)
{
    m_hasHoverState = hasHover;
}

void ToolButton::setOptionButtonFlag(const bool flag)
{
    m_isOptionButton = flag;
}

void ToolButton::setUndoButtonFlag(const bool flag)
{
    m_isUndoButton = flag;
}

void ToolButton::setBadgeIcon(const QIcon &icon)
{
    m_badgeIcon = icon;
    update();
}

void ToolButton::setBadgeIcon(const QString &iconPath)
{
    if (!iconPath.isEmpty()) {
        m_badgeIcon = QIcon(iconPath);
    } else {
        m_badgeIcon = QIcon();
    }
    update();
}

QRect ToolButton::getIconRect(const QStyleOptionToolButton &opt) const
{
    QSize iconSize = opt.iconSize.isValid() ? opt.iconSize : QSize(24, 24);
    int iconX = (width() - iconSize.width()) / 2;
    int iconY = (height() - iconSize.height()) / 2;
    return QRect(iconX, iconY, iconSize.width(), iconSize.height());
}

void ToolButton::drawRedDot(QPainter &painter, const QStyleOptionToolButton &opt, const QRect &iconRect)
{
    if (!m_showRedDot) {
        return;
    }
    
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    const int actualDotX = iconRect.left() + (SVG_DOT_X / SVG_WIDTH) * iconRect.width();
    const int actualDotY = iconRect.top() + (SVG_DOT_Y / SVG_HEIGHT) * iconRect.height();
    const int actualDotRadius = qMax(MIN_DOT_RADIUS, (int)((SVG_DOT_RADIUS / SVG_WIDTH) * iconRect.width()));
    
    QPoint dotCenter(actualDotX, actualDotY);
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(RED_DOT_COLOR);
    painter.drawEllipse(dotCenter, actualDotRadius, actualDotRadius);
    
    painter.restore();
}

void ToolButton::drawBadge(QPainter &painter, const QStyleOptionToolButton &opt, const QRect &iconRect)
{
    if (m_badgeIcon.isNull()) {
        return;
    }
    
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    QSize badgeSize = m_badgeSize;
    int badgeX = iconRect.right() - badgeSize.width(); 
    int badgeY = iconRect.bottom() - badgeSize.height() + 1;  
    
    QRect badgeRect(badgeX, badgeY, badgeSize.width(), badgeSize.height());
    m_badgeIcon.paint(&painter, badgeRect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
    
    painter.restore();
}

void ToolButton::paintEvent(QPaintEvent *event)
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    
    QPainter p(this);
    QRect iconRect = getIconRect(opt);

    const bool hasMenu = menu();
    const bool isColorButton = property("name").isValid();
    
    if (hasMenu) {
        opt.features &= ~QStyleOptionToolButton::HasMenu;
    }
    if (!isChecked() && !m_menuActive && !isColorButton && m_hasHoverState) {
        if (opt.state & QStyle::State_MouseOver) {
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            
            QColor windowColor = palette().color(QPalette::Window);
            bool isLightTheme = windowColor.lightness() > 128;
            
            QColor bgColor;
            if (isLightTheme) {
                if (opt.state & QStyle::State_Sunken) {
                    bgColor = QColor(0, 0, 0, 38);
                } else {
                    bgColor = QColor(0, 0, 0, 25);
                }
            } else {
                if (opt.state & QStyle::State_Sunken) {
                    bgColor = QColor(255, 255, 255, 38);
                } else {
                    bgColor = QColor(255, 255, 255, 25);
                }
            }
            
            p.setBrush(bgColor);
            p.drawRoundedRect(rect(), 8, 8);
        }
    }

    if (isChecked() || m_menuActive) {
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        QColor bg = palette().highlight().color();
        if (m_menuActive && !property("name").isValid()) {
            bg = bg.lighter(110);
        }
        p.setBrush(bg);
        
        if (isColorButton) {
            QRect circleRect = QRect((width() - 18) / 2, (height() - 18) / 2, 18, 18);
            p.drawEllipse(circleRect);
        } else {
            p.drawRoundedRect(rect(), 8, 8);
        }

        if (isColorButton) {
            if (!opt.icon.isNull()) {
                QIcon::Mode iconMode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
                QRect colorIconRect = QRect((width() - 14) / 2, (height() - 14) / 2, 14, 14);
                opt.icon.paint(&p, colorIconRect, Qt::AlignCenter, iconMode, QIcon::On);
            }
        } else {
            opt.palette.setColor(QPalette::ButtonText, Qt::white);
            opt.palette.setColor(QPalette::WindowText, Qt::white);
            opt.palette.setColor(QPalette::Text, Qt::white);

            if (!opt.icon.isNull()) {
                const QSize targetIconSize = opt.iconSize.isValid() ? opt.iconSize : QSize(16, 16);
                const QIcon::Mode iconMode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
                const qreal devicePixelRatio = this->devicePixelRatioF();
                

                QPixmap base = opt.icon.pixmap(targetIconSize, devicePixelRatio, iconMode, QIcon::On);
                if (!base.isNull()) {
                    QPixmap tinted(base.size());
                    tinted.fill(Qt::transparent);
                    tinted.setDevicePixelRatio(devicePixelRatio);
                    
                    QPainter ip(&tinted);
                    ip.setRenderHint(QPainter::Antialiasing);
                    ip.drawPixmap(0, 0, base);
                    ip.setCompositionMode(QPainter::CompositionMode_SourceIn);
                    ip.fillRect(tinted.rect(), Qt::white);
                    ip.end();
                    
                    opt.icon = QIcon(tinted);
                }
            }

            opt.state &= ~QStyle::State_MouseOver;
            opt.state &= ~QStyle::State_Sunken;
            style()->drawControl(QStyle::CE_ToolButtonLabel, &opt, &p, this);
        }
        if (m_showRedDot) {
            drawRedDot(p, opt, iconRect);
        }
        if (!m_badgeIcon.isNull()) {
            drawBadge(p, opt, iconRect);
        }
        
        return;
    }

    if (isColorButton) {
        if (!opt.icon.isNull()) {
            QIcon::Mode iconMode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
            QRect colorIconRect = QRect((width() - 14) / 2, (height() - 14) / 2, 14, 14);
            opt.icon.paint(&p, colorIconRect, Qt::AlignCenter, iconMode, QIcon::Off);
        }
    } else if (hasMenu) {
        opt.state &= ~QStyle::State_MouseOver;
        opt.state &= ~QStyle::State_Sunken;
        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
    } else {
        opt.state &= ~QStyle::State_MouseOver;
        opt.state &= ~QStyle::State_Sunken;
        style()->drawControl(QStyle::CE_ToolButtonLabel, &opt, &p, this);
    }

    if (m_showRedDot) {
        drawRedDot(p, opt, iconRect);
    }
    if (!m_badgeIcon.isNull()) {
        drawBadge(p, opt, iconRect);
    }
}

void ToolButton::enterEvent(QEnterEvent *e)
{
    m_lastCursorShape = qApp->overrideCursor();
    qApp->setOverrideCursor(Qt::ArrowCursor);
    if (this->isEnabled() && m_hasHoverState) {
        // setFlat(false);
    }
    if (m_isUndoButton) {
        emit isInUndoBtn(true);
    }
    DToolButton::enterEvent(e);
}

void ToolButton::leaveEvent(QEvent *e)
{
    if (m_hasHoverState) {
        // setFlat(true);
    }
    DToolButton::leaveEvent(e);

    if (m_isMousePress) {
        qApp->setOverrideCursor(Qt::ArrowCursor);
        m_isMousePress = false;
    }

    if (m_isUndoButton) {
        emit isInUndoBtn(false);
    }
}

void ToolButton::mousePressEvent(QMouseEvent *e)
{
    if (m_isOptionButton) {
        m_isMousePress = true;
    }
    

    if (menu() && e->button() == Qt::LeftButton) {
        if (!isCheckable() || !isChecked()) {
            setDown(true);
        }
        update();
        return;
    }
    
    DToolButton::mousePressEvent(e);
}

void ToolButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (menu() && e->button() == Qt::LeftButton) {
        if (!isCheckable() || !isChecked()) {
            setDown(false);
        }
        update();
        m_menuActive = true;
        QMenu *m = menu();
        if (m) {
            QObject::connect(m, &QMenu::aboutToHide, this, [this]() {
                m_menuActive = false;
                update();
            }, Qt::SingleShotConnection);
        }
        this->showMenu();
        return;
    }
    
    DToolButton::mouseReleaseEvent(e);
}
