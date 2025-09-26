// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbutton.h"
#include <QPainter>
#include <QStyleOptionButton>
#include <QPaintEvent>
#include <QApplication>

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
    if (menu()) {
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        opt.features &= ~QStyleOptionToolButton::HasMenu; // 注意：用 ToolButton 的枚举
        QPainter p(this);
        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this); // 按 ToolButton 画
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
    DToolButton::mousePressEvent(e);
}
