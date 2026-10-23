// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shapeswidgetannotationeditor.h"

#include "widgets/shapeswidget.h"

#include <QPainter>
#include <QTransform>

namespace Dsr::Screenshot {

namespace {

QString shapeTypeForTool(AnnotationTool tool)
{
    switch (tool) {
    case AnnotationTool::Rectangle:
        return QStringLiteral("rectangle");
    case AnnotationTool::Ellipse:
        return QStringLiteral("oval");
    case AnnotationTool::Arrow:
        return QStringLiteral("arrow");
    case AnnotationTool::Line:
        return QStringLiteral("line");
    case AnnotationTool::Pen:
        return QStringLiteral("pen");
    case AnnotationTool::Effect:
        return QStringLiteral("effect");
    case AnnotationTool::Pointer:
        return QString();
    }
    return QString();
}

CursorIntent cursorForShapeHit(const ShapesWidget::InteractionHit &hit)
{
    switch (hit.kind) {
    case ShapesWidget::InteractionKind::None:
        return CursorIntent::Cross;
    case ShapesWidget::InteractionKind::TextEditor:
        return CursorIntent::Text;
    case ShapesWidget::InteractionKind::Rotate:
        return CursorIntent::Rotate;
    case ShapesWidget::InteractionKind::Move:
        return CursorIntent::Move;
    case ShapesWidget::InteractionKind::Resize:
        break;
    }

    switch (hit.handle) {
    case ShapesWidget::InteractionHandle::Left:
    case ShapesWidget::InteractionHandle::Right:
        return CursorIntent::ResizeHorizontal;
    case ShapesWidget::InteractionHandle::Top:
    case ShapesWidget::InteractionHandle::Bottom:
        return CursorIntent::ResizeVertical;
    case ShapesWidget::InteractionHandle::TopLeft:
    case ShapesWidget::InteractionHandle::BottomRight:
        return CursorIntent::ResizeDiagonalForward;
    case ShapesWidget::InteractionHandle::TopRight:
    case ShapesWidget::InteractionHandle::BottomLeft:
        return CursorIntent::ResizeDiagonalBackward;
    case ShapesWidget::InteractionHandle::None:
        return CursorIntent::Move;
    }
    return CursorIntent::Move;
}

AnnotationHitKind kindForShapeHit(ShapesWidget::InteractionKind kind)
{
    switch (kind) {
    case ShapesWidget::InteractionKind::None:
        return AnnotationHitKind::None;
    case ShapesWidget::InteractionKind::Move:
        return AnnotationHitKind::Content;
    case ShapesWidget::InteractionKind::Resize:
        return AnnotationHitKind::ResizeHandle;
    case ShapesWidget::InteractionKind::Rotate:
        return AnnotationHitKind::RotateHandle;
    case ShapesWidget::InteractionKind::TextEditor:
        return AnnotationHitKind::TextEditor;
    }
    return AnnotationHitKind::None;
}

} // namespace

ShapesWidgetAnnotationEditor::ShapesWidgetAnnotationEditor(ShapesWidget *widget,
                                                           QObject *parent)
    : IAnnotationEditor(parent)
    , m_widget(widget)
{
    Q_ASSERT(m_widget);
    connect(m_widget, &ShapesWidget::shapesChanged,
            this, [this]() {
        publishBounds();
        emit contentChanged();
    });
    m_lastBounds = contentBoundsGlobal();
}

void ShapesWidgetAnnotationEditor::setDesktopBounds(const QRect &globalDesktopBounds)
{
    m_desktopBounds = globalDesktopBounds.normalized();
}

void ShapesWidgetAnnotationEditor::setGlobalOrigin(const QPoint &globalOrigin)
{
    m_globalOrigin = globalOrigin;
    m_lastBounds = contentBoundsGlobal();
}

void ShapesWidgetAnnotationEditor::setTool(AnnotationTool tool)
{
    m_tool = tool;
    if (m_widget)
        m_widget->setCurrentShape(shapeTypeForTool(tool));
}

AnnotationTool ShapesWidgetAnnotationEditor::tool() const
{
    return m_tool;
}

QRectF ShapesWidgetAnnotationEditor::contentBoundsGlobal() const
{
    if (!m_widget)
        return QRectF();
    const QRectF local = m_widget->contentBounds();
    if (local.isEmpty())
        return QRectF();
    return local.translated(m_globalOrigin).normalized();
}

bool ShapesWidgetAnnotationEditor::isInteracting() const
{
    return m_widget && m_widget->isPressed();
}

AnnotationHit ShapesWidgetAnnotationEditor::hitTest(const QPoint &globalPos) const
{
    if (!m_widget)
        return {};
    const QPoint local = globalPos - m_globalOrigin;
    if (!m_widget->rect().contains(local))
        return {};

    const ShapesWidget::InteractionHit hit = m_widget->interactionAt(local);
    AnnotationHit result;
    if (!hit.isValid() && m_tool != AnnotationTool::Pointer) {
        result.kind = AnnotationHitKind::CreateContent;
        result.cursor = CursorIntent::Cross;
        return result;
    }
    result.kind = kindForShapeHit(hit.kind);
    result.cursor = cursorForShapeHit(hit);
    result.handleId = static_cast<int>(hit.handle);
    return result;
}

bool ShapesWidgetAnnotationEditor::beginInteraction(const QPoint &globalPos)
{
    if (!m_widget)
        return false;
    const bool started = m_widget->beginInteraction(localPosition(globalPos));
    if (started)
        publishBounds();
    return started;
}

void ShapesWidgetAnnotationEditor::updateInteraction(const QPoint &globalPos)
{
    if (!m_widget)
        return;
    m_widget->updateInteraction(localPosition(globalPos));
}

void ShapesWidgetAnnotationEditor::endInteraction()
{
    if (!m_widget)
        return;
    m_widget->endInteraction();
}

void ShapesWidgetAnnotationEditor::cancelInteraction()
{
    if (!m_widget)
        return;
    m_widget->cancelInteraction();
}

void ShapesWidgetAnnotationEditor::undo()
{
    if (!m_widget)
        return;
    m_widget->undoDrawShapes();
}

void ShapesWidgetAnnotationEditor::render(QPainter &painter,
                                          const QRect &globalSelection,
                                          qreal pixelScale) const
{
    if (!m_widget || globalSelection.isEmpty() || pixelScale <= 0.0)
        return;

    QTransform transform;
    transform.translate(-globalSelection.left() * pixelScale,
                        -globalSelection.top() * pixelScale);
    transform.scale(pixelScale, pixelScale);
    transform.translate(m_globalOrigin.x(), m_globalOrigin.y());

    painter.save();
    painter.setClipRect(QRectF(0, 0,
                               globalSelection.width() * pixelScale,
                               globalSelection.height() * pixelScale));
    painter.setTransform(transform, true);
    m_widget->paintContent(painter);
    painter.restore();
}

QPoint ShapesWidgetAnnotationEditor::localPosition(const QPoint &globalPos) const
{
    QPoint bounded = globalPos;
    if (m_desktopBounds.isValid()) {
        bounded.setX(qBound(m_desktopBounds.left(), bounded.x(), m_desktopBounds.right()));
        bounded.setY(qBound(m_desktopBounds.top(), bounded.y(), m_desktopBounds.bottom()));
    }
    return bounded - m_globalOrigin;
}

QRectF ShapesWidgetAnnotationEditor::publishBounds()
{
    const QRectF bounds = contentBoundsGlobal();
    if (bounds != m_lastBounds) {
        m_lastBounds = bounds;
        emit contentBoundsChanged(bounds);
    }
    return bounds;
}

} // namespace Dsr::Screenshot
