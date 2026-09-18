// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// KF6 compatibility stub for removed KWayland/Client/remote_access.h
// This header provides minimal type declarations to allow compilation
// when RemoteAccess API is not available in KF6 KWayland.

#ifndef KWAYLANDCLIENT_REMOTE_ACCESS_STUB_H
#define KWAYLANDCLIENT_REMOTE_ACCESS_STUB_H

#include <QObject>

namespace KWayland
{
namespace Client
{

class RemoteBuffer : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void parametersObtained();
};

class RemoteAccessManager : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void bufferReady(const void *output, KWayland::Client::RemoteBuffer *rbuf);
};

}
}

#endif // KWAYLANDCLIENT_REMOTE_ACCESS_STUB_H
