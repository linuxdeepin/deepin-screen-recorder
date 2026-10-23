// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandmaskwindow.h"

#include "../../core/coordinatemapper.h"
#include "../../../utils/log.h"

#include <QCloseEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <QtMath>

namespace Dsr::Screenshot {

namespace {

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
    setAttribute(Qt::WA_OpaquePaintEvent, true);
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
    const QRect localSelection =
            CoordinateMapper::globalToOutputLocal(globalSelection, m_output).intersected(rect());
    if (m_selection == localSelection)
        return;
    m_selection = localSelection;
    update();
}

void TreelandMaskWindow::setSnapHint(const QRect &globalRegion)
{
    const QRect snapHint =
            CoordinateMapper::globalToOutputLocal(globalRegion, m_output).intersected(rect());
    if (m_snapHint == snapHint)
        return;
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

void TreelandMaskWindow::setCursorIntent(CursorIntent intent)
{
    if (m_cursorIntent == intent)
        return;
    m_cursorIntent = intent;
    switch (intent) {
    case CursorIntent::Move:
        setCursor(Qt::SizeAllCursor);
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
        setCursor(Qt::CrossCursor);
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

void TreelandMaskWindow::prepareWindow()
{
    create();
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

void TreelandMaskWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

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
        painter.setPen(QPen(QColor(75, 160, 255), 2));
        painter.drawRect(m_snapHint.adjusted(1, 1, -1, -1));
    }

    if (!visibleSelection.isEmpty()) {
        painter.setPen(QPen(QColor(62, 156, 255), m_finalized ? 2 : 1));
        painter.drawRect(visibleSelection.adjusted(0, 0, -1, -1));

        if (m_finalized) {
            painter.setBrush(QColor(245, 248, 252));
            painter.setPen(QPen(QColor(43, 124, 228), 1));
            const QList<QPoint> handles = {
                visibleSelection.topLeft(), visibleSelection.topRight(),
                visibleSelection.bottomLeft(), visibleSelection.bottomRight(),
                QPoint(visibleSelection.center().x(), visibleSelection.top()),
                QPoint(visibleSelection.right(), visibleSelection.center().y()),
                QPoint(visibleSelection.center().x(), visibleSelection.bottom()),
                QPoint(visibleSelection.left(), visibleSelection.center().y())
            };
            for (const QPoint &point : handles)
                painter.drawRect(QRect(point - QPoint(4, 4), QSize(9, 9)));
        }
    }

    QString hint = QStringLiteral("拖拽框选窗口或区域  ·  Enter 保存  ·  Ctrl+C 复制  ·  Esc 取消");
    if (m_finalized) {
        hint = QStringLiteral("%1 × %2 px  ·  可拖动选区或拖拽控制点调整大小")
                .arg(m_selection.width())
                .arg(m_selection.height());
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

void TreelandMaskWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    emit pointerPressed(toGlobal(eventPosition(event)));
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
    emit pointerReleased(toGlobal(eventPosition(event)));
    event->accept();
}

void TreelandMaskWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit cancelRequested();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter
            || event->matches(QKeySequence::Save)) {
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
