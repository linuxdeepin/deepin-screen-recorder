// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandpinplacement.h"

#include "../../../utils/log.h"
#include "qwayland-treeland-dde-shell-v2.h"

#include <QGuiApplication>
#include <QPlatformSurfaceEvent>
#include <QScreen>
#include <QWindow>
#include <QtWaylandClient/QWaylandClientExtensionTemplate>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

namespace Dsr::PinScreenshots {
namespace {

class PinShellManager : public QWaylandClientExtensionTemplate<PinShellManager>,
                        public QtWayland::treeland_dde_shell_manager_v2
{
public:
    PinShellManager() : QWaylandClientExtensionTemplate<PinShellManager>(1) { initialize(); }
    ~PinShellManager() override
    {
        if (isInitialized())
            destroy();
    }
};

class PinInitialPlacement : public QObject
{
public:
    PinInitialPlacement(QWindow *window, QNativeInterface::Private::QWaylandWindow *native,
                        const QPoint &position)
        : m_window(window), m_position(position)
    {
        window->installEventFilter(this);
        connect(native, &QNativeInterface::Private::QWaylandWindow::surfaceCreated,
                this, [this, native] { setHint(native->surface()); });
        connect(native, &QNativeInterface::Private::QWaylandWindow::surfaceDestroyed,
                this, &PinInitialPlacement::release);
        setHint(native->surface());
    }

    ~PinInitialPlacement() override { release(); }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::PlatformSurface
                && static_cast<QPlatformSurfaceEvent *>(event)->surfaceEventType()
                    == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
            release();
        }
        return QObject::eventFilter(watched, event);
    }

private:
    void setHint(wl_surface *surface)
    {
        if (!surface || m_shellSurface)
            return;

        PinShellManager manager;
        if (!manager.isActive()) {
            qCDebug(dsrApp) << "Treeland pin: initial placement protocol unavailable";
            return;
        }

        // Treeland consumes the hint when it creates the toplevel wrapper.
        auto *screen = m_window->screen();
        auto *nativeScreen = screen ? screen->nativeInterface<QNativeInterface::QWaylandScreen>() : nullptr;
        auto *output = nativeScreen ? nativeScreen->output() : nullptr;
        auto *anchor = output ? screen : QGuiApplication::primaryScreen();
        const QPoint offset = anchor ? m_position - anchor->geometry().topLeft() : m_position;
        m_shellSurface = manager.get_shell_surface(surface);
        if (m_shellSurface) {
            treeland_dde_shell_surface_v2_set_position_hint(m_shellSurface, output, offset.x(), offset.y());
            qCDebug(dsrApp) << "Treeland pin: requested initial position" << m_position;
        }
    }

    void release()
    {
        if (m_shellSurface) {
            treeland_dde_shell_surface_v2_destroy(m_shellSurface);
            m_shellSurface = nullptr;
        }
    }

    QWindow *m_window;
    QPoint m_position;
    treeland_dde_shell_surface_v2 *m_shellSurface = nullptr;
};

} // namespace

std::unique_ptr<QObject> requestTreelandPinPlacement(QWindow *window, const QPoint &position)
{
    if (!window)
        return {};
    auto *native = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!native)
        return {};
    return std::make_unique<PinInitialPlacement>(window, native, position);
}

} // namespace Dsr::PinScreenshots
