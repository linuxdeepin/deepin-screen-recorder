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

namespace Dsr::Screenshot {

namespace {

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
    const quint64 timestampNs = frames.generation * 1000000ULL;

    for (QScreen *screen : screens) {
        const OutputDescriptor output = descriptorForScreen(screen);
        if (!output.logicalGeometry.isValid()) {
            emit failed(QStringLiteral("输出 %1 的逻辑区域无效").arg(output.outputId));
            return;
        }

        const QImage image = m_capture->capture(screen, &error);
        if (m_cancelled)
            return;
        if (image.isNull()) {
            emit failed(QStringLiteral("抓取输出 %1 失败：%2")
                                .arg(output.outputId, error));
            return;
        }

        OutputFrame frame;
        frame.output = output;
        frame.image = image;
        frame.pixelScale = output.logicalGeometry.width() > 0
                ? static_cast<qreal>(image.width()) / output.logicalGeometry.width()
                : 1.0;
        frame.timestampNs = timestampNs;
        frames.outputs.append(frame);
        virtualDesktop = virtualDesktop.united(output.logicalGeometry);
    }

    if (frames.outputs.isEmpty() || !virtualDesktop.isValid()) {
        emit failed(QStringLiteral("截图后端没有生成有效冻结帧"));
        return;
    }

    frames.virtualDesktopRect = virtualDesktop;
    qCInfo(dsrApp) << "Treeland frozen frame set prepared:"
                   << frames.outputs.size() << "outputs, desktop:" << virtualDesktop;
    emit prepared(frames);
}

void TreelandScreenshotBackend::cancel()
{
    m_cancelled = true;
}

} // namespace Dsr::Screenshot
