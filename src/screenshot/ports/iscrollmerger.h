// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ISCROLL_MERGER_H
#define ISCROLL_MERGER_H

#include "../core/screenshottypes.h"

#include <QImage>
#include <QObject>
#include <QRect>

namespace Dsr::Screenshot {

class IScrollImageMerger : public QObject
{
    Q_OBJECT
public:
    explicit IScrollImageMerger(QObject *parent = nullptr) : QObject(parent) {}
    ~IScrollImageMerger() override = default;

    virtual void start(const QRect &globalSelection) = 0;
    virtual bool addFrame(const ScrollFrame &frame) = 0;
    virtual QImage finish() = 0;
    virtual void cancel() = 0;

signals:
    void previewUpdated(const QImage &image);
    void reachedBottom();
    void maxHeightReached();
    void quickScrollDetected();
    void invalidAreaDetected();
    void failed(const QString &reason);
};

} // namespace Dsr::Screenshot

#endif // ISCROLL_MERGER_H
