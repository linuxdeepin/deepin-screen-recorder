#include "framepumpworker.h"

#include "../utils/log.h"

#include <QDebug>
#include <QProcess>
#include <QTimer>

void LatestFrameStore::set(const CaptureFrameSnapshotPtr &frame)
{
    QMutexLocker locker(&m_mutex);
    m_frame = frame;
}

CaptureFrameSnapshotPtr LatestFrameStore::get() const
{
    QMutexLocker locker(&m_mutex);
    return m_frame;
}

void LatestFrameStore::clear()
{
    QMutexLocker locker(&m_mutex);
    m_frame.reset();
}

FramePumpWorker::FramePumpWorker(int frameRate,
                                 const QString &outputPath,
                                 const QString &cropFilter,
                                 LatestFrameStore *store,
                                 const AudioCaptureOptions &audio,
                                 QObject *parent)
    : QObject(parent)
    , m_frameRate(qMax(1, frameRate))
    , m_outputPath(outputPath)
    , m_cropFilter(cropFilter)
    , m_store(store)
    , m_audio(audio)
    , m_intervalNs(1000000000LL / qMax(1, frameRate))
{
}

FramePumpWorker::~FramePumpWorker()
{
    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
    }
    if (m_ffmpeg) {
        m_ffmpeg->kill();
        m_ffmpeg->waitForFinished(1000);
        delete m_ffmpeg;
        m_ffmpeg = nullptr;
    }
}

void FramePumpWorker::start(int width, int height, int stride)
{
    if (m_started || m_stopping)
        return;

    m_width = width;
    m_height = height;
    m_stride = stride;
    if (m_width <= 0 || m_height <= 0 || m_stride <= 0) {
        fail(QStringLiteral("Invalid frame dimensions for frame pump"));
        return;
    }

    if (!m_store || !m_store->get()) {
        fail(QStringLiteral("No first frame available for frame pump"));
        return;
    }

    if (!startFfmpeg())
        return;

    m_clock.start();
    m_nextDeadlineNs = m_intervalNs;
    m_lastStatsNs = 0;

    // The first frame establishes the video and audio timeline origin. Every
    // subsequent frame is sampled on a target-rate slot and timestamped by
    // FFmpeg's wallclock input clock; fps_mode=cfr turns the resulting
    // irregular arrival times into a constant-frame-rate output stream.
    if (!writeCurrentFrame()) {
        fail(QStringLiteral("Failed to write first frame to encoder"));
        return;
    }

    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->setInterval(qBound(1, 1000 / (m_frameRate * 4), 10));
    connect(m_timer, &QTimer::timeout, this, &FramePumpWorker::onTick);
    m_timer->start();

    m_started = true;
    qCWarning(dsrApp) << "FramePumpWorker started:"
                      << m_width << "x" << m_height
                      << "fps:" << m_frameRate
                      << "timeline: wallclock-pts/cfr"
                      << "output:" << m_outputPath;
    emit started();
}

void FramePumpWorker::stop()
{
    if (m_stopping)
        return;

    if (m_timer) {
        m_timer->stop();
    }

    // Stamp one final frame at the stop instant. This closes the last CFR
    // interval and keeps the encoded duration tied to the user's stop time.
    if (m_started && m_ffmpeg && m_ffmpeg->state() == QProcess::Running) {
        if (!writeCurrentFrame())
            qCWarning(dsrApp) << "FramePumpWorker: failed to write final frame";
    }

    m_stopping = true;
    stopFfmpeg();

    qCWarning(dsrApp) << "FramePumpWorker stopped, output frames:" << m_outputFrames
                      << "repeated frames:" << m_repeatedFrames
                      << "skipped cfr slots:" << m_skippedSlots;
    emit finished();
}

void FramePumpWorker::onTick()
{
    if (m_stopping || !m_started)
        return;

    const qint64 now = m_clock.nsecsElapsed();
    if (now >= m_nextDeadlineNs) {
        if (!writeCurrentFrame()) {
            fail(QStringLiteral("Failed to write frame to encoder"));
            return;
        }
        m_nextDeadlineNs += m_intervalNs;
        if (m_nextDeadlineNs <= now) {
            // Do not burst stale frames into the pipe: wallclock timestamps
            // would collapse together and make the video run ahead of audio.
            // Realign to the next future slot and let CFR duplicate the last
            // frame across the gap.
            m_skippedSlots += static_cast<int>((now - m_nextDeadlineNs) / m_intervalNs) + 1;
            m_nextDeadlineNs = now + m_intervalNs;
            qCWarning(dsrApp) << "FramePumpWorker aligned to current wallclock slot, skipped:"
                              << m_skippedSlots;
        }
    }

    if (now - m_lastStatsNs >= 1000000000LL) {
        m_lastStatsNs = now;
        const qint64 behindMs = now >= m_nextDeadlineNs
                ? (now - m_nextDeadlineNs) / 1000000LL
                : 0;
        emit statsUpdated(m_outputFrames, m_repeatedFrames, behindMs);
    }
}

QStringList FramePumpWorker::buildFfmpegArguments(int width,
                                                  int height,
                                                  int frameRate,
                                                  const QString &cropFilter,
                                                  const AudioCaptureOptions &audio,
                                                  const QString &outputPath)
{
    QStringList arguments;
    arguments << "-hide_banner"
              << "-loglevel" << "warning";

    // X11 opens Pulse inputs before the video input. Keeping the same order
    // avoids the first video frames getting ahead of the audio timeline.
    int nextInputIndex = 0;
    int systemAudioIndex = -1;
    int microphoneIndex = -1;
    if (audio.hasSystemAudio()) {
        arguments << "-thread_queue_size" << "2048"
                  << "-fragment_size" << "4096"
                  << "-f" << "pulse"
                  << "-ac" << "2"
                  << "-ar" << "44100"
                  << "-i" << audio.systemDevice;
        systemAudioIndex = nextInputIndex++;
    }
    if (audio.hasMicrophone()) {
        arguments << "-thread_queue_size" << "2048"
                  << "-fragment_size" << "4096"
                  << "-f" << "pulse"
                  << "-ac" << "2"
                  << "-ar" << "44100"
                  << "-i" << audio.microphoneDevice;
        microphoneIndex = nextInputIndex++;
    }

    const int videoIndex = nextInputIndex;
    arguments << "-thread_queue_size" << "64"
              << "-use_wallclock_as_timestamps" << "1"
              << "-f" << "rawvideo"
              << "-pix_fmt" << "rgba"
              << "-s" << QStringLiteral("%1x%2").arg(width).arg(height)
              << "-i" << "-";

    QStringList filters;
    if (!cropFilter.isEmpty())
        filters << cropFilter;
    filters << QStringLiteral("scale=trunc(iw/2)*2:trunc(ih/2)*2");
    const QString videoFilter = filters.join(',');
    if (systemAudioIndex >= 0 && microphoneIndex >= 0) {
        const QString audioFilter = QStringLiteral(
                "[%1:a][%2:a]amerge=inputs=2,"
                "pan=stereo|c0=0.5*c0+0.5*c2|c1=0.5*c1+0.5*c3")
                .arg(systemAudioIndex)
                .arg(microphoneIndex);
        // Keep automatic stream mapping here. Explicit [vout]/[aout] maps make
        // FFmpeg wait for the unbounded Pulse inputs after rawvideo EOF.
        arguments << "-filter_complex" << audioFilter;
        arguments << "-vf" << videoFilter;
    } else {
        arguments << "-vf" << videoFilter;
        arguments << "-map" << QStringLiteral("%1:v:0").arg(videoIndex);
        if (systemAudioIndex >= 0)
            arguments << "-map" << QStringLiteral("%1:a:0").arg(systemAudioIndex);
        else if (microphoneIndex >= 0)
            arguments << "-map" << QStringLiteral("%1:a:0").arg(microphoneIndex);
    }

    if (audio.hasAudio()) {
        arguments << "-c:a" << "aac" << "-b:a" << "128k";
        // Pulse inputs are unbounded. Closing the rawvideo stdin must end the
        // mux instead of waiting for audio indefinitely.
        arguments << "-shortest";
    }

    arguments << "-c:v" << "libx264"
              << "-preset" << "ultrafast"
              << "-crf" << "23"
              << "-pix_fmt" << "yuv420p"
              << "-r" << QString::number(qMax(1, frameRate))
              << "-fps_mode" << "cfr"
              << "-y" << outputPath;
    return arguments;
}

bool FramePumpWorker::startFfmpeg()
{
    if (m_audio.hasSystemAudio() && m_audio.systemDevice.trimmed().isEmpty()) {
        fail(QStringLiteral("System audio device is unavailable"));
        return false;
    }

    m_ffmpeg = new QProcess(this);
    m_ffmpeg->setProcessChannelMode(QProcess::SeparateChannels);
    connect(m_ffmpeg, &QProcess::readyReadStandardError, this, [this]() {
        const QByteArray err = m_ffmpeg->readAllStandardError().trimmed();
        if (!err.isEmpty())
            qCWarning(dsrApp) << "FramePump ffmpeg stderr:" << err;
    });
    connect(m_ffmpeg, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus status) {
        qCWarning(dsrApp) << "FramePump ffmpeg finished, exit code:" << exitCode
                          << "status:" << status;
        if (!m_stopping) {
            QTimer::singleShot(0, this, [this, exitCode] {
                fail(QStringLiteral("FFmpeg exited unexpectedly (code %1)").arg(exitCode));
            });
        }
    });

    const QStringList arguments = buildFfmpegArguments(m_width, m_height, m_frameRate,
                                                       m_cropFilter, m_audio, m_outputPath);
    qCWarning(dsrApp) << "FramePump ffmpeg:" << arguments.join(' ');
    m_ffmpeg->start(QStringLiteral("ffmpeg"), arguments);
    if (!m_ffmpeg->waitForStarted(5000)) {
        fail(QStringLiteral("Failed to start ffmpeg: %1").arg(m_ffmpeg->errorString()));
        return false;
    }
    return true;
}

bool FramePumpWorker::writeCurrentFrame()
{
    if (!m_ffmpeg || m_ffmpeg->state() != QProcess::Running)
        return false;

    const CaptureFrameSnapshotPtr frame = m_store ? m_store->get() : CaptureFrameSnapshotPtr();
    if (!frame || frame->data.isEmpty())
        return false;

    if (frame->sequence != 0 && frame->sequence == m_lastFrameSequence)
        ++m_repeatedFrames;
    if (frame->sequence != 0)
        m_lastFrameSequence = frame->sequence;

    const qint64 expected = static_cast<qint64>(frame->stride) * frame->height;
    if (expected <= 0 || frame->data.size() < expected)
        return false;

    qint64 written = 0;
    const char *data = frame->data.constData();
    while (written < expected) {
        const qint64 n = m_ffmpeg->write(data + written, expected - written);
        if (n <= 0)
            return false;
        written += n;
    }

    if (!m_ffmpeg->waitForBytesWritten(2000))
        return false;

    ++m_outputFrames;
    emit frameWritten(m_outputFrames);
    return true;
}

void FramePumpWorker::fail(const QString &message)
{
    qCCritical(dsrApp) << "FramePumpWorker error:" << message;
    emit errorOccurred(message);
    stop();
}

void FramePumpWorker::stopFfmpeg()
{
    if (!m_ffmpeg)
        return;

    if (m_ffmpeg->state() != QProcess::NotRunning) {
        m_ffmpeg->closeWriteChannel();
        if (!m_ffmpeg->waitForFinished(5000)) {
            qCWarning(dsrApp) << "FramePump ffmpeg did not finish in 5s, killing";
            m_ffmpeg->kill();
            m_ffmpeg->waitForFinished(1000);
        }
        const QByteArray err = m_ffmpeg->readAllStandardError().trimmed();
        if (!err.isEmpty())
            qCWarning(dsrApp) << "FramePump ffmpeg final stderr:" << err;
        if (m_ffmpeg->exitStatus() != QProcess::NormalExit || m_ffmpeg->exitCode() != 0) {
            qCWarning(dsrApp) << "FramePump ffmpeg exit code:" << m_ffmpeg->exitCode()
                              << "status:" << m_ffmpeg->exitStatus();
        }
    }

    delete m_ffmpeg;
    m_ffmpeg = nullptr;
}
