// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_SCROLL_CAPTURE_H
#define TREELAND_SCROLL_CAPTURE_H

#include "../../ports/iscrollcapture.h"

#include <QList>
#include <QRect>
#include <QString>
#include <QTimer>

#include <cstdint>

class QImage;
class QScreen;
struct wl_display;
struct wl_output;
struct wl_registry;
struct wl_shm;
struct zwlr_screencopy_manager_v1;

namespace Dsr::Screenshot {

class TreelandSnapSession;
struct TreelandWaylandOutput;

class TreelandScrollCapture final : public IScrollCapture
{
    Q_OBJECT
public:
    explicit TreelandScrollCapture(QObject *parent = nullptr);
    ~TreelandScrollCapture() override;

    void setSnapSession(TreelandSnapSession *snapSession);
    void setFrozenFrames(const FrozenFrameSet &frames) override;

    bool start(const QList<OutputDescriptor> &outputs,
               const QRect &globalSelection,
               QString *error) override;
    void requestFrame() override;
    void stop() override;

    static void registryGlobal(void *data,
                               wl_registry *registry,
                               uint32_t name,
                               const char *interface,
                               uint32_t version);
    static void registryGlobalRemove(void *data,
                                     wl_registry *registry,
                                     uint32_t name);

private:
    bool initializeWayland(QString *error);
    void cleanupWayland();
    wl_output *findWaylandOutput(const QString &outputId) const;
    bool captureFrame(QImage *image, quint64 *timestampNs, QString *error);
    void emitFailure(const QString &reason);

    TreelandSnapSession *m_snapSession = nullptr;
    QTimer *m_captureTimer = nullptr;
    QRect m_globalSelection;
    QRect m_targetWindowRegion;
    QString m_targetProcessName;
    QRect m_captureRegion;
    OutputDescriptor m_captureOutput;
    QScreen *m_captureScreen = nullptr;
    FrozenFrameSet m_referenceFrames;
    bool m_started = false;
    bool m_stopping = false;
    bool m_frameInFlight = false;

    wl_display *m_display = nullptr;
    wl_registry *m_registry = nullptr;
    wl_shm *m_shm = nullptr;
    zwlr_screencopy_manager_v1 *m_screencopyManager = nullptr;
    uint32_t m_screencopyVersion = 0;
    QList<TreelandWaylandOutput *> m_waylandOutputs;
    wl_output *m_output = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_SCROLL_CAPTURE_H
