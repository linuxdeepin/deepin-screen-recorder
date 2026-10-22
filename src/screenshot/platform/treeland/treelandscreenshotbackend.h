// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_SCREENSHOT_BACKEND_H
#define TREELAND_SCREENSHOT_BACKEND_H

#include "../../ports/iscreenshotbackend.h"

namespace Dsr::Screenshot {

class TreelandSnapSession;
class TreelandStillCapture;

class TreelandScreenshotBackend : public IScreenshotBackend
{
    Q_OBJECT
public:
    explicit TreelandScreenshotBackend(TreelandSnapSession *snapSession,
                                       QObject *parent = nullptr);

    BackendCapabilities capabilities() const override;
    QList<OutputDescriptor> outputs() const override;
    void prepare(const Request &request) override;
    void cancel() override;

private:
    TreelandSnapSession *m_snapSession = nullptr;
    TreelandStillCapture *m_capture = nullptr;
    bool m_cancelled = false;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_SCREENSHOT_BACKEND_H
