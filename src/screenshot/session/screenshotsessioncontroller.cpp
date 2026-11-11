// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshotsessioncontroller.h"

#include "annotationcoordinator.h"
#include "../core/selectionoverlaygeometry.h"
#include "../export/screenshotexportservice.h"
#include "../ports/iannotationeditor.h"
#include "../ports/irecordingsession.h"
#include "../ports/iscrollcapture.h"
#include "../ports/iscrollmerger.h"
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
#include <QFileInfo>
#include <QScreen>
#include <QTimer>
#include <QDebug>
#include <QWindow>

namespace Dsr::Screenshot {

namespace {

constexpr int MinimumSelectionSize = 10;
// A single missed frame is usually a transient compositor hiccup and must not
// flash the X11 merge-error prompt. Only a run of failures is user-visible.
constexpr int ScrollFrameFailureTipThreshold = 2;

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

bool isRecordMode(Mode mode)
{
    return mode == Mode::Record || mode == Mode::RecordFullScreen;
}

bool modeStartsAfterSelection(Mode mode)
{
    return mode == Mode::Scroll || mode == Mode::Ocr || mode == Mode::TopWindow;
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

int recordingFormatForOutputPath(int configuredFormat, const QString &outputPath)
{
    if (outputPath.isEmpty())
        return configuredFormat;

    const QString suffix = QFileInfo(outputPath).suffix().toLower();
    if (suffix == QStringLiteral("gif"))
        return Utils::kGIF;
    if (suffix == QStringLiteral("mkv"))
        return Utils::kMKV;
    if (suffix == QStringLiteral("mp4"))
        return Utils::kMP4;
    return configuredFormat;
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

    m_startDelayTimer = new QTimer(this);
    m_startDelayTimer->setSingleShot(true);
    connect(m_startDelayTimer, &QTimer::timeout,
            this, &ScreenshotSessionController::beginBackendPrepare);

    m_scrollStopTimeout = new QTimer(this);
    m_scrollStopTimeout->setSingleShot(true);
    m_scrollStopTimeout->setInterval(500);
    connect(m_scrollStopTimeout, &QTimer::timeout,
            this, &ScreenshotSessionController::finalizeScrollStop);

    m_scrollTipHideTimeout = new QTimer(this);
    m_scrollTipHideTimeout->setSingleShot(true);
    m_scrollTipHideTimeout->setInterval(2000);
    connect(m_scrollTipHideTimeout, &QTimer::timeout, this, [this] {
        if (m_overlay)
            m_overlay->hideScrollTip();
    });

    m_scrollResumeTimeout = new QTimer(this);
    m_scrollResumeTimeout->setSingleShot(true);
    m_scrollResumeTimeout->setInterval(2000);
    connect(m_scrollResumeTimeout, &QTimer::timeout,
            this, &ScreenshotSessionController::resumeScrollCaptureAfterFailure);

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
    connect(m_overlay, &IScreenshotOverlayHost::selectionRatioChangeRequested,
            this, &ScreenshotSessionController::onSelectionRatioChangeRequested);
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
    connect(m_overlay, &IScreenshotOverlayHost::ocrRequested,
            this, &ScreenshotSessionController::onOcrRequested);
    connect(m_overlay, &IScreenshotOverlayHost::tableRecognizeRequested,
            this, &ScreenshotSessionController::onTableRecognizeRequested);
    connect(m_overlay, &IScreenshotOverlayHost::aiAssistantRequested,
            this, &ScreenshotSessionController::onAiAssistantRequested);
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
    connect(m_overlay, &IScreenshotOverlayHost::scrollStartRequested,
            this, &ScreenshotSessionController::onScrollStartRequested);
    connect(m_overlay, &IScreenshotOverlayHost::scrollStopRequested,
            this, &ScreenshotSessionController::onScrollStopRequested);
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
    if (m_scrollCapture)
        m_scrollCapture->stop();
    if (m_scrollMerger)
        m_scrollMerger->cancel();
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
    m_hasGlobalCursor = false;
    m_snapTagsPrepared = false;
    m_snapStarted = false;
    m_expectedSnapMaskCount = 0;
    m_taggedSnapMaskCount = 0;
    m_snapMaskTagError.clear();
    m_finished = false;
    m_inputTarget = InputTarget::Inactive;
    m_activeCursorIntent = CursorIntent::Cross;
    m_recordMode = false;
    m_initialModeActionTriggered = false;
    m_roundedCornerRadius = 0;
    m_scrollSelection = QRect();
    m_lastScrollPreview = QImage();
    m_scrollExport = ScrollExport::Save;
    m_scrollPaused = false;
    m_scrollStopping = false;
    m_scrollFrameFailureCount = 0;
    if (m_startDelayTimer)
        m_startDelayTimer->stop();
    if (m_scrollStopTimeout)
        m_scrollStopTimeout->stop();
    if (m_scrollTipHideTimeout)
        m_scrollTipHideTimeout->stop();
    if (m_scrollResumeTimeout)
        m_scrollResumeTimeout->stop();
    m_resultSink->setNotificationsEnabled(m_request.notificationsEnabled);
    stopRecordingCountdown();
    m_selection->clear();
    m_state = State::PreparingFrame;
    if (m_request.delayMs > 0) {
        m_startDelayTimer->start(m_request.delayMs);
    } else {
        beginBackendPrepare();
    }
    return true;
}

void ScreenshotSessionController::beginBackendPrepare()
{
    if (m_state != State::PreparingFrame || m_finished)
        return;
    m_backend->prepare(m_request);
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

void ScreenshotSessionController::setScrollCapture(IScrollCapture *scrollCapture)
{
    if (m_scrollCapture == scrollCapture)
        return;
    if (m_scrollCapture)
        m_scrollCapture->deleteLater();
    m_scrollCapture = scrollCapture;
    if (!m_scrollCapture)
        return;
    m_scrollCapture->setParent(this);
    connect(m_scrollCapture, &IScrollCapture::frameReady,
            this, &ScreenshotSessionController::onScrollFrameReady);
    connect(m_scrollCapture, &IScrollCapture::failed,
            this, &ScreenshotSessionController::onScrollCaptureFailed);
    connect(m_scrollCapture, &IScrollCapture::frameFailed,
            this, &ScreenshotSessionController::onScrollCaptureFrameFailed);
    connect(m_scrollCapture, &IScrollCapture::captureAboutToStart,
            this, [this]() {
        if (m_overlay && m_state == State::Scrolling)
            m_overlay->beginScrollFrameCapture();
    });
    connect(m_scrollCapture, &IScrollCapture::captureFinished,
            this, [this]() {
        if (m_overlay && (m_state == State::Scrolling || m_scrollStopping))
            m_overlay->endScrollFrameCapture();
    });
}

void ScreenshotSessionController::setScrollMerger(IScrollImageMerger *scrollMerger)
{
    if (m_scrollMerger == scrollMerger)
        return;
    if (m_scrollMerger)
        m_scrollMerger->deleteLater();
    m_scrollMerger = scrollMerger;
    if (!m_scrollMerger)
        return;
    m_scrollMerger->setParent(this);
    connect(m_scrollMerger, &IScrollImageMerger::reachedBottom,
            this, &ScreenshotSessionController::onScrollReachedBottom);
    connect(m_scrollMerger, &IScrollImageMerger::maxHeightReached,
            this, &ScreenshotSessionController::onScrollMaxHeightReached);
    connect(m_scrollMerger, &IScrollImageMerger::quickScrollDetected,
            this, &ScreenshotSessionController::onScrollQuickScrolling);
    connect(m_scrollMerger, &IScrollImageMerger::invalidAreaDetected,
            this, &ScreenshotSessionController::onScrollInvalidArea);
    connect(m_scrollMerger, &IScrollImageMerger::failed,
            this, &ScreenshotSessionController::onScrollMergeFailed);
    connect(m_scrollMerger, &IScrollImageMerger::previewUpdated,
            this, [this](const QImage &image) {
        if (!m_overlay || image.isNull())
            return;
        if (m_state != State::Scrolling && !m_scrollStopping)
            return;
        m_lastScrollPreview = image;
        m_overlay->setScrollPreview(image);
    });
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
    if (m_scrollCapture)
        m_scrollCapture->setFrozenFrames(frames);
    m_selection->setDesktopBounds(frames.virtualDesktopRect);
    m_annotationCoordinator->syncBounds();
    if (m_request.mode == Mode::FullScreen
            || m_request.mode == Mode::RecordFullScreen) {
        m_selection->setGeometry(frames.virtualDesktopRect);
    } else if (!m_request.initialRegion.isEmpty()) {
        m_selection->setGeometry(m_request.initialRegion);
    }

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
    const bool recordMode = isRecordMode(m_request.mode);
    m_recordMode = recordMode;
    if (m_overlay)
        m_overlay->setInitialMode(m_request.mode);
    m_overlay->setRecordModeActive(recordMode);
    updateOverlayHints();
    if (m_request.enableWindowSnap)
        startSnapHints();
    if (m_request.mode == Mode::RecordFullScreen) {
        QTimer::singleShot(0, this, [this]() {
            if (m_state == State::Selecting && !m_finished)
                onRecordStartRequested();
        });
    } else if (m_request.mode == Mode::FullScreen) {
        // Match the X11 fullscreen command: the whole virtual desktop is the
        // selection and the result is saved without waiting for a toolbar
        // confirmation. This also keeps --delay/--fullscreen deterministic.
        QTimer::singleShot(0, this, [this]() {
            if (m_state == State::Selecting && !m_finished)
                onConfirmRequested();
        });
    }
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
    m_hasGlobalCursor = true;
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
    m_hasGlobalCursor = true;
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
    m_hasGlobalCursor = true;
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
        if (modeStartsAfterSelection(m_request.mode))
            scheduleInitialModeAction();
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
                                   QSize(logicalWidth, logicalHeight)),
                             SelectionGeometryMode::IgnoreAnnotationBounds);
}

void ScreenshotSessionController::onSelectionRatioChangeRequested(int width, int height)
{
    if (m_state != State::Selecting && m_state != State::Editing)
        return;

    const QRect current = m_selection->geometry();
    if (current.isEmpty() || width <= 0 || height <= 0)
        return;

    const qreal ratio = static_cast<qreal>(width) / static_cast<qreal>(height);
    if (ratio <= 0.0)
        return;

    const QRect bounds = m_frames.virtualDesktopRect;
    const int maxWidth = qMax(MinimumSelectionSize,
                              bounds.isValid() ? bounds.right() - current.left() + 1
                                               : current.width());
    const int maxHeight = qMax(MinimumSelectionSize,
                               bounds.isValid() ? bounds.bottom() - current.top() + 1
                                                : current.height());

    int logicalWidth = qMax(MinimumSelectionSize, current.width());
    int logicalHeight = qMax(MinimumSelectionSize, qRound(logicalWidth / ratio));
    if (logicalHeight > maxHeight) {
        logicalHeight = maxHeight;
        logicalWidth = qMax(MinimumSelectionSize, qRound(logicalHeight * ratio));
    }
    if (logicalWidth > maxWidth) {
        logicalWidth = maxWidth;
        logicalHeight = qMax(MinimumSelectionSize, qRound(logicalWidth / ratio));
    }

    m_selection->setGeometry(QRect(current.topLeft(),
                                   QSize(logicalWidth, logicalHeight)),
                             SelectionGeometryMode::IgnoreAnnotationBounds);
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

    if (m_request.mode == Mode::TopWindow
            && !m_snapHint.isEmpty()
            && !m_initialModeActionTriggered) {
        // TreeLand does not expose window geometry through the foreign
        // toplevel protocol. The compositor snap target already provides the
        // topmost window under the pointer, so reuse that region and finish
        // the top-window capture without asking the user to drag a second time.
        m_selection->setGeometry(m_snapHint);
        m_state = State::Editing;
        stopSnapHints();
        m_overlay->setSnapHint(QRect());
        syncOverlaySelection();
        scheduleInitialModeAction();
    }
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
    request.audioType = qBound(static_cast<int>(Utils::kNoAudio),
                               settings->getValue(QStringLiteral("recorder"),
                                                  QStringLiteral("audio"))
                                       .toInt(),
                               static_cast<int>(Utils::kMicAndSystemAudio));
    request.outputPath = m_request.outputPath;
    request.frameRate = settings->getValue(QStringLiteral("recorder"),
                                           QStringLiteral("frame_rate"))
                                .toInt();
    if (request.frameRate <= 0)
        request.frameRate = 30;
    request.format = settings->getValue(QStringLiteral("recorder"),
                                        QStringLiteral("format"))
                             .toInt();
    request.format = recordingFormatForOutputPath(request.format,
                                                  m_request.outputPath);

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

void ScreenshotSessionController::onScrollStartRequested()
{
    if (m_finished || (m_state != State::Selecting && m_state != State::Editing))
        return;

    const QRect selection = m_selection->geometry().normalized();
    if (selection.isEmpty()) {
        showScrollTip(ScrollTipKind::Error);
        return;
    }
    if (!m_scrollCapture || !m_scrollMerger) {
        qCWarning(dsrApp) << "Scroll capture backend is unavailable";
        showScrollTip(ScrollTipKind::Error);
        return;
    }

    int outputCount = 0;
    QList<OutputDescriptor> outputs;
    for (const OutputFrame &frame : m_frames.outputs) {
        outputs.append(frame.output);
        if (frame.output.logicalGeometry.intersects(selection))
            ++outputCount;
    }
    if (outputCount < 1) {
        qCWarning(dsrApp) << "Scroll capture requires at least one intersecting output";
        showScrollTip(ScrollTipKind::Error);
        return;
    }

    stopSnapHints();
    m_scrollSelection = selection;
    m_state = State::Scrolling;
    m_overlay->setSelectionFinalized(true);
    if (!m_overlay->beginScroll(selection)) {
        m_state = State::Editing;
        qCWarning(dsrApp) << "Unable to start the TreeLand scroll overlay";
        showScrollTip(ScrollTipKind::Error);
        return;
    }

    m_scrollMerger->start(selection);
    QString error;
    if (!m_scrollCapture->start(outputs, selection, &error)) {
        m_scrollMerger->cancel();
        m_overlay->finishScroll();
        m_state = State::Editing;
        qCWarning(dsrApp) << "Unable to start TreeLand scroll capture:" << error;
        showScrollTip(ScrollTipKind::Error);
        return;
    }
    m_scrollFrameFailureCount = 0;
    showScrollTip(ScrollTipKind::Start);
}

void ScreenshotSessionController::onScrollStopRequested()
{
    requestScrollExport(ScrollExport::Save);
}

void ScreenshotSessionController::requestScrollExport(ScrollExport exportMode)
{
    if (m_state != State::Scrolling || m_scrollStopping)
        return;

    m_scrollExport = exportMode;
    m_scrollStopping = true;
    m_state = State::Exporting;
    if (m_scrollResumeTimeout)
        m_scrollResumeTimeout->stop();
    if (m_scrollTipHideTimeout)
        m_scrollTipHideTimeout->stop();
    // Stop the periodic timer before requesting the final frame. Otherwise a
    // timer tick can race the export and leave the final request in flight.
    if (m_scrollCapture)
        m_scrollCapture->pause();
    if (m_overlay)
        m_overlay->finishScroll();

    if (m_scrollPaused) {
        finalizeScrollStop();
        return;
    }

    // Ask the platform for one final frame before tearing down the capture
    // session. This closes the gap between the last timer tick and the user's
    // confirm/save action. The timeout keeps export responsive if a frame is
    // lost.
    if (m_scrollStopTimeout)
        m_scrollStopTimeout->start();
    if (m_scrollCapture)
        m_scrollCapture->requestFrame();
    else
        finalizeScrollStop();
}

void ScreenshotSessionController::onScrollFrameReady(const ScrollFrame &frame)
{
    if (!m_scrollMerger || (m_state != State::Scrolling && !m_scrollStopping))
        return;
    const bool recovered = m_scrollFrameFailureCount > 0;
    m_scrollFrameFailureCount = 0;
    if (recovered && !m_scrollStopping && m_overlay) {
        m_overlay->hideScrollTip();
        if (m_scrollTipHideTimeout)
            m_scrollTipHideTimeout->stop();
    }
    m_scrollMerger->addFrame(frame);
    if (m_scrollStopping)
        finalizeScrollStop();
}

void ScreenshotSessionController::onScrollCaptureFrameFailed(const QString &reason)
{
    if (m_scrollStopping) {
        // The final frame is an optimization. Export the image stitched so far
        // instead of failing the whole screenshot session.
        qCWarning(dsrApp) << "Final scroll frame failed; exporting stitched image:"
                          << reason;
        finalizeScrollStop();
        return;
    }
    if (m_state != State::Scrolling || m_finished)
        return;

    ++m_scrollFrameFailureCount;
    qCWarning(dsrApp) << "Scroll frame failed" << m_scrollFrameFailureCount
                      << "time(s) in a row:" << reason;
    if (m_scrollPaused)
        return;
    if (m_scrollFrameFailureCount < ScrollFrameFailureTipThreshold)
        return;

    // Match X11's error state: keep the stitched preview and toolbar, pause
    // capture, and show the same prompt instead of closing the session. TreeLand
    // cannot see wheel events inside the pass-through hole, so retry
    // automatically after the prompt's 2s display window.
    pauseScrollCaptureForTip(ScrollTipKind::Error, true);
}

void ScreenshotSessionController::onScrollCaptureFailed(const QString &reason)
{
    if (m_scrollStopping) {
        // The final frame is an optimization: it closes the small gap between
        // the last periodic capture and the save action. If the compositor
        // does not deliver that frame, export the image we already stitched
        // instead of failing the whole screenshot session.
        qCWarning(dsrApp) << "Final scroll frame failed; exporting stitched image:"
                          << reason;
        finalizeScrollStop();
        return;
    }
    if (m_state != State::Scrolling || m_finished)
        return;
    // Terminal backend failures still keep the session alive. The user can
    // copy/save the frames already stitched or cancel the screenshot.
    qCWarning(dsrApp) << "Scroll capture stopped:" << reason;
    pauseScrollCaptureForTip(ScrollTipKind::Error);
}

void ScreenshotSessionController::finalizeScrollStop()
{
    if (!m_scrollStopping)
        return;
    m_scrollStopping = false;
    if (m_scrollStopTimeout)
        m_scrollStopTimeout->stop();
    if (m_scrollResumeTimeout)
        m_scrollResumeTimeout->stop();
    if (m_scrollTipHideTimeout)
        m_scrollTipHideTimeout->stop();
    stopScrollCapture();
    exportScrollResult();
}

void ScreenshotSessionController::onScrollReachedBottom()
{
    if (m_state != State::Scrolling || m_scrollPaused || m_scrollStopping)
        return;
    // Match X11: reaching the end stops capture but keeps the stitched
    // preview and toolbar alive. The user decides whether to copy or save.
    pauseScrollCaptureForTip(ScrollTipKind::End);
}

void ScreenshotSessionController::onScrollMaxHeightReached()
{
    if (m_state != State::Scrolling || m_scrollPaused || m_scrollStopping)
        return;
    pauseScrollCaptureForTip(ScrollTipKind::MaxLength);
}

void ScreenshotSessionController::onScrollQuickScrolling()
{
    if (m_state != State::Scrolling || m_scrollPaused || m_scrollStopping)
        return;
    showScrollTip(ScrollTipKind::QuickScroll);
}

void ScreenshotSessionController::onScrollInvalidArea()
{
    if (m_state != State::Scrolling || m_scrollPaused || m_scrollStopping)
        return;
    // X11 switches to the "adjust capture area" state. TreeLand cannot resize
    // the target window, so pause and keep the result saveable.
    pauseScrollCaptureForTip(ScrollTipKind::InvalidArea);
}

void ScreenshotSessionController::onScrollMergeFailed(const QString &reason)
{
    if (m_state != State::Scrolling || m_finished)
        return;
    qCWarning(dsrApp) << "Scroll merge failed:" << reason;
    pauseScrollCaptureForTip(ScrollTipKind::Error);
}

void ScreenshotSessionController::showScrollTip(ScrollTipKind kind)
{
    if (!m_overlay || m_finished)
        return;
    m_overlay->showScrollTip(kind);
    if (m_scrollTipHideTimeout)
        m_scrollTipHideTimeout->start();
}

void ScreenshotSessionController::pauseScrollCaptureForTip(ScrollTipKind kind,
                                                           bool retryAfterDelay)
{
    if (m_scrollPaused && !retryAfterDelay)
        return;
    m_scrollPaused = true;
    if (m_scrollCapture)
        m_scrollCapture->pause();
    showScrollTip(kind);
    if (retryAfterDelay && m_scrollResumeTimeout)
        m_scrollResumeTimeout->start();
}

void ScreenshotSessionController::resumeScrollCaptureAfterFailure()
{
    if (m_finished || m_state != State::Scrolling || m_scrollStopping
            || !m_scrollPaused) {
        return;
    }
    m_scrollPaused = false;
    m_scrollFrameFailureCount = 0;
    if (m_scrollTipHideTimeout)
        m_scrollTipHideTimeout->stop();
    if (m_overlay)
        m_overlay->hideScrollTip();
    if (m_scrollCapture)
        m_scrollCapture->resume();
}

void ScreenshotSessionController::stopScrollCapture()
{
    if (m_scrollCapture)
        m_scrollCapture->stop();
}

void ScreenshotSessionController::exportScrollResult()
{
    QImage image = m_scrollMerger ? m_scrollMerger->finish() : QImage();
    if (image.isNull())
        image = m_lastScrollPreview;
    if (image.isNull()) {
        fail(QStringLiteral("滚动截图没有可保存的图像"));
        return;
    }

    Result result;
    result.image = image;
    result.globalSelection = QRect(m_scrollSelection.topLeft(),
                                   QSize(m_scrollSelection.width(), image.height()));
    result.frameGeneration = m_frames.generation;
    for (const OutputFrame &frame : m_frames.outputs)
        result.outputs.append(frame.output);

    if (m_scrollExport == ScrollExport::Copy) {
        QString error;
        if (!m_resultSink->copy(result, &error)) {
            fail(error);
            return;
        }
        m_state = State::Completed;
        qCInfo(dsrApp) << "Scroll screenshot copied:"
                       << "selection:" << result.globalSelection
                       << "image:" << result.image.size();
        emit copied();
        finish(State::Completed);
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
    qCInfo(dsrApp) << "Scroll screenshot saved:" << path
                   << "selection:" << result.globalSelection
                   << "image:" << result.image.size();
    emit saved(path);
    finish(State::Completed);
}

void ScreenshotSessionController::scheduleInitialModeAction()
{
    if (m_initialModeActionTriggered || m_finished)
        return;
    m_initialModeActionTriggered = true;

    // Let the current pointer-release / overlay update finish before switching
    // into scrolling, OCR, or top-window export.
    QTimer::singleShot(60, this, [this]() {
        if (m_finished)
            return;
        switch (m_request.mode) {
        case Mode::Scroll:
            if (m_state == State::Selecting || m_state == State::Editing)
                onScrollStartRequested();
            break;
        case Mode::Ocr:
            if (m_state == State::Selecting || m_state == State::Editing)
                onOcrRequested();
            break;
        case Mode::TopWindow:
            if (m_state == State::Selecting || m_state == State::Editing)
                onConfirmRequested();
            break;
        case Mode::Region:
        case Mode::FullScreen:
        case Mode::Record:
        case Mode::RecordFullScreen:
            break;
        }
    });
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
    if (m_finished)
        return;
    if (m_state == State::Scrolling) {
        requestScrollExport(ScrollExport::Save);
        return;
    }
    if (m_selection->geometry().isEmpty())
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
    if (m_finished)
        return;

    if (m_state == State::Scrolling) {
        requestScrollExport(ScrollExport::Copy);
        return;
    }

    // X11 gives Ctrl+C to the magnifier while it is visible; otherwise the
    // same shortcut copies the selected screenshot. Preserve that arbitration
    // in TreeLand before checking whether a selection has been created.
    const QString magnifierHex = m_overlay ? m_overlay->magnifierColorHex() : QString();
    if (!magnifierHex.isEmpty()) {
        QString error;
        if (!m_resultSink->copyText(magnifierHex, &error)) {
            fail(error);
            return;
        }
        m_state = State::Completed;
        qCInfo(dsrApp) << "Color copied from selection magnifier:" << magnifierHex;
        finish(State::Completed);
        return;
    }

    if (m_selection->geometry().isEmpty())
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

void ScreenshotSessionController::onOcrRequested()
{
    if (m_finished || m_selection->geometry().isEmpty())
        return;
    if (m_state != State::Selecting && m_state != State::Editing)
        return;

    Result result;
    const State previousState = m_state;
    m_state = State::Exporting;
    // Match X11: OCR consumes the original annotation-composited pixels and
    // does not apply the rounded-corner screenshot effect.
    if (!renderResult(&result, false)) {
        qWarning() << "Screenshot OCR returned an empty result";
        m_state = previousState;
        return;
    }

    QString error;
    if (!m_resultSink->ocr(result, &error)) {
        fail(error);
        return;
    }
    m_state = State::Completed;
    qCInfo(dsrApp) << "Screenshot session sent to OCR:" << result.image.size()
                   << "selection:" << result.globalSelection
                   << "generation:" << result.frameGeneration;
    finish(State::Completed);
}

void ScreenshotSessionController::onTableRecognizeRequested()
{
    if (m_finished || m_selection->geometry().isEmpty())
        return;
    if (m_state != State::Selecting && m_state != State::Editing)
        return;

    Result result;
    const State previousState = m_state;
    m_state = State::Exporting;
    // Match X11: table recognition consumes the original image without the
    // rounded-corner screenshot effect.
    if (!renderResult(&result, false)) {
        qWarning() << "Screenshot table recognition returned an empty result";
        m_state = previousState;
        return;
    }

    QString error;
    if (!m_resultSink->tableRecognize(result, &error)) {
        fail(error);
        return;
    }
    m_state = State::Completed;
    qCInfo(dsrApp) << "Screenshot session sent to table recognition:"
                   << result.image.size()
                   << "selection:" << result.globalSelection
                   << "generation:" << result.frameGeneration;
    finish(State::Completed);
}

void ScreenshotSessionController::onAiAssistantRequested(int function)
{
    if (m_finished || m_selection->geometry().isEmpty())
        return;
    if (m_state != State::Selecting && m_state != State::Editing)
        return;

    Result result;
    const State previousState = m_state;
    m_state = State::Exporting;
    if (!renderResult(&result)) {
        qWarning() << "Screenshot AI assistant returned an empty result";
        m_state = previousState;
        return;
    }

    QString error;
    if (!m_resultSink->aiAssistant(result, function, &error)) {
        fail(error);
        return;
    }
    m_state = State::Completed;
    qCInfo(dsrApp) << "Screenshot session sent to AI assistant:"
                   << "function:" << function
                   << result.image.size()
                   << "selection:" << result.globalSelection
                   << "generation:" << result.frameGeneration;
    finish(State::Completed);
}

void ScreenshotSessionController::onCancelRequested()
{
    if (m_finished)
        return;

    if (m_state == State::Scrolling || m_scrollStopping) {
        m_scrollPaused = false;
        m_scrollStopping = false;
        if (m_scrollStopTimeout)
            m_scrollStopTimeout->stop();
        stopScrollCapture();
        if (m_scrollMerger)
            m_scrollMerger->cancel();
        if (m_overlay)
            m_overlay->finishScroll();
        finish(State::Cancelled);
        return;
    }

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
    if (m_startDelayTimer)
        m_startDelayTimer->stop();
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
                                 m_globalCursor,
                                 m_hasGlobalCursor);
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
    if (m_startDelayTimer)
        m_startDelayTimer->stop();
    if (m_scrollResumeTimeout)
        m_scrollResumeTimeout->stop();
    if (m_scrollTipHideTimeout)
        m_scrollTipHideTimeout->stop();
    stopScrollCapture();
    if (m_scrollMerger)
        m_scrollMerger->cancel();
    if (m_overlay)
        m_overlay->finishScroll();
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
    if (m_startDelayTimer)
        m_startDelayTimer->stop();
    m_scrollStopping = false;
    m_scrollPaused = false;
    if (m_scrollStopTimeout)
        m_scrollStopTimeout->stop();
    if (m_scrollResumeTimeout)
        m_scrollResumeTimeout->stop();
    if (m_scrollTipHideTimeout)
        m_scrollTipHideTimeout->stop();
    stopScrollCapture();
    if (m_scrollMerger)
        m_scrollMerger->cancel();
    setRecordingState(false);
    if (m_overlay) {
        m_overlay->finishScroll();
        m_overlay->finishRecording();
        m_overlay->close();
    }
    m_finished = true;
    m_state = state;
    emit finished();
}

bool ScreenshotSessionController::renderResult(Result *result,
                                                bool applyRoundedCorners) const
{
    if (!result)
        return false;
    ScreenshotExportService service;
    *result = service.render(m_frames,
                             m_selection->geometry(),
                             m_annotationEditor,
                             applyRoundedCorners ? m_roundedCornerRadius : 0);
    return result->isValid();
}

} // namespace Dsr::Screenshot
