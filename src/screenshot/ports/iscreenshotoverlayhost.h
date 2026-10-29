// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ISCREENSHOT_OVERLAY_HOST_H
#define ISCREENSHOT_OVERLAY_HOST_H

#include "../core/screenshottypes.h"

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QWindow>

namespace Dsr::Screenshot {

class IAnnotationEditor;

class IScreenshotOverlayHost : public QObject
{
    Q_OBJECT
public:
    explicit IScreenshotOverlayHost(QObject *parent = nullptr) : QObject(parent) {}
    ~IScreenshotOverlayHost() override = default;

    virtual bool create(const QList<OutputDescriptor> &outputs) = 0;
    // The returned editor is owned by the overlay and must remain valid for
    // the controller lifetime. Implementations may expose it only after
    // create(); the controller resolves it again once windows exist.
    virtual IAnnotationEditor *annotationEditor() const { return nullptr; }
    virtual QList<QWindow *> maskWindows() const = 0;
    virtual void setFrozenFrames(const FrozenFrameSet &frames) = 0;
    // Materialize native windows without mapping them. Platform integrations
    // that need to configure the compositor-side window role must do that
    // between prepare() and show().
    virtual bool prepare() { return true; }
    virtual void show() = 0;
    virtual void setSelection(const QRect &globalSelection) = 0;
    virtual void setSnapHint(const QRect &globalRegion) = 0;
    virtual void setSelectionFinalized(bool finalized) = 0;
    virtual void setCursorIntent(CursorIntent intent) = 0;
    // TreeLand hosts may expose the same rounded-corner and pixel-adjustment
    // aids as the X11 window. Other hosts keep the default no-op behaviour.
    virtual void setSelectionHints(bool visible,
                                   bool selectionMode,
                                   const QPoint &globalCursor)
    {
        Q_UNUSED(visible);
        Q_UNUSED(selectionMode);
        Q_UNUSED(globalCursor);
    }
    virtual void setRecordModeActive(bool active) { Q_UNUSED(active); }
    // Host the existing CountdownTooltip inside the platform overlay. A
    // separate top-level tool window cannot be kept above TreeLand's
    // per-output mask surfaces.
    virtual bool startRecordingCountdown(const QRect &globalSelection)
    {
        Q_UNUSED(globalSelection);
        return false;
    }
    virtual void stopRecordingCountdown() {}
    virtual bool beginRecording(const QRect &globalSelection) { Q_UNUSED(globalSelection); return false; }
    virtual void finishRecording() {}
    virtual bool isRecordingActive() const { return false; }
    virtual void setRecordingControlsActive(bool active) { Q_UNUSED(active); }
    virtual void close() = 0;

signals:
    // Emitted synchronously when a native mask window has just acquired its
    // compositor role, before the first surface commit is sent.
    void maskWindowRoleCreated(QWindow *window);
    void inputReady();
    void inputFailed(const QString &reason);
    void pointerPressed(const QPoint &globalPos);
    void pointerMoved(const QPoint &globalPos);
    void pointerReleased(const QPoint &globalPos);
    void selectionMoveRequested(const QPoint &delta);
    void selectionResizeRequested(Qt::Key key, Qt::KeyboardModifiers modifiers);
    void selectionSizeChangeRequested(int width, int height, qreal pixelScale);
    void roundedCornerRadiusChanged(int radius);
    void confirmRequested();
    void copyRequested();
    void pinRequested();
    void cancelRequested();
    void recordModeChanged(bool enabled);
    void recordStartRequested();
    void recordStopRequested();
    void recordingCountdownFinished();
};

} // namespace Dsr::Screenshot

#endif // ISCREENSHOT_OVERLAY_HOST_H
