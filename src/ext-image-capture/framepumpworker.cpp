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
                                 QObject *parent)
    : QObject(parent)
    , m_frameRate(qMax(1, frameRate))
    , m_outputPath(outputPath)
    , m_cropFilter(cropFilter)
    , m_store(store)
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

void FramePumpWorker::start(int width, int height, int stride, qint64 startOffsetNs)
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
    m_nextDeadlineNs = 0;
    m_lastStatsNs = 0;
    const qint64 offsetNs = qMax<qint64>(0, startOffsetNs);
    const int backlogFrames = qBound(1,
            static_cast<int>((offsetNs + m_intervalNs / 2) / m_intervalNs),
            m_frameRate * 2);
    for (int i = 0; i < backlogFrames; ++i) {
        if (!writeCurrentFrame()) {
            fail(QStringLiteral("Failed to write first frame to encoder"));
            return;
        }
    }
    // 已写入的 backlog 帧代表录制开始到首帧就绪之间的时间；后续节拍仍从
    // 当前时刻继续，因此总帧数才能覆盖完整的用户录制时长。
    m_nextDeadlineNs = 0;

    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->setInterval(qBound(1, 1000 / (m_frameRate * 4), 10));
    connect(m_timer, &QTimer::timeout, this, &FramePumpWorker::onTick);
    m_timer->start();

    m_started = true;
    qCWarning(dsrApp) << "FramePumpWorker started:"
                      << m_width << "x" << m_height
                      << "fps:" << m_frameRate
                      << "backlog frames:" << backlogFrames
                      << "output:" << m_outputPath;
    emit started();
}

void FramePumpWorker::stop()
{
    if (m_stopping)
        return;
    m_stopping = true;

    if (m_timer) {
        m_timer->stop();
    }
    stopFfmpeg();

    qCWarning(dsrApp) << "FramePumpWorker stopped, output frames:" << m_outputFrames
                      << "repeated frames:" << m_repeatedFrames;
    emit finished();
}

void FramePumpWorker::onTick()
{
    if (m_stopping || !m_started)
        return;

    const qint64 now = m_clock.nsecsElapsed();
    int emitted = 0;
    while (now >= m_nextDeadlineNs && emitted < 2) {
        if (!writeCurrentFrame()) {
            fail(QStringLiteral("Failed to write frame to encoder"));
            return;
        }
        m_nextDeadlineNs += m_intervalNs;
        ++emitted;
    }

    if (emitted == 2 && now >= m_nextDeadlineNs) {
        qCWarning(dsrApp) << "FramePumpWorker is behind by"
                          << (now - m_nextDeadlineNs) / 1000000.0 << "ms";
    }

    if (now - m_lastStatsNs >= 1000000000LL) {
        m_lastStatsNs = now;
        const qint64 behindMs = qMax<qint64>(0, (now - m_nextDeadlineNs) / 1000000LL);
        emit statsUpdated(m_outputFrames, m_repeatedFrames, behindMs);
    }
}

bool FramePumpWorker::startFfmpeg()
{
    m_ffmpeg = new QProcess(this);
    m_ffmpeg->setProcessChannelMode(QProcess::SeparateChannels);
    connect(m_ffmpeg, &QProcess::readyReadStandardError, this, [this]() {
        const QByteArray err = m_ffmpeg->readAllStandardError().trimmed();
        if (!err.isEmpty())
            qCWarning(dsrApp) << "FramePump ffmpeg stderr:" << err;
    });
    connect(m_ffmpeg, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart)
            qCWarning(dsrApp) << "FramePump ffmpeg process error:" << error;
    });

    QStringList arguments;
    arguments << "-hide_banner"
              << "-loglevel" << "warning"
              << "-f" << "rawvideo"
              << "-pix_fmt" << "rgba"
              << "-s" << QStringLiteral("%1x%2").arg(m_width).arg(m_height)
              << "-r" << QString::number(m_frameRate)
              << "-i" << "-";
    QStringList filters;
    if (!m_cropFilter.isEmpty())
        filters << m_cropFilter;
    filters << QStringLiteral("scale=trunc(iw/2)*2:trunc(ih/2)*2");
    arguments << "-vf" << filters.join(',');
    arguments << "-c:v" << "libx264"
              << "-preset" << "ultrafast"
              << "-crf" << "23"
              << "-pix_fmt" << "yuv420p"
              << "-vsync" << "passthrough"
              << "-y" << m_outputPath;

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
