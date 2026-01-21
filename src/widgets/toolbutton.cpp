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

// 静态成员：背景图指针
static const QPixmap *s_backgroundPixmap = nullptr;

void ToolButton::setBackgroundPixmap(const QPixmap *pixmap)
{
    s_backgroundPixmap = pixmap;
}

void ToolButton::clearBackgroundPixmap()
{
    s_backgroundPixmap = nullptr;
}

const QPixmap *ToolButton::backgroundPixmap()
{
    return s_backgroundPixmap;
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
    if (!isChecked() && !m_menuActive && !isColorButton && m_hasHoverState && isEnabled()) {
        if (opt.state & QStyle::State_MouseOver) {
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            
            QColor windowColor = palette().color(QPalette::Window);
            bool isLightTheme = windowColor.lightness() > 128;
            
            QColor bgColor;
            if (isLightTheme) {
                // 浅色模式: hover #000000 10%, press #000000 20%
                if (opt.state & QStyle::State_Sunken) {
                    bgColor = QColor(0, 0, 0, 51);      // 20% = 0.2 * 255 ≈ 51
                } else {
                    bgColor = QColor(0, 0, 0, 25);      // 10% = 0.1 * 255 ≈ 25
                }
            } else {
                // 深色模式: hover #FFFFFF 10%, press #000000 15%
                if (opt.state & QStyle::State_Sunken) {
                    bgColor = QColor(0, 0, 0, 38);      // 15% = 0.15 * 255 ≈ 38
                } else {
                    bgColor = QColor(255, 255, 255, 25); // 10% = 0.1 * 255 ≈ 25
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

    const bool isSunken = opt.state & QStyle::State_Sunken;
    
    if (isColorButton) {
        if (!opt.icon.isNull()) {
            QIcon::Mode iconMode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
            QRect colorIconRect = QRect((width() - 14) / 2, (height() - 14) / 2, 14, 14);
            opt.icon.paint(&p, colorIconRect, Qt::AlignCenter, iconMode, QIcon::Off);
        }
    } else if (isSunken && !opt.icon.isNull()) {
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
            ip.fillRect(tinted.rect(), palette().highlight().color());
            ip.end();
            
            opt.icon = QIcon(tinted);
        }
        opt.state &= ~QStyle::State_MouseOver;
        opt.state &= ~QStyle::State_Sunken;
        style()->drawControl(QStyle::CE_ToolButtonLabel, &opt, &p, this);
    } else if (hasMenu) {
        opt.state &= ~QStyle::State_MouseOver;
        opt.state &= ~QStyle::State_Sunken;
        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
    } else {
        // disabled 状态：根据背景亮度对图标着色
        if (!isEnabled() && !opt.icon.isNull()) {
            const QSize iconSize = opt.iconSize.isValid() ? opt.iconSize : QSize(16, 16);
            
            // 检查是否需要重新计算
            QPoint currentGlobalPos = mapToGlobal(QPoint(0, 0));
            if (m_needRecalculateIcon || 
                m_lastGlobalPos != currentGlobalPos || 
                m_lastSize != this->size()) {
                
                m_cachedDisabledIcon = tintIconByBackground(opt.icon, iconSize);
                m_lastGlobalPos = currentGlobalPos;
                m_lastSize = this->size();
                m_needRecalculateIcon = false;
            }
            
            opt.icon = m_cachedDisabledIcon;
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

void ToolButton::moveEvent(QMoveEvent *event)
{
    DToolButton::moveEvent(event);
    m_needRecalculateIcon = true;
}

void ToolButton::resizeEvent(QResizeEvent *event)
{
    DToolButton::resizeEvent(event);
    m_needRecalculateIcon = true;
}

QIcon ToolButton::tintIconByBackground(const QIcon &icon, const QSize &iconSize) const
{
    if (!s_backgroundPixmap || s_backgroundPixmap->isNull()) {
        return icon;
    }
    
    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    QRect btnRect(globalPos, this->size());
    QRect intersect = btnRect.intersected(s_backgroundPixmap->rect());
    
    if (intersect.isEmpty()) {
        return icon;
    }
    
    // 计算按钮区域的背景亮度
    QImage bgImage = s_backgroundPixmap->copy(intersect).toImage();
    qint64 totalBrightness = 0;
    int pixelCount = 0;
    const int step = 2;
    
    for (int y = 0; y < bgImage.height(); y += step) {
        for (int x = 0; x < bgImage.width(); x += step) {
            QColor color = bgImage.pixelColor(x, y);
            int brightness = (color.red() * 299 + color.green() * 587 + color.blue() * 114) / 1000;
            totalBrightness += brightness;
            pixelCount++;
        }
    }
    
    if (pixelCount == 0) {
        return icon;
    }
    
    // 暗背景用白色，亮背景用黑色
    int avgBrightness = totalBrightness / pixelCount;
    QColor tintColor = (avgBrightness < 128) ? Qt::white : Qt::black;
    tintColor.setAlpha(100);
    
    // 着色
    const qreal dpr = this->devicePixelRatioF();
    QPixmap base = icon.pixmap(iconSize, dpr, QIcon::Normal, QIcon::Off);
    if (base.isNull()) {
        return icon;
    }
    
    QPixmap tinted(base.size());
    tinted.fill(Qt::transparent);
    tinted.setDevicePixelRatio(dpr);
    
    QPainter p(&tinted);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPixmap(0, 0, base);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(tinted.rect(), tintColor);
    p.end();
    
    return QIcon(tinted);
}
