// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshotsessioncontroller.h"

#include "annotationcoordinator.h"
#include "../export/screenshotexportservice.h"
#include "../ports/iannotationeditor.h"
#include "../ports/iscreenshotbackend.h"
#include "../ports/iscreenshotoverlayhost.h"
#include "../ports/iscreenshotresultsink.h"
#include "../ports/isnaphintsource.h"
#include "../ports/iwindowtagger.h"
#include "../../utils/log.h"

#include <QDebug>
#include <QWindow>

namespace Dsr::Screenshot {

ScreenshotSessionController::ScreenshotSessionController(IScreenshotBackend *backend,
                                                         IScreenshotOverlayHost *overlay,
                                                         ISnapHintSource *snapHints,
                                                         IWindowTagger *windowTagger,
                                                         IScreenshotResultSink *resultSink,
                                                         IAnnotationEditor *annotationEditor,
                                                         QObject *parent)
    : QObject(parent)
    , m_backend(backend)
    , m_overlay(overlay)
    , m_snapHints(snapHints)
    , m_windowTagger(windowTagger)
    , m_resultSink(resultSink)
    , m_annotationEditor(annotationEditor)
    , m_selection(new SelectionModel(this))
{
    Q_ASSERT(m_backend);
    Q_ASSERT(m_overlay);
    Q_ASSERT(m_resultSink);

    m_backend->setParent(this);
    m_overlay->setParent(this);
    m_resultSink->setParent(this);
    if (m_snapHints)
        m_snapHints->setParent(this);
    if (m_windowTagger)
        m_windowTagger->setParent(this);
    if (m_annotationEditor)
        m_annotationEditor->setParent(this);
    m_annotationCoordinator = new AnnotationCoordinator(m_selection,
                                                        m_annotationEditor,
                                                        this);

    connect(m_backend, &IScreenshotBackend::prepared,
            this, &ScreenshotSessionController::onBackendPrepared);
    connect(m_backend, &IScreenshotBackend::failed,
            this, &ScreenshotSessionController::onBackendFailed);
    connect(m_overlay, &IScreenshotOverlayHost::inputReady,
            this, &ScreenshotSessionController::onOverlayReady);
    connect(m_overlay, &IScreenshotOverlayHost::pointerPressed,
            this, &ScreenshotSessionController::onPointerPressed);
    connect(m_overlay, &IScreenshotOverlayHost::pointerMoved,
            this, &ScreenshotSessionController::onPointerMoved);
    connect(m_overlay, &IScreenshotOverlayHost::pointerReleased,
            this, &ScreenshotSessionController::onPointerReleased);
    connect(m_overlay, &IScreenshotOverlayHost::confirmRequested,
            this, &ScreenshotSessionController::onConfirmRequested);
    connect(m_overlay, &IScreenshotOverlayHost::copyRequested,
            this, &ScreenshotSessionController::onCopyRequested);
    connect(m_overlay, &IScreenshotOverlayHost::cancelRequested,
            this, &ScreenshotSessionController::onCancelRequested);
    if (m_snapHints) {
        connect(m_snapHints, &ISnapHintSource::regionChanged,
                this, &ScreenshotSessionController::onSnapRegionChanged);
    }
    connect(m_selection, &SelectionModel::geometryChanged,
            this, [this](const QRect &, const QRect &) { syncOverlaySelection(); });
    connect(m_selection, &SelectionModel::stateChanged,
            this, [this](InteractionState) { syncOverlaySelection(); });
}

ScreenshotSessionController::~ScreenshotSessionController() = default;

bool ScreenshotSessionController::start(const Request &request)
{
    if (m_state != State::Idle && m_state != State::Completed
            && m_state != State::Cancelled && m_state != State::Failed) {
        return false;
    }

    m_request = request;
    m_frames = FrozenFrameSet();
    m_snapHint = QRect();
    m_pressed = false;
    m_dragged = false;
    m_pressStartedEmpty = false;
    m_snapStarted = false;
    m_finished = false;
    m_selection->clear();
    m_state = State::PreparingFrame;
    m_backend->prepare(m_request);
    return true;
}

void ScreenshotSessionController::cancel()
{
    onCancelRequested();
}

ScreenshotSessionController::State ScreenshotSessionController::state() const
{
    return m_state;
}

void ScreenshotSessionController::onBackendPrepared(const FrozenFrameSet &frames)
{
    if (m_state != State::PreparingFrame)
        return;
    if (frames.isEmpty() || !frames.virtualDesktopRect.isValid()) {
        fail(QStringLiteral("截图后端没有返回有效冻结帧"));
        return;
    }

    m_frames = frames;
    m_selection->setDesktopBounds(frames.virtualDesktopRect);
    m_annotationCoordinator->syncBounds();
    if (m_request.mode == Mode::FullScreen)
        m_selection->setGeometry(frames.virtualDesktopRect);
    else if (!m_request.initialRegion.isEmpty())
        m_selection->setGeometry(m_request.initialRegion);

    QList<OutputDescriptor> outputs;
    outputs.reserve(frames.outputs.size());
    for (const OutputFrame &frame : frames.outputs)
        outputs.append(frame.output);

    m_state = State::CreatingMasks;
    if (!m_overlay->create(outputs)) {
        fail(QStringLiteral("创建截图覆盖窗口失败"));
        return;
    }
    m_overlay->setFrozenFrames(m_frames);
    m_overlay->show();
}

void ScreenshotSessionController::onBackendFailed(const QString &reason)
{
    fail(reason);
}

void ScreenshotSessionController::onOverlayReady()
{
    if (m_state != State::CreatingMasks)
        return;

    m_state = State::Selecting;
    if (m_request.enableWindowSnap)
        startSnapHints();
}

void ScreenshotSessionController::onPointerPressed(const QPoint &globalPos)
{
    if (m_state != State::Selecting && m_state != State::Editing)
        return;

    m_pressed = true;
    m_dragged = false;
    m_pressPos = globalPos;
    m_pressStartedEmpty = m_selection->geometry().isEmpty();
    m_overlay->setSnapHint(QRect());
    m_selection->beginInteraction(globalPos);
    syncOverlayCursor(&globalPos);
}

void ScreenshotSessionController::onPointerMoved(const QPoint &globalPos)
{
    if (!m_pressed) {
        syncOverlayCursor(&globalPos);
        return;
    }

    if ((globalPos - m_pressPos).manhattanLength() >= 3)
        m_dragged = true;
    m_selection->updateInteraction(globalPos);
    syncOverlayCursor(&globalPos);
}

void ScreenshotSessionController::onPointerReleased(const QPoint &globalPos)
{
    if (!m_pressed)
        return;

    m_pressed = false;
    if (!m_dragged && m_pressStartedEmpty && !m_snapHint.isEmpty())
        m_selection->setGeometry(m_snapHint);
    else
        m_selection->endInteraction();

    if (m_state == State::Selecting && m_selection->state() == InteractionState::Ready) {
        stopSnapHints();
        m_state = State::Editing;
        m_snapHint = QRect();
        m_overlay->setSnapHint(QRect());
    } else if (m_state == State::Selecting) {
        m_overlay->setSnapHint(m_snapHint);
    }
    syncOverlaySelection();
    syncOverlayCursor(&globalPos);
}

void ScreenshotSessionController::onSnapRegionChanged(const QRect &globalRegion)
{
    if (m_state != State::Selecting || m_pressed || m_selection->state() == InteractionState::Ready)
        return;

    m_snapHint = globalRegion.normalized().intersected(m_frames.virtualDesktopRect);
    m_overlay->setSnapHint(m_snapHint);
}

void ScreenshotSessionController::onConfirmRequested()
{
    if (m_finished || m_selection->geometry().isEmpty())
        return;
    if (m_state != State::Selecting && m_state != State::Editing)
        return;

    Result result;
    const State previousState = m_state;
    m_state = State::Exporting;
    if (!renderResult(&result)) {
        qWarning() << "Screenshot export returned an empty result";
        m_state = previousState;
        return;
    }

    QString path;
    QString error;
    if (!m_resultSink->save(result, m_request.outputPath, &path, &error)) {
        fail(error);
        return;
    }

    m_state = State::Completed;
    qCInfo(dsrApp) << "Screenshot session saved:" << path
                   << "selection:" << result.globalSelection
                   << "image:" << result.image.size()
                   << "generation:" << result.frameGeneration;
    emit saved(path);
    finish(State::Completed);
}

void ScreenshotSessionController::onCopyRequested()
{
    if (m_finished || m_selection->geometry().isEmpty())
        return;
    if (m_state != State::Selecting && m_state != State::Editing)
        return;

    Result result;
    const State previousState = m_state;
    m_state = State::Exporting;
    if (!renderResult(&result)) {
        qWarning() << "Screenshot copy returned an empty result";
        m_state = previousState;
        return;
    }

    QString error;
    if (!m_resultSink->copy(result, &error)) {
        fail(error);
        return;
    }
    m_state = State::Completed;
    qCInfo(dsrApp) << "Screenshot session copied:" << result.image.size()
                   << "generation:" << result.frameGeneration;
    emit copied();
    finish(State::Completed);
}

void ScreenshotSessionController::onCancelRequested()
{
    if (m_finished)
        return;

    m_backend->cancel();
    finish(State::Cancelled);
}

void ScreenshotSessionController::syncOverlaySelection()
{
    if (!m_overlay)
        return;
    m_overlay->setSelection(m_selection->geometry());
    m_overlay->setSelectionFinalized(m_selection->state() == InteractionState::Ready);
}

void ScreenshotSessionController::syncOverlayCursor(const QPoint *globalPos)
{
    if (!m_overlay)
        return;

    SelectionAction action = m_selection->action();
    if (action == SelectionAction::NoAction && globalPos)
        action = m_selection->hitTest(*globalPos);
    m_overlay->setCursorIntent(cursorIntentFor(action));
}

void ScreenshotSessionController::startSnapHints()
{
    if (m_snapStarted || !m_snapHints)
        return;

    const QList<QWindow *> windows = m_overlay->maskWindows();
    for (QWindow *window : windows) {
        QString error;
        if (m_windowTagger && !m_windowTagger->tagMaskWindow(window, &error)) {
            qWarning() << "Screenshot mask tagging failed; free selection remains active:" << error;
            return;
        }
    }

    QString error;
    if (!m_snapHints->start(&error)) {
        qWarning() << "Screenshot snap hints failed; free selection remains active:" << error;
        return;
    }
    m_snapStarted = true;
}

void ScreenshotSessionController::stopSnapHints()
{
    if (m_snapStarted) {
        m_snapHints->stop();
        m_snapStarted = false;
    }
}

void ScreenshotSessionController::fail(const QString &reason)
{
    if (m_finished)
        return;

    qCritical() << "Screenshot session failed:" << reason;
    m_backend->cancel();
    emit failed(reason);
    finish(State::Failed);
}

void ScreenshotSessionController::finish(State state)
{
    if (m_finished)
        return;
    stopSnapHints();
    if (m_overlay)
        m_overlay->close();
    m_finished = true;
    m_state = state;
    emit finished();
}

bool ScreenshotSessionController::renderResult(Result *result) const
{
    if (!result)
        return false;
    ScreenshotExportService service;
    *result = service.render(m_frames, m_selection->geometry(), m_annotationEditor);
    return result->isValid();
}

} // namespace Dsr::Screenshot
