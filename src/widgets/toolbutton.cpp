// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbutton.h"
#include <QPainter>
#include <QStyleOptionButton>
#include <QPaintEvent>

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
