#ifndef DESKTOPINFO_H
#define DESKTOPINFO_H

#include <QString>

class DesktopInfo
{
public:
    DesktopInfo();
    bool waylandDectected();

private:
    QString XDG_SESSION_TYPE;
    QString WAYLAND_DISPLAY;

};

#endif // DESKTOPINFO_H
