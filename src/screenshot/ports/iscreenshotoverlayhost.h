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
    void confirmRequested();
    void copyRequested();
    void cancelRequested();
};

} // namespace Dsr::Screenshot

#endif // ISCREENSHOT_OVERLAY_HOST_H
