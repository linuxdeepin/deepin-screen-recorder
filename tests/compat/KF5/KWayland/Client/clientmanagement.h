// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// KF6 compatibility stub for KWayland/Client/clientmanagement.h.
// The ClientManagement API was removed from KF6 KWayland. This stub provides
// minimal type declarations (WindowState struct + ClientManagement class with
// the signals referenced by the recorder code) so that the recorder source and
// its unit tests compile against KF6 without modifying src/.

#ifndef KF5_COMPAT_CLIENTMANAGEMENT_H
#define KF5_COMPAT_CLIENTMANAGEMENT_H

#include <QObject>
#include <QVector>

namespace KWayland
{
namespace Client
{

struct WindowGeometry
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

struct WindowState
{
    int pid = 0;
    int windowId = 0;
    char resourceName[256] = {0};
    WindowGeometry geometry;
    bool isMinimized = false;
    bool isFullScreen = false;
    bool isActive = false;
};

class ClientManagement : public QObject
{
    Q_OBJECT
public:
    explicit ClientManagement(QObject *parent = nullptr) : QObject(parent) {}
    QVector<WindowState> getWindowStates() const { return QVector<WindowState>(); }

Q_SIGNALS:
    void windowStatesChanged();
};

}
}

#endif // KF5_COMPAT_CLIENTMANAGEMENT_H
