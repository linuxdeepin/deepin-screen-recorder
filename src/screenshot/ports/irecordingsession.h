// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IRECORDING_SESSION_H
#define IRECORDING_SESSION_H

#include <QObject>
#include <QList>
#include <QRect>
#include <QString>

class QScreen;

namespace Dsr::Screenshot {

struct RecordingRequest {
    QList<QScreen *> screens;
    QScreen *screen = nullptr;
    QRect outputGeometry;
    QRect selection;
    bool includeCursor = false;
    int frameRate = 30;
    int format = 1;
    QString outputPath;
};

class IRecordingSession : public QObject
{
    Q_OBJECT
public:
    enum class State {
        Idle,
        Starting,
        Recording,
        Stopping,
        Completed,
        Cancelled,
        Failed
    };
    Q_ENUM(State)

    explicit IRecordingSession(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~IRecordingSession() override = default;

    virtual bool start(const RecordingRequest &request) = 0;
    virtual void stop() = 0;
    virtual void cancel() = 0;
    virtual State state() const = 0;
    virtual QString outputPath() const = 0;

signals:
    void started();
    void saved(const QString &path);
    void cancelled();
    void failed(const QString &reason);
};

} // namespace Dsr::Screenshot

#endif // IRECORDING_SESSION_H
