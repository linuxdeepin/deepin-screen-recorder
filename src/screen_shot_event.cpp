#include "screen_shot_event.h"
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include "keydefine.h"
#include <iostream>

ScreenShotEvent::ScreenShotEvent(QObject *parent) : QThread(parent)
{

}

void ScreenShotEvent::run()
{
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr)
    {
        fprintf(stderr, "unable to open display\n");
        return;
    }
    // Receive from ALL clients, including future clients.
    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange *range = XRecordAllocRange();
    if (range == nullptr)
    {
        fprintf(stderr, "unable to allocate XRecordRange\n");
        return;
    }
    // Receive KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events.
    memset(range, 0, sizeof(XRecordRange));
    range->device_events.first = KeyPress;
    range->device_events.last  = MotionNotify;
    // And create the XRECORD context.
    XRecordContext context = XRecordCreateContext (display, 0, &clients, 1, &range, 1);
    if (context == 0)
    {
        fprintf(stderr, "XRecordCreateContext failed\n");
        return;
    }
    XFree(range);
    XSync(display, True);
    Display *display_datalink = XOpenDisplay(nullptr);
    if (display_datalink == nullptr)
    {
        fprintf(stderr, "unable to open second display\n");
        return;
    }
    if (!XRecordEnableContext(display_datalink, context,  callback, reinterpret_cast<XPointer>(this)))
    {
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
    if (data->category == XRecordFromServer)
    {
        emit activateWindow();
    }
    fflush(stdout);
    XRecordFreeData(data);
}


















