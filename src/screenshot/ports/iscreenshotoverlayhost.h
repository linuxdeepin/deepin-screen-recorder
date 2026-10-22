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

class IScreenshotOverlayHost : public QObject
{
    Q_OBJECT
public:
    explicit IScreenshotOverlayHost(QObject *parent = nullptr) : QObject(parent) {}
    ~IScreenshotOverlayHost() override = default;

    virtual bool create(const QList<OutputDescriptor> &outputs) = 0;
    virtual QList<QWindow *> maskWindows() const = 0;
    virtual void setFrozenFrames(const FrozenFrameSet &frames) = 0;
    virtual void show() = 0;
    virtual void setSelection(const QRect &globalSelection) = 0;
    virtual void setSnapHint(const QRect &globalRegion) = 0;
    virtual void setSelectionFinalized(bool finalized) = 0;
    virtual void setCursorIntent(CursorIntent intent) = 0;
    virtual void close() = 0;

signals:
    void inputReady();
    void pointerPressed(const QPoint &globalPos);
    void pointerMoved(const QPoint &globalPos);
    void pointerReleased(const QPoint &globalPos);
    void confirmRequested();
    void copyRequested();
    void cancelRequested();
};

} // namespace Dsr::Screenshot

#endif // ISCREENSHOT_OVERLAY_HOST_H
