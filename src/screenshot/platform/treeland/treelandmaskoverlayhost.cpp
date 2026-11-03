// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandmaskoverlayhost.h"

#include "treelandannotationeditor.h"
#include "treelandmaskwindow.h"
#include "../../../countdown_tooltip.h"
#include "widgets/sidebar.h"
#include "widgets/toolbar.h"
#include "widgets/toolbarhost.h"
#include "widgets/fineadjusttips.h"
#include "widgets/roundedcornerwidget.h"
#include "widgets/zoomIndicator.h"
#include "../../export/screenshotexportservice.h"
#include "../../core/coordinatemapper.h"
#include "../../core/selectionoverlaygeometry.h"
#include "../../ports/iwindowtagger.h"
#include "../../../utils/configsettings.h"
#include "../../../utils/log.h"
#include "../../../utils/tempfile.h"

#include <QPixmap>
#include <QGuiApplication>
#include <QRegion>
#include <QTimer>
#include <QWindow>
#include <QtMath>

namespace Dsr::Screenshot {

namespace {

constexpr int ToolBarMargin = 8;
constexpr int ToolBarGap = 8;
constexpr int MagnifierOffset = 12;
constexpr int InputReadyCheckIntervalMs = 8;
constexpr int InputReadyMaxChecks = 250;

bool isTreeLandAnnotationTool(const QString &tool)
{
    return tool == QStringLiteral("gio")
            || tool == QStringLiteral("rectangle")
            || tool == QStringLiteral("oval")
            || tool == QStringLiteral("line")
            || tool == QStringLiteral("arrow")
            || tool == QStringLiteral("pen")
            || tool == QStringLiteral("effect")
            || tool == QStringLiteral("sequence")
            || tool == QStringLiteral("text");
}

} // namespace

class TreelandToolBarHost final : public ToolBarHost
{
public:
    explicit TreelandToolBarHost(TreelandMaskOverlayHost *overlay, QWidget *anchor)
        : ToolBarHost(anchor)
        , m_overlay(overlay)
    {
    }

    void confirm() override
    {
        if (m_overlay->isRecordingActive())
            emit m_overlay->recordStopRequested();
        else if (m_recordMode)
            emit m_overlay->recordStartRequested();
        else
            emit m_overlay->copyRequested();
    }

    void close(bool) override
    {
        emit m_overlay->cancelRequested();
    }

    void save() override
    {
        emit m_overlay->confirmRequested();
    }

    void undo() override
    {
        m_overlay->undoAnnotation();
    }

    void undoHover(bool) override
    {
    }

    void toolChanged(const QString &tool) override
    {
        if (tool == QStringLiteral("pinScreenshots")) {
            emit m_overlay->pinRequested();
            return;
        }
        if (tool == QStringLiteral("ocr")) {
            emit m_overlay->ocrRequested();
            return;
        }
        if (tool == QStringLiteral("tableRecogn")) {
            emit m_overlay->tableRecognizeRequested();
            return;
        }
        m_overlay->handleToolChanged(tool);
    }

    void optionMenuShown() override
    {
    }

    void keyboardToggled(bool) override
    {
    }

    void mouseToggled(bool) override
    {
    }

    void cameraToggled(bool) override
    {
    }

    void aiAssistantSelected(int function) override
    {
        emit m_overlay->aiAssistantRequested(function);
    }

    void toolBarPointRequested() override
    {
    }

    void switchToShotUI() override
    {
        if (m_overlay->isRecordingActive())
            return;
        if (!m_recordMode)
            return;
        m_recordMode = false;
        emit m_overlay->recordModeChanged(false);
    }

    void switchToRecordUI() override
    {
        if (m_overlay->isRecordingActive() || m_recordMode)
            return;
        m_recordMode = true;
        emit m_overlay->recordModeChanged(true);
    }

    QPoint sideBarStartPressPoint() const override
    {
        rememberToolBarPositions();
        return m_sideBarStartPoint;
    }

    QPoint toolBarStartPressPoint() const override
    {
        rememberToolBarPositions();
        return m_toolBarStartPoint;
    }

    bool moveToolBars(const QPoint &, const QPoint &delta) override
    {
        if (!m_overlay || !m_overlay->m_toolBar || !m_dragPositionsValid)
            return false;

        QWidget *toolBar = m_overlay->m_toolBar;
        QWidget *sideBar = m_overlay->m_sideBar;

        QWidget *anchor = anchorWidget();
        if (!anchor)
            return false;

        const QRect available = anchor->rect();
        const int minDx = -m_groupStartRect.left();
        const int maxDx = available.width() - m_groupStartRect.right() - 1;
        const int minDy = -m_groupStartRect.top();
        const int maxDy = available.height() - m_groupStartRect.bottom() - 1;
        const QPoint boundedDelta(clampToRange(delta.x(), minDx, maxDx),
                                  clampToRange(delta.y(), minDy, maxDy));

        // Match MainWindow::moveToolBars(): once the user has positioned the
        // toolbar manually, later selection changes must not pull it back to
        // the automatic position.
        m_overlay->m_toolBarManuallyMoved = true;
        toolBar->move(m_toolBarStartPoint + boundedDelta);
        if (sideBar && m_sideBarVisibleAtDragStart)
            sideBar->move(m_sideBarStartPoint + boundedDelta);
        toolBar->raise();
        if (sideBar && m_sideBarVisibleAtDragStart)
            sideBar->raise();
        return true;
    }

private:
    static int clampToRange(int value, int minimum, int maximum)
    {
        return maximum < minimum ? minimum : qBound(minimum, value, maximum);
    }

    void rememberToolBarPositions() const
    {
        if (!m_overlay || !m_overlay->m_toolBar)
            return;
        m_toolBarStartPoint = m_overlay->m_toolBar->pos();
        m_sideBarStartPoint = m_overlay->m_sideBar ? m_overlay->m_sideBar->pos()
                                                   : QPoint();
        m_sideBarVisibleAtDragStart = m_overlay->m_sideBar
                && m_overlay->m_sideBar->isVisible();
        m_groupStartRect = m_overlay->m_toolBar->geometry();
        if (m_sideBarVisibleAtDragStart)
            m_groupStartRect = m_groupStartRect.united(m_overlay->m_sideBar->geometry());
        m_dragPositionsValid = true;
    }

    TreelandMaskOverlayHost *m_overlay = nullptr;
    mutable QPoint m_toolBarStartPoint;
    mutable QPoint m_sideBarStartPoint;
    mutable QRect m_groupStartRect;
    mutable bool m_sideBarVisibleAtDragStart = false;
    mutable bool m_dragPositionsValid = false;
    bool m_recordMode = false;
};

TreelandMaskOverlayHost::TreelandMaskOverlayHost(IWindowTagger *windowTagger,
                                                 QObject *parent)
    : IScreenshotOverlayHost(parent)
    , m_annotationEditor(new TreelandAnnotationEditor(this))
    , m_windowTagger(windowTagger)
{
    connect(m_annotationEditor, &TreelandAnnotationEditor::shapeSelected,
            this, [this](const QString &shape) {
        if (m_toolBar && !shape.isEmpty())
            m_toolBar->shapeClickedFromMain(shape);
    });
    connect(m_annotationEditor, &IAnnotationEditor::contentBoundsChanged,
            this, [this](const QRectF &bounds) {
        if (m_toolBarHost)
            emit m_toolBarHost->undoAvailabilityChanged(!bounds.isEmpty());
    });
    connect(m_annotationEditor, &IAnnotationEditor::contentChanged,
            this, [this] {
        if (m_toolBarHost) {
            emit m_toolBarHost->undoAvailabilityChanged(
                    !m_annotationEditor->contentBoundsGlobal().isEmpty());
        }
    });
}

TreelandMaskOverlayHost::~TreelandMaskOverlayHost()
{
    destroyWindows();
}

bool TreelandMaskOverlayHost::create(const QList<OutputDescriptor> &outputs)
{
    if (outputs.isEmpty())
        return false;

    destroyWindows();
    m_annotationEditor->clearViews();

    QRect desktopBounds;
    for (const OutputDescriptor &output : outputs) {
        desktopBounds = desktopBounds.isNull()
                ? output.logicalGeometry
                : desktopBounds.united(output.logicalGeometry);
    }
    m_annotationEditor->setDesktopBounds(desktopBounds);

    for (const OutputDescriptor &output : outputs) {
        auto *window = new TreelandMaskWindow(output);
        QWidget *annotationView = m_annotationEditor->createView(window, output.logicalGeometry);
        window->setAnnotationView(annotationView);

        connect(window, &TreelandMaskWindow::pointerPressed,
                this, &IScreenshotOverlayHost::pointerPressed);
        connect(window, &TreelandMaskWindow::pointerMoved,
                this, &IScreenshotOverlayHost::pointerMoved);
        connect(window, &TreelandMaskWindow::pointerReleased,
                this, &IScreenshotOverlayHost::pointerReleased);
        connect(window, &TreelandMaskWindow::pointerDoubleClicked,
                this, [this](const QPoint &globalPos) {
            if (m_annotationEditor)
                m_annotationEditor->beginInlineEdit(globalPos);
        });
        connect(window, &TreelandMaskWindow::selectionMoveRequested,
                this, [this](const QPoint &delta) {
            if (m_annotationToolActive || m_recordingActive || m_recordModeActive)
                return;
            emit selectionMoveRequested(delta);
        });
        connect(window, &TreelandMaskWindow::selectionResizeRequested,
                this, [this](Qt::Key key, Qt::KeyboardModifiers modifiers) {
            // In annotation mode X11 still resizes the capture area when no
            // annotation is selected. The controller performs that arbitration.
            if (m_recordingActive || m_recordModeActive)
                return;
            if (m_roundedCornerWidget && m_roundedCornerWidget->isRatioLocked())
                return;
            emit selectionResizeRequested(key, modifiers);
        });
        connect(window, &TreelandMaskWindow::confirmRequested,
                this, &IScreenshotOverlayHost::confirmRequested);
        connect(window, &TreelandMaskWindow::copyRequested,
                this, &IScreenshotOverlayHost::copyRequested);
        connect(window, &TreelandMaskWindow::ocrRequested,
                this, &IScreenshotOverlayHost::ocrRequested);
        connect(window, &TreelandMaskWindow::tableRecognizeRequested,
                this, &IScreenshotOverlayHost::tableRecognizeRequested);
        connect(window, &TreelandMaskWindow::cancelRequested,
                this, &IScreenshotOverlayHost::cancelRequested);
        connect(window, &TreelandMaskWindow::undoRequested,
                this, &TreelandMaskOverlayHost::undoAnnotation);
        connect(window,
                &TreelandMaskWindow::windowRoleCreated,
                this,
                &IScreenshotOverlayHost::maskWindowRoleCreated,
                Qt::DirectConnection);
        m_windows.append(window);
    }

    ensureToolBar();
    ensureSelectionWidgets();
    return true;
}

IAnnotationEditor *TreelandMaskOverlayHost::annotationEditor() const
{
    return m_annotationEditor;
}

QList<QWindow *> TreelandMaskOverlayHost::maskWindows() const
{
    QList<QWindow *> result;
    for (TreelandMaskWindow *window : m_windows) {
        if (window && window->windowHandle())
            result.append(window->windowHandle());
    }
    return result;
}

void TreelandMaskOverlayHost::setFrozenFrames(const FrozenFrameSet &frames)
{
    m_frozenFrames = frames;
    m_effectSourceReady = false;

    for (const OutputFrame &frame : frames.outputs) {
        for (TreelandMaskWindow *window : m_windows) {
            if (window && window->output().outputId == frame.output.outputId) {
                window->setFrozenFrame(frame.image);
                break;
            }
        }
    }
    updateMagnifier();
}

bool TreelandMaskOverlayHost::prepare()
{
    if (m_windows.isEmpty())
        return false;

    for (TreelandMaskWindow *window : m_windows) {
        if (!window)
            return false;
        if (!window->prepareWindow())
            return false;
    }

    m_prepared = true;
    return true;
}

void TreelandMaskOverlayHost::show()
{
    if (m_windows.isEmpty()) {
        emit inputFailed(QStringLiteral("截图覆盖窗口未创建"));
        return;
    }

    if (!m_prepared && !prepare()) {
        emit inputFailed(QStringLiteral("截图覆盖窗口未能创建原生窗口"));
        return;
    }

    for (TreelandMaskWindow *window : m_windows) {
        window->showWindow();
    }
    updateToolBar();

    // xdg_toplevel and the compositor-side mask configuration become usable
    // only after every per-output surface has been exposed. Starting snap from
    // a single deferred callback can race the slower output in a multi-screen
    // session, so wait for all windows explicitly.
    m_inputReadyPending = true;
    m_inputReadyChecks = 0;
    QTimer::singleShot(0, this, &TreelandMaskOverlayHost::checkInputReady);
}

void TreelandMaskOverlayHost::checkInputReady()
{
    if (!m_inputReadyPending)
        return;

    for (TreelandMaskWindow *window : m_windows) {
        if (!window || !window->windowHandle()
                || !window->windowHandle()->isExposed()) {
            if (++m_inputReadyChecks >= InputReadyMaxChecks) {
                m_inputReadyPending = false;
                emit inputFailed(QStringLiteral("截图覆盖窗口未能在超时前完成映射"));
                return;
            }
            QTimer::singleShot(InputReadyCheckIntervalMs,
                               this,
                               &TreelandMaskOverlayHost::checkInputReady);
            return;
        }
    }

    m_inputReadyPending = false;
    emit inputReady();
}

void TreelandMaskOverlayHost::setSelection(const QRect &globalSelection)
{
    m_selection = globalSelection.normalized();
    for (TreelandMaskWindow *window : m_windows) {
        if (!window)
            continue;
        window->setSelection(m_selection);
    }
    updateRoundedCornerWidget();
    updateToolBar();
    updateFineAdjustTips();
    updateMagnifier();
}

void TreelandMaskOverlayHost::setSnapHint(const QRect &globalRegion)
{
    for (TreelandMaskWindow *window : m_windows) {
        if (window)
            window->setSnapHint(globalRegion);
    }
}

void TreelandMaskOverlayHost::setSelectionFinalized(bool finalized)
{
    if (m_finalized == finalized)
        return;
    m_finalized = finalized;
    for (TreelandMaskWindow *window : m_windows) {
        if (window)
            window->setSelectionFinalized(finalized);
    }
    updateRoundedCornerWidget();
    updateToolBar();
    updateFineAdjustTips();
    updateMagnifier();
}

void TreelandMaskOverlayHost::setCursorIntent(CursorIntent intent)
{
    for (TreelandMaskWindow *window : m_windows) {
        if (window)
            window->setCursorIntent(intent);
    }
}

void TreelandMaskOverlayHost::setSelectionHints(bool visible,
                                                bool selectionMode,
                                                const QPoint &globalCursor,
                                                bool hasGlobalCursor)
{
    m_hintsVisible = visible;
    m_hintsSelectionMode = selectionMode;
    m_globalCursor = globalCursor;
    m_hasGlobalCursor = hasGlobalCursor;
    updateFineAdjustTips();
    updateMagnifier();
}

void TreelandMaskOverlayHost::setRecordModeActive(bool active)
{
    if (m_recordModeActive == active)
        return;
    m_recordModeActive = active;
    updateRoundedCornerWidget();
    updateFineAdjustTips();
    updateMagnifier();
}

QString TreelandMaskOverlayHost::magnifierColorHex() const
{
    if (!m_magnifier || !m_magnifier->isVisible())
        return {};
    return m_magnifier->currentColorHex();
}

bool TreelandMaskOverlayHost::startRecordingCountdown(const QRect &globalSelection)
{
    destroyRecordingCountdown();

    const QRect selection = globalSelection.normalized();
    if (selection.isEmpty() || m_frozenFrames.isEmpty())
        return false;

    TreelandMaskWindow *owner = selectionOwnerWindow();
    if (!owner)
        return false;

    // Match MainWindow::startRecord(): a selection inside one output shows the
    // countdown in that output's center; a cross-output selection prefers the
    // toolbar output, then the primary output.
    TreelandMaskWindow *selectionOutput = nullptr;
    for (TreelandMaskWindow *window : std::as_const(m_windows)) {
        if (window && window->output().logicalGeometry.normalized().contains(selection)) {
            selectionOutput = window;
            break;
        }
    }

    if (!selectionOutput && m_toolBar) {
        TreelandMaskWindow *toolbarOutput = qobject_cast<TreelandMaskWindow *>(
                m_toolBar->parentWidget());
        if (toolbarOutput
                && toolbarOutput->output().logicalGeometry.normalized().contains(
                        QRect(toolbarOutput->output().logicalGeometry.topLeft()
                                      + m_toolBar->pos(),
                              m_toolBar->size()))) {
            owner = toolbarOutput;
        } else {
            owner = nullptr;
        }
    }

    if (!selectionOutput && !owner) {
        const QString primaryName = QGuiApplication::primaryScreen()
                ? QGuiApplication::primaryScreen()->name()
                : QString();
        for (TreelandMaskWindow *window : std::as_const(m_windows)) {
            if (window && window->output().outputId == primaryName) {
                owner = window;
                break;
            }
        }
    }
    if (!owner)
        owner = m_windows.value(0);
    if (!owner)
        return false;

    ScreenshotExportService exportService;
    const Result desktop = exportService.render(m_frozenFrames,
                                                m_frozenFrames.virtualDesktopRect,
                                                nullptr);
    QPixmap background;
    if (desktop.isValid()) {
        background = QPixmap::fromImage(desktop.image);
        TempFile::instance()->setFullScreenPixmap(background);
    }

    m_countdown = new CountdownTooltip(owner);
    if (!background.isNull()) {
        m_countdown->setBackgroundPixmap(background,
                                         m_frozenFrames.virtualDesktopRect.topLeft());
    }

    const QPoint centerLocal = selectionOutput
            ? selection.center() - owner->output().logicalGeometry.topLeft()
            : owner->rect().center();
    m_countdown->move(centerLocal.x() - m_countdown->width() / 2,
                      centerLocal.y() - m_countdown->height() / 2);
    connect(m_countdown, &CountdownTooltip::finished,
            this, &IScreenshotOverlayHost::recordingCountdownFinished,
            Qt::UniqueConnection);
    m_countdown->start();
    m_countdown->show();
    m_countdown->raise();
    return true;
}

void TreelandMaskOverlayHost::stopRecordingCountdown()
{
    destroyRecordingCountdown();
}

void TreelandMaskOverlayHost::destroyRecordingCountdown()
{
    if (!m_countdown)
        return;
    disconnect(m_countdown, nullptr, this, nullptr);
    m_countdown->hide();
    m_countdown->deleteLater();
    m_countdown = nullptr;
}

bool TreelandMaskOverlayHost::beginRecording(const QRect &globalSelection)
{
    if (m_recordingActive || globalSelection.isEmpty())
        return m_recordingActive;

    if (m_sideBar)
        m_sideBar->hide();
    if (m_roundedCornerWidget)
        m_roundedCornerWidget->hide();
    if (m_fineAdjustTips)
        m_fineAdjustTips->hide();
    if (m_toolBar) {
        m_toolBar->hide();
        m_toolBar->setEnabled(false);
    }

    const QRect selection = globalSelection.normalized();

    // X11 uses the tray recording plugin as the stop entry. Keep the TreeLand
    // recording frame and input pass-through, but do not create a second
    // in-overlay recording toolbar.
    for (TreelandMaskWindow *window : std::as_const(m_windows)) {
        if (!window)
            continue;
        window->setRecordingMode(true, selection, true);
        window->clearMask();
        window->show();
        window->raise();
        window->setInputRegion(QRegion());
    }

    m_recordingActive = true;
    return true;
}

void TreelandMaskOverlayHost::finishRecording()
{
    stopRecordingCountdown();
    if (!m_recordingActive)
        return;
    m_recordingActive = false;
    for (TreelandMaskWindow *window : std::as_const(m_windows)) {
        if (!window)
            continue;
        window->setRecordingMode(false);
        window->resetInputRegion();
        window->clearMask();
        window->hide();
    }
    setRecordingControlsActive(false);
    if (m_toolBar) {
        m_toolBar->hide();
        m_toolBar->setEnabled(false);
    }
    updateRoundedCornerWidget();
    updateFineAdjustTips();
}

bool TreelandMaskOverlayHost::isRecordingActive() const
{
    return m_recordingActive;
}

void TreelandMaskOverlayHost::setRecordingControlsActive(bool active)
{
    if (m_toolBar)
        m_toolBar->setRecordingActive(active);
}

void TreelandMaskOverlayHost::close()
{
    stopRecordingCountdown();
    finishRecording();
    hideMagnifier();
    // Mask windows are destroyed asynchronously below. Release the magnifier
    // now so it cannot outlive its parent and leave m_magnifier dangling.
    delete m_magnifier;
    m_magnifier = nullptr;
    m_inputReadyPending = false;
    m_prepared = false;
    // close() can run from a toolbar button's clicked handler (Save, Cancel,
    // Confirm).  Destroying the toolbar host synchronously invalidates the
    // object currently executing that handler and can crash in the remaining
    // button-release dispatch.  Hide and disarm the UI immediately, then let
    // the event loop delete the window/host objects once the signal stack has
    // unwound.
    ToolBar *toolBar = m_toolBar;
    SideBar *sideBar = m_sideBar;
    ToolBarHost *toolBarHost = m_toolBarHost;
    m_toolBar = nullptr;
    m_sideBar = nullptr;
    m_toolBarHost = nullptr;

    if (toolBar) {
        toolBar->hide();
        toolBar->setEnabled(false);
        if (!toolBar->parent())
            toolBar->deleteLater();
    }
    if (sideBar) {
        sideBar->hide();
        sideBar->setEnabled(false);
    }
    if (toolBarHost) {
        disconnect(toolBarHost, nullptr, this, nullptr);
        toolBarHost->deleteLater();
    }
    if (m_roundedCornerWidget) {
        m_roundedCornerWidget->hide();
        m_roundedCornerWidget->deleteLater();
        m_roundedCornerWidget = nullptr;
    }
    if (m_fineAdjustTips) {
        m_fineAdjustTips->hide();
        m_fineAdjustTips->deleteLater();
        m_fineAdjustTips = nullptr;
    }
    for (TreelandMaskWindow *window : m_windows) {
        if (!window)
            continue;
        window->closeSilently();
        window->deleteLater();
    }
    m_windows.clear();
    m_annotationEditor->clearViews();
    m_selection = QRect();
    m_finalized = false;
    m_recordModeActive = false;
    m_hintsVisible = false;
    m_toolBarManuallyMoved = false;
    m_prepared = false;
}

TreelandMaskWindow *TreelandMaskOverlayHost::selectionOwnerWindow() const
{
    QList<QRect> outputGeometries;
    outputGeometries.reserve(m_windows.size());
    for (TreelandMaskWindow *window : m_windows) {
        outputGeometries.append(window ? window->output().logicalGeometry : QRect());
    }
    const int ownerIndex = SelectionOverlayGeometry::largestSelectionIntersectionIndex(
            m_selection, outputGeometries);
    return ownerIndex >= 0 ? m_windows.value(ownerIndex) : nullptr;
}

void TreelandMaskOverlayHost::ensureToolBar()
{
    if (m_windows.isEmpty() || m_toolBar)
        return;

    QWidget *anchor = m_windows.first();
    m_toolBar = new ToolBar(anchor);
    m_sideBar = new SideBar(anchor);
    m_toolBarHost = new TreelandToolBarHost(this, anchor);

    m_toolBar->initToolBar(m_toolBarHost);
    m_sideBar->initSideBar(m_toolBarHost);
    m_toolBar->hide();
    m_sideBar->hide();
}

void TreelandMaskOverlayHost::ensureSelectionWidgets()
{
    if (m_windows.isEmpty() || (m_roundedCornerWidget && m_fineAdjustTips))
        return;

    QWidget *anchor = m_windows.first();
    if (!m_roundedCornerWidget) {
        m_roundedCornerWidget = new RoundedCornerWidget(anchor);
        connect(m_roundedCornerWidget, &RoundedCornerWidget::radiusChanged,
                this, [this](int radius) {
            applyRoundedCornerRadiusToWindows();
            emit roundedCornerRadiusChanged(radius);
            updateFineAdjustTips();
        });
        connect(m_roundedCornerWidget, &RoundedCornerWidget::selectionSizeChanged,
                this, [this](int width, int height) {
            emit selectionSizeChangeRequested(width, height, selectionPixelScale());
        });
        connect(m_roundedCornerWidget, &RoundedCornerWidget::ratioChanged,
                this, [this](int width, int height) {
            emit selectionRatioChangeRequested(width, height);
        });
        connect(m_roundedCornerWidget, &RoundedCornerWidget::panelExpandedChanged,
                this, [this](bool) { updateToolBar(); });
        connect(m_roundedCornerWidget, &RoundedCornerWidget::groupMoved,
                this, [this] { updateToolBar(); });
        applyRoundedCornerRadiusToWindows();
        emit roundedCornerRadiusChanged(m_roundedCornerWidget->radius());
        m_roundedCornerWidget->hide();
    }

    if (!m_fineAdjustTips) {
        m_fineAdjustTips = new FineAdjustTips(anchor);
        m_fineAdjustTips->hide();
    }
}

TreelandMaskWindow *TreelandMaskOverlayHost::windowForGlobalPoint(
        const QPoint &globalPos) const
{
    for (TreelandMaskWindow *window : m_windows) {
        if (window && window->output().logicalGeometry.contains(globalPos))
            return window;
    }

    const QString primaryName = QGuiApplication::primaryScreen()
            ? QGuiApplication::primaryScreen()->name()
            : QString();
    for (TreelandMaskWindow *window : m_windows) {
        if (window && window->output().outputId == primaryName)
            return window;
    }
    return m_windows.value(0);
}

qreal TreelandMaskOverlayHost::selectionPixelScale() const
{
    qreal scale = 1.0;
    for (const OutputFrame &frame : m_frozenFrames.outputs) {
        if (frame.pixelScale <= 0.0)
            continue;
        if (m_selection.isEmpty()
                || m_selection.intersects(frame.output.logicalGeometry)) {
            scale = qMax(scale, frame.pixelScale);
        }
    }
    return scale;
}

void TreelandMaskOverlayHost::updateRoundedCornerWidget()
{
    applyRoundedCornerRadiusToWindows();
    if (!m_roundedCornerWidget)
        return;

    if (!m_finalized || m_selection.isEmpty()
            || m_recordingActive || m_recordModeActive) {
        m_roundedCornerWidget->hide();
        return;
    }

    TreelandMaskWindow *active = selectionOwnerWindow();
    if (!active) {
        m_roundedCornerWidget->hide();
        return;
    }

    if (m_roundedCornerWidget->parentWidget() != active)
        m_roundedCornerWidget->setParent(active);

    const QRect localSelection =
            m_selection.translated(-active->output().logicalGeometry.topLeft());
    const qreal scale = selectionPixelScale();
    m_roundedCornerWidget->setSelectionSize(
            qMax(1, qRound(localSelection.width() * scale)),
            qMax(1, qRound(localSelection.height() * scale)));
    m_roundedCornerWidget->followSelection(localSelection, active->rect());
    m_roundedCornerWidget->show();
    m_roundedCornerWidget->raise();
}

void TreelandMaskOverlayHost::applyRoundedCornerRadiusToWindows()
{
    // Recording always uses a rectangular region. Hide the corner editor when
    // record mode is selected, but also clear the radius painted by every mask
    // window so the region becomes rectangular before the countdown starts.
    const int radius = (m_recordingActive || m_recordModeActive || !m_roundedCornerWidget)
            ? 0
            : m_roundedCornerWidget->radius();
    for (TreelandMaskWindow *window : std::as_const(m_windows)) {
        if (window)
            window->setRoundedCornerRadius(radius);
    }
}

void TreelandMaskOverlayHost::updateFineAdjustTips()
{
    if (!m_fineAdjustTips)
        return;

    if (!m_hintsVisible || m_recordingActive || m_recordModeActive
            || m_annotationToolActive) {
        m_fineAdjustTips->hide();
        return;
    }

    TreelandMaskWindow *owner = m_hintsSelectionMode
            ? selectionOwnerWindow()
            : windowForGlobalPoint(m_globalCursor);
    if (!owner) {
        m_fineAdjustTips->hide();
        return;
    }

    if (m_fineAdjustTips->parentWidget() != owner)
        m_fineAdjustTips->setParent(owner);

    m_fineAdjustTips->setMode(m_hintsSelectionMode ? FineAdjustTips::SelectionMode
                                                   : FineAdjustTips::CursorMode);
    m_fineAdjustTips->setResizeEnabled(
            !(m_roundedCornerWidget && m_roundedCornerWidget->isRatioLocked()));
    m_fineAdjustTips->dockToBottomLeft();

    const QRect tipsRect(m_fineAdjustTips->pos(), m_fineAdjustTips->size());
    QRect blockedRect;
    // Match MainWindow::updateFineAdjustTips(): during the first drag the
    // selection is not finalized yet, but the growing selection rectangle
    // must already hide the hint as soon as it reaches the panel.
    const bool selectionDrag = !m_finalized && !m_selection.isEmpty();
    if ((m_hintsSelectionMode || selectionDrag) && !m_selection.isEmpty()) {
        blockedRect = m_selection.translated(-owner->output().logicalGeometry.topLeft());
    } else if (m_globalCursor.x() >= 0 && m_globalCursor.y() >= 0) {
        blockedRect = QRect(m_globalCursor
                                    - owner->output().logicalGeometry.topLeft(),
                            QSize(1, 1));
    }
    if (tipsRect.intersects(blockedRect)) {
        m_fineAdjustTips->hide();
        return;
    }

    m_fineAdjustTips->show();
    m_fineAdjustTips->raise();
}

void TreelandMaskOverlayHost::updateMagnifier()
{
    // Keep magnifier visibility aligned with X11: it is available while the
    // toolbar is hidden (initial selection and resize/move drag), but not
    // while editing annotations or after the selection becomes ready.
    const bool shouldShow = m_hintsVisible
            && !m_finalized
            && !m_annotationToolActive
            && !m_recordingActive
            && !m_recordModeActive
            && m_hasGlobalCursor;
    if (!shouldShow) {
        hideMagnifier();
        return;
    }

    TreelandMaskWindow *owner = windowForGlobalPoint(m_globalCursor);
    if (!owner) {
        hideMagnifier();
        return;
    }

    const OutputFrame *source = nullptr;
    for (const OutputFrame &frame : m_frozenFrames.outputs) {
        if (frame.output.outputId == owner->output().outputId) {
            source = &frame;
            break;
        }
    }
    if (!source || source->image.isNull()) {
        hideMagnifier();
        return;
    }

    if (!m_magnifier)
        m_magnifier = new ZoomIndicator(owner, false, true);
    else if (m_magnifier->parentWidget() != owner)
        m_magnifier->setParent(owner);

    const QPoint localCursor = m_globalCursor - owner->output().logicalGeometry.topLeft();
    const qreal scale = source->pixelScale > 0.0 ? source->pixelScale : 1.0;
    const QPoint imageCursor(
            qBound(0, qFloor(localCursor.x() * scale), source->image.width() - 1),
            qBound(0, qFloor(localCursor.y() * scale), source->image.height() - 1));
    m_magnifier->setSourceImage(source->image);
    m_magnifier->setCursorPos(imageCursor);
    m_magnifier->setScreenPos(m_globalCursor);

    int magnifierX = localCursor.x() + MagnifierOffset;
    if (magnifierX + ZoomIndicator::PANEL_WIDTH > owner->width())
        magnifierX = localCursor.x() - MagnifierOffset - ZoomIndicator::PANEL_WIDTH;
    magnifierX = qBound(0, magnifierX,
                        qMax(0, owner->width() - ZoomIndicator::PANEL_WIDTH));

    int magnifierY = localCursor.y() + MagnifierOffset;
    if (magnifierY + ZoomIndicator::PANEL_HEIGHT > owner->height())
        magnifierY = localCursor.y() - MagnifierOffset - ZoomIndicator::PANEL_HEIGHT;
    magnifierY = qBound(0, magnifierY,
                        qMax(0, owner->height() - ZoomIndicator::PANEL_HEIGHT));

    m_magnifier->showMagnifier(QPoint(magnifierX, magnifierY));
    m_magnifier->raise();
}

void TreelandMaskOverlayHost::hideMagnifier()
{
    if (m_magnifier)
        m_magnifier->hideMagnifier();
}

void TreelandMaskOverlayHost::updateToolBar()
{
    if (!m_toolBar || !m_sideBar || !m_finalized || m_selection.isEmpty()) {
        if (m_toolBar)
            m_toolBar->hide();
        if (m_sideBar)
            m_sideBar->hide();
        return;
    }

    TreelandMaskWindow *active = selectionOwnerWindow();
    if (!active)
        return;

    if (m_toolBar->parentWidget() != active) {
        m_toolBar->setParent(active);
        m_sideBar->setParent(active);
        if (m_toolBarHost)
            m_toolBarHost->setAnchorWidget(active);
    }

    const QRect localSelection = m_selection.translated(-active->output().logicalGeometry.topLeft());
    if (m_annotationToolActive) {
        // Selection interaction temporarily hides the tool bars. Restore the
        // active annotation options when the selection becomes ready again.
        m_sideBar->showWidget();
        m_sideBar->show();
    }
    m_toolBar->showWidget();
    m_toolBar->adjustSize();

    // X11 keeps the user's manual toolbar position for the rest of the
    // session. Only automatic placement participates in rounded-corner group
    // avoidance.
    if (!m_toolBarManuallyMoved) {
        const QSize toolBarSize = m_toolBar->size();

        int toolBarX = localSelection.center().x() - toolBarSize.width() / 2;
        toolBarX = qBound(ToolBarMargin,
                          toolBarX,
                          qMax(ToolBarMargin, active->width() - toolBarSize.width() - ToolBarMargin));
        int toolBarY = localSelection.top() - toolBarSize.height() - ToolBarGap;
        if (toolBarY < ToolBarMargin)
            toolBarY = localSelection.bottom() + ToolBarGap;
        toolBarY = qBound(ToolBarMargin,
                          toolBarY,
                          qMax(ToolBarMargin, active->height() - toolBarSize.height() - ToolBarMargin));

        if (m_roundedCornerWidget && m_roundedCornerWidget->isVisible()) {
            const QRect groupPlate = m_roundedCornerWidget->isUserMoved()
                    ? m_roundedCornerWidget->currentGroupPlateRect()
                    : m_roundedCornerWidget->groupPlateRect(localSelection, active->rect());
            const QRect toolbarRect(QPoint(toolBarX, toolBarY), toolBarSize);
            if (!groupPlate.isEmpty() && toolbarRect.intersects(groupPlate)) {
                toolBarY = qMin(groupPlate.bottom() + 1 + ToolBarGap,
                                qMax(ToolBarMargin,
                                     active->height() - toolBarSize.height() - ToolBarMargin));
            }
        }
        m_toolBar->showAt(QPoint(toolBarX, toolBarY));
    }
    m_toolBar->raise();
    if (m_sideBar->isVisible()) {
        // Keep the full DFloatingWidget geometry. SideBarWidget reserves a 6px
        // shadow/rounded-corner margin around its contents; shrinking the
        // wrapper to contentsRect clips the right and bottom rounded corners.
        const QSize sideBarSize = m_sideBar->size();
        int sideBarX = m_toolBar->x() + m_toolBar->width() - sideBarSize.width();
        sideBarX = qBound(ToolBarMargin,
                          sideBarX,
                          qMax(ToolBarMargin, active->width() - sideBarSize.width() - ToolBarMargin));
        int sideBarY = m_toolBar->y() - sideBarSize.height() - ToolBarGap;
        if (sideBarY < ToolBarMargin)
            sideBarY = m_toolBar->y() + m_toolBar->height() + ToolBarGap;
        sideBarY = qBound(ToolBarMargin,
                          sideBarY,
                          qMax(ToolBarMargin, active->height() - sideBarSize.height() - ToolBarMargin));
        m_sideBar->showAt(QPoint(sideBarX, sideBarY));
        m_sideBar->raise();
    }
}

void TreelandMaskOverlayHost::handleToolChanged(const QString &tool)
{
    if (!m_sideBar || !m_annotationEditor)
        return;
    if (tool.isEmpty()) {
        // Match X11: clicking the active annotation tool again exits drawing
        // mode, keeps existing annotations, and restores normal selection
        // interaction.
        m_annotationToolActive = false;
        m_annotationEditor->setTool(AnnotationTool::Pointer);
        m_sideBar->hide();
        updateToolBar();
        updateFineAdjustTips();
        updateMagnifier();
        return;
    }
    if (tool == QStringLiteral("aiassistant")) {
        m_annotationToolActive = false;
        m_sideBar->changeShotToolFunc(tool);
        m_sideBar->showWidget();
        m_sideBar->show();
        updateToolBar();
        updateFineAdjustTips();
        updateMagnifier();
        return;
    }
    if (!isTreeLandAnnotationTool(tool))
        return;

    m_annotationToolActive = true;
    if (m_roundedCornerWidget)
        m_roundedCornerWidget->collapsePanels();
    m_sideBar->showWidget();
    m_sideBar->changeShotToolFunc(tool);
    m_sideBar->show();

    if (tool == "gio") {
        const QString shape = ConfigSettings::instance()->getValue("shape", "current").toString();
        m_annotationEditor->setTool(shape == QStringLiteral("oval")
                                             ? AnnotationTool::Ellipse
                                             : AnnotationTool::Rectangle);
    } else if (tool == "rectangle") {
        m_annotationEditor->setTool(AnnotationTool::Rectangle);
    } else if (tool == "oval") {
        m_annotationEditor->setTool(AnnotationTool::Ellipse);
    } else if (tool == "line") {
        m_annotationEditor->setTool(AnnotationTool::Line);
    } else if (tool == "arrow") {
        m_annotationEditor->setTool(AnnotationTool::Arrow);
    } else if (tool == "pen") {
        m_annotationEditor->setTool(AnnotationTool::Pen);
    } else if (tool == "effect") {
        ensureEffectSource();
        m_annotationEditor->setTool(AnnotationTool::Effect);
    } else if (tool == "sequence") {
        m_annotationEditor->setTool(AnnotationTool::Sequence);
    } else if (tool == "text") {
        m_annotationEditor->setTool(AnnotationTool::Text);
    }

    updateToolBar();
    updateFineAdjustTips();
    updateMagnifier();
}

void TreelandMaskOverlayHost::ensureEffectSource()
{
    if (m_effectSourceReady || m_frozenFrames.isEmpty() || !m_annotationEditor)
        return;

    // Effect annotations need the unfiltered frozen desktop as their source.
    // Compose it lazily so ordinary rectangle/text screenshots do not pay for
    // a full virtual-desktop image that they never use.
    ScreenshotExportService exportService;
    const Result effectSource = exportService.render(m_frozenFrames,
                                                     m_frozenFrames.virtualDesktopRect,
                                                     nullptr);
    if (!effectSource.isValid())
        return;

    m_annotationEditor->setEffectSourceImage(effectSource.image);
    m_effectSourceReady = true;
}

void TreelandMaskOverlayHost::undoAnnotation()
{
    if (m_annotationEditor)
        m_annotationEditor->undo();
}

void TreelandMaskOverlayHost::destroyWindows()
{
    hideMagnifier();
    delete m_magnifier;
    m_magnifier = nullptr;

    if (m_toolBar)
        m_toolBar->hide();
    if (m_sideBar)
        m_sideBar->hide();

    delete m_roundedCornerWidget;
    m_roundedCornerWidget = nullptr;
    delete m_fineAdjustTips;
    m_fineAdjustTips = nullptr;

    m_annotationEditor->clearViews();
    for (TreelandMaskWindow *window : m_windows) {
        if (!window)
            continue;
        window->closeSilently();
        delete window;
    }
    m_windows.clear();
    m_selection = QRect();
    m_frozenFrames = FrozenFrameSet();
    m_effectSourceReady = false;
    m_finalized = false;
    m_recordingActive = false;
    m_recordModeActive = false;
    m_hintsVisible = false;
    m_hintsSelectionMode = false;
    m_hasGlobalCursor = false;
    m_toolBarManuallyMoved = false;
    m_annotationToolActive = false;
    m_prepared = false;
    m_inputReadyPending = false;
    m_inputReadyChecks = 0;
    if (m_toolBarHost) {
        delete m_toolBarHost;
        m_toolBarHost = nullptr;
    }
    m_toolBar = nullptr;
    m_sideBar = nullptr;
}

} // namespace Dsr::Screenshot
