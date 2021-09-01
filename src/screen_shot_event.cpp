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

#include "screen_shot_event.h"
#include "keydefine.h"

#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#include <iostream>

ScreenShotEvent::ScreenShotEvent(QObject *parent) : QThread(parent)
{

}

void ScreenShotEvent::run()
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

void ScreenShotEvent::callback(XPointer ptr, XRecordInterceptData *data)
{
    (reinterpret_cast<ScreenShotEvent *>(ptr))->handleRecordEvent(data);
}

void ScreenShotEvent::handleRecordEvent(XRecordInterceptData *data)
{
    if (data->category == XRecordFromServer) {
        emit activateWindow();
        xEvent *event = reinterpret_cast<xEvent *>(data->data);
        switch (event->u.u.type) {
        case KeyPress: {
            emit shotKeyPressEvent((reinterpret_cast<unsigned char *>(data->data))[1]);
        }
        break;
        case ButtonPress: {
            if (event->u.u.detail == Button1) {
                emit mouseClick(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            } else if (event->u.u.detail == Button4 || event->u.u.detail == Button5) {
                emit mouseScroll(event->u.u.detail, event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
        }
        break;
        case MotionNotify:
            emit mouseMove(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            break;

        default:
            break;
        }
    }
    fflush(stdout);
    XRecordFreeData(data);
}

XFixesCursorImage *ScreenShotEvent::getCursorImage()
{
    Display *x11Display = XOpenDisplay(nullptr);
    if (!x11Display) {
        fprintf(stderr, "unable to open display\n");
        return nullptr;
    }
    return XFixesGetCursorImage(x11Display);
}
