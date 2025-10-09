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

void ToolButton::paintEvent(QPaintEvent *event)
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    const bool hasMenu = menu();
    const bool isColorButton = property("name").isValid();
    
    if (hasMenu) {
        opt.features &= ~QStyleOptionToolButton::HasMenu;
    }

    if (isChecked() || m_menuActive) {
        QPainter p(this);
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
                QRect iconRect = QRect((width() - 14) / 2, (height() - 14) / 2, 14, 14);
                opt.icon.paint(&p, iconRect, Qt::AlignCenter, iconMode, QIcon::On);
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
        return;
    }

    if (isColorButton) {
        QPainter p(this);
        if (!opt.icon.isNull()) {
            QIcon::Mode iconMode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
            QRect iconRect = QRect((width() - 14) / 2, (height() - 14) / 2, 14, 14);
            opt.icon.paint(&p, iconRect, Qt::AlignCenter, iconMode, QIcon::Off);
        }
    } else if (hasMenu) {
        QPainter p(this);
        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
    } else {
        DToolButton::paintEvent(event);
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
