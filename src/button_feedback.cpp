/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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


#include "button_feedback.h"
#include "utils.h"

#include <DHiDPIHelper>

#include <QPainter>
#include <QTimer>
#include <QApplication>
#include <QBitmap>
#include <QPaintEvent>
#include <QDebug>

const int ButtonFeedback::FRAME_RATE = 40; // ms

DWIDGET_USE_NAMESPACE

ButtonFeedback::ButtonFeedback(DWidget *parent) : DWidget(parent)
{
    installEventFilter(this);  // add event filter
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    for(int i = 0; i < 10; ++i){
        QString svgName = QString("button_feedback_%1.svg").arg(i);
        buttonFeedbackImg[i] = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath(svgName));
    }
    /*
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
    */
    setFixedSize(buttonFeedbackImg[0].width(), buttonFeedbackImg[0].height());

    frameIndex = 0;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));

    Utils::passInputEvent(static_cast<int>(this->winId()));
    m_painter =  new QPainter();
}

ButtonFeedback::~ButtonFeedback()
{
    delete m_painter;
    if(nullptr != timer){
        delete  timer;
        timer = nullptr;
    }
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

void ButtonFeedback::paintEvent(QPaintEvent *event)
{
    /*
    QPainter painter(this);
    QPixmap pixmap(width(), height());
    pixmap.fill(Qt::transparent);
    painter.begin( &pixmap );
    painter.setRenderHints( QPainter::Antialiasing, true);



    painter.setOpacity(1);
    painter.drawPixmap(QPoint(0, 0), buttonFeedbackImg[frameIndex]);
    painter.end();

    painter.begin(this);
    painter.drawPixmap(QPoint(0, 0), pixmap);
    painter.end();
    //setMask(pixmap.mask());
    //event->accept();
    */

    QPixmap pixmap(width() + 6, height() + 6);
    pixmap.fill(Qt::transparent);
    m_painter->begin( &pixmap );
    m_painter->setRenderHints( QPainter::Antialiasing, true);

    m_painter->setOpacity(1);
    m_painter->drawPixmap(QPoint(0, 0), buttonFeedbackImg[frameIndex]);
    m_painter->end();

    m_painter->begin(this);
    m_painter->drawPixmap(QPoint(0, 0), pixmap);
    m_painter->end();
    setMask(pixmap.mask());
    event->accept();
}

void ButtonFeedback::showPressFeedback(int x, int y)
{
    frameIndex = 1;

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
