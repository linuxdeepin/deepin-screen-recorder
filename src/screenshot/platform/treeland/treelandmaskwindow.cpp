// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandmaskwindow.h"

#include "../../core/coordinatemapper.h"
#include "../../core/selectionoverlaygeometry.h"
#include "../../../utils/baseutils.h"
#include "../../../utils/log.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPainter>
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

void TreelandMaskWindow::setSelectionHintOwner(bool owner)
{
    if (m_selectionHintOwner == owner)
        return;
    m_selectionHintOwner = owner;
    update();
}

void TreelandMaskWindow::setRecordingMode(bool enabled,
                                          const QRect &globalSelection,
                                          bool dimOutside)
{
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

void TreelandMaskWindow::drawSelectionOutline(QPainter &painter,
                                              const QRect &globalRect,
                                              const QColor &color,
                                              int width,
                                              bool dashed) const
{
    const QList<QLineF> lines =
            SelectionOverlayGeometry::strokedOutlineForOutput(
                    globalRect, m_output.logicalGeometry, width);
    if (lines.isEmpty())
        return;

    painter.save();
    // X11 的选区边框为 1px 白色 DashLine，并关闭抗锯齿，避免虚线被灰边糊开。
    painter.setRenderHint(QPainter::Antialiasing, !dashed);
    QPen pen(color, width);
    if (dashed) {
        pen.setStyle(Qt::DashLine);
        pen.setDashOffset(0);
    }
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

    if (m_recordingMode) {
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
                // 与 X11 RecorderRegionShow 保持一致：2px 白色虚线，
                // dash pattern 为 1,2，启用抗锯齿，并仅绘制真实选区边界。
                QPen recordingPen(Qt::white, 2.0);
                recordingPen.setStyle(Qt::DashLine);
                QVector<qreal> dashes;
                dashes << 1 << 2;
                recordingPen.setDashPattern(dashes);
                recordingPen.setDashOffset(0);

                painter.save();
                painter.setClipping(false);
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setBrush(Qt::NoBrush);
                painter.setPen(recordingPen);
                const QRect recordingOutline =
                        m_globalRecordingSelection.adjusted(-1, -1, 1, 1);
                const QList<QLineF> recordingLines =
                        SelectionOverlayGeometry::strokedOutlineForOutput(
                                recordingOutline, m_output.logicalGeometry, 2.0);
                for (const QLineF &line : recordingLines)
                    painter.drawLine(line);
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
    if (visibleSelection.isEmpty()) {
        painter.fillRect(rect(), QColor(0, 0, 0, 110));
    } else {
        const QColor mask(0, 0, 0, 120);
        painter.fillRect(QRect(0, 0, width(), visibleSelection.top()), mask);
        painter.fillRect(QRect(0, visibleSelection.bottom() + 1,
                               width(), height() - visibleSelection.bottom() - 1), mask);
        painter.fillRect(QRect(0, visibleSelection.top(),
                               visibleSelection.left(), visibleSelection.height()), mask);
        painter.fillRect(QRect(visibleSelection.right() + 1, visibleSelection.top(),
                               width() - visibleSelection.right() - 1,
                               visibleSelection.height()), mask);
    }

    if (!m_finalized && !m_snapHint.isEmpty()) {
        painter.fillRect(m_snapHint, QColor(46, 132, 255, 42));
        drawSelectionOutline(painter, m_globalSnapHint, QColor(75, 160, 255), 2, false);
    }

    if (!visibleSelection.isEmpty()) {
        drawSelectionOutline(painter, m_globalSelection, Qt::white, 1, true);

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

    if (m_selectionHintOwner) {
        QString hint = QStringLiteral("拖拽框选窗口或区域  ·  Enter 复制  ·  Ctrl+S 保存  ·  Esc 取消");
        if (m_finalized) {
            hint = QStringLiteral("%1 × %2 px  ·  可拖动选区或拖拽控制点调整大小")
                    .arg(m_globalSelection.width())
                    .arg(m_globalSelection.height());
        }

        QFont font = painter.font();
        font.setPixelSize(13);
        painter.setFont(font);
        const QRect hintRect(0, 22, width(), 36);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 135));
        const int textWidth = painter.fontMetrics().horizontalAdvance(hint) + 36;
        painter.drawRoundedRect(QRect((width() - textWidth) / 2, hintRect.top(), textWidth, 32), 6, 6);
        painter.setPen(Qt::white);
        painter.drawText(hintRect, Qt::AlignHCenter | Qt::AlignVCenter, hint);
    }

}

void TreelandMaskWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    const QPoint globalPos = toGlobal(eventPosition(event));
    emit pointerPressed(globalPos);
    event->accept();
}

void TreelandMaskWindow::mouseMoveEvent(QMouseEvent *event)
{
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
    if (event->key() == Qt::Key_Escape) {
        emit cancelRequested();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        emit copyRequested();
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
