// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ISCREENSHOT_BACKEND_H
#define ISCREENSHOT_BACKEND_H

#include "../core/screenshottypes.h"

#include <QObject>

namespace Dsr::Screenshot {

class IScreenshotBackend : public QObject
{
    Q_OBJECT
public:
    explicit IScreenshotBackend(QObject *parent = nullptr) : QObject(parent) {}
    ~IScreenshotBackend() override = default;

    virtual BackendCapabilities capabilities() const = 0;
    virtual QList<OutputDescriptor> outputs() const = 0;
    virtual void prepare(const Request &request) = 0;
    virtual void cancel() = 0;

signals:
    void prepared(const Dsr::Screenshot::FrozenFrameSet &frames);
    void failed(const QString &reason);
};

} // namespace Dsr::Screenshot

#endif // ISCREENSHOT_BACKEND_H
