#ifndef FRAMEPUMPWORKER_H
#define FRAMEPUMPWORKER_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>

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

struct AudioCaptureOptions
{
    enum Type {
        NoAudio = 0,
        Microphone = 1,
        SystemAudio = 2,
        MicrophoneAndSystemAudio = 3,
    };

    int type = NoAudio;
    QString systemDevice;
    QString microphoneDevice = QStringLiteral("default");

    bool hasMicrophone() const
    {
        return type == Microphone || type == MicrophoneAndSystemAudio;
    }

    bool hasSystemAudio() const
    {
        return type == SystemAudio || type == MicrophoneAndSystemAudio;
    }

    bool hasAudio() const
    {
        return hasMicrophone() || hasSystemAudio();
    }
};

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
                    const AudioCaptureOptions &audio = AudioCaptureOptions(),
                    QObject *parent = nullptr);
    ~FramePumpWorker() override;

    int outputFrames() const { return m_outputFrames; }
    int repeatedFrames() const { return m_repeatedFrames; }

    static QStringList buildFfmpegArguments(int width,
                                            int height,
                                            int frameRate,
                                            const QString &cropFilter,
                                            const AudioCaptureOptions &audio,
                                            const QString &outputPath);

public slots:
    void start(int width, int height, int stride);
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
    AudioCaptureOptions m_audio;
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
    int m_skippedSlots = 0;
    quint64 m_lastFrameSequence = 0;
    qint64 m_lastStatsNs = 0;
    bool m_started = false;
    bool m_stopping = false;
};

#endif // FRAMEPUMPWORKER_H
