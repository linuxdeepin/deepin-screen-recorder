/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
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

#include <QWidget>
#include <QTimer>
#include <QPen>
#include <QPainter>
#include "countdown_tooltip.h"
#include "utils.h"
#include "constant.h"

const int CountdownTooltip::NUMBER_PADDING_Y = 30;

CountdownTooltip::CountdownTooltip(QWidget *parent) : QWidget(parent)
{
    installEventFilter(this);

    countdown1Img = QImage(Utils::getQrcPath("countdown_1.png"));
    countdown2Img = QImage(Utils::getQrcPath("countdown_2.png"));
    countdown3Img = QImage(Utils::getQrcPath("countdown_3.png"));

    showCountdownCounter = 0;
}

void CountdownTooltip::paintEvent(QPaintEvent *)
{
    if (showCountdownCounter > 0) {
        QPainter painter(this);
        
        QPainterPath path;
        painter.setOpacity(0.4);
        path.addRoundedRect(QRectF(rect()), Constant::RECTANGLE_RADIUS, Constant::RECTANGLE_RADIUS);
        painter.fillPath(path, QColor("#F5F5F5"));

        QPen pen(QColor("#000000"));
        painter.setOpacity(0.04);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawPath(path);

        painter.setOpacity(1);
        int countdownX = rect().x() + (rect().width() - countdown1Img.width()) / 2;
        int countdownY = rect().y() + NUMBER_PADDING_Y;
        if (showCountdownCounter == 1) {
            painter.drawImage(QPoint(countdownX, countdownY), countdown1Img);
        } else if (showCountdownCounter == 2) {
            painter.drawImage(QPoint(countdownX, countdownY), countdown2Img);
        } else if (showCountdownCounter == 3) {
            painter.drawImage(QPoint(countdownX, countdownY), countdown3Img);
        }

        QRectF tooltipRect(rect().x(),
                           rect().y() + countdown1Img.height() + NUMBER_PADDING_Y,
                           rect().width(),
                           rect().height() - countdown1Img.height() - NUMBER_PADDING_Y);
        Utils::setFontSize(painter, Constant::RECTANGLE_FONT_SIZE);
        painter.setPen(QPen(QColor("#000000")));
        painter.drawText(tooltipRect, Qt::AlignCenter, text);
    }
}

void CountdownTooltip::start()
{
    text = tr("Click tray icon \nor press the shortcut again to stop recording");
    QSize size = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, text);
    int width = size.width() + Constant::RECTANGLE_PADDING * 2;
    int height = size.height() + Constant::RECTANGLE_PADDING * 2 + countdown1Img.height() + NUMBER_PADDING_Y;
    setFixedSize(width, height);
    
    showCountdownCounter = 3;
    showCountdownTimer = new QTimer(this);
    connect(showCountdownTimer, SIGNAL(timeout()), this, SLOT(update()));
    showCountdownTimer->start(1000);
}

void CountdownTooltip::update()
{
    showCountdownCounter--;
    
    if (showCountdownCounter <= 0) {
        showCountdownTimer->stop();
        
        emit finished();
    }

    repaint();
}
