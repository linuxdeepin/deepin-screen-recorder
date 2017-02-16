/* -*- Mode: Vala; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2017 Deepin, Inc.
 *               2011 ~ 2017 Wang Yong
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
#include <QDebug>
#include "record_button.h"
#include "utils.h"

const int RecordButton::RECTANGLE_RAIUDS = 8;
const int RecordButton::WIDTH = 128;
const int RecordButton::HEIGHT = 86;
const int RecordButton::PADDING = 20;
const int RecordButton::TEXT_PADDING = 0;
    
RecordButton::RecordButton(QPushButton *parent) : QPushButton(parent)
{
    installEventFilter(this);  // add event filter

    isFocus = false;
    isPress = false;

    normalImg = QImage(Utils::getQrcPath("record_icon_normal.png"));
    hoverImg = QImage(Utils::getQrcPath("record_icon_hover.png"));
    pressImg = QImage(Utils::getQrcPath("record_icon_press.png"));
}

void RecordButton::setText(QString string)
{
    text = string;
    textSize = Utils::getRenderSize(11, string);
    
    setFixedSize(std::max(textSize.width() + PADDING * 2, WIDTH), std::max(textSize.height() + PADDING * 2, HEIGHT));
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
    if (status == "NORMAL") {
        painter.setOpacity(0.6);
    } else if (status == "PRESS") {
        painter.setOpacity(0.2);
    } else if (status == "HOVER") {
        painter.setOpacity(0.7);
    }

    QPainterPath path;
    path.addRoundedRect(QRectF(rect()), RECTANGLE_RAIUDS, RECTANGLE_RAIUDS);
    painter.fillPath(path, QColor("#F5F5F5"));

    QPen pen(QColor("#000000"));
    painter.setOpacity(0.04);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);

    // Draw icon.
    painter.setOpacity(1);
    int iconX = rect().x() + (rect().width() - normalImg.width()) / 2;
    int iconY = rect().y() + (rect().height() - normalImg.height() - textSize.height() - TEXT_PADDING) / 2;
    if (status == "NORMAL") {
        painter.drawImage(QPoint(iconX, iconY), normalImg);
    } else if (status == "PRESS") {
        painter.drawImage(QPoint(iconX, iconY), pressImg);
    } else if (status == "HOVER") {
        painter.drawImage(QPoint(iconX, iconY), hoverImg);
    }

    // Draw text.
    int textX = rect().x();
    int textY = iconY + normalImg.height() + TEXT_PADDING;
    Utils::setFontSize(painter, 11);
    painter.setOpacity(1);
    painter.setPen(QPen(QColor("#e34342")));
    painter.drawText(QRect(textX, textY, rect().width(), textSize.height()), Qt::AlignCenter, text);
}

bool RecordButton::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        isPress = true;
        repaint();
    } else if (event->type() == QEvent::MouseButtonRelease) {
        isPress = false;
        repaint();
    } else if (event->type() == QEvent::FocusIn) {
        isFocus = true;
        repaint();
    } else if (event->type() == QEvent::FocusOut) {
        isFocus = false;
        repaint();
    }

    return false;
}
