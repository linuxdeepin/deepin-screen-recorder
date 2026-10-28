// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_RECORDING_SESSION_H
#define TREELAND_RECORDING_SESSION_H

#include "recordingfiletransaction.h"

#include "../../ports/irecordingsession.h"

#include <QPointer>

class ExtCaptureRecorder;
class QTimer;

namespace Dsr::Screenshot {

class TreelandRecordingSession final : public IRecordingSession
{
    Q_OBJECT
public:
    explicit TreelandRecordingSession(QObject *parent = nullptr);
    ~TreelandRecordingSession() override;

    bool start(const RecordingRequest &request) override;
    void stop() override;
    void cancel() override;
    State state() const override;
    QString outputPath() const override;

private:
    void onRecorderStarted();
    void onRecorderStopped();
    void onRecorderError(const QString &reason);
    void startRecorderWhenAvailable();
    void onStartTimeout();
    void finishSaved();
    void finishCancelled();
    void finishFailed(const QString &reason);
    void startGifTranscode();
    void notifySaved(const QString &path);
    bool copyToClipboard(const QString &path);
    QString createOutputPath(int format) const;
    void cleanupTemporaryFiles();
    void setState(State state);
    void cleanupRecorder();

private:
    QPointer<ExtCaptureRecorder> m_recorder;
    QTimer *m_recordingNotifyTimer = nullptr;
    QTimer *m_startTimeoutTimer = nullptr;
    RecordingRequest m_request;
    State m_state = State::Idle;
    QString m_capturePath;
    QString m_finalPath;
    QString m_publishPath;
    QString m_pendingError;
    RecordingFileTransaction m_fileTransaction;
    bool m_stopRequested = false;
    bool m_startIssued = false;
    bool m_cancelled = false;
    bool m_terminal = false;
    QString m_clipboardError;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_RECORDING_SESSION_H
