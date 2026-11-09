// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandscrollcapture.h"

#include "../../../utils/log.h"
#include "treelandoutputcapture.h"
#include "treelandscrollcompositor.h"
#include "treelandsnapproxy.h"

#include <QEventLoop>
#include <QGuiApplication>
#include <QImage>
#include <QRegion>
#include <QScreen>
#include <QTimer>

namespace Dsr::Screenshot {

namespace {

constexpr int CaptureIntervalMs = 150;

qreal screenDprForOutput(const QString &outputId)
{
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen && screen->name() == outputId)
            return screen->devicePixelRatio();
    }
    return 0.0;
}

} // namespace

TreelandScrollCapture::TreelandScrollCapture(QObject *parent)
    : IScrollCapture(parent)
    , m_captureTimer(new QTimer(this))
{
    m_captureTimer->setInterval(CaptureIntervalMs);
    connect(m_captureTimer, &QTimer::timeout,
            this, &TreelandScrollCapture::requestFrame);
}

void TreelandScrollCapture::setSnapSession(TreelandSnapSession *snapSession)
{
    m_snapSession = snapSession;
}

void TreelandScrollCapture::setFrozenFrames(const FrozenFrameSet &frames)
{
    Q_UNUSED(frames)
}

TreelandScrollCapture::~TreelandScrollCapture()
{
    stop();
    qDeleteAll(m_outputCaptures);
    m_outputCaptures.clear();
}

bool TreelandScrollCapture::start(const QList<OutputDescriptor> &outputs,
                                  const QRect &globalSelection,
                                  QString *error)
{
    stop();
    m_stopping = false;
    m_started = false;
    m_frameInFlight = false;
    m_consecutiveFailures = 0;
    qDeleteAll(m_outputCaptures);
    m_outputCaptures.clear();
    m_captureOutputs.clear();
    m_globalSelection = globalSelection.normalized();
    m_targetWindowRegion = m_snapSession
            ? m_snapSession->region().normalized()
            : QRect();
    m_targetProcessName = m_snapSession
            ? m_snapSession->processName().trimmed()
            : QString();
    m_targetProcessId = m_snapSession ? m_snapSession->processId() : -1;

    if (m_globalSelection.isEmpty()) {
        if (error)
            *error = QStringLiteral("滚动截图选区为空");
        return false;
    }
    if (m_targetWindowRegion.isEmpty()
            || (m_targetProcessName.isEmpty() && m_targetProcessId <= 0)) {
        if (error)
            *error = QStringLiteral("TreeLand 滚动截图需要选区位于一个应用窗口内");
        return false;
    }
    const QRect tolerantWindowRegion = m_targetWindowRegion.adjusted(-1, -1, 1, 1);
    if (!tolerantWindowRegion.contains(m_globalSelection)) {
        if (error) {
            *error = QStringLiteral(
                    "滚动截图选区必须完整位于目标窗口内，不能跨窗口或包含桌面");
        }
        return false;
    }

    QList<OutputDescriptor> matchingOutputs;
    QRegion coveredOutputs;
    for (const OutputDescriptor &output : outputs) {
        if (!output.logicalGeometry.isValid()
                || !output.logicalGeometry.intersects(m_globalSelection)) {
            continue;
        }
        OutputDescriptor resolved = output;
        if (resolved.devicePixelRatio <= 0)
            resolved.devicePixelRatio = screenDprForOutput(resolved.outputId);
        if (resolved.devicePixelRatio <= 0)
            resolved.devicePixelRatio = 1.0;
        matchingOutputs.append(resolved);
        coveredOutputs += resolved.logicalGeometry;
    }
    if (matchingOutputs.isEmpty()) {
        if (error)
            *error = QStringLiteral("滚动截图选区没有对应的输出");
        return false;
    }
    // A cross-output selection must be fully covered by the captured outputs,
    // otherwise the composed selection canvas would contain uncovered edges.
    if (!coveredOutputs.contains(m_globalSelection)) {
        if (error)
            *error = QStringLiteral("滚动截图选区超出了可用输出范围");
        return false;
    }

    for (const OutputDescriptor &output : matchingOutputs) {
        auto *capture = new TreelandOutputCapture;
        QString captureError;
        if (!capture->start(output.outputId, output.logicalGeometry, &captureError)) {
            delete capture;
            qDeleteAll(m_outputCaptures);
            m_outputCaptures.clear();
            if (error) {
                *error = QStringLiteral("输出 %1 启动截图失败：%2")
                                 .arg(output.outputId, captureError);
            }
            return false;
        }
        m_outputCaptures.append(capture);
    }
    m_captureOutputs = matchingOutputs;

    qCInfo(dsrApp) << "TreeLand scroll capture started: outputs" << m_captureOutputs.size()
                   << "window" << m_targetWindowRegion
                   << "selection" << m_globalSelection
                   << "pid" << m_targetProcessId
                   << "process" << m_targetProcessName;
    for (const OutputDescriptor &output : qAsConst(m_captureOutputs)) {
        qCInfo(dsrApp) << "TreeLand scroll capture output" << output.outputId
                       << output.logicalGeometry << "scale" << output.devicePixelRatio;
    }
    m_started = true;
    m_captureTimer->start();
    QTimer::singleShot(0, this, &TreelandScrollCapture::requestFrame);
    return true;
}

void TreelandScrollCapture::requestFrame()
{
    if (m_stopping || !m_started || m_frameInFlight)
        return;

    m_frameInFlight = true;
    emit captureAboutToStart();
    QEventLoop guardLoop;
    QTimer::singleShot(8, &guardLoop, &QEventLoop::quit);
    guardLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QImage image;
    quint64 timestampNs = 0;
    QString error;
    if (!captureFrame(&image, &timestampNs, &error)) {
        m_frameInFlight = false;
        emit captureFinished();
        if (m_stopping)
            return;
        ++m_consecutiveFailures;
        qCWarning(dsrApp) << "TreeLand scroll frame capture failed ("
                          << m_consecutiveFailures << "consecutive):" << error;
        // TreelandOutputCapture uses a fresh protocol session for every frame,
        // so the next timer tick recovers by itself. The controller decides
        // when repeated misses become user-visible.
        emit frameFailed(error);
        return;
    }
    m_consecutiveFailures = 0;
    m_frameInFlight = false;
    emit captureFinished();
    if (m_stopping || image.isNull())
        return;

    ScrollFrame frame;
    // The image is already composed from every intersecting output; keep the
    // first output as metadata for consumers that still expect one descriptor.
    frame.output = m_captureOutputs.first();
    frame.globalSelection = m_globalSelection;
    frame.image = image;
    frame.timestampNs = timestampNs;
    emit frameReady(frame);
}

void TreelandScrollCapture::pause()
{
    if (m_captureTimer)
        m_captureTimer->stop();
}

void TreelandScrollCapture::resume()
{
    if (m_stopping || !m_started || !m_captureTimer)
        return;
    m_captureTimer->start();
    QTimer::singleShot(0, this, &TreelandScrollCapture::requestFrame);
}

void TreelandScrollCapture::stop()
{
    m_stopping = true;
    m_started = false;
    m_frameInFlight = false;
    m_consecutiveFailures = 0;
    if (m_captureTimer)
        m_captureTimer->stop();
    for (TreelandOutputCapture *capture : qAsConst(m_outputCaptures)) {
        if (capture)
            capture->stop();
    }
}

bool TreelandScrollCapture::captureFrame(QImage *image,
                                         quint64 *timestampNs,
                                         QString *error)
{
    if (!image || !timestampNs || !error)
        return false;
    *image = QImage();
    *timestampNs = 0;

    if (m_outputCaptures.isEmpty()
            || m_outputCaptures.size() != m_captureOutputs.size()) {
        *error = QStringLiteral("TreeLand 输出捕获器未初始化");
        return false;
    }

    QList<QImage> images;
    images.reserve(m_outputCaptures.size());
    quint64 latestTimestamp = 0;
    for (int i = 0; i < m_outputCaptures.size(); ++i) {
        QImage frameImage;
        quint64 frameTimestamp = 0;
        QString frameError;
        if (!m_outputCaptures[i]->capture(&frameImage, &frameTimestamp, &frameError)) {
            *error = QStringLiteral("输出 %1 抓帧失败：%2")
                             .arg(m_captureOutputs[i].outputId, frameError);
            return false;
        }
        if (frameImage.isNull()) {
            *error = QStringLiteral("输出 %1 返回了空帧")
                             .arg(m_captureOutputs[i].outputId);
            return false;
        }
        images.append(frameImage);
        latestTimestamp = qMax(latestTimestamp, frameTimestamp);
    }

    const QImage composed = composeScrollSelectionFrame(m_captureOutputs,
                                                        images,
                                                        m_globalSelection);
    if (composed.isNull()) {
        *error = QStringLiteral("滚动截图跨屏合成失败");
        return false;
    }

    *image = composed;
    *timestampNs = latestTimestamp;
    return true;
}

} // namespace Dsr::Screenshot
