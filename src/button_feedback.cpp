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
#include "button_feedback.h"
#include "utils.h"
#include <QTimer>
#include <QApplication>
#include <DHiDPIHelper>

#include <QDebug>

const int ButtonFeedback::FRAME_RATE = 40; // ms

DWIDGET_USE_NAMESPACE

ButtonFeedback::ButtonFeedback(DWidget *parent) : DWidget(parent)
{
    installEventFilter(this);  // add event filter
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    buttonFeedback0Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_0.svg"));
    buttonFeedback1Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_1.svg"));
    buttonFeedback2Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_2.svg"));
    buttonFeedback3Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_3.svg"));
    buttonFeedback4Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_4.svg"));
    buttonFeedback5Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_5.svg"));
    buttonFeedback6Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_6.svg"));
    buttonFeedback7Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_7.svg"));
    buttonFeedback8Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_8.svg"));
    buttonFeedback9Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_9.svg"));

    setFixedSize(buttonFeedback0Img.width(), buttonFeedback0Img.height());

    frameIndex = 0;

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));

    Utils::passInputEvent(this->winId());
}

void ButtonFeedback::update()
{
    repaint();

    frameIndex += 1;

    if (frameIndex > 9) {
        frameIndex = 0;

        if (timer->isActive()) {
            timer->stop();
        }

        hide();
    }
}

void ButtonFeedback::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setOpacity(1);

    switch (frameIndex) {
    case 0:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback0Img);
        break;
    case 1:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback1Img);
        break;
    case 2:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback2Img);
        break;
    case 3:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback3Img);
        break;
    case 4:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback4Img);
        break;
    case 5:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback5Img);
        break;
    case 6:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback6Img);
        break;
    case 7:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback7Img);
        break;
    case 8:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback8Img);
        break;
    case 9:
        painter.drawPixmap(QPoint(0, 0), buttonFeedback9Img);
        break;
    }
}

void ButtonFeedback::showPressFeedback(int x, int y)
{
    frameIndex = 0;

    show();
    repaint();
    qreal devicePixelRatio = qApp->devicePixelRatio();
    move(static_cast<int>(x / devicePixelRatio - rect().width() / devicePixelRatio / 2), static_cast<int>(y / devicePixelRatio - rect().height() / devicePixelRatio / 2));
    timer->start(FRAME_RATE);
}

void ButtonFeedback::showDragFeedback(int x, int y)
{
    frameIndex = 2;

    show();
    repaint();
    qreal devicePixelRatio = qApp->devicePixelRatio();
    move(static_cast<int>(x / devicePixelRatio - rect().width() / devicePixelRatio / 2), static_cast<int>(y / devicePixelRatio - rect().height() / devicePixelRatio / 2));

    if (timer->isActive()) {
        timer->stop();
    }
}

void ButtonFeedback::showReleaseFeedback(int x, int y)
{
    frameIndex = 3;

    show();
    repaint();
    qreal devicePixelRatio = qApp->devicePixelRatio();
    move(static_cast<int>(x / devicePixelRatio - rect().width() / devicePixelRatio / 2), static_cast<int>(y / devicePixelRatio - rect().height() / devicePixelRatio / 2));
    timer->start(FRAME_RATE);
}
