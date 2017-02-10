#include <QObject>
#include <QDebug>
#include <QtX11Extras/QX11Info>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include "window_manager.h"

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{
    int screenNum;
    conn = xcb_connect(0, &screenNum);
    screen = xcb_aux_get_screen(conn, screenNum);
    rootWindow = screen->root;
}

xcb_atom_t WindowManager::getAtom(QString name)
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

xcb_get_property_reply_t* WindowManager::getProperty(xcb_window_t window, QString propertyName, xcb_atom_t type)
{
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, 0, window, getAtom(propertyName), type, 0, UINT32_MAX);
    return xcb_get_property_reply(conn, cookie, NULL);
}

QString WindowManager::getAtomName(xcb_atom_t atom)
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

QStringList WindowManager::getWindowTypes(xcb_window_t window)
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

QList<int> WindowManager::getWindowFrameExtents(xcb_window_t window)
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

QString WindowManager::getWindowClass(xcb_window_t window)
{
    if (window == rootWindow) {
        return "desktop";
    } else {
        xcb_get_property_reply_t *reply = getProperty(window, "WM_CLASS", getAtom("STRING"));

        if(reply) {
            QList<QByteArray> rawClasses = QByteArray(static_cast<char*>(xcb_get_property_value(reply)), xcb_get_property_value_length(reply)).split('\0');

            free(reply);

            return QString::fromLatin1(rawClasses[0]);
        } else {
            return QString();
        }
    }
}

QString WindowManager::getWindowName(xcb_window_t window)
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

int WindowManager::getCurrentWorkspace(xcb_window_t window)
{

    xcb_get_property_reply_t *reply = getProperty(window, "_NET_CURRENT_DESKTOP", XCB_ATOM_CARDINAL);
    int desktop = 0;

    if (reply) {
        desktop = *((int *) xcb_get_property_value(reply));
    }

    free(reply);

    return desktop;
}

int WindowManager::getWindowWorkspace(xcb_window_t window)
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

QList<xcb_window_t> WindowManager::getWindows()
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

xcb_get_geometry_reply_t* WindowManager::getWindowGeometry(xcb_window_t window)
{
    return xcb_get_geometry_reply(conn, xcb_get_geometry(conn, window), 0);
}

WindowRect WindowManager::getRootWindowRect() {
    WindowRect rect;
    xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, rootWindow), 0);

    rect.x = 0;
    rect.y = 0;
    rect.width = geometry->width;
    rect.height = geometry->height;

    return rect;
}

WindowRect WindowManager::getWindowRect(xcb_window_t window)
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

template <typename... ArgTypes, typename... ArgTypes2>
static inline unsigned int XcbCallVoid(xcb_void_cookie_t (*func)(xcb_connection_t *, ArgTypes...), ArgTypes2... args...)
{
    return func(QX11Info::connection(), args...).sequence;
}

void WindowManager::setWindowBlur(int wid, QVector<uint32_t> &data)
{
    auto atom = getAtom("_NET_WM_DEEPIN_BLUR_REGION_ROUNDED");
    XcbCallVoid(
        xcb_change_property,
        XCB_PROP_MODE_REPLACE,
        wid,
        atom,
        XCB_ATOM_CARDINAL,
        32,
        data.size(),
        data.constData());
    xcb_flush(conn);
}

