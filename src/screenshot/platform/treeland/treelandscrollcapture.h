// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_SCROLL_CAPTURE_H
#define TREELAND_SCROLL_CAPTURE_H

#include "../../ports/iscrollcapture.h"

#include <QImage>
#include <QList>
#include <QRect>
#include <QString>
#include <QTimer>

#include <cstdint>

namespace Dsr::Screenshot {

class TreelandSnapSession;
class TreelandOutputCapture;

class TreelandScrollCapture final : public IScrollCapture
{
    Q_OBJECT
public:
    explicit TreelandScrollCapture(QObject *parent = nullptr);
    ~TreelandScrollCapture() override;

    void setSnapSession(TreelandSnapSession *snapSession);
    void setFrozenFrames(const FrozenFrameSet &frames) override;

    bool start(const QList<OutputDescriptor> &outputs,
               const QRect &globalSelection,
               QString *error) override;
    void requestFrame() override;
    void resume() override;
    void pause() override;
    void stop() override;

private:
    bool captureFrame(QImage *image, quint64 *timestampNs, QString *error);

    TreelandSnapSession *m_snapSession = nullptr;
    QTimer *m_captureTimer = nullptr;
    QRect m_globalSelection;
    QRect m_targetWindowRegion;
    QString m_targetProcessName;
    qint64 m_targetProcessId = -1;
    QList<OutputDescriptor> m_captureOutputs;
    bool m_started = false;
    bool m_stopping = false;
    bool m_frameInFlight = false;
    int m_consecutiveFailures = 0;

    QList<TreelandOutputCapture *> m_outputCaptures;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_SCROLL_CAPTURE_H
