/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QPainter>
#include <QEvent>
#include <QApplication>
#include "record_button.h"
#include "utils.h"
#include "constant.h"
#include <DHiDPIHelper>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

const int RecordButton::WIDTH = 128;
const int RecordButton::HEIGHT = 86;
const int RecordButton::TEXT_PADDING = 0;

RecordButton::RecordButton(DPushButton *parent) : DPushButton(parent)
{
    installEventFilter(this);  // add event filter
    setMouseTracking(true);

    isFocus = false;
    isPress = false;

    normalImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("record_icon_normal.svg"));
    hoverImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("record_icon_hover.svg"));
    pressImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("record_icon_press.svg"));
}

void RecordButton::setText(QString string)
{
    text = string;
    textSize = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, string);

    setFixedSize(std::max(textSize.width() + Constant::RECTANGLE_PADDING * 2, WIDTH), std::max(textSize.height() + Constant::RECTANGLE_PADDING * 2, HEIGHT));
}

void RecordButton::paintEvent(QPaintEvent *)
{
    // Init.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QString status = "NORMAL";
    if (isFocus) {
        if (isPress) {
            status = "PRESS";
        } else {
            status = "HOVER";
        }
    }

    // Draw background.
    qreal backgroundOpacity = 1.0;
    if (status == "NORMAL") {
        backgroundOpacity = 0.6;
    } else if (status == "PRESS") {
        backgroundOpacity = 0.2;
    } else if (status == "HOVER") {
        backgroundOpacity = 0.7;
    }

    Utils::drawTooltipBackground(painter, rect(), backgroundOpacity);

    // Draw icon.
    qreal devicePixelRatio = qApp->devicePixelRatio();
    painter.setOpacity(1);
    int iconX = rect().x() + (rect().width() - normalImg.width() / devicePixelRatio) / 2;
    int iconY = rect().y() + (rect().height() - normalImg.height() / devicePixelRatio - textSize.height() - TEXT_PADDING) / 2;
    if (status == "NORMAL") {
        painter.drawPixmap(QPoint(iconX, iconY), normalImg);
    } else if (status == "PRESS") {
        painter.drawPixmap(QPoint(iconX, iconY), pressImg);
    } else if (status == "HOVER") {
        painter.drawPixmap(QPoint(iconX, iconY), hoverImg);
    }

    // Draw text.
    int textX = rect().x();
    int textY = iconY + normalImg.height() / devicePixelRatio + TEXT_PADDING;
    Utils::drawTooltipText(painter, text, "#e34342", Constant::RECTANGLE_FONT_SIZE, QRect(textX, textY, rect().width(), textSize.height()));
}

bool RecordButton::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        isPress = true;
        repaint();
    } else if (event->type() == QEvent::MouseButtonRelease) {
        isPress = false;
        repaint();
    } else if (event->type() == QEvent::Enter) {
        isFocus = true;
        repaint();
    } else if (event->type() == QEvent::Leave) {
        isFocus = false;
        repaint();
    }

    return false;
}
