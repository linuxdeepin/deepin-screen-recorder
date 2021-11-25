/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:    Zhang Wenchao <zhangwenchao@uniontech.com>
 *
 * Maintainer: He Mingyang <hemingyang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WAYLANDSCROLLMONITOR_H
#define WAYLANDSCROLLMONITOR_H

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

#endif // WAYLANDSCROLLMONITOR_H
