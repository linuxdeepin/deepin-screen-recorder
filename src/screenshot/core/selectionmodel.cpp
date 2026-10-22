// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectionmodel.h"

#include "selectionconstraints.h"

#include <QtMath>

namespace Dsr::Screenshot {

namespace {

constexpr int CreationMinimumSize = 8;
constexpr int EditingMinimumSize = 20;
constexpr qreal HandleHitRadius = 8.0;

bool near(const QPoint &point, const QPoint &target)
{
    return (QPointF(point) - QPointF(target)).manhattanLength() <= HandleHitRadius;
}

QRect clampedMove(const QRect &start, const QPoint &delta, const QRect &bounds)
{
    QRect moved = start.translated(delta);
    if (!bounds.isValid())
        return moved;

    if (moved.left() < bounds.left())
        moved.moveLeft(bounds.left());
    if (moved.top() < bounds.top())
        moved.moveTop(bounds.top());
    if (moved.right() > bounds.right())
        moved.moveRight(bounds.right());
    if (moved.bottom() > bounds.bottom())
        moved.moveBottom(bounds.bottom());
    return moved;
}

} // namespace

CursorIntent cursorIntentFor(SelectionAction action)
{
    switch (action) {
    case SelectionAction::Move:
        return CursorIntent::Move;
    case SelectionAction::ResizeLeft:
    case SelectionAction::ResizeRight:
        return CursorIntent::ResizeHorizontal;
    case SelectionAction::ResizeTop:
    case SelectionAction::ResizeBottom:
        return CursorIntent::ResizeVertical;
    case SelectionAction::ResizeTopLeft:
    case SelectionAction::ResizeBottomRight:
        return CursorIntent::ResizeDiagonalForward;
    case SelectionAction::ResizeTopRight:
    case SelectionAction::ResizeBottomLeft:
        return CursorIntent::ResizeDiagonalBackward;
    case SelectionAction::CreateFreeform:
    case SelectionAction::NoAction:
        return CursorIntent::Cross;
    }
    return CursorIntent::Cross;
}

SelectionModel::SelectionModel(QObject *parent)
    : QObject(parent)
{
}

QRect SelectionModel::geometry() const
{
    return m_geometry;
}

InteractionState SelectionModel::state() const
{
    return m_state;
}

SelectionAction SelectionModel::action() const
{
    return m_action;
}

void SelectionModel::setDesktopBounds(const QRect &bounds)
{
    m_desktopBounds = bounds;
}

void SelectionModel::setAnnotationBounds(const QRectF &bounds)
{
    m_annotationBounds = bounds;
    if (m_state == InteractionState::Ready && !m_geometry.isEmpty())
        setGeometry(m_geometry);
}

SelectionAction SelectionModel::beginInteraction(const QPoint &globalPos)
{
    m_startRect = m_geometry;
    m_startPos = globalPos;
    m_action = hitTest(globalPos);

    if (m_action == SelectionAction::NoAction) {
        m_action = SelectionAction::CreateFreeform;
        m_startRect = QRect(globalPos, QSize());
        applyCandidate(m_startRect, QSize(1, 1));
        setState(InteractionState::Creating);
    } else if (m_action == SelectionAction::Move) {
        setState(InteractionState::Moving);
    } else {
        setState(InteractionState::Resizing);
    }
    return m_action;
}

void SelectionModel::updateInteraction(const QPoint &globalPos)
{
    if (m_state == InteractionState::Creating) {
        applyCandidate(QRect(m_startPos, globalPos).normalized().intersected(m_desktopBounds),
                       QSize(1, 1));
        return;
    }

    if (m_state == InteractionState::Moving) {
        applyCandidate(clampedMove(m_startRect, globalPos - m_startPos, m_desktopBounds),
                       QSize(EditingMinimumSize, EditingMinimumSize));
        return;
    }

    if (m_state == InteractionState::Resizing)
        applyCandidate(resizedRect(globalPos), QSize(EditingMinimumSize, EditingMinimumSize));
}

void SelectionModel::endInteraction()
{
    if (m_state == InteractionState::Creating) {
        if (m_geometry.width() < CreationMinimumSize || m_geometry.height() < CreationMinimumSize) {
            clear();
            return;
        }
    }

    if (m_state == InteractionState::Creating
            || m_state == InteractionState::Moving
            || m_state == InteractionState::Resizing) {
        m_action = SelectionAction::NoAction;
        setState(InteractionState::Ready);
    }
}

void SelectionModel::cancelInteraction()
{
    const QRect oldRect = m_geometry;
    m_geometry = m_startRect;
    m_action = SelectionAction::NoAction;
    setState(m_geometry.isEmpty() ? InteractionState::Empty : InteractionState::Ready);
    if (oldRect != m_geometry)
        emit geometryChanged(oldRect, m_geometry);
}

void SelectionModel::setGeometry(const QRect &globalRect)
{
    if (globalRect.isEmpty()) {
        clear();
        return;
    }

    const QRect oldRect = m_geometry;
    SelectionConstraints constraints;
    constraints.desktopBounds = m_desktopBounds;
    constraints.annotationBounds = m_annotationBounds;
    constraints.minimumSize = QSize(1, 1);
    QRect resolved = resolveSelectionGeometry(globalRect, constraints);
    if (resolved.width() < CreationMinimumSize || resolved.height() < CreationMinimumSize)
        resolved = QRect();

    if (m_geometry != resolved) {
        m_geometry = resolved;
        emit geometryChanged(oldRect, m_geometry);
    }
    m_action = SelectionAction::NoAction;
    setState(m_geometry.isEmpty() ? InteractionState::Empty : InteractionState::Ready);
}

void SelectionModel::clear()
{
    const QRect oldRect = m_geometry;
    m_geometry = QRect();
    m_action = SelectionAction::NoAction;
    if (!oldRect.isEmpty())
        emit geometryChanged(oldRect, m_geometry);
    setState(InteractionState::Empty);
}

SelectionAction SelectionModel::hitTest(const QPoint &globalPos) const
{
    if (m_geometry.isEmpty())
        return SelectionAction::NoAction;

    const QPoint center = m_geometry.center();
    const QList<QPair<QPoint, SelectionAction>> handles = {
        {m_geometry.topLeft(), SelectionAction::ResizeTopLeft},
        {m_geometry.topRight(), SelectionAction::ResizeTopRight},
        {m_geometry.bottomLeft(), SelectionAction::ResizeBottomLeft},
        {m_geometry.bottomRight(), SelectionAction::ResizeBottomRight},
        {QPoint(center.x(), m_geometry.top()), SelectionAction::ResizeTop},
        {QPoint(m_geometry.right(), center.y()), SelectionAction::ResizeRight},
        {QPoint(center.x(), m_geometry.bottom()), SelectionAction::ResizeBottom},
        {QPoint(m_geometry.left(), center.y()), SelectionAction::ResizeLeft}
    };

    for (const auto &entry : handles) {
        if (near(globalPos, entry.first))
            return entry.second;
    }
    return m_geometry.contains(globalPos) ? SelectionAction::Move : SelectionAction::NoAction;
}

void SelectionModel::applyCandidate(const QRect &candidate, const QSize &minimumSize)
{
    const QRect oldRect = m_geometry;
    SelectionConstraints constraints;
    constraints.desktopBounds = m_desktopBounds;
    constraints.annotationBounds = m_annotationBounds;
    constraints.minimumSize = minimumSize;
    m_geometry = resolveSelectionGeometry(candidate, constraints);
    if (oldRect != m_geometry)
        emit geometryChanged(oldRect, m_geometry);
}

void SelectionModel::setState(InteractionState state)
{
    if (m_state == state)
        return;
    m_state = state;
    emit stateChanged(m_state);
}

QRect SelectionModel::resizedRect(const QPoint &globalPos) const
{
    int left = m_startRect.left();
    int top = m_startRect.top();
    int right = m_startRect.right();
    int bottom = m_startRect.bottom();
    const QPoint delta = globalPos - m_startPos;
    const QRect bounds = m_desktopBounds;
    if (!bounds.isValid())
        return m_startRect;

    if (m_action == SelectionAction::ResizeTopLeft
            || m_action == SelectionAction::ResizeLeft
            || m_action == SelectionAction::ResizeBottomLeft) {
        left = qBound(bounds.left(), left + delta.x(), right - EditingMinimumSize);
    }
    if (m_action == SelectionAction::ResizeTopRight
            || m_action == SelectionAction::ResizeRight
            || m_action == SelectionAction::ResizeBottomRight) {
        right = qBound(left + EditingMinimumSize, right + delta.x(), bounds.right());
    }
    if (m_action == SelectionAction::ResizeTopLeft
            || m_action == SelectionAction::ResizeTop
            || m_action == SelectionAction::ResizeTopRight) {
        top = qBound(bounds.top(), top + delta.y(), bottom - EditingMinimumSize);
    }
    if (m_action == SelectionAction::ResizeBottomLeft
            || m_action == SelectionAction::ResizeBottom
            || m_action == SelectionAction::ResizeBottomRight) {
        bottom = qBound(top + EditingMinimumSize, bottom + delta.y(), bounds.bottom());
    }

    return QRect(QPoint(left, top), QPoint(right, bottom));
}

} // namespace Dsr::Screenshot
