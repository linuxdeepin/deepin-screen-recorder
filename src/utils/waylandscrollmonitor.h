// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WAYLANDSCROLLMONITOR_H
#define WAYLANDSCROLLMONITOR_H

#if defined(KF5_WAYLAND_FLAGE_ON) && !defined(DWAYLAND_SUPPORT)

#include <QObject>
#include <QApplication>
#include <QTimer>
#include <QDebug>

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/output.h>
#include <KWayland/Client/remote_access.h>
#include <KWayland/Client/fakeinput.h>

class WaylandScrollMonitor : public QObject
{
    Q_OBJECT
public:
    WaylandScrollMonitor(QObject *parent = nullptr);
    ~WaylandScrollMonitor();
protected:
    void initWaylandScrollThread();
    void releaseWaylandScrollThread();

public slots:
    void doWaylandAutoScroll(); //自动滚动
    void slotManualScroll(float direction); // 手动滚动
protected slots:
    void setupRegistry(); //注册
    void setupFakeinput(quint32 name, quint32 version); // 初始化fakeinput
private:
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    KWayland::Client::Registry *m_registry;
    KWayland::Client::FakeInput *m_fakeinput;
};

#endif // defined(KF5_WAYLAND_FLAGE_ON) && !defined(DWAYLAND_SUPPORT)

#endif // WAYLANDSCROLLMONITOR_H
