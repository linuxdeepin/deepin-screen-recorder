// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorbutton.h"
#include "../utils/log.h"

#include <QPainter>
#include <QDebug>

const QSize BUTTON_SIZE = QSize(16, 16);
//const int  ELLIPSE_MARGIN = 1;
ColorButton::ColorButton(QColor bgColor, DWidget *parent)
    : DPushButton(parent)
{
    setFixedSize(BUTTON_SIZE);
    setCheckable(true);
    m_bgColor = bgColor;
    qCDebug(dsrApp) << "ColorButton created with color:" << m_bgColor;

    connect(this, &ColorButton::clicked, this, &ColorButton::setColorBtnChecked);
}

void ColorButton::setColorBtnChecked()
{
    qCDebug(dsrApp) << "setColorBtnChecked called";
    update();
    if (this->isChecked()) {
        qCDebug(dsrApp) << "Color button is checked, emitting updatePaintColor:" << m_bgColor;
        emit updatePaintColor(m_bgColor);
    }
}
ColorButton::~ColorButton()
{
    qCDebug(dsrApp) << "ColorButton destructor called";
}
