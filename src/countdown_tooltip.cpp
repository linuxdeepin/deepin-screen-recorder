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

#include <DWidget>
#include <QTimer>
#include <QApplication>
#include <QPen>
#include <QPainter>
#include "countdown_tooltip.h"
#include "utils.h"
#include "constant.h"
#include <DHiDPIHelper>

const int CountdownTooltip::NUMBER_PADDING_Y = 30;

DWIDGET_USE_NAMESPACE

CountdownTooltip::CountdownTooltip(DWidget *parent) : DWidget(parent)
{
    installEventFilter(this);

    countdown1Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("countdown_1.svg"));
    countdown2Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("countdown_2.svg"));
    countdown3Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("countdown_3.svg"));

    showCountdownCounter = 0;
}

void CountdownTooltip::paintEvent(QPaintEvent *)
{
    if (showCountdownCounter > 0) {
        QPainter painter(this);

        Utils::drawTooltipBackground(painter, rect());

        qreal devicePixelRatio = qApp->devicePixelRatio();
        painter.setOpacity(1);
        int countdownX = static_cast<int>(rect().x() + (rect().width() - countdown1Img.width() / devicePixelRatio) / 2);
        int countdownY = static_cast<int>(rect().y() + NUMBER_PADDING_Y * devicePixelRatio);
        if (showCountdownCounter == 1) {
            painter.drawPixmap(QPoint(countdownX, countdownY), countdown1Img);
        } else if (showCountdownCounter == 2) {
            painter.drawPixmap(QPoint(countdownX, countdownY), countdown2Img);
        } else if (showCountdownCounter == 3) {
            painter.drawPixmap(QPoint(countdownX, countdownY), countdown3Img);
        }

        QRectF tooltipRect(rect().x(),
                           rect().y() + countdown1Img.height() / devicePixelRatio + NUMBER_PADDING_Y,
                           rect().width(),
                           rect().height() - countdown1Img.height() / devicePixelRatio - NUMBER_PADDING_Y);
        Utils::drawTooltipText(painter, text, "#000000", Constant::RECTANGLE_FONT_SIZE, tooltipRect);
    }
}

void CountdownTooltip::start()
{
    text = tr("Click the tray icon \nor press the shortcut again to stop recording");
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
