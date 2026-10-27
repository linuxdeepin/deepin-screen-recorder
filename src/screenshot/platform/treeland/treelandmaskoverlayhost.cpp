// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandmaskoverlayhost.h"

#include "treelandannotationeditor.h"
#include "treelandmaskwindow.h"
#include "widgets/sidebar.h"
#include "widgets/toolbar.h"
#include "widgets/toolbarhost.h"
#include "../../core/selectionoverlaygeometry.h"
#include "../../../utils/configsettings.h"
#include "../../../utils/log.h"

#include <QTimer>
#include <QWindow>

namespace Dsr::Screenshot {

namespace {

constexpr int ToolBarMargin = 8;
constexpr int ToolBarGap = 8;
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
            || tool == QStringLiteral("effect");
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

    void aiAssistantSelected(int) override
    {
    }

    void toolBarPointRequested() override
    {
    }

    void switchToShotUI() override
    {
    }

    void switchToRecordUI() override
    {
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
};

TreelandMaskOverlayHost::TreelandMaskOverlayHost(QObject *parent)
    : IScreenshotOverlayHost(parent)
    , m_annotationEditor(new TreelandAnnotationEditor(this))
{
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
        connect(window, &TreelandMaskWindow::confirmRequested,
                this, &IScreenshotOverlayHost::confirmRequested);
        connect(window, &TreelandMaskWindow::copyRequested,
                this, &IScreenshotOverlayHost::copyRequested);
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
    for (const OutputFrame &frame : frames.outputs) {
        for (TreelandMaskWindow *window : m_windows) {
            if (window && window->output().outputId == frame.output.outputId) {
                window->setFrozenFrame(frame.image);
                break;
            }
        }
    }
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
    TreelandMaskWindow *hintOwner = selectionOwnerWindow();
    for (TreelandMaskWindow *window : m_windows) {
        if (!window)
            continue;
        window->setSelection(m_selection);
        window->setSelectionHintOwner(window == hintOwner);
    }
    updateToolBar();
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
    updateToolBar();
}

void TreelandMaskOverlayHost::setCursorIntent(CursorIntent intent)
{
    for (TreelandMaskWindow *window : m_windows) {
        if (window)
            window->setCursorIntent(intent);
    }
}

void TreelandMaskOverlayHost::close()
{
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
    }
    if (sideBar) {
        sideBar->hide();
        sideBar->setEnabled(false);
    }
    if (toolBarHost) {
        disconnect(toolBarHost, nullptr, this, nullptr);
        toolBarHost->deleteLater();
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
    m_toolBar->showAt(QPoint(toolBarX, toolBarY));
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
    if (!isTreeLandAnnotationTool(tool))
        return;

    m_annotationToolActive = true;
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
        m_annotationEditor->setTool(AnnotationTool::Effect);
    }

    updateToolBar();
}

void TreelandMaskOverlayHost::undoAnnotation()
{
    if (m_annotationEditor)
        m_annotationEditor->undo();
}

void TreelandMaskOverlayHost::destroyWindows()
{
    if (m_toolBar)
        m_toolBar->hide();
    if (m_sideBar)
        m_sideBar->hide();

    for (TreelandMaskWindow *window : m_windows) {
        if (!window)
            continue;
        window->closeSilently();
        delete window;
    }
    m_windows.clear();
    m_annotationEditor->clearViews();
    m_selection = QRect();
    m_finalized = false;
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
