// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshotsessioncontroller.h"

#include "annotationcoordinator.h"
#include "../core/selectionoverlaygeometry.h"
#include "../export/screenshotexportservice.h"
#include "../ports/iannotationeditor.h"
#include "../ports/irecordingsession.h"
#include "../ports/iscreenshotbackend.h"
#include "../ports/iscreenshotoverlayhost.h"
#include "../ports/iscreenshotresultsink.h"
#include "../ports/isnaphintsource.h"
#include "../ports/iwindowtagger.h"
#include "../../utils/log.h"
#include "../../utils/configsettings.h"
#include "../../utils.h"
#include "../../recordertablet.h"

#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QDebug>
#include <QWindow>

namespace Dsr::Screenshot {

namespace {

constexpr int MinimumSelectionSize = 10;

bool isSelectionResize(SelectionAction action)
{
    return action != SelectionAction::NoAction
            && action != SelectionAction::CreateFreeform
            && action != SelectionAction::Move;
}

CursorIntent cursorIntentWhilePressed(CursorIntent intent)
{
    return intent == CursorIntent::Move ? CursorIntent::Grabbing : intent;
}

QRect clampedSelectionMove(const QRect &selection,
                           const QPoint &delta,
                           const QRect &desktopBounds,
                           const QRect &annotationBounds)
{
    QRect moved = selection.translated(delta);
    if (desktopBounds.isValid()) {
        if (moved.left() < desktopBounds.left())
            moved.moveLeft(desktopBounds.left());
        if (moved.top() < desktopBounds.top())
            moved.moveTop(desktopBounds.top());
        if (moved.right() > desktopBounds.right())
            moved.moveRight(desktopBounds.right());
        if (moved.bottom() > desktopBounds.bottom())
            moved.moveBottom(desktopBounds.bottom());
    }
    if (!annotationBounds.isEmpty()) {
        if (moved.left() > annotationBounds.left())
            moved.moveLeft(annotationBounds.left());
        if (moved.top() > annotationBounds.top())
            moved.moveTop(annotationBounds.top());
        if (moved.right() < annotationBounds.right())
            moved.moveRight(annotationBounds.right());
        if (moved.bottom() < annotationBounds.bottom())
            moved.moveBottom(annotationBounds.bottom());
    }
    return moved;
}

QList<QScreen *> recordingScreensForSelection(const QRect &selection,
                                               const QList<OutputFrame> &outputs)
{
    QList<QScreen *> result;
    const QRect normalized = selection.normalized();
    for (const OutputFrame &output : outputs) {
        if (output.output.logicalGeometry.intersected(normalized).isEmpty())
            continue;

        QScreen *matched = nullptr;
        for (QScreen *screen : QGuiApplication::screens()) {
            if (screen->name() == output.output.outputId
                    || screen->geometry() == output.output.logicalGeometry) {
                matched = screen;
                break;
            }
        }
        if (matched && !result.contains(matched))
            result.append(matched);
    }
    return result;
}

} // namespace

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
    if (m_annotationEditor && !m_annotationEditor->parent())
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
    connect(m_overlay, &IScreenshotOverlayHost::inputFailed,
            this, &ScreenshotSessionController::onOverlayFailed);
    connect(m_overlay, &IScreenshotOverlayHost::pointerPressed,
            this, &ScreenshotSessionController::onPointerPressed);
    connect(m_overlay, &IScreenshotOverlayHost::pointerMoved,
            this, &ScreenshotSessionController::onPointerMoved);
    connect(m_overlay, &IScreenshotOverlayHost::pointerReleased,
            this, &ScreenshotSessionController::onPointerReleased);
    connect(m_overlay, &IScreenshotOverlayHost::selectionMoveRequested,
            this, &ScreenshotSessionController::onSelectionMoveRequested);
    connect(m_overlay, &IScreenshotOverlayHost::selectionResizeRequested,
            this, &ScreenshotSessionController::onSelectionResizeRequested);
    connect(m_overlay, &IScreenshotOverlayHost::selectionSizeChangeRequested,
            this, &ScreenshotSessionController::onSelectionSizeChangeRequested);
    connect(m_overlay, &IScreenshotOverlayHost::roundedCornerRadiusChanged,
            this, &ScreenshotSessionController::onRoundedCornerRadiusChanged);
    connect(m_overlay,
            &IScreenshotOverlayHost::maskWindowRoleCreated,
            this,
            &ScreenshotSessionController::onMaskWindowRoleCreated,
            Qt::DirectConnection);
    connect(m_overlay, &IScreenshotOverlayHost::confirmRequested,
            this, &ScreenshotSessionController::onConfirmRequested);
    connect(m_overlay, &IScreenshotOverlayHost::copyRequested,
            this, &ScreenshotSessionController::onCopyRequested);
    connect(m_overlay, &IScreenshotOverlayHost::pinRequested,
            this, &ScreenshotSessionController::onPinRequested);
    connect(m_overlay, &IScreenshotOverlayHost::cancelRequested,
            this, &ScreenshotSessionController::onCancelRequested);
    connect(m_overlay, &IScreenshotOverlayHost::recordModeChanged,
            this, &ScreenshotSessionController::onRecordModeChanged);
    connect(m_overlay, &IScreenshotOverlayHost::recordStartRequested,
            this, &ScreenshotSessionController::onRecordStartRequested);
    connect(m_overlay, &IScreenshotOverlayHost::recordStopRequested,
            this, &ScreenshotSessionController::onRecordStopRequested);
    connect(m_overlay, &IScreenshotOverlayHost::recordingCountdownFinished,
            this, &ScreenshotSessionController::startRecordingAfterCountdown);
    if (m_snapHints) {
        connect(m_snapHints, &ISnapHintSource::regionChanged,
                this, &ScreenshotSessionController::onSnapRegionChanged);
    }
    connect(m_selection, &SelectionModel::geometryChanged,
            this, [this](const QRect &, const QRect &) { syncOverlaySelection(); });
    connect(m_selection, &SelectionModel::stateChanged,
            this, [this](InteractionState) { syncOverlaySelection(); });

}

ScreenshotSessionController::~ScreenshotSessionController()
{
    if (m_recordingSession
            && (m_recordingSession->state() == IRecordingSession::State::Starting
                || m_recordingSession->state() == IRecordingSession::State::Recording)) {
        m_recordingSession->cancel();
    }
}

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
    m_globalCursor = QPoint(-1, -1);
    m_snapTagsPrepared = false;
    m_snapStarted = false;
    m_expectedSnapMaskCount = 0;
    m_taggedSnapMaskCount = 0;
    m_snapMaskTagError.clear();
    m_finished = false;
    m_inputTarget = InputTarget::Inactive;
    m_activeCursorIntent = CursorIntent::Cross;
    m_recordMode = false;
    m_roundedCornerRadius = 0;
    stopRecordingCountdown();
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

bool ScreenshotSessionController::isRecording() const
{
    return m_recordingStateActive;
}

void ScreenshotSessionController::setRecordingSession(IRecordingSession *recordingSession)
{
    if (m_recordingSession == recordingSession)
        return;
    if (m_recordingSession)
        m_recordingSession->deleteLater();
    m_recordingSession = recordingSession;
    if (!m_recordingSession)
        return;
    m_recordingSession->setParent(this);
    connect(m_recordingSession, &IRecordingSession::started,
            this, &ScreenshotSessionController::onRecordingStarted);
    connect(m_recordingSession, &IRecordingSession::saved,
            this, &ScreenshotSessionController::onRecordingSaved);
    connect(m_recordingSession, &IRecordingSession::failed,
            this, &ScreenshotSessionController::onRecordingFailed);
    connect(m_recordingSession, &IRecordingSession::cancelled,
            this, &ScreenshotSessionController::onRecordingCancelled);
}

void ScreenshotSessionController::stopRecord()
{
    onRecordStopRequested();
}

void ScreenshotSessionController::stopApp()
{
    cancel();
}

QString ScreenshotSessionController::getRecorderNormalIcon()
{
    return RecorderTablet::getRecorderNormalIcon();
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
    if (IAnnotationEditor *overlayEditor = m_overlay->annotationEditor()) {
        m_annotationEditor = overlayEditor;
        m_annotationCoordinator->setEditor(overlayEditor);
    }
    // Selection is prepared before the overlay exists. Push it again once the
    // platform windows have been created so initial-region and full-screen
    // sessions do not lose their initial geometry.
    syncOverlaySelection();
    m_overlay->setFrozenFrames(m_frames);
    if (m_request.enableWindowSnap && m_snapHints)
        m_expectedSnapMaskCount = outputs.size();
    if (!m_overlay->prepare()) {
        fail(QStringLiteral("截图覆盖窗口未能完成原生窗口准备"));
        return;
    }
    if (m_request.enableWindowSnap && m_snapHints) {
        if (!m_snapMaskTagError.isEmpty()) {
            fail(QStringLiteral("截图 Mask 窗口准备失败：%1").arg(m_snapMaskTagError));
            return;
        }
        if (m_taggedSnapMaskCount != m_expectedSnapMaskCount) {
            fail(QStringLiteral("截图 Mask 窗口准备失败：仅完成 %1/%2 个窗口 tag")
                         .arg(m_taggedSnapMaskCount)
                         .arg(m_expectedSnapMaskCount));
            return;
        }
        m_snapTagsPrepared = true;
    }
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
    m_overlay->setRecordModeActive(false);
    updateOverlayHints();
    if (m_request.enableWindowSnap)
        startSnapHints();
}

void ScreenshotSessionController::onOverlayFailed(const QString &reason)
{
    if (m_state != State::CreatingMasks
            && m_state != State::Selecting
            && m_state != State::Editing) {
        return;
    }
    fail(reason);
}

void ScreenshotSessionController::onPointerPressed(const QPoint &globalPos)
{
    if (m_state != State::Selecting && m_state != State::Editing)
        return;
    m_globalCursor = globalPos;
    m_pressed = true;
    m_dragged = false;
    m_pressPos = globalPos;
    m_pressStartedEmpty = m_selection->geometry().isEmpty();
    m_overlay->setSnapHint(QRect());

    const SelectionAction selectionHit = m_selection->hitTest(globalPos);
    if (isSelectionResize(selectionHit)) {
        startSelectionInteraction(globalPos);
    } else {
        const AnnotationHit annotationHit = m_annotationEditor
                ? m_annotationEditor->hitTest(globalPos)
                : AnnotationHit();
        if (annotationHit.isValid() && startAnnotationInteraction(globalPos))
            m_activeCursorIntent = annotationHit.cursor;
        else
            startSelectionInteraction(globalPos);
    }
    syncOverlayCursor(&globalPos);
    updateOverlayHints();
}

void ScreenshotSessionController::onPointerMoved(const QPoint &globalPos)
{
    m_globalCursor = globalPos;
    if (!m_pressed) {
        syncOverlayCursor(&globalPos);
        updateOverlayHints();
        return;
    }

    if ((globalPos - m_pressPos).manhattanLength() >= 3)
        m_dragged = true;
    if (m_inputTarget == InputTarget::Annotation && m_annotationEditor)
        m_annotationEditor->updateInteraction(globalPos);
    else if (m_inputTarget == InputTarget::Selection)
        m_selection->updateInteraction(globalPos);
    syncOverlayCursor(&globalPos);
    updateOverlayHints();
}

void ScreenshotSessionController::onPointerReleased(const QPoint &globalPos)
{
    if (!m_pressed)
        return;
    m_globalCursor = globalPos;
    m_pressed = false;
    if (m_inputTarget == InputTarget::Annotation && m_annotationEditor) {
        m_annotationEditor->endInteraction();
        m_annotationCoordinator->syncBounds();
    } else if (m_inputTarget == InputTarget::Selection) {
        if (!m_dragged && m_pressStartedEmpty && !m_snapHint.isEmpty())
            m_selection->setGeometry(m_snapHint);
        else
            m_selection->endInteraction();
    }
    m_inputTarget = InputTarget::Inactive;

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
    updateOverlayHints();
}

void ScreenshotSessionController::onSelectionMoveRequested(const QPoint &delta)
{
    if (m_state != State::Selecting && m_state != State::Editing)
        return;
    if (m_annotationEditor && m_annotationEditor->isInteracting())
        return;
    const QRect selection = m_selection->geometry();
    if (selection.isEmpty() || delta.isNull())
        return;

    const QRect annotationBounds = m_annotationEditor
            ? m_annotationEditor->contentBoundsGlobal().toAlignedRect().normalized()
            : QRect();
    m_selection->setGeometry(clampedSelectionMove(selection,
                                                  delta,
                                                  m_frames.virtualDesktopRect,
                                                  annotationBounds));
}

void ScreenshotSessionController::onSelectionResizeRequested(
        Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    if (m_state != State::Selecting && m_state != State::Editing)
        return;
    if (m_annotationEditor && m_annotationEditor->hasSelectedContent())
        return;
    if (key != Qt::Key_Left && key != Qt::Key_Right
            && key != Qt::Key_Up && key != Qt::Key_Down) {
        return;
    }

    const bool enlarge = modifiers == Qt::ControlModifier;
    const bool shrink = modifiers == (Qt::ControlModifier | Qt::ShiftModifier);
    if (!enlarge && !shrink)
        return;

    QRect selection = m_selection->geometry();
    if (selection.isEmpty())
        return;

    const QRect desktopBounds = m_frames.virtualDesktopRect;
    const QRect annotationBounds = m_annotationEditor
            ? m_annotationEditor->contentBoundsGlobal().toAlignedRect().normalized()
            : QRect();
    int left = selection.left();
    int top = selection.top();
    int right = selection.right();
    int bottom = selection.bottom();

    switch (key) {
    case Qt::Key_Left:
        if (enlarge) {
            left = qMax(desktopBounds.left(), left - 1);
        } else if (selection.width() > MinimumSelectionSize
                   && (!annotationBounds.isValid() || left < annotationBounds.left())) {
            ++left;
        }
        break;
    case Qt::Key_Right:
        if (enlarge) {
            right = qMin(desktopBounds.right(), right + 1);
        } else if (selection.width() > MinimumSelectionSize
                   && (!annotationBounds.isValid() || right > annotationBounds.right())) {
            --right;
        }
        break;
    case Qt::Key_Up:
        if (enlarge) {
            top = qMax(desktopBounds.top(), top - 1);
        } else if (selection.height() > MinimumSelectionSize
                   && (!annotationBounds.isValid() || top < annotationBounds.top())) {
            ++top;
        }
        break;
    case Qt::Key_Down:
        if (enlarge) {
            bottom = qMin(desktopBounds.bottom(), bottom + 1);
        } else if (selection.height() > MinimumSelectionSize
                   && (!annotationBounds.isValid() || bottom > annotationBounds.bottom())) {
            --bottom;
        }
        break;
    default:
        return;
    }

    const QRect resized(QPoint(left, top), QPoint(right, bottom));
    if (resized.width() >= MinimumSelectionSize
            && resized.height() >= MinimumSelectionSize) {
        m_selection->setGeometry(resized);
    }
}

void ScreenshotSessionController::onSelectionSizeChangeRequested(
        int width, int height, qreal pixelScale)
{
    if (m_state != State::Selecting && m_state != State::Editing)
        return;
    const QRect current = m_selection->geometry();
    if (current.isEmpty() || width <= 0 || height <= 0)
        return;

    const qreal scale = pixelScale > 0.0 ? pixelScale : 1.0;
    const int logicalWidth = qMax(MinimumSelectionSize, qRound(width / scale));
    const int logicalHeight = qMax(MinimumSelectionSize, qRound(height / scale));
    m_selection->setGeometry(QRect(current.topLeft(),
                                   QSize(logicalWidth, logicalHeight)));
}

void ScreenshotSessionController::onRoundedCornerRadiusChanged(int radius)
{
    m_roundedCornerRadius = qMax(0, radius);
}

void ScreenshotSessionController::onSnapRegionChanged(const QRect &globalRegion)
{
    if (m_state != State::Selecting || m_pressed || m_selection->state() == InteractionState::Ready)
        return;

    QList<QRect> outputGeometries;
    outputGeometries.reserve(m_frames.outputs.size());
    for (const OutputFrame &frame : m_frames.outputs)
        outputGeometries.append(frame.output.logicalGeometry);

    const QRect rawRegion =
            globalRegion.normalized().intersected(m_frames.virtualDesktopRect);
    m_snapHint = SelectionOverlayGeometry::snapRegionForOutputs(
            rawRegion, outputGeometries).intersected(m_frames.virtualDesktopRect);
    m_overlay->setSnapHint(m_snapHint);
}

void ScreenshotSessionController::onRecordModeChanged(bool enabled)
{
    if (m_state != State::Selecting && m_state != State::Editing)
        return;
    m_recordMode = enabled;
    m_overlay->setRecordModeActive(enabled);
    updateOverlayHints();
}

void ScreenshotSessionController::onRecordStartRequested()
{
    if (!m_recordMode || m_finished)
        return;
    if (m_state != State::Selecting && m_state != State::Editing)
        return;
    if (!m_recordingSession) {
        fail(QStringLiteral("当前平台没有可用的录屏后端"));
        return;
    }

    const QRect selection = m_selection->geometry().normalized();
    if (selection.isEmpty()) {
        fail(QStringLiteral("录屏前请先选择区域"));
        return;
    }

    const QList<QScreen *> screens =
            recordingScreensForSelection(selection, m_frames.outputs);
    if (screens.isEmpty()) {
        fail(QStringLiteral("录屏选区没有可用的输出屏幕"));
        return;
    }

    stopSnapHints();
    m_state = State::Countdown;
    m_overlay->setSelectionFinalized(true);
    if (!m_overlay->startRecordingCountdown(selection))
        fail(QStringLiteral("无法显示录屏倒计时"));
}

void ScreenshotSessionController::stopRecordingCountdown()
{
    if (m_overlay)
        m_overlay->stopRecordingCountdown();
}

void ScreenshotSessionController::startRecordingAfterCountdown()
{
    if (m_state != State::Countdown || m_finished)
        return;
    stopRecordingCountdown();

    const QRect selection = m_selection->geometry().normalized();
    const QList<QScreen *> screens =
            recordingScreensForSelection(selection, m_frames.outputs);
    if (screens.isEmpty()) {
        fail(QStringLiteral("录屏选区不再对应可用输出"));
        return;
    }
    if (!m_overlay->beginRecording(selection)) {
        fail(QStringLiteral("无法切换 TreeLand 录屏界面"));
        return;
    }

    ConfigSettings *settings = ConfigSettings::instance();
    RecordingRequest request;
    request.screens = screens;
    request.screen = QGuiApplication::screenAt(selection.center());
    if (!request.screen || !screens.contains(request.screen))
        request.screen = screens.first();
    request.outputGeometry = request.screen->geometry();
    request.selection = selection;
    request.includeCursor = settings->getValue(QStringLiteral("recorder"),
                                                QStringLiteral("cursor"))
                                    .toInt()
            != 0;
    request.frameRate = settings->getValue(QStringLiteral("recorder"),
                                           QStringLiteral("frame_rate"))
                                .toInt();
    if (request.frameRate <= 0)
        request.frameRate = 30;
    request.format = settings->getValue(QStringLiteral("recorder"),
                                        QStringLiteral("format"))
                             .toInt();

    // Let the compositor remove the full-screen mask before the first frame is
    // requested; otherwise the frozen screenshot surface can leak into video.
    QTimer::singleShot(120, this, [this, request] {
        if (m_finished || m_state != State::Countdown)
            return;
        if (!m_recordingSession->start(request)) {
            fail(QStringLiteral("启动 TreeLand 录屏失败"));
            return;
        }
    });
}

void ScreenshotSessionController::onRecordStopRequested()
{
    if (m_state == State::Countdown) {
        stopRecordingCountdown();
        if (m_recordingSession) {
            const IRecordingSession::State recordingState = m_recordingSession->state();
            if (recordingState == IRecordingSession::State::Starting
                    || recordingState == IRecordingSession::State::Recording
                    || recordingState == IRecordingSession::State::Stopping) {
                m_state = State::Exporting;
                m_recordingSession->cancel();
                return;
            }
        }
        finish(State::Cancelled);
        return;
    }
    if (m_state != State::Recording || !m_recordingSession)
        return;
    m_state = State::Exporting;
    m_recordingSession->stop();
}

void ScreenshotSessionController::onRecordingStarted()
{
    if (m_finished || m_state != State::Countdown)
        return;
    m_state = State::Recording;
    setRecordingState(true);
}

void ScreenshotSessionController::onRecordingSaved(const QString &path)
{
    if (m_finished)
        return;
    setRecordingState(false);
    m_state = State::Completed;
    emit saved(path);
    finish(State::Completed);
}

void ScreenshotSessionController::onRecordingFailed(const QString &reason)
{
    if (m_finished)
        return;
    setRecordingState(false);
    fail(reason);
}

void ScreenshotSessionController::onRecordingCancelled()
{
    if (m_finished)
        return;
    setRecordingState(false);
    finish(State::Cancelled);
}

void ScreenshotSessionController::setRecordingState(bool recording)
{
    if (m_recordingStateActive == recording)
        return;
    m_recordingStateActive = recording;
    if (m_overlay)
        m_overlay->setRecordingControlsActive(recording);
    emit RecorderState(recording);
    emit RecordingModeChanged(recording);
    emit recordingStateChanged();
}

void ScreenshotSessionController::onConfirmRequested()
{
    if (m_finished || m_selection->geometry().isEmpty())
        return;
    if (m_recordMode) {
        onRecordStartRequested();
        return;
    }
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
    bool cancelled = false;
    if (!m_resultSink->save(result,
                            m_request.outputPath,
                            &path,
                            &error,
                            &cancelled)) {
        if (cancelled) {
            finish(State::Cancelled);
            return;
        }
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

void ScreenshotSessionController::onPinRequested()
{
    if (m_finished || m_selection->geometry().isEmpty())
        return;
    if (m_state != State::Selecting && m_state != State::Editing)
        return;

    Result result;
    const State previousState = m_state;
    m_state = State::Exporting;
    if (!renderResult(&result)) {
        qWarning() << "Screenshot pin returned an empty result";
        m_state = previousState;
        return;
    }

    QString error;
    if (!m_resultSink->pin(result, &error)) {
        fail(error);
        return;
    }
    m_state = State::Completed;
    qCInfo(dsrApp) << "Screenshot session pinned:" << result.image.size()
                   << "generation:" << result.frameGeneration;
    finish(State::Completed);
}

void ScreenshotSessionController::onCancelRequested()
{
    if (m_finished)
        return;

    if ((m_state == State::Countdown
         || m_state == State::Recording
         || m_state == State::Exporting)
            && m_recordingSession) {
        const IRecordingSession::State recordingState = m_recordingSession->state();
        if (recordingState == IRecordingSession::State::Starting
                || recordingState == IRecordingSession::State::Recording
                || recordingState == IRecordingSession::State::Stopping) {
            m_state = State::Exporting;
            m_recordingSession->cancel();
            return;
        }
    }
    if (m_state == State::Recording && m_recordingSession) {
        m_recordingSession->cancel();
        setRecordingState(false);
        finish(State::Cancelled);
        return;
    }
    if (m_state == State::Countdown) {
        stopRecordingCountdown();
        finish(State::Cancelled);
        return;
    }

    if (m_inputTarget == InputTarget::Annotation && m_annotationEditor)
        m_annotationEditor->cancelInteraction();
    else if (m_inputTarget == InputTarget::Selection)
        m_selection->cancelInteraction();
    m_inputTarget = InputTarget::Inactive;
    m_pressed = false;
    m_backend->cancel();
    finish(State::Cancelled);
}

void ScreenshotSessionController::syncOverlaySelection()
{
    if (!m_overlay)
        return;
    m_overlay->setSelection(m_selection->geometry());
    m_overlay->setSelectionFinalized(m_selection->state() == InteractionState::Ready);
    updateOverlayHints();
}

void ScreenshotSessionController::updateOverlayHints()
{
    if (!m_overlay)
        return;
    const bool visible = (m_state == State::Selecting || m_state == State::Editing)
            && !m_recordMode;
    m_overlay->setSelectionHints(visible,
                                 m_selection->state() == InteractionState::Ready,
                                 m_globalCursor);
}

void ScreenshotSessionController::syncOverlayCursor(const QPoint *globalPos)
{
    if (!m_overlay)
        return;

    if (m_pressed) {
        m_overlay->setCursorIntent(cursorIntentWhilePressed(m_activeCursorIntent));
        return;
    }

    if (globalPos) {
        const SelectionAction selectionHit = m_selection->hitTest(*globalPos);
        if (isSelectionResize(selectionHit)) {
            m_overlay->setCursorIntent(cursorIntentFor(selectionHit));
            return;
        }
        if (m_annotationEditor) {
            const AnnotationHit annotationHit = m_annotationEditor->hitTest(*globalPos);
            if (annotationHit.isValid()) {
                m_overlay->setCursorIntent(annotationHit.cursor);
                return;
            }
        }
        if (selectionHit != SelectionAction::NoAction) {
            m_overlay->setCursorIntent(cursorIntentFor(selectionHit));
            return;
        }
    }

    SelectionAction action = m_selection->action();
    if (action == SelectionAction::NoAction && globalPos)
        action = m_selection->hitTest(*globalPos);
    m_overlay->setCursorIntent(cursorIntentFor(action));
}

bool ScreenshotSessionController::startAnnotationInteraction(const QPoint &globalPos)
{
    if (!m_annotationEditor || !m_annotationEditor->beginInteraction(globalPos))
        return false;
    m_inputTarget = InputTarget::Annotation;
    return true;
}

bool ScreenshotSessionController::startSelectionInteraction(const QPoint &globalPos)
{
    const SelectionAction action = m_selection->beginInteraction(globalPos);
    m_inputTarget = InputTarget::Selection;
    m_activeCursorIntent = cursorIntentFor(action);
    return true;
}

void ScreenshotSessionController::onMaskWindowRoleCreated(QWindow *window)
{
    if (m_state != State::CreatingMasks || !m_request.enableWindowSnap || !m_snapHints)
        return;
    if (!window || !m_snapMaskTagError.isEmpty())
        return;
    if (!m_windowTagger) {
        m_snapMaskTagError = QStringLiteral("缺少 TreeLand Mask window tagger");
        return;
    }

    QString error;
    if (!m_windowTagger->tagMaskWindow(window, &error)) {
        m_snapMaskTagError = error;
        return;
    }
    ++m_taggedSnapMaskCount;
}

void ScreenshotSessionController::startSnapHints()
{
    // Tags must be applied before the masks are mapped. Starting snap from an
    // already-active ordinary toplevel moves it between TreeLand containers
    // and can trip TreeLand's workspace assertion.
    if (m_snapStarted || !m_snapHints || !m_snapTagsPrepared)
        return;

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
    setRecordingState(false);
    if (m_recordingSession
            && (m_recordingSession->state() == IRecordingSession::State::Starting
                || m_recordingSession->state() == IRecordingSession::State::Recording
                || m_recordingSession->state() == IRecordingSession::State::Stopping)) {
        disconnect(m_recordingSession, nullptr, this, nullptr);
        m_recordingSession->cancel();
    }
    emit failed(reason);
    finish(State::Failed);
}

void ScreenshotSessionController::finish(State state)
{
    if (m_finished)
        return;
    stopRecordingCountdown();
    stopSnapHints();
    setRecordingState(false);
    if (m_overlay) {
        m_overlay->finishRecording();
        m_overlay->close();
    }
    m_finished = true;
    m_state = state;
    emit finished();
}

bool ScreenshotSessionController::renderResult(Result *result) const
{
    if (!result)
        return false;
    ScreenshotExportService service;
    *result = service.render(m_frames,
                             m_selection->geometry(),
                             m_annotationEditor,
                             m_roundedCornerRadius);
    return result->isValid();
}

} // namespace Dsr::Screenshot
