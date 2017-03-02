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

#ifndef BUTTONFEEDBACK_H
#define BUTTONFEEDBACK_H

#include <QWidget>
#include <QTimer>

class ButtonFeedback : public QWidget
{
    Q_OBJECT
    
    static const int FRAME_RATE;
    
public:
    ButtonFeedback(QWidget *parent = 0);
                                       
protected:
    void paintEvent(QPaintEvent *event);
    
public slots:
    void showPressFeedback(int x, int y);
    void showDragFeedback(int x, int y);
    void showReleaseFeedback(int x, int y);
    void update();
    
private:
    QImage buttonFeedback0Img;
    QImage buttonFeedback1Img;
    QImage buttonFeedback2Img;
    QImage buttonFeedback3Img;
    QImage buttonFeedback4Img;
    QImage buttonFeedback5Img;
    QImage buttonFeedback6Img;
    QImage buttonFeedback7Img;
    QImage buttonFeedback8Img;
    QImage buttonFeedback9Img;
    
    QTimer* timer;
    
    int frameIndex;
};

#endif    
