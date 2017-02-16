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
#include <QMouseEvent>
#include <QDebug>
#include "utils.h"
#include "record_option_panel.h"

const int RecordOptionPanel::RECTANGLE_RAIUDS = 8;
const int RecordOptionPanel::WIDTH = 124;
const int RecordOptionPanel::HEIGHT = 36;
const int RecordOptionPanel::ICON_OFFSET_X = 14;
    
RecordOptionPanel::RecordOptionPanel(QPushButton *parent) : QPushButton(parent)
{
    installEventFilter(this);
    setMouseTracking(true);

    gifNormalImg = QImage(Utils::getQrcPath("gif_normal.png"));
    gifPressImg = QImage(Utils::getQrcPath("gif_press.png"));
    gifCheckedImg = QImage(Utils::getQrcPath("gif_checked.png"));

    mp4NormalImg = QImage(Utils::getQrcPath("mp4_normal.png"));
    mp4PressImg = QImage(Utils::getQrcPath("mp4_press.png"));
    mp4CheckedImg = QImage(Utils::getQrcPath("mp4_checked.png"));

    settings = new Settings();
    saveAsGif = settings->getOption("save_as_gif").toBool();

    isPressGif = false;
    isPressMp4 = false;

    setFixedSize(WIDTH, HEIGHT);
}

void RecordOptionPanel::paintEvent(QPaintEvent *)
{
    // Init.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw background.
    painter.setOpacity(0.4);
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
    Utils::setFontSize(painter, 9);

    int gifIconX = rect().x() + ICON_OFFSET_X;
    int gifIconY = rect().y() + (rect().height() - gifCheckedImg.height()) / 2;
    int gifTextX = gifIconX + gifCheckedImg.width();
    int gifTextWidth = rect().width() / 2 - ICON_OFFSET_X - gifCheckedImg.width();
    QString gifColor;
    if (saveAsGif) {
        painter.drawImage(QPoint(gifIconX, gifIconY), gifCheckedImg);
        gifColor = "#217DFF";
    } else if (isPressGif) {
        painter.drawImage(QPoint(gifIconX, gifIconY), gifPressImg);
        gifColor = "#004BCA";
    } else {
        painter.drawImage(QPoint(gifIconX, gifIconY), gifNormalImg);
        gifColor = "#000000";
    }
    painter.setPen(QPen(QColor(gifColor)));
    painter.drawText(QRectF(gifTextX, rect().y(), gifTextWidth, rect().height()), Qt::AlignVCenter, "GIF");

    int mp4IconX = rect().x() + rect().width() / 2;
    int mp4IconY = rect().y() + (rect().height() - mp4CheckedImg.height()) / 2;
    int mp4TextX = mp4IconX + mp4CheckedImg.width();
    int mp4TextWidth = rect().width() / 2 - ICON_OFFSET_X - gifCheckedImg.width();
    QString mp4Color;
    if (!saveAsGif) {
        painter.drawImage(QPoint(mp4IconX, mp4IconY), mp4CheckedImg);
        mp4Color = "#217DFF";
    } else if (isPressMp4) {
        painter.drawImage(QPoint(mp4IconX, mp4IconY), mp4PressImg);
        mp4Color = "#004BCA";
    } else {
        painter.drawImage(QPoint(mp4IconX, mp4IconY), mp4NormalImg);
        mp4Color = "#000000";
    }
    painter.setPen(QPen(QColor(mp4Color)));
    painter.drawText(QRectF(mp4TextX, rect().y(), mp4TextWidth, rect().height()), Qt::AlignVCenter, "MP4");
}

bool RecordOptionPanel::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int pressX = mouseEvent->x();

        if (pressX > rect().x() && pressX < rect().x() + rect().width() / 2) {
            isPressGif = true;
            isPressMp4 = false;
        } else {
            isPressMp4 = true;
            isPressGif = false;
        }
        
        repaint();
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int releaseX = mouseEvent->x();
        
        if (releaseX > rect().x() && releaseX < rect().x() + rect().width() / 2) {
            if (isPressGif) {
                saveAsGif = true;
                settings->setOption("save_as_gif", saveAsGif);
            } else if (isPressMp4) {
                isPressMp4 = false;
            }
        } else {
            if (isPressMp4) {
                saveAsGif = false;
                settings->setOption("save_as_gif", saveAsGif);
            } else if (isPressGif) {
                isPressGif = false;
            }
        }
        
        repaint();
    }

    return false;
}

bool RecordOptionPanel::isSaveAsGif()
{
    return saveAsGif;
}    
