// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCAL_SCREENSHOT_RESULT_SINK_H
#define LOCAL_SCREENSHOT_RESULT_SINK_H

#include "../ports/iscreenshotresultsink.h"

namespace Dsr::Screenshot {

class LocalScreenshotResultSink : public IScreenshotResultSink
{
    Q_OBJECT
public:
    explicit LocalScreenshotResultSink(QObject *parent = nullptr);

    bool save(const Result &result,
              const QString &requestedPath,
              QString *savedPath,
              QString *error,
              bool *cancelled = nullptr) override;
    bool copy(const Result &result, QString *error) override;
    bool pin(const Result &result, QString *error) override;

signals:
    void clipboardDataComing();

private slots:
    void onClipboardDataComing(const QByteArray &data);

private:
    bool m_clipboardSignalConnected = false;
    bool m_clipboardReborned = false;
    bool m_clipboardRebornSucceeded = false;
    QString m_clipboardRebornError;
};

} // namespace Dsr::Screenshot

#endif // LOCAL_SCREENSHOT_RESULT_SINK_H
