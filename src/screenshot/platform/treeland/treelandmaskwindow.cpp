// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandmaskwindow.h"

#include "../../core/coordinatemapper.h"
#include "../../core/selectionoverlaygeometry.h"
#include "../../../utils/baseutils.h"
#include "../../../utils/log.h"

#include <QAbstractSpinBox>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QRegion>
#include <QResizeEvent>
#include <QScreen>
#include <QVector>
#include <QWindow>
#include <QtMath>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <wayland-client-protocol.h>

struct xdg_toplevel;

namespace Dsr::Screenshot {

namespace {

struct ::xdg_toplevel *getXdgToplevel(QWindow *window)
{
    if (!window || !window->handle())
        return nullptr;

    auto *waylandWindow = static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (!waylandWindow)
        return nullptr;
    return waylandWindow->surfaceRole<::xdg_toplevel>();
}

QPoint eventPosition(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position().toPoint();
#else
    return event->pos();
#endif
}

QScreen *screenForOutput(const OutputDescriptor &output)
{
    const auto screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        if (screen->name() == output.outputId)
            return screen;
    }
    return QGuiApplication::primaryScreen();
}

} // namespace

TreelandMaskWindow::TreelandMaskWindow(const OutputDescriptor &output, QWidget *parent)
    : QWidget(parent)
    , m_output(output)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::CrossCursor);
    setWindowTitle(QStringLiteral("Treeland Screenshot Mask"));

    if (QScreen *screen = screenForOutput(output))
        setScreen(screen);
    setGeometry(output.logicalGeometry);
}

TreelandMaskWindow::~TreelandMaskWindow() = default;

OutputDescriptor TreelandMaskWindow::output() const
{
    return m_output;
}

void TreelandMaskWindow::setFrozenFrame(const QImage &image)
{
    m_sourceImage = image;
    update();
}

void TreelandMaskWindow::setSelection(const QRect &globalSelection)
{
    const QRect normalizedSelection = globalSelection.normalized();
    const QRect localSelection =
            CoordinateMapper::globalToOutputLocal(normalizedSelection, m_output).intersected(rect());
    if (m_globalSelection == normalizedSelection && m_selection == localSelection)
        return;
    m_globalSelection = normalizedSelection;
    m_selection = localSelection;
    update();
}

void TreelandMaskWindow::setSnapHint(const QRect &globalRegion)
{
    const QRect normalizedRegion = globalRegion.normalized();
    const QRect snapHint =
            CoordinateMapper::globalToOutputLocal(normalizedRegion, m_output).intersected(rect());
    if (m_globalSnapHint == normalizedRegion && m_snapHint == snapHint)
        return;
    m_globalSnapHint = normalizedRegion;
    m_snapHint = snapHint;
    update();
}

void TreelandMaskWindow::setSelectionFinalized(bool finalized)
{
    if (m_finalized == finalized)
        return;
    m_finalized = finalized;
    update();
}

void TreelandMaskWindow::setRecordingMode(bool enabled,
                                          const QRect &globalSelection,
                                          bool dimOutside)
{
    if (enabled)
        m_scrollMode = false;
    m_recordingMode = enabled;
    m_recordingDimOutside = enabled && dimOutside;
    m_globalRecordingSelection = enabled ? globalSelection.normalized() : QRect();
    m_recordingSelection = enabled
            ? CoordinateMapper::globalToOutputLocal(m_globalRecordingSelection, m_output)
                      .intersected(rect())
            : QRect();
    qCDebug(dsrApp) << "Recording overlay mode:" << enabled
                    << "global selection:" << globalSelection
                    << "local selection:" << m_recordingSelection
                    << "dim outside:" << m_recordingDimOutside;
    repaint();
}

void TreelandMaskWindow::setScrollMode(bool enabled,
                                       const QRect &globalSelection,
                                       bool showFrame)
{
    if (enabled)
        m_recordingMode = false;
    m_scrollMode = enabled;
    m_scrollFrameVisible = enabled && showFrame;
    m_recordingDimOutside = enabled;
    m_globalRecordingSelection = enabled ? globalSelection.normalized() : QRect();
    m_recordingSelection = enabled
            ? CoordinateMapper::globalToOutputLocal(m_globalRecordingSelection, m_output)
                      .intersected(rect())
            : QRect();
    qCDebug(dsrApp) << "Scroll overlay mode:" << enabled
                    << "global selection:" << globalSelection
                    << "local selection:" << m_recordingSelection;
    repaint();
}

void TreelandMaskWindow::setInputRegion(const QRegion &region)
{
    if (!windowHandle())
        return;

    auto *waylandWindow =
            static_cast<QtWaylandClient::QWaylandWindow *>(windowHandle()->handle());
    if (!waylandWindow || !waylandWindow->display() || !waylandWindow->wlSurface())
        return;

    wl_region *waylandRegion = waylandWindow->display()->createRegion(region);
    if (!waylandRegion)
        return;

    wl_surface_set_input_region(waylandWindow->wlSurface(), waylandRegion);
    wl_region_destroy(waylandRegion);
    wl_surface_commit(waylandWindow->wlSurface());
}

void TreelandMaskWindow::resetInputRegion()
{
    if (!windowHandle())
        return;

    auto *waylandWindow =
            static_cast<QtWaylandClient::QWaylandWindow *>(windowHandle()->handle());
    if (!waylandWindow || !waylandWindow->wlSurface())
        return;

    wl_surface_set_input_region(waylandWindow->wlSurface(), nullptr);
    wl_surface_commit(waylandWindow->wlSurface());
}

void TreelandMaskWindow::setCursorIntent(CursorIntent intent)
{
    if (m_cursorIntent == intent)
        return;
    m_cursorIntent = intent;
    const QCursor *overrideCursor = QApplication::overrideCursor();
    qCDebug(dsrApp) << "Overlay cursor intent:" << static_cast<int>(intent)
                    << "application override:"
                    << (overrideCursor ? static_cast<int>(overrideCursor->shape()) : -1);
    switch (intent) {
    case CursorIntent::Move:
        setCursor(Qt::OpenHandCursor);
        break;
    case CursorIntent::Grabbing:
        setCursor(Qt::ClosedHandCursor);
        break;
    case CursorIntent::ResizeHorizontal:
        setCursor(Qt::SizeHorCursor);
        break;
    case CursorIntent::ResizeVertical:
        setCursor(Qt::SizeVerCursor);
        break;
    case CursorIntent::ResizeDiagonalForward:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case CursorIntent::ResizeDiagonalBackward:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case CursorIntent::Text:
        setCursor(Qt::IBeamCursor);
        break;
    case CursorIntent::Rotate:
        setCursor(BaseUtils::setCursorShape(QStringLiteral("rotate")));
        break;
    case CursorIntent::Default:
    case CursorIntent::Cross:
        setCursor(Qt::CrossCursor);
        break;
    }
}

void TreelandMaskWindow::setRoundedCornerRadius(int radius)
{
    const int normalized = qMax(0, radius);
    if (m_roundedCornerRadius == normalized)
        return;
    m_roundedCornerRadius = normalized;
    update();
}

void TreelandMaskWindow::setAnnotationView(QWidget *widget)
{
    m_annotationView = widget;
    if (m_annotationView) {
        m_annotationView->setGeometry(rect());
        m_annotationView->show();
        m_annotationView->raise();
    }
}

bool TreelandMaskWindow::prepareWindow()
{
    if (!windowHandle())
        create();
    if (!windowHandle())
        return false;

    auto *waylandWindow =
            windowHandle()->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!waylandWindow)
        return false;

    if (!m_roleCreatedConnected) {
        connect(waylandWindow,
                &QNativeInterface::Private::QWaylandWindow::surfaceRoleCreated,
                this,
                [this] {
                    if (windowHandle())
                        emit windowRoleCreated(windowHandle());
                },
                Qt::DirectConnection);
        m_roleCreatedConnected = true;
    }

    // QtWayland creates xdg_toplevel from QWaylandWindow::initWindow(), which
    // is called synchronously by setVisible(true). surfaceRoleCreated is
    // emitted before the first wl_surface commit, so the tag must be sent from
    // that callback rather than after expose.
    if (windowHandle()) {
        windowHandle()->setProperty("_d_dwayland_window-type", "override");
    }
    showFullScreen();
    return getXdgToplevel(windowHandle()) != nullptr;
}

void TreelandMaskWindow::showWindow()
{
    showFullScreen();
    raise();
    activateWindow();
    setFocus(Qt::ActiveWindowFocusReason);
}

void TreelandMaskWindow::closeSilently()
{
    m_silentClose = true;
    close();
}

void TreelandMaskWindow::drawSelectionFrame(QPainter &painter,
                                            const QRect &globalRect) const
{
    const QList<QLineF> lines =
            SelectionOverlayGeometry::dashedFrameForOutput(
                    globalRect, m_output.logicalGeometry);
    if (lines.isEmpty())
        return;

    painter.save();
    // X11 截图/滚动截图选区边框：1px 白色 DashLine，关闭抗锯齿，绘制在
    // 选区外侧 1px 处，避免虚线进入截图/长图内容。
    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen pen(Qt::white, 1.0);
    pen.setStyle(Qt::DashLine);
    pen.setDashOffset(0);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(pen);
    for (const QLineF &line : lines)
        painter.drawLine(line);
    painter.restore();
}

void TreelandMaskWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (m_recordingMode || m_scrollMode) {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(rect(), Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        if (m_recordingDimOutside) {
            const QColor maskColor(0, 0, 0, 105);
            if (m_recordingSelection.isEmpty()) {
                // 不属于选中区域的输出也要保留整屏遮罩。多屏录制时，
                // 只有选区覆盖的输出需要挖空，其余输出应全部变暗。
                painter.fillRect(rect(), maskColor);
            } else {
                QRegion outside(rect());
                outside -= QRegion(m_recordingSelection);
                painter.setClipRegion(outside);
                painter.fillRect(rect(), maskColor);
                painter.save();
                painter.setClipping(false);
                painter.setBrush(Qt::NoBrush);
                // Recording mode paints no frame here: this window is part of
                // the captured output, so any border would leak into the video.
                if (m_scrollMode) {
                    if (m_scrollFrameVisible) {
                        // 滚动截图沿用 X11 截图选区边框，而不是录屏虚线。
                        const QList<QLineF> scrollLines =
                                SelectionOverlayGeometry::dashedFrameForOutput(
                                        m_globalRecordingSelection,
                                        m_output.logicalGeometry);
                        QPen scrollPen(Qt::white, 1.0);
                        scrollPen.setStyle(Qt::DashLine);
                        scrollPen.setDashOffset(0);
                        painter.setRenderHint(QPainter::Antialiasing, false);
                        painter.setPen(scrollPen);
                        for (const QLineF &line : scrollLines)
                            painter.drawLine(line);
                    }
                }
                painter.restore();
            }
        }
        return;
    }

    if (!m_sourceImage.isNull())
        painter.drawImage(rect(), m_sourceImage);
    else
        painter.fillRect(rect(), QColor(32, 32, 32));

    const QRect visibleSelection = m_selection.intersected(rect());
    // X11 auto-recognizes the window under the cursor and draws the normal
    // capture frame around it before the click. Reuse the snap region as the
    // preview selection instead of painting a separate highlight.
    const bool snapPreviewActive =
            visibleSelection.isEmpty() && !m_finalized && !m_snapHint.isEmpty();
    const QRect visibleRegion = snapPreviewActive ? m_snapHint : visibleSelection;
    const QRect frameRectGlobal =
            snapPreviewActive ? m_globalSnapHint : m_globalSelection;
    if (visibleRegion.isEmpty()) {
        painter.fillRect(rect(), QColor(0, 0, 0, 110));
    } else {
        const QColor mask(0, 0, 0, 120);
        painter.fillRect(QRect(0, 0, width(), visibleRegion.top()), mask);
        painter.fillRect(QRect(0, visibleRegion.bottom() + 1,
                               width(), height() - visibleRegion.bottom() - 1), mask);
        painter.fillRect(QRect(0, visibleRegion.top(),
                               visibleRegion.left(), visibleRegion.height()), mask);
        painter.fillRect(QRect(visibleRegion.right() + 1, visibleRegion.top(),
                               width() - visibleRegion.right() - 1,
                               visibleRegion.height()), mask);
    }

    if (!visibleRegion.isEmpty()) {
        if (m_roundedCornerRadius > 0) {
            const QRect localSelection =
                    CoordinateMapper::globalToOutputLocal(frameRectGlobal, m_output);
            const qreal radius = m_roundedCornerRadius;
            QPainterPath squarePath;
            squarePath.addRect(QRectF(localSelection));
            QPainterPath roundedPath;
            roundedPath.addRoundedRect(QRectF(localSelection), radius, radius);
            painter.save();
            painter.setClipRect(rect());
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, 120));
            painter.drawPath(squarePath.subtracted(roundedPath));
            painter.restore();
        }

        // Draw the selection outline after the rounded-corner mask. Otherwise
        // the extra corner mask covers the white dashes and makes four corner
        // segments look grey.
        drawSelectionFrame(painter, frameRectGlobal);

        if (m_finalized) {
            painter.setBrush(QColor(245, 248, 252));
            painter.setPen(QPen(QColor(43, 124, 228), 1));
            const QList<QPoint> localHandles =
                    SelectionOverlayGeometry::handlesForOutput(
                            m_globalSelection, m_output.logicalGeometry);
            for (const QPoint &localPoint : localHandles)
                painter.drawRect(QRect(localPoint - QPoint(4, 4), QSize(9, 9)));
        }
    }

}

void TreelandMaskWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    const QPoint globalPos = toGlobal(eventPosition(event));
    emit drawingModifiersChanged(event->modifiers());
    emit pointerPressed(globalPos);
    event->accept();
}

void TreelandMaskWindow::mouseMoveEvent(QMouseEvent *event)
{
    emit drawingModifiersChanged(event->modifiers());
    emit pointerMoved(toGlobal(eventPosition(event)));
    event->accept();
}

void TreelandMaskWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    const QPoint globalPos = toGlobal(eventPosition(event));
    emit drawingModifiersChanged(event->modifiers());
    emit pointerReleased(globalPos);
    event->accept();
}

void TreelandMaskWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    emit pointerDoubleClicked(toGlobal(eventPosition(event)));
    event->accept();
}

void TreelandMaskWindow::keyPressEvent(QKeyEvent *event)
{
    // Ignored key events from child editors can propagate to the mask.
    for (QWidget *focus = QApplication::focusWidget(); focus && focus->window() == this;
         focus = focus->parentWidget()) {
        if (qobject_cast<QLineEdit *>(focus) || qobject_cast<QAbstractSpinBox *>(focus)
                || qobject_cast<QPlainTextEdit *>(focus)) {
            QWidget::keyPressEvent(event);
            return;
        }
    }

    const int key = event->key();
    const Qt::KeyboardModifiers modifiers = event->modifiers() & ~Qt::KeypadModifier;
    if (m_recordingMode) {
        if (!event->isAutoRepeat() && key == Qt::Key_R
                && modifiers == (Qt::ControlModifier | Qt::AltModifier)) {
            emit stopRecordingRequested();
            return;
        }
        QWidget::keyPressEvent(event);
        return;
    }

    if (key == Qt::Key_Escape && modifiers == Qt::NoModifier) {
        emit cancelRequested();
        return;
    }
    if ((key == Qt::Key_Return || key == Qt::Key_Enter) && modifiers == Qt::NoModifier) {
        emit enterRequested();
        return;
    }
    if (event->matches(QKeySequence::Save)) {
        emit confirmRequested();
        return;
    }
    if (event->matches(QKeySequence::Copy)) {
        emit copyRequested();
        return;
    }
    if (event->matches(QKeySequence::Undo)) {
        emit undoRequested();
        return;
    }
    if (key == Qt::Key_Z && modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
        emit undoAllRequested();
        return;
    }
    if ((key == Qt::Key_Delete || key == Qt::Key_Backspace) && modifiers == Qt::NoModifier) {
        emit deleteAnnotationRequested();
        return;
    }
    if (key == Qt::Key_F1 && modifiers == Qt::NoModifier) {
        if (!event->isAutoRepeat())
            emit helpRequested();
        return;
    }
    if ((key == Qt::Key_Slash && modifiers == (Qt::ControlModifier | Qt::ShiftModifier))
            || (key == Qt::Key_Question && (modifiers & ~Qt::ShiftModifier) == Qt::ControlModifier)) {
        if (!event->isAutoRepeat())
            emit shortcutPanelRequested();
        return;
    }

    const char *tool = nullptr;
    if (modifiers == Qt::AltModifier) {
        switch (key) {
        case Qt::Key_P: tool = "pinScreenshots"; break;
        case Qt::Key_A: tool = "aiassistant"; break;
        case Qt::Key_S: tool = "sequence"; break;
        case Qt::Key_I: tool = "scrollShot"; break;
        case Qt::Key_O: tool = "ocr"; break;
        case Qt::Key_T: tool = "tableRecogn"; break;
        }
    } else if (modifiers == Qt::NoModifier) {
        switch (key) {
        case Qt::Key_R: tool = "rect"; break;
        case Qt::Key_O: tool = "circ"; break;
        case Qt::Key_L: tool = "line"; break;
        case Qt::Key_X: tool = "arrow"; break;
        case Qt::Key_B: tool = "effect"; break;
        case Qt::Key_P: tool = "pen"; break;
        case Qt::Key_T: tool = "text"; break;
        case Qt::Key_F3: tool = "option"; break;
        case Qt::Key_C: tool = "camera"; break;
        case Qt::Key_K: tool = "keyBoard"; break;
        }
    }
    if (tool) {
        if (!event->isAutoRepeat())
            emit toolShortcutRequested(QString::fromLatin1(tool));
        return;
    }

    if ((key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down)
            && (modifiers == Qt::NoModifier || modifiers == Qt::ControlModifier
                || modifiers == (Qt::ControlModifier | Qt::ShiftModifier))) {
        emit arrowShortcutRequested(static_cast<Qt::Key>(key), modifiers);
        return;
    }

    if (modifiers == Qt::NoModifier) {
        QPoint delta;
        switch (key) {
        case Qt::Key_A:
            delta = QPoint(-1, 0);
            break;
        case Qt::Key_D:
            delta = QPoint(1, 0);
            break;
        case Qt::Key_W:
            delta = QPoint(0, -1);
            break;
        case Qt::Key_S:
            delta = QPoint(0, 1);
            break;
        default:
            break;
        }
        if (!delta.isNull()) {
            emit selectionMoveRequested(delta);
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

QPoint TreelandMaskWindow::toGlobal(const QPoint &localPos) const
{
    return CoordinateMapper::outputLocalToGlobal(localPos, m_output);
}

void TreelandMaskWindow::closeEvent(QCloseEvent *event)
{
    if (!m_silentClose && !m_cancelEmitted) {
        m_cancelEmitted = true;
        emit cancelRequested();
    }
    QWidget::closeEvent(event);
}

void TreelandMaskWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_annotationView)
        m_annotationView->setGeometry(rect());
}


} // namespace Dsr::Screenshot
