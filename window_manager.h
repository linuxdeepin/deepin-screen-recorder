#ifndef LINE_H
#define LINE_H

#include <QObject>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

struct WindowRect {
    int x;
    int y;
    int width;
    int height;
};

class WindowManager : public QObject
{
    Q_OBJECT

public:
    WindowManager(QObject *parent = 0);

    QList<int> getWindowFrameExtents(xcb_window_t window);
    QList<xcb_window_t> getWindows();
    QString getAtomName(xcb_atom_t atom);
    QString getWindowName(xcb_window_t window);
    QStringList getWindowTypes(xcb_window_t window);
    WindowRect getRootWindowRect();
    WindowRect getWindowRect(xcb_window_t window);
    int getCurrentWorkspace(xcb_window_t window);
    int getWindowWorkspace(xcb_window_t window);
    xcb_atom_t getAtom(QString name);
    xcb_get_geometry_reply_t* getWindowGeometry(xcb_window_t window);
    xcb_get_property_reply_t* getProperty(xcb_window_t window, QString propertyName, xcb_atom_t type);

    xcb_window_t rootWindow;

protected:
    xcb_connection_t* conn;
    xcb_screen_t* screen;
};

#endif
