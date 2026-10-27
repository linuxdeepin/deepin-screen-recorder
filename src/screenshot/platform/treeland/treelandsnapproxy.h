// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELANDSNAPPROXY_H
#define TREELANDSNAPPROXY_H

#include "../../../protocols/treeland-snap-target/qwayland-treeland-snap-target-unstable-v1.h"
#include "../../../protocols/treeland-snap-target/qwayland-xdg-toplevel-tag-v1.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QString>
#include <QWaylandClientExtension>

class QWindow;
struct wl_registry;
struct wl_seat;

namespace Dsr::Screenshot {

class TreelandToplevelTagManager
    : public QWaylandClientExtensionTemplate<TreelandToplevelTagManager>
    , public QtWayland::xdg_toplevel_tag_manager_v1
{
    Q_OBJECT
public:
    TreelandToplevelTagManager();
    ~TreelandToplevelTagManager() override;
};

class TreelandSnapTarget
    : public QWaylandClientExtensionTemplate<TreelandSnapTarget>
    , public QtWayland::treeland_snap_target_v1
{
    Q_OBJECT
public:
    TreelandSnapTarget();
    ~TreelandSnapTarget() override;

signals:
    void snapRegion(int x, int y, quint32 width, quint32 height);
    void pidfd(int fd);
    void failed(quint32 reason);

protected:
    void treeland_snap_target_v1_snap_region(int32_t x,
                                             int32_t y,
                                             uint32_t width,
                                             uint32_t height) override;
    void treeland_snap_target_v1_pidfd(int32_t pidfd) override;
    void treeland_snap_target_v1_failed(uint32_t reason) override;
};

/**
 * Process-wide treeland_snap_target_v1 session.
 *
 * Treeland exposes one snap object per client, shared by all per-output mask
 * windows. Every mask must be tagged before it is mapped; tagging an already
 * active toplevel moves it between compositor containers and is unsafe.
 */
class TreelandSnapSession : public QObject
{
    Q_OBJECT
public:
    explicit TreelandSnapSession(QObject *parent = nullptr);
    ~TreelandSnapSession() override;

    bool waitUntilReady(int timeoutMs, QString *error);
    bool registerWindow(QWindow *window, QString *error);
    bool start(QString *error);
    void stop();

    QRect region() const;
    QString processName() const;

signals:
    void ready();
    void regionChanged(const QRect &region);
    void failed(const QString &reason);

private:
    void maybeInitialize();
    bool tagWindow(QWindow *window);
    void onSnapRegion(int x, int y, quint32 width, quint32 height);
    void onFailed(quint32 reason);
    void setProcessName(const QString &name);
    void clearPendingPidfd();

    TreelandToplevelTagManager m_tagManager;
    TreelandSnapTarget m_snap;

    wl_registry *m_seatRegistry = nullptr;
    wl_seat *m_seat = nullptr;

    QList<QPointer<QWindow>> m_windows;
    bool m_initialized = false;
    bool m_startRequested = false;
    bool m_started = false;
    QRect m_region;
    QString m_processName;
    int m_pendingPidfd = -1;
};

} // namespace Dsr::Screenshot

#endif // TREELANDSNAPPROXY_H
