// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SELECTION_MODEL_H
#define SELECTION_MODEL_H

#include "screenshottypes.h"

#include <QObject>
#include <QRect>
#include <QRectF>

namespace Dsr::Screenshot {

enum class SelectionAction {
    NoAction,
    CreateFreeform,
    Move,
    ResizeTopLeft,
    ResizeTop,
    ResizeTopRight,
    ResizeRight,
    ResizeBottomRight,
    ResizeBottom,
    ResizeBottomLeft,
    ResizeLeft
};

enum class InteractionState {
    Empty,
    Creating,
    Moving,
    Resizing,
    Ready
};

CursorIntent cursorIntentFor(SelectionAction action);

class SelectionModel : public QObject
{
    Q_OBJECT
public:
    explicit SelectionModel(QObject *parent = nullptr);

    QRect geometry() const;
    InteractionState state() const;
    SelectionAction action() const;

    void setDesktopBounds(const QRect &bounds);
    void setAnnotationBounds(const QRectF &bounds);

    SelectionAction beginInteraction(const QPoint &globalPos);
    void updateInteraction(const QPoint &globalPos);
    void endInteraction();
    void cancelInteraction();

    void setGeometry(const QRect &globalRect);
    void clear();

    SelectionAction hitTest(const QPoint &globalPos) const;

signals:
    void geometryChanged(const QRect &oldRect, const QRect &newRect);
    void stateChanged(InteractionState state);

private:
    void applyCandidate(const QRect &candidate, const QSize &minimumSize);
    void setState(InteractionState state);
    QRect resizedRect(const QPoint &globalPos) const;

    QRect m_geometry;
    QRect m_desktopBounds;
    QRectF m_annotationBounds;
    QRect m_startRect;
    QPoint m_startPos;
    SelectionAction m_action = SelectionAction::NoAction;
    InteractionState m_state = InteractionState::Empty;
};

} // namespace Dsr::Screenshot

#endif // SELECTION_MODEL_H
