// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylandmousesimulator.h"

#if defined(KF5_WAYLAND_FLAGE_ON) && defined(DWAYLAND_SUPPORT)

#define SCROLL_DOWN 15.0

WaylandMouseSimulator::WaylandMouseSimulator(QObject *parent) : QObject(parent)
    , m_connection(nullptr)
    , m_queue(nullptr)
    , m_registry(nullptr)
    , m_fakeinput(nullptr)
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "WaylandMouseSimulator initialized.";
    //initWaylandScrollThread();
}

WaylandMouseSimulator::~WaylandMouseSimulator()
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "WaylandMouseSimulator destructor called.";
    releaseWaylandScrollThread();
}

WaylandMouseSimulator *WaylandMouseSimulator::instance()
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Getting WaylandMouseSimulator instance.";
    static WaylandMouseSimulator *waylandMouseSimulator;
    if (!waylandMouseSimulator) {
        waylandMouseSimulator = new WaylandMouseSimulator();
        qCDebug(dsrApp) << "New WaylandMouseSimulator instance created.";
    }
    return waylandMouseSimulator;
}

void WaylandMouseSimulator::setCursorPoint(QPointF pos)
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Setting cursor point to:" << pos;
    if (m_fakeinput != nullptr) {
        m_fakeinput->requestPointerMoveAbsolute(pos);
        qCDebug(dsrApp) << "Requested pointer move absolute.";
    }
}

// 手动滚动模拟
void WaylandMouseSimulator::slotManualScroll(float direction)
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Manual scroll with direction:" << direction;
    if (m_fakeinput != nullptr) {
        // direction 滚动方向
        m_fakeinput->requestPointerAxisForCapture(Qt::Vertical, static_cast<double>(direction));
        qCDebug(dsrApp) << "Requested pointer axis for capture (manual scroll).";
    }
}

// 自动滚动执行
void WaylandMouseSimulator::doWaylandAutoScroll()
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Performing Wayland auto scroll.";
    if (m_fakeinput != nullptr) {
        // Qt::Vertical 垂直滚动，15.0 代表向下滚动
        m_fakeinput->requestPointerAxisForCapture(Qt::Vertical, SCROLL_DOWN);
        qCDebug(dsrApp) << "Requested pointer axis for capture (auto scroll).";
    }
}

// 初始化队列、开启注册器
void WaylandMouseSimulator::setupRegistry()
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Setting up Wayland registry.";
    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);
    qCDebug(dsrApp) << "Event queue created and setup.";

    m_registry = new KWayland::Client::Registry(this);
    qCInfo(dsrApp) << "==========setupRegistry=====";
    connect(m_registry, &KWayland::Client::Registry::fakeInputAnnounced, this, &WaylandMouseSimulator::setupFakeinput);
    //connect(m_registry, &KWayland::Client::Registry::fakeInputRemoved, this, &WaylandMonitor::removeOutput);
//    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, this, [this] {
//        m_registryInitialized = true;
//    });
    qCDebug(dsrApp) << "Registry created and fakeInputAnnounced signal connected.";

    m_registry->create(m_connection);
    m_registry->setEventQueue(m_queue);
    m_registry->setup();
    qCDebug(dsrApp) << "Registry created, event queue set, and setup completed.";
}

// 初始化Fakeinput
void WaylandMouseSimulator::setupFakeinput(quint32 name, quint32 version)
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Setting up FakeInput with name:" << name << ", version:" << version;
    if (m_fakeinput == nullptr) {
        m_fakeinput = new KWayland::Client::FakeInput(this);
        m_fakeinput->setup(m_registry->bindFakeInput(name, version));
        m_fakeinput->authenticate(qAppName(), "wayland scroll monitor");
        qCDebug(dsrApp) << "FakeInput initialized and authenticated.";
    }
}

// 滚动模拟初始化流程
void WaylandMouseSimulator::initWaylandScrollThread()
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Initializing Wayland scroll thread.";
    // 初始化ConnectionThread
    m_connection = new KWayland::Client::ConnectionThread(this);
    // 注册器
    connect(m_connection, &KWayland::Client::ConnectionThread::connected, this, &WaylandMouseSimulator::setupRegistry, Qt::QueuedConnection);
    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied, this, [this] {
        qCDebug(dsrApp) << "Wayland connection died.";
        if (m_queue)
        {
            delete m_queue;
            m_queue = nullptr;
            qCDebug(dsrApp) << "Event queue deleted.";
        }

        m_connection->deleteLater();
        m_connection = nullptr;
        qCDebug(dsrApp) << "Connection thread deleted.";
    });
    connect(m_connection, &KWayland::Client::ConnectionThread::failed, this, [this] {
        qCWarning(dsrApp) << "Wayland connection failed.";
        m_connection->deleteLater();
        m_connection = nullptr;
        qCDebug(dsrApp) << "Connection thread deleted on failure.";
    });
    m_connection->initConnection();
    qCDebug(dsrApp) << "Connection thread initialized.";
}

// 释放注册资源
void WaylandMouseSimulator::releaseWaylandScrollThread()
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Releasing Wayland scroll thread resources.";

    if (m_fakeinput) {
        m_fakeinput->release();
        delete m_fakeinput;
        m_fakeinput = nullptr;
        qCDebug(dsrApp) << "FakeInput released and deleted.";
    }

    if (m_registry) {
        m_registry->release();
        delete  m_registry;
        m_registry = nullptr;
        qCDebug(dsrApp) << "Registry released and deleted.";
    }

    if (m_queue) {
        m_queue->release();
        delete m_queue;
        m_queue = nullptr;
        qCDebug(dsrApp) << "Event queue released and deleted.";
    }

    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
        qCDebug(dsrApp) << "Connection thread deleted during release.";
    }
}

#endif
