// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandmaskoverlayhost.h"

#include "treelandmaskwindow.h"

#include <QTimer>
#include <QWindow>

namespace Dsr::Screenshot {

TreelandMaskOverlayHost::TreelandMaskOverlayHost(QObject *parent)
    : IScreenshotOverlayHost(parent)
{
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
    for (const OutputDescriptor &output : outputs) {
        auto *window = new TreelandMaskWindow(output);
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
        m_windows.append(window);
    }
    return true;
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
    QTimer::singleShot(0, this, [this]() { emit inputReady(); });
}

void TreelandMaskOverlayHost::setSelection(const QRect &globalSelection)
{
    m_selection = globalSelection;
    for (TreelandMaskWindow *window : m_windows) {
        if (window)
            window->setSelection(globalSelection);
    }
    updateToolbarVisibility();
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
    m_finalized = finalized;
    for (TreelandMaskWindow *window : m_windows) {
        if (window)
            window->setSelectionFinalized(finalized);
    }
    updateToolbarVisibility();
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
    for (TreelandMaskWindow *window : m_windows) {
        if (!window)
            continue;
        window->closeSilently();
        window->deleteLater();
    }
    m_windows.clear();
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

void TreelandMaskOverlayHost::updateToolbarVisibility()
{
    TreelandMaskWindow *active = toolbarWindow();
    for (TreelandMaskWindow *window : m_windows) {
        if (window)
            window->setToolbarVisible(m_finalized && window == active);
    }
}

void TreelandMaskOverlayHost::destroyWindows()
{
    for (TreelandMaskWindow *window : m_windows) {
        if (!window)
            continue;
        window->closeSilently();
        delete window;
    }
    m_windows.clear();
}

} // namespace Dsr::Screenshot
