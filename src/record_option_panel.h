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

#ifndef RECORDOPTIONALPANEL_H
#define RECORDOPTIONALPANEL_H

#include <QPushButton>
#include "settings.h"

class RecordOptionPanel : public QPushButton
{
    Q_OBJECT
    
    static const int WIDTH;
    static const int HEIGHT;
    static const int ICON_OFFSET_X;
    
public:
    RecordOptionPanel(QPushButton *parent = 0);
    bool isSaveAsGif();
    
protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *, QEvent *event);
    
private:
    bool saveAsGif;
    bool isPressGif;
    bool isPressMp4;
    
    QImage gifNormalImg;
    QImage gifPressImg;
    QImage gifCheckedImg;
    
    QImage mp4NormalImg;
    QImage mp4PressImg;
    QImage mp4CheckedImg;
    
    Settings* settings;
};

#endif // RECORDOPTIONPANEL_H
