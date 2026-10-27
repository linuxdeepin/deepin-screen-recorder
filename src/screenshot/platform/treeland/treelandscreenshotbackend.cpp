// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandscreenshotbackend.h"

#include "../../../utils/log.h"
#include "treelandsnapproxy.h"
#include "treelandstillcapture.h"

#include <QDateTime>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QtMath>

#include <limits>

namespace Dsr::Screenshot {

namespace {

constexpr int MaxBatchCaptureAttempts = 3;
constexpr quint64 DefaultFrameSyncToleranceNs = 20000000ULL;

OutputDescriptor descriptorForScreen(QScreen *screen)
{
    OutputDescriptor output;
    if (!screen)
        return output;

    output.outputId = screen->name();
    output.logicalGeometry = screen->geometry();
    output.devicePixelRatio = screen->devicePixelRatio();
    return output;
}

struct FrameSpread
{
    bool complete = false;
    quint64 nanoseconds = 0;
};

FrameSpread frameSpread(const QList<TreelandStillCapture::CapturedOutput> &outputs)
{
    FrameSpread spread;
    quint64 oldest = std::numeric_limits<quint64>::max();
    quint64 newest = 0;

    for (const auto &output : outputs) {
        if (output.timestampNs == 0)
            return spread;
        oldest = qMin(oldest, output.timestampNs);
        newest = qMax(newest, output.timestampNs);
    }

    spread.complete = !outputs.isEmpty();
    spread.nanoseconds = spread.complete ? newest - oldest : 0;
    return spread;
}

quint64 frameSyncTolerance(const QList<QScreen *> &screens)
{
    qreal lowestRefreshRate = 0;
    for (QScreen *screen : screens) {
        const qreal refreshRate = screen ? screen->refreshRate() : 0;
        if (refreshRate <= 0)
            continue;
        if (lowestRefreshRate <= 0 || refreshRate < lowestRefreshRate)
            lowestRefreshRate = refreshRate;
    }

    return lowestRefreshRate > 0
            ? static_cast<quint64>(qCeil(1000000000.0 / lowestRefreshRate))
            : DefaultFrameSyncToleranceNs;
}

} // namespace

TreelandScreenshotBackend::TreelandScreenshotBackend(TreelandSnapSession *snapSession,
                                                     QObject *parent)
    : IScreenshotBackend(parent)
    , m_snapSession(snapSession)
    , m_capture(new TreelandStillCapture(this))
{
    Q_ASSERT(m_snapSession);
}

BackendCapabilities TreelandScreenshotBackend::capabilities() const
{
    BackendCapabilities capabilities;
    capabilities.frozenFrame = true;
    capabilities.compositorSnap = true;
    capabilities.multiOutputMasks = true;
    capabilities.topWindowCapture = false;
    capabilities.cursorInFrame = false;
    return capabilities;
}

QList<OutputDescriptor> TreelandScreenshotBackend::outputs() const
{
    QList<OutputDescriptor> result;
    for (QScreen *screen : QGuiApplication::screens()) {
        const OutputDescriptor output = descriptorForScreen(screen);
        if (!output.outputId.isEmpty() && output.logicalGeometry.isValid())
            result.append(output);
    }
    return result;
}

void TreelandScreenshotBackend::prepare(const Request &request)
{
    m_cancelled = false;
    if (request.mode == Mode::TopWindow) {
        emit failed(QStringLiteral("Treeland 新截图后端暂不支持顶层窗口截图"));
        return;
    }
    if (request.includeCursor) {
        emit failed(QStringLiteral("Treeland 新截图后端暂不支持把光标写入冻结帧"));
        return;
    }

    QString error;
    if (!m_snapSession->waitUntilReady(3000, &error)) {
        emit failed(error);
        return;
    }

    const QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        emit failed(QStringLiteral("没有可用的输出屏幕"));
        return;
    }

    FrozenFrameSet frames;
    frames.generation = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
    QRect virtualDesktop;
    const quint64 syncToleranceNs = frameSyncTolerance(screens);
    QList<TreelandStillCapture::CapturedOutput> captured;
    FrameSpread bestSpread;
    int attempts = 0;

    for (int attempt = 0; attempt < MaxBatchCaptureAttempts; ++attempt) {
        QString attemptError;
        const QList<TreelandStillCapture::CapturedOutput> candidate =
                m_capture->capture(screens, &attemptError);
        if (m_cancelled)
            return;

        ++attempts;
        if (candidate.size() != screens.size()) {
            error = attemptError.isEmpty() ? QStringLiteral("多屏批量截图失败") : attemptError;
            break;
        }

        const FrameSpread spread = frameSpread(candidate);
        qCInfo(dsrApp) << "Treeland batch capture attempt:" << attempts
                       << "presentation spread(ms):"
                       << (spread.complete ? spread.nanoseconds / 1000000ULL : 0ULL)
                       << "complete:" << spread.complete;

        const bool betterCandidate = captured.isEmpty()
                || (!bestSpread.complete && spread.complete)
                || (bestSpread.complete && spread.complete
                    && spread.nanoseconds < bestSpread.nanoseconds);
        if (betterCandidate) {
            captured = candidate;
            bestSpread = spread;
        }

        if (!spread.complete || spread.nanoseconds <= syncToleranceNs
                || screens.size() < 2) {
            break;
        }
    }

    if (captured.size() != screens.size()) {
        emit failed(error.isEmpty() ? QStringLiteral("多屏批量截图失败") : error);
        return;
    }

    for (int i = 0; i < captured.size(); ++i) {
        const auto &capture = captured.at(i);
        const OutputDescriptor output = descriptorForScreen(capture.screen);
        if (!output.logicalGeometry.isValid()) {
            emit failed(QStringLiteral("输出 %1 的逻辑区域无效").arg(output.outputId));
            return;
        }

        if (capture.image.isNull()) {
            emit failed(QStringLiteral("抓取输出 %1 失败：%2")
                                .arg(output.outputId, error));
            return;
        }

        OutputFrame frame;
        frame.output = output;
        frame.image = capture.image;
        frame.pixelScale = output.logicalGeometry.width() > 0
                ? static_cast<qreal>(capture.image.width()) / output.logicalGeometry.width()
                : 1.0;
        frame.timestampNs = capture.timestampNs;
        frames.outputs.append(frame);
        virtualDesktop = virtualDesktop.united(output.logicalGeometry);
    }

    if (frames.outputs.isEmpty() || !virtualDesktop.isValid()) {
        emit failed(QStringLiteral("截图后端没有生成有效冻结帧"));
        return;
    }

    frames.virtualDesktopRect = virtualDesktop;
    qCInfo(dsrApp) << "Treeland frozen frame set prepared:"
                   << frames.outputs.size() << "outputs, desktop:" << virtualDesktop
                   << "attempts:" << attempts
                   << "best presentation spread(ms):"
                   << (bestSpread.complete ? bestSpread.nanoseconds / 1000000ULL : 0ULL)
                   << "sync tolerance(ms):" << syncToleranceNs / 1000000ULL;
    emit prepared(frames);
}

void TreelandScreenshotBackend::cancel()
{
    m_cancelled = true;
}

} // namespace Dsr::Screenshot
