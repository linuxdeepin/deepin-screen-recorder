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

#ifndef EVENTMONITOR_H
#define EVENTMONITOR_H

#include <QThread>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>


class EventMonitor : public QThread
{
    Q_OBJECT

public:
    EventMonitor(QObject *parent = 0);
    static void callback(XPointer trash, XRecordInterceptData* data);
    void handleRecordEvent(XRecordInterceptData *);
    
signals:
    void buttonedPress(int x, int y);
    void buttonedDrag(int x, int y);
    void buttonedRelease(int x, int y);
    void pressEsc();
    //键盘按钮事件采集信号
    void pressKeyButton(unsigned char keyCode);
    void releaseKeyButton(unsigned char keyCode);

protected:
    void run();
    
private:
    bool isPress;
};

#endif
