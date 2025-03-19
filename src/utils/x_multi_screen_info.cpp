#include "x_multi_screen_info.h"


#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>

#include <stdio.h>

XMultiScreenInfo::XMultiScreenInfo() {}

/*
 * FIXME(205567): If multi-screen, the coordinates of the screen
 *      need to be flush on the horizontal or vertical axis.
 */
bool XMultiScreenInfo::screenNeedResetScale()
{
    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        return false;
    }

    bool needResetScale = false;
    do {
        int event_base, error_base;
        if (!XineramaQueryExtension(display, &event_base, &error_base)) {
            break;
        }

        if (!XineramaIsActive(display)) {
            break;
        }

        int num_screens;
        XineramaScreenInfo *screens = XineramaQueryScreens(display, &num_screens);
        if (!screens || num_screens <= 1) {
            break;
        }

        int xOffset = 0;
        int yOffset = 0;
        for (int i = 0; i < num_screens; i++) {
            printf("screen %d: x = %d, y = %d, width = %d, height = %d\n",
                   i, screens[i].x_org, screens[i].y_org, screens[i].width, screens[i].height);

            xOffset += screens[i].x_org;
            yOffset += screens[i].y_org;
        }
        needResetScale = (0 != xOffset) && (0 != yOffset);

        XFree(screens);
    } while (false);

    XCloseDisplay(display);
    return needResetScale;
}
