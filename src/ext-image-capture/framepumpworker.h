#ifndef FRAMEPUMPWORKER_H
#define FRAMEPUMPWORKER_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QMutex>
#include <QObject>
#include <QString>

#include <memory>

class QProcess;
class QTimer;

struct CaptureFrameSnapshot
{
    QByteArray data;
    int width = 0;
    int height = 0;
    int stride = 0;
    qint64 ptsNs = 0;
    quint64 sequence = 0;
};

using CaptureFrameSnapshotPtr = std::shared_ptr<CaptureFrameSnapshot>;

class LatestFrameStore
{
public:
    void set(const CaptureFrameSnapshotPtr &frame);
    CaptureFrameSnapshotPtr get() const;
    void clear();

private:
    mutable QMutex m_mutex;
    CaptureFrameSnapshotPtr m_frame;
};

class FramePumpWorker : public QObject
{
    Q_OBJECT

public:
    FramePumpWorker(int frameRate,
                    const QString &outputPath,
                    const QString &cropFilter,
                    LatestFrameStore *store,
                    QObject *parent = nullptr);
    ~FramePumpWorker() override;

    int outputFrames() const { return m_outputFrames; }
    int repeatedFrames() const { return m_repeatedFrames; }

public slots:
    void start(int width, int height, int stride, qint64 startOffsetNs);
    void stop();

signals:
    void started();
    void frameWritten(int totalFrames);
    void statsUpdated(int outputFrames, int repeatedFrames, qint64 behindMs);
    void errorOccurred(const QString &message);
    void finished();

private slots:
    void onTick();

private:
    bool startFfmpeg();
    bool writeCurrentFrame();
    void fail(const QString &message);
    void stopFfmpeg();

    int m_frameRate;
    QString m_outputPath;
    QString m_cropFilter;
    LatestFrameStore *m_store;
    QProcess *m_ffmpeg = nullptr;
    QTimer *m_timer = nullptr;
    QElapsedTimer m_clock;
    qint64 m_intervalNs = 0;
    qint64 m_nextDeadlineNs = 0;
    int m_width = 0;
    int m_height = 0;
    int m_stride = 0;
    int m_outputFrames = 0;
    int m_repeatedFrames = 0;
    quint64 m_lastFrameSequence = 0;
    qint64 m_lastStatsNs = 0;
    bool m_started = false;
    bool m_stopping = false;
};

#endif // FRAMEPUMPWORKER_H
