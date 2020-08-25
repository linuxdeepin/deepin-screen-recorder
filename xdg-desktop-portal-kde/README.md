# xdg-desktop-portal-kde

A backend implementation for [xdg-desktop-portal](http://github.com/flatpak/xdg-desktop-portal)
that is using Qt/KF5.

## Building xdg-desktop-portal-kde

### Dependencies:
 - xdg-desktop-portal (runtime dependency)
 - Qt 5 (build dependency)
 - KDE Frameworks - KCoreAddons, KI18n, KNotifications(build dependency)

### Build instructions:
```
$ mkdir build && cd build
$ cmake .. [your_options]
$ make -j5
$ make install
```
