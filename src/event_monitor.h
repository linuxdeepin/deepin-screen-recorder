/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
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

#ifndef EVENTMONITOR_H
#define EVENTMONITOR_H

#include <QThread>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/extensions/Xfixes.h>

class EventMonitor : public QThread
{
    Q_OBJECT

public:
    explicit EventMonitor(QObject *parent = nullptr);
    static void callback(XPointer trash, XRecordInterceptData *data);
    void handleRecordEvent(XRecordInterceptData *);
    //XFixesCursorImage* GetCursorImage();

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
