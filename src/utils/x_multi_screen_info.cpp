#include "x_multi_screen_info.h"
#include "utils/log.h"


#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>

#include <stdio.h>

XMultiScreenInfo::XMultiScreenInfo()
{
    qCDebug(dsrApp) << "XMultiScreenInfo initialized.";
}

/*
 * FIXME(205567): If multi-screen, the coordinates of the screen
 *      need to be flush on the horizontal or vertical axis.
 */
bool XMultiScreenInfo::screenNeedResetScale()
{
    qCDebug(dsrApp) << "Checking if screen needs reset scale.";
    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        qCWarning(dsrApp) << "Failed to open X display.";
        return false;
    }

    bool needResetScale = false;
    do {
        int event_base, error_base;
        if (!XineramaQueryExtension(display, &event_base, &error_base)) {
            qCWarning(dsrApp) << "Xinerama extension not available.";
            break;
        }

        if (!XineramaIsActive(display)) {
            qCDebug(dsrApp) << "Xinerama is not active.";
            break;
        }

        int num_screens;
        XineramaScreenInfo *screens = XineramaQueryScreens(display, &num_screens);
        if (!screens || num_screens <= 1) {
            qCDebug(dsrApp) << "Less than or equal to one screen or screens info is null. Number of screens:" << num_screens;
            break;
        }

        qCDebug(dsrApp) << "Found" << num_screens << "screens.";
        int xOffset = 0;
        int yOffset = 0;
        for (int i = 0; i < num_screens; i++) {
            printf("screen %d: x = %d, y = %d, width = %d, height = %d\n",
                   i, screens[i].x_org, screens[i].y_org, screens[i].width, screens[i].height);
            qCDebug(dsrApp) << "Screen" << i << ": x=" << screens[i].x_org << ", y=" << screens[i].y_org
                              << ", width=" << screens[i].width << ", height=" << screens[i].height;

            xOffset += screens[i].x_org;
            yOffset += screens[i].y_org;
        }
        needResetScale = (0 != xOffset) && (0 != yOffset);
        qCDebug(dsrApp) << "Calculated needResetScale:" << needResetScale << ", xOffset:" << xOffset << ", yOffset:" << yOffset;

        XFree(screens);
    } while (false);

    XCloseDisplay(display);
    qCDebug(dsrApp) << "Closed X display. Returning needResetScale:" << needResetScale;
    return needResetScale;
}
