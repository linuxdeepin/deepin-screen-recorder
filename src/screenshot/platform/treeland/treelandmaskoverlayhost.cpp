// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandmaskoverlayhost.h"

#include "treelandannotationeditor.h"
#include "treelandmaskwindow.h"
#include "widgets/sidebar.h"
#include "widgets/toolbar.h"
#include "widgets/toolbarhost.h"
#include "../../../utils/configsettings.h"
#include "../../../utils/log.h"

#include <QTimer>
#include <QWindow>

namespace Dsr::Screenshot {

namespace {

constexpr int ToolBarMargin = 8;
constexpr int ToolBarGap = 8;

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
        return QPoint();
    }

    QPoint toolBarStartPressPoint() const override
    {
        return QPoint();
    }

    bool moveToolBars(const QPoint &, const QPoint &) override
    {
        return false;
    }

private:
    TreelandMaskOverlayHost *m_overlay = nullptr;
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

void TreelandMaskOverlayHost::show()
{
    if (m_windows.isEmpty())
        return;

    for (TreelandMaskWindow *window : m_windows) {
        window->prepareWindow();
        window->showWindow();
    }
    updateToolBar();
    QTimer::singleShot(0, this, [this]() { emit inputReady(); });
}

void TreelandMaskOverlayHost::setSelection(const QRect &globalSelection)
{
    if (m_selection == globalSelection)
        return;
    m_selection = globalSelection;
    for (TreelandMaskWindow *window : m_windows) {
        if (window)
            window->setSelection(globalSelection);
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
}

TreelandMaskWindow *TreelandMaskOverlayHost::toolbarWindow() const
{
    TreelandMaskWindow *best = nullptr;
    qint64 bestArea = -1;
    for (TreelandMaskWindow *window : m_windows) {
        if (!window)
            continue;
        const QRect intersection = m_selection.intersected(window->output().logicalGeometry);
        const qint64 area = static_cast<qint64>(intersection.width()) * intersection.height();
        if (area > bestArea) {
            best = window;
            bestArea = area;
        }
    }
    return best;
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

    TreelandMaskWindow *active = toolbarWindow();
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
        // SideBar has no layout of its own; adjustSize() would collapse it to
        // the empty DLabel size and clip SideBarWidget outside the output.
        m_sideBar->resize(m_sideBar->getInnerWidgetRect().size());
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
    m_sideBar->resize(m_sideBar->getInnerWidgetRect().size());
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
    if (m_toolBarHost) {
        delete m_toolBarHost;
        m_toolBarHost = nullptr;
    }
    m_toolBar = nullptr;
    m_sideBar = nullptr;
}

} // namespace Dsr::Screenshot
