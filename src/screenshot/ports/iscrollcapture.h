// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ISCROLL_CAPTURE_H
#define ISCROLL_CAPTURE_H

#include "../core/screenshottypes.h"

#include <QObject>

namespace Dsr::Screenshot {

class IScrollCapture : public QObject
{
    Q_OBJECT
public:
    explicit IScrollCapture(QObject *parent = nullptr) : QObject(parent) {}
    ~IScrollCapture() override = default;

    virtual bool start(const QList<OutputDescriptor> &outputs,
                       const QRect &globalSelection,
                       QString *error) = 0;
    virtual void setFrozenFrames(const FrozenFrameSet &frames) { Q_UNUSED(frames) }
    virtual void requestFrame() = 0;
    virtual void stop() = 0;

signals:
    // Emitted immediately before the backend submits a capture request and
    // after the frame has been consumed. The overlay uses this window to hide
    // widgets that intersect the selection so they do not end up in the
    // stitched image.
    void captureAboutToStart();
    void captureFinished();
    void frameReady(const Dsr::Screenshot::ScrollFrame &frame);
    void failed(const QString &reason);
};

} // namespace Dsr::Screenshot

#endif // ISCROLL_CAPTURE_H
