#include <QObject>
#include <QDebug>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include "screen_windows_info.h"

ScreenWindowsInfo::ScreenWindowsInfo(QObject *parent) : QObject(parent) 
{
    int screenNum;
    conn = xcb_connect(0, &screenNum);
    screen = xcb_aux_get_screen(conn, screenNum);
    rootWindow = screen->root;
}

xcb_atom_t ScreenWindowsInfo::getAtom(QString name)
{
    QByteArray rawName = name.toLatin1();
    xcb_atom_t result = XCB_ATOM_NONE;
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, 0, rawName.size(), rawName.data());
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, NULL);
    if(reply) {
        result = reply->atom;
        free(reply);
    }
    
    return result;
}

xcb_get_property_reply_t* ScreenWindowsInfo::getProperty(xcb_window_t window, QString propertyName, xcb_atom_t type)
{
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, 0, window, getAtom(propertyName), type, 0, UINT32_MAX);
    return xcb_get_property_reply(conn, cookie, NULL);
}

QString ScreenWindowsInfo::getAtomName(xcb_atom_t atom)
{
    QString result;

    xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(conn, atom);
    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(conn, cookie, NULL);

    if (reply)
    {
        result = QString::fromLatin1(xcb_get_atom_name_name(reply), xcb_get_atom_name_name_length(reply));
        free(reply);
    }    
    
    return result;
}

QStringList ScreenWindowsInfo::getWindowTypes(xcb_window_t window) 
{
    QStringList types;
    xcb_get_property_reply_t *reply = getProperty(window, "_NET_WM_WINDOW_TYPE", XCB_ATOM_ATOM);

    if(reply) {
        xcb_atom_t *typesA = static_cast<xcb_atom_t*>(xcb_get_property_value(reply));
        int typeNum = reply->length;
        
        for(int i = 0; i < typeNum; i++) {
            types.append(getAtomName(typesA[i]));
        }
        
        free(reply);
    }
    
    return types;
}

QList<int> ScreenWindowsInfo::getWindowFrameExtents(xcb_window_t window)
{
    QList<int> extents;
    
    if (window != rootWindow) {
        xcb_get_property_reply_t *reply = getProperty(window, "_GTK_FRAME_EXTENTS", XCB_ATOM_CARDINAL);
        
        if (reply) {
            // Because XCB haven't function to check property is exist,
            // we will got wrong value if property '_GTK_FRAME_EXTENTS' is not exist in window xprop attributes.
            // So we check reply->format, if it equal 16 or 32, '_GTK_FRAME_EXTENTS' property is exist.
            if (reply->format == 32 || reply->format == 16) {
                int32_t *value = (int32_t *)xcb_get_property_value(reply);
                for (int i = 0; i < 4; ++i) {
                    extents.append(value[i]);
                }
            } else {
                for (int j = 0; j < 4; ++j) {
                    extents.append(0);
                }
            }
        }
    
        free(reply);
    }

    return extents;
}

QString ScreenWindowsInfo::getWindowName(xcb_window_t window)
{
    if (window == rootWindow) {
        return "Desktop";
    } else {
        xcb_get_property_reply_t *reply = getProperty(window, "_NET_WM_NAME", getAtom("UTF8_STRING"));

        if(reply) {
            QString result = QString::fromUtf8(static_cast<char*>(xcb_get_property_value(reply)), xcb_get_property_value_length(reply));

            free(reply);

            return result;
        } else {
            return QString();
        }
    }
}

int ScreenWindowsInfo::getCurrentWorkspace(xcb_window_t window)
{
    
    xcb_get_property_reply_t *reply = getProperty(window, "_NET_CURRENT_DESKTOP", XCB_ATOM_CARDINAL);
    int desktop = 0;

    if (reply) {
        desktop = *((int *) xcb_get_property_value(reply));
    }

    free(reply);
    
    return desktop;
}

int ScreenWindowsInfo::getWindowWorkspace(xcb_window_t window)
{
    if (window == rootWindow) {
        return getCurrentWorkspace(rootWindow);
    } else {
        xcb_get_property_reply_t *reply = getProperty(window, "_NET_WM_DESKTOP", XCB_ATOM_CARDINAL);
        int desktop = 0;

        if (reply) {
            desktop = *((int *) xcb_get_property_value(reply));
        }

        free(reply);
    
        return desktop;
    }
}

QList<xcb_window_t> ScreenWindowsInfo::getWindows()
{
    QList<xcb_window_t> windows;
    xcb_get_property_reply_t *listReply = getProperty(rootWindow, "_NET_CLIENT_LIST_STACKING", XCB_ATOM_WINDOW);
    
    if (listReply) {
        xcb_window_t *windowList = static_cast<xcb_window_t*>(xcb_get_property_value(listReply));
        int windowListLength = listReply->length;
        
        for (int i = 0; i < windowListLength; i++) {
            xcb_window_t window = windowList[i];
            
            foreach(QString type, getWindowTypes(window)) {
                if (type == "_NET_WM_WINDOW_TYPE_NORMAL") {
                    if (getWindowWorkspace(window) == getCurrentWorkspace(rootWindow)) {
                        windows.append(window);
                    }
                    break;
                }
            }
        }
    }
    
    // We need add root window in first index of list.
    windows.prepend(rootWindow);
    
    // We need re-sort windows list from up to bottom, 
    // to make compare cursor with window area from up to bottom.
    std::reverse(windows.begin(), windows.end());
    
    return windows;
}

xcb_get_geometry_reply_t* ScreenWindowsInfo::getWindowGeometry(xcb_window_t window) 
{
    return xcb_get_geometry_reply(conn, xcb_get_geometry(conn, window), 0);
}        

WindowRect ScreenWindowsInfo::getWindowRect(xcb_window_t window) 
{
    WindowRect rect;
    
    xcb_get_geometry_reply_t *geometry = getWindowGeometry(window);
    QList<int> extents = getWindowFrameExtents(window);
    
    rect.x = geometry->x;
    rect.y = geometry->y;
    rect.width = geometry->width;
    rect.height = geometry->height;
    
    
    if (extents.length() == 4) {
        // _GTK_FRAME_EXTENTS: left, right, top, bottom
        rect.x += extents[0];  
        rect.y += extents[2];   
        rect.width -= extents[0] + extents[1];
        rect.height -= extents[2] + extents[3];
    }
    
    return rect;
}
