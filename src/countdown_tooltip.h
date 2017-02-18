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

#ifndef COUNTDOWNTOOLTIP_H
#define COUNTDOWNTOOLTIP_H

#include <QWidget>
#include <QTimer>

class CountdownTooltip : public QWidget 
{
    Q_OBJECT
    
    static const int NUMBER_PADDING_Y;
    static const int PADDING_X;
    static const int PADDING_Y;
    static const int RECTANGLE_RAIUDS;
    
public:
    CountdownTooltip(QWidget *parent = 0);
    void start();
                         
signals:
    void finished();

public slots:
    void update();
    
protected:
    void paintEvent(QPaintEvent *event);
    
private:
    QImage countdown1Img;
    QImage countdown2Img;
    QImage countdown3Img;
    
    int showCountdownCounter;
    
    QTimer* showCountdownTimer;
    
    QString text;
};

#endif // COUNTDOWNTOOLTIP_H
