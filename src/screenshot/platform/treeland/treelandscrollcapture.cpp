// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandscrollcapture.h"

#include "../../../utils/log.h"
#include "treelandoutputcapture.h"
#include "treelandsnapproxy.h"

#include <QEventLoop>
#include <QGuiApplication>
#include <QImage>
#include <QScreen>
#include <QTimer>

namespace Dsr::Screenshot {

namespace {

constexpr int CaptureIntervalMs = 150;

QRect selectionRectInImage(const QImage &image,
                           const QRect &windowRegion,
                           const QRect &selection)
{
    if (image.isNull() || windowRegion.isEmpty() || selection.isEmpty())
        return {};

    const QRect local = selection.translated(-windowRegion.topLeft());
    const qreal scaleX = static_cast<qreal>(image.width()) / windowRegion.width();
    const qreal scaleY = static_cast<qreal>(image.height()) / windowRegion.height();
    const int left = qRound(local.left() * scaleX);
    const int top = qRound(local.top() * scaleY);
    const int right = qRound(local.right() * scaleX);
    const int bottom = qRound(local.bottom() * scaleY);
    return QRect(QPoint(left, top), QPoint(right, bottom)).intersected(image.rect());
}

QImage cropSelection(const QImage &image,
                     const QRect &windowRegion,
                     const QRect &selection)
{
    const QRect crop = selectionRectInImage(image, windowRegion, selection);
    return crop.isEmpty() ? QImage() : image.copy(crop);
}

} // namespace

TreelandScrollCapture::TreelandScrollCapture(QObject *parent)
    : IScrollCapture(parent)
    , m_captureTimer(new QTimer(this))
    , m_outputCapture(new TreelandOutputCapture)
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
    delete m_outputCapture;
    m_outputCapture = nullptr;
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
    for (const OutputDescriptor &output : outputs) {
        if (output.logicalGeometry.isValid()
                && output.logicalGeometry.intersects(m_globalSelection)) {
            matchingOutputs.append(output);
        }
    }
    if (matchingOutputs.size() != 1) {
        if (error) {
            *error = matchingOutputs.isEmpty()
                    ? QStringLiteral("滚动截图选区没有对应的输出")
                    : QStringLiteral("TreeLand 窗口滚动截图仅支持单输出");
        }
        return false;
    }

    m_captureOutput = matchingOutputs.first();
    if (!m_captureOutput.logicalGeometry.contains(m_globalSelection)) {
        if (error)
            *error = QStringLiteral("滚动截图选区超出了目标输出范围");
        return false;
    }

    m_captureScreen = nullptr;
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen && screen->name() == m_captureOutput.outputId) {
            m_captureScreen = screen;
            break;
        }
    }
    if (!m_captureScreen) {
        if (error) {
            *error = QStringLiteral("找不到滚动截图输出 %1")
                             .arg(m_captureOutput.outputId);
        }
        return false;
    }

    qreal outputScale = m_captureOutput.devicePixelRatio;
    if (outputScale <= 0 && m_captureScreen)
        outputScale = m_captureScreen->devicePixelRatio();
    if (outputScale <= 0)
        outputScale = 1.0;

    if (!m_outputCapture->start(m_captureOutput.outputId,
                                m_captureOutput.logicalGeometry,
                                error)) {
        return false;
    }

    qCInfo(dsrApp) << "TreeLand scroll capture started: output" << m_captureOutput.outputId
                   << "output geometry" << m_captureOutput.logicalGeometry
                   << "window" << m_targetWindowRegion
                   << "selection" << m_globalSelection
                   << "pid" << m_targetProcessId
                   << "process" << m_targetProcessName
                   << "scale" << outputScale;
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
    frame.output = m_captureOutput;
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
    if (m_outputCapture)
        m_outputCapture->stop();
}

bool TreelandScrollCapture::captureFrame(QImage *image,
                                         quint64 *timestampNs,
                                         QString *error)
{
    if (!image || !timestampNs || !error)
        return false;
    *image = QImage();
    *timestampNs = 0;

    if (!m_outputCapture) {
        *error = QStringLiteral("TreeLand 输出捕获器未初始化");
        return false;
    }

    QImage outputImage;
    quint64 outputTimestamp = 0;
    if (!m_outputCapture->capture(&outputImage, &outputTimestamp, error))
        return false;

    const QImage selectionImage = cropSelection(outputImage,
                                                m_captureOutput.logicalGeometry,
                                                m_globalSelection);
    if (selectionImage.isNull()) {
        *error = QStringLiteral("TreeLand 输出截图裁剪失败");
        return false;
    }

    *image = selectionImage;
    *timestampNs = outputTimestamp;
    return true;
}

} // namespace Dsr::Screenshot
