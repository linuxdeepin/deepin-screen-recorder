// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbutton.h"
#include <QPainter>
#include <QStyleOptionButton>
#include <QPaintEvent>
#include <QApplication>

ToolButton::ToolButton(DWidget *parent)
    : DPushButton(parent)
    , m_hasHoverState(true)
    , m_isOptionButton(false)
    , m_isUndoButton(false)
    , m_isMousePress(false)
    , m_lastCursorShape(nullptr)
{
    setCheckable(true);
    setFlat(true);
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
        QStyleOptionButton option;
        initStyleOption(&option);
        option.features &= ~QStyleOptionButton::HasMenu;
        QPainter painter(this);
        style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
    } else {
        DPushButton::paintEvent(event);
    }
}

void ToolButton::enterEvent(QEnterEvent *e)
{
    m_lastCursorShape = qApp->overrideCursor();
    qApp->setOverrideCursor(Qt::ArrowCursor);
    if (this->isEnabled() && m_hasHoverState) {
        setFlat(false);
    }
    if (m_isUndoButton) {
        emit isInUndoBtn(true);
    }
    DPushButton::enterEvent(e);
}

void ToolButton::leaveEvent(QEvent *e)
{
    if (m_hasHoverState) {
        setFlat(true);
    }
    DPushButton::leaveEvent(e);

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
    DPushButton::mousePressEvent(e);
}
