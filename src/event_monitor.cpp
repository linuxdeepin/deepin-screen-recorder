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

#include "event_monitor.h"
#include "keydefine.h"

#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#include <iostream>
#include <dlfcn.h>

EventMonitor::EventMonitor(QObject *parent) : QThread(parent)
{
    isPress = false;
}

void EventMonitor::run()
{
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        fprintf(stderr, "unable to open display\n");
        return;
    }

    // Receive from ALL clients, including future clients.
    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange *range = XRecordAllocRange();
    if (range == nullptr) {
        fprintf(stderr, "unable to allocate XRecordRange\n");
        return;
    }

    // Receive KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events.
    memset(range, 0, sizeof(XRecordRange));
    range->device_events.first = KeyPress;
    range->device_events.last  = MotionNotify;

    // And create the XRECORD context.
    XRecordContext context = XRecordCreateContext(display, 0, &clients, 1, &range, 1);
    if (context == 0) {
        fprintf(stderr, "XRecordCreateContext failed\n");
        return;
    }
    XFree(range);

    XSync(display, True);

    Display *display_datalink = XOpenDisplay(nullptr);
    if (display_datalink == nullptr) {
        fprintf(stderr, "unable to open second display\n");
        return;
    }

    if (!XRecordEnableContext(display_datalink, context,  callback, reinterpret_cast<XPointer>(this))) {
        fprintf(stderr, "XRecordEnableContext() failed\n");
        return;
    }
}

void EventMonitor::callback(XPointer ptr, XRecordInterceptData *data)
{
    (reinterpret_cast<EventMonitor *>(ptr))->handleEvent(data);
}

void EventMonitor::handleEvent(XRecordInterceptData *data)
{
    if (data->category == XRecordFromServer) {
        emit activateWindow();

        xEvent *event = reinterpret_cast<xEvent *>(data->data);
        switch (event->u.u.type) {
        case ButtonPress:
            if (event->u.u.detail != WheelUp &&
                    event->u.u.detail != WheelDown &&
                    event->u.u.detail != WheelLeft &&
                    event->u.u.detail != WheelRight) {
                isPress = true;
                //鼠标按压
                emit mousePress(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            } else if (event->u.u.detail == WheelUp || event->u.u.detail == WheelDown) {
                //鼠标滚动
                emit mouseScroll(static_cast<int>(event->u.enterLeave.time), event->u.u.detail, event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
            break;
        case MotionNotify:
            if (isPress) {
                emit mouseDrag(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            } else {
                //鼠标移动
                emit mouseMove(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
            break;
        case ButtonRelease:
            if (event->u.u.detail != WheelUp &&
                    event->u.u.detail != WheelDown &&
                    event->u.u.detail != WheelLeft &&
                    event->u.u.detail != WheelRight) {
                isPress = false;
                emit mouseRelease(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
            break;
        case KeyPress:
            // If key is equal to esc, emit pressEsc signal.
//            if ((reinterpret_cast<unsigned char *>(data->data))[1] == KEY_ESCAPE) {
//                emit pressEsc();
//            } else {
//                emit pressKeyButton((reinterpret_cast<unsigned char *>(data->data))[1]);
//            }

            //键盘按键按下
            emit keyboardPress((reinterpret_cast<unsigned char *>(data->data))[1]);
            break;
        case KeyRelease:
            //键盘按键释放
            emit keyboardRelease((reinterpret_cast<unsigned char *>(data->data))[1]);
            break;
        default:
            break;
        }
    }

    fflush(stdout);
    XRecordFreeData(data);
}

XFixesCursorImage *EventMonitor::getCursorImage()
{
    Display *x11Display = XOpenDisplay(nullptr);
    if (!x11Display) {
        fprintf(stderr, "unable to open display\n");
        return nullptr;
    }
    return XFixesGetCursorImage(x11Display);
}
