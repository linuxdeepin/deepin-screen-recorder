// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ISCREENSHOT_RESULT_SINK_H
#define ISCREENSHOT_RESULT_SINK_H

#include "../core/screenshottypes.h"

#include <QObject>
#include <QString>

namespace Dsr::Screenshot {

/**
 * Side-effect boundary for screenshot results.
 *
 * The session controller decides when an export happens. Concrete sinks own
 * filesystem, clipboard and later OCR/pin/AI integration details.
 */
class IScreenshotResultSink : public QObject
{
    Q_OBJECT
public:
    explicit IScreenshotResultSink(QObject *parent = nullptr) : QObject(parent) {}
    ~IScreenshotResultSink() override = default;

    virtual bool save(const Result &result,
                      const QString &requestedPath,
                      QString *savedPath,
                      QString *error) = 0;
    virtual bool copy(const Result &result, QString *error) = 0;
};

} // namespace Dsr::Screenshot

#endif // ISCREENSHOT_RESULT_SINK_H
