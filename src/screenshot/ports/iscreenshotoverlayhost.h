// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ISCREENSHOT_OVERLAY_HOST_H
#define ISCREENSHOT_OVERLAY_HOST_H

#include "../core/screenshottypes.h"

#include <QImage>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QWindow>

namespace Dsr::Screenshot {

class IAnnotationEditor;

// Platform-neutral mirror of the X11 ScrollShotTip types. Keeping the enum in
// the port lets the session controller drive TreeLand prompts without a
// dependency on the widget implementation.
enum class ScrollTipKind {
    None,
    Start,
    Error,
    End,
    MaxLength,
    QuickScroll,
    InvalidArea
};

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
                                   const QPoint &globalCursor,
                                   bool hasGlobalCursor)
    {
        Q_UNUSED(visible);
        Q_UNUSED(selectionMode);
        Q_UNUSED(globalCursor);
        Q_UNUSED(hasGlobalCursor);
    }
    virtual void setRecordModeActive(bool active) { Q_UNUSED(active); }
    // Apply the launch mode before the first user interaction. This lets a
    // --record session open directly on the recorder toolbar instead of
    // starting in screenshot mode and requiring an extra UI switch.
    virtual void setInitialMode(Mode mode) { Q_UNUSED(mode); }
    // Returns the HEX value under the selection magnifier. Empty means the
    // magnifier is not active and Ctrl+C should keep its normal screenshot
    // semantics.
    virtual QString magnifierColorHex() const { return {}; }
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
    virtual bool beginScroll(const QRect &globalSelection) { Q_UNUSED(globalSelection); return false; }
    // The stitched image produced while scrolling. Hosts that can display a
    // live preview update it here; the default implementation is a no-op for
    // platforms without an in-overlay preview.
    virtual void setScrollPreview(const QImage &image) { Q_UNUSED(image); }
    // Called around each scroll capture request. Hosts hide any overlay
    // widgets that intersect the selection so output-based capture cannot
    // include the preview or toolbar in the stitched image.
    virtual void beginScrollFrameCapture() {}
    virtual void endScrollFrameCapture() {}
    // Shows the same prompt types and placement rules as the X11
    // ScrollShotTip. Hosts without an in-overlay prompt keep the no-op.
    virtual void showScrollTip(ScrollTipKind kind) { Q_UNUSED(kind); }
    virtual void hideScrollTip() {}
    virtual void finishScroll(bool restoreSelectionUi = true)
    {
        Q_UNUSED(restoreSelectionUi);
    }
    virtual bool isScrollActive() const { return false; }
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
    void selectionRatioChangeRequested(int width, int height);
    void roundedCornerRadiusChanged(int radius);
    void confirmRequested();
    void copyRequested();
    void pinRequested();
    void ocrRequested();
    void tableRecognizeRequested();
    void aiAssistantRequested(int function);
    void cancelRequested();
    void recordModeChanged(bool enabled);
    void recordStartRequested();
    void recordStopRequested();
    void recordingCountdownFinished();
    void scrollStartRequested();
    void scrollStopRequested();
};

} // namespace Dsr::Screenshot

#endif // ISCREENSHOT_OVERLAY_HOST_H
