#include "desktopinfo.h"

#include <QProcessEnvironment>

DesktopInfo::DesktopInfo()
{
    auto e = QProcessEnvironment::systemEnvironment();
    XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

}

bool DesktopInfo::waylandDectected()
{
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||
           WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
}
