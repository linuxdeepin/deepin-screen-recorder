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

#include <QPainter>
#include "button_feedback.h"
#include "utils.h"
#include <QTimer>

#include <QDebug>

const int ButtonFeedback::FRAME_RATE = 40; // ms

ButtonFeedback::ButtonFeedback(QWidget *parent) : QWidget(parent)
{
    installEventFilter(this);  // add event filter
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    
    buttonFeedback0Img = QImage(Utils::getQrcPath("button_feedback_0.png"));
    buttonFeedback1Img = QImage(Utils::getQrcPath("button_feedback_1.png"));
    buttonFeedback2Img = QImage(Utils::getQrcPath("button_feedback_2.png"));
    buttonFeedback3Img = QImage(Utils::getQrcPath("button_feedback_3.png"));
    buttonFeedback4Img = QImage(Utils::getQrcPath("button_feedback_4.png"));
    buttonFeedback5Img = QImage(Utils::getQrcPath("button_feedback_5.png"));
    buttonFeedback6Img = QImage(Utils::getQrcPath("button_feedback_6.png"));
    buttonFeedback7Img = QImage(Utils::getQrcPath("button_feedback_7.png"));
    buttonFeedback8Img = QImage(Utils::getQrcPath("button_feedback_8.png"));
    buttonFeedback9Img = QImage(Utils::getQrcPath("button_feedback_9.png"));
    
    setFixedSize(buttonFeedback0Img.width(), buttonFeedback0Img.height());
    
    frameIndex = 0;
    
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
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
    
    switch(frameIndex) {
    case 0:
        painter.drawImage(QPoint(0, 0), buttonFeedback0Img);
        break;
    case 1:
        painter.drawImage(QPoint(0, 0), buttonFeedback1Img);
        break;
    case 2:
        painter.drawImage(QPoint(0, 0), buttonFeedback2Img);
        break;
    case 3:
        painter.drawImage(QPoint(0, 0), buttonFeedback3Img);
        break;
    case 4:
        painter.drawImage(QPoint(0, 0), buttonFeedback4Img);
        break;
    case 5:
        painter.drawImage(QPoint(0, 0), buttonFeedback5Img);
        break;
    case 6:
        painter.drawImage(QPoint(0, 0), buttonFeedback6Img);
        break;
    case 7:
        painter.drawImage(QPoint(0, 0), buttonFeedback7Img);
        break;
    case 8:
        painter.drawImage(QPoint(0, 0), buttonFeedback8Img);
        break;
    case 9:
        painter.drawImage(QPoint(0, 0), buttonFeedback9Img);
        break;
    }
}

void ButtonFeedback::showPressFeedback(int x, int y)
{
    frameIndex = 0;
    
    show();
    repaint();
    move(x - rect().width() / 2, y - rect().height() / 2);
    timer->start(FRAME_RATE);
}

void ButtonFeedback::showDragFeedback(int x, int y)
{
    frameIndex = 2;
    
    show();
    repaint();
    move(x - rect().width() / 2, y - rect().height() / 2);
    
    if (timer->isActive()) {
        timer->stop();
    }
}

void ButtonFeedback::showReleaseFeedback(int x, int y)
{
    frameIndex = 3;
    
    show();
    repaint();
    move(x - rect().width() / 2, y - rect().height() / 2);
    timer->start(FRAME_RATE);
}
