// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandsnapproxy.h"

#include "../../../utils/log.h"

#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QGuiApplication>
#include <QTimer>
#include <QWindow>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <qnativeinterface.h>

#include <any>
#include <unistd.h>

namespace Dsr::Screenshot {

namespace {

constexpr auto SnapMaskTag = "org.deepin.treeland.snap-mask";

QString processNameForPidfd(int pidfd)
{
    QFile fdinfo(QStringLiteral("/proc/self/fdinfo/%1").arg(pidfd));
    if (!fdinfo.open(QIODevice::ReadOnly))
        return QString();

    qint64 pid = 0;
    const QList<QByteArray> lines = fdinfo.readAll().split('\n');
    for (const QByteArray &line : lines) {
        if (line.startsWith("Pid:")) {
            pid = line.mid(4).trimmed().toLongLong();
            break;
        }
    }
    if (pid <= 0)
        return QString();

    QFile comm(QStringLiteral("/proc/%1/comm").arg(pid));
    if (!comm.open(QIODevice::ReadOnly))
        return QString();
    return QString::fromLocal8Bit(comm.readAll()).trimmed();
}

void seatRegistryGlobal(void *data,
                        wl_registry *registry,
                        uint32_t name,
                        const char *interface,
                        uint32_t version)
{
    auto **seat = static_cast<wl_seat **>(data);
    if (*seat || qstrcmp(interface, wl_seat_interface.name) != 0)
        return;
    *seat = static_cast<wl_seat *>(
        wl_registry_bind(registry, name, &wl_seat_interface, qMin<uint32_t>(version, 9)));
}

void seatRegistryGlobalRemove(void *data, wl_registry *registry, uint32_t name)
{
    Q_UNUSED(data)
    Q_UNUSED(registry)
    Q_UNUSED(name)
}

const wl_registry_listener seatRegistryListener = {
    seatRegistryGlobal,
    seatRegistryGlobalRemove,
};

struct xdg_toplevel *getXdgToplevel(QWindow *window)
{
    if (!window || !window->handle())
        return nullptr;

    auto *waylandWindow = static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (!waylandWindow)
        return nullptr;

    auto *shellSurface = waylandWindow->shellSurface();
    if (!shellSurface)
        return nullptr;

    try {
        return std::any_cast<struct xdg_toplevel *>(shellSurface->surfaceRole());
    } catch (const std::bad_any_cast &) {
        return nullptr;
    }
}

} // namespace

TreelandToplevelTagManager::TreelandToplevelTagManager()
    : QWaylandClientExtensionTemplate<TreelandToplevelTagManager>(1)
{
}

TreelandToplevelTagManager::~TreelandToplevelTagManager()
{
    if (isActive())
        destroy();
}

TreelandSnapTarget::TreelandSnapTarget()
    : QWaylandClientExtensionTemplate<TreelandSnapTarget>(1)
{
}

TreelandSnapTarget::~TreelandSnapTarget()
{
    if (isActive())
        destroy();
}

void TreelandSnapTarget::treeland_snap_target_v1_snap_region(int32_t x,
                                                              int32_t y,
                                                              uint32_t width,
                                                              uint32_t height)
{
    emit snapRegion(static_cast<int>(x),
                    static_cast<int>(y),
                    static_cast<quint32>(width),
                    static_cast<quint32>(height));
}

void TreelandSnapTarget::treeland_snap_target_v1_pidfd(int32_t pidfd)
{
    emit this->pidfd(static_cast<int>(pidfd));
}

void TreelandSnapTarget::treeland_snap_target_v1_failed(uint32_t reason)
{
    emit failed(reason);
}

TreelandSnapSession::TreelandSnapSession(QObject *parent)
    : QObject(parent)
{
    if (auto *waylandApp =
            qGuiApp ? qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>() : nullptr) {
        if (auto *display = waylandApp->display()) {
            m_seatRegistry = wl_display_get_registry(display);
            wl_registry_add_listener(m_seatRegistry, &seatRegistryListener, &m_seat);
            wl_display_roundtrip(display);
        }
    }

    connect(&m_tagManager,
            &QWaylandClientExtension::activeChanged,
            this,
            &TreelandSnapSession::maybeInitialize);
    connect(&m_snap,
            &QWaylandClientExtension::activeChanged,
            this,
            &TreelandSnapSession::maybeInitialize);
    connect(&m_snap, &TreelandSnapTarget::pidfd, this, [this](int fd) {
        clearPendingPidfd();
        if (fd >= 0)
            m_pendingPidfd = fd;
    });
    connect(&m_snap, &TreelandSnapTarget::snapRegion, this, &TreelandSnapSession::onSnapRegion);
    connect(&m_snap, &TreelandSnapTarget::failed, this, &TreelandSnapSession::onFailed);
}

TreelandSnapSession::~TreelandSnapSession()
{
    stop();
    clearPendingPidfd();
    if (m_seat) {
        wl_seat_destroy(m_seat);
        m_seat = nullptr;
    }
    if (m_seatRegistry) {
        wl_registry_destroy(m_seatRegistry);
        m_seatRegistry = nullptr;
    }
}

bool TreelandSnapSession::waitUntilReady(int timeoutMs, QString *error)
{
    maybeInitialize();
    if (m_initialized)
        return true;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &TreelandSnapSession::ready, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();
    maybeInitialize();

    if (m_initialized)
        return true;

    if (error) {
        QStringList missing;
        if (!m_tagManager.isActive())
            missing << QStringLiteral("xdg_toplevel_tag_manager_v1");
        if (!m_snap.isActive())
            missing << QStringLiteral("treeland_snap_target_v1");
        if (!m_seat)
            missing << QStringLiteral("wl_seat");
        *error = QStringLiteral("Treeland 截图协议不可用：%1").arg(missing.join(QStringLiteral(", ")));
    }
    return false;
}

bool TreelandSnapSession::registerWindow(QWindow *window, QString *error)
{
    if (!window) {
        if (error)
            *error = QStringLiteral("无效的截图覆盖窗口");
        return false;
    }

    if (!m_windows.contains(window))
        m_windows.append(window);

    if (!m_initialized) {
        if (error)
            *error = QStringLiteral("Treeland snap 会话尚未就绪");
        return false;
    }

    tagWindow(window);
    if (!getXdgToplevel(window)) {
        if (error)
            *error = QStringLiteral("无法获取 xdg_toplevel，Mask 窗口未成功创建");
        return false;
    }
    return true;
}

bool TreelandSnapSession::start(QString *error)
{
    m_startRequested = true;
    if (!m_initialized) {
        if (error)
            *error = QStringLiteral("Treeland snap 会话尚未就绪");
        return false;
    }
    if (!m_started) {
        m_started = true;
        m_snap.start(m_seat, TREELAND_SNAP_TARGET_V1_EVENT_TYPE_PIDFD);
    }
    return true;
}

void TreelandSnapSession::stop()
{
    if (m_started) {
        m_snap.stop();
        m_started = false;
    }
    m_startRequested = false;
}

QRect TreelandSnapSession::region() const
{
    return m_region;
}

QString TreelandSnapSession::processName() const
{
    return m_processName;
}

void TreelandSnapSession::maybeInitialize()
{
    if (m_initialized)
        return;
    if (!m_tagManager.isActive() || !m_snap.isActive() || !m_seat)
        return;

    for (const QPointer<QWindow> &window : m_windows) {
        if (window)
            tagWindow(window);
    }

    m_initialized = true;
    emit ready();
    if (m_startRequested)
        start(nullptr);
}

void TreelandSnapSession::tagWindow(QWindow *window)
{
    auto *toplevel = getXdgToplevel(window);
    if (!toplevel) {
        qWarning() << "Treeland snap: xdg_toplevel is not available for mask window";
        return;
    }
    m_tagManager.set_toplevel_tag(toplevel, SnapMaskTag);
    qCInfo(dsrApp) << "Treeland snap: mask tag set after the overlay is mapped";
}

void TreelandSnapSession::onSnapRegion(int x, int y, quint32 width, quint32 height)
{
    if (m_pendingPidfd >= 0)
        setProcessName(processNameForPidfd(m_pendingPidfd));
    else
        setProcessName(QString());
    clearPendingPidfd();

    m_region = (width != 0 && height != 0)
            ? QRect(x, y, static_cast<int>(width), static_cast<int>(height))
            : QRect();
    emit regionChanged(m_region);
}

void TreelandSnapSession::onFailed(quint32 reason)
{
    m_started = false;
    const QString description = reason == QtWayland::treeland_snap_target_v1::failure_reason_snap_busy
            ? QStringLiteral("另一个客户端正在使用 Treeland snap")
            : QStringLiteral("Treeland snap 启动失败，reason=%1").arg(reason);
    qWarning() << description;
    emit failed(description);
}

void TreelandSnapSession::setProcessName(const QString &name)
{
    if (m_processName == name)
        return;
    m_processName = name;
}

void TreelandSnapSession::clearPendingPidfd()
{
    if (m_pendingPidfd >= 0) {
        ::close(m_pendingPidfd);
        m_pendingPidfd = -1;
    }
}

} // namespace Dsr::Screenshot
