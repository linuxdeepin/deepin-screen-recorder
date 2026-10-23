// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IANNOTATION_EDITOR_H
#define IANNOTATION_EDITOR_H

#include "../core/screenshottypes.h"

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QRectF>

class QPainter;

namespace Dsr::Screenshot {

enum class AnnotationHitKind {
    None,
    CreateContent,
    Content,
    ResizeHandle,
    RotateHandle,
    TextEditor
};

struct AnnotationHit {
    AnnotationHitKind kind = AnnotationHitKind::None;
    CursorIntent cursor = CursorIntent::Cross;
    int handleId = -1;

    bool isValid() const
    {
        return kind != AnnotationHitKind::None;
    }
};

/**
 * Platform-independent annotation editing port.
 *
 * Implementations own their editing technology (for example ShapesWidget),
 * but all geometry crossing this boundary is in global logical coordinates.
 */
class IAnnotationEditor : public QObject
{
    Q_OBJECT
public:
    explicit IAnnotationEditor(QObject *parent = nullptr) : QObject(parent) {}
    ~IAnnotationEditor() override = default;

    virtual QRectF contentBoundsGlobal() const = 0;
    virtual bool isInteracting() const = 0;
    virtual void setTool(AnnotationTool tool) = 0;
    virtual AnnotationTool tool() const = 0;

    /**
     * Resolve what annotation interaction, if any, belongs at globalPos.
     *
     * hitTest() must be side-effect free. The controller uses it to arbitrate
     * selection handles, annotation handles/content and drawing tools before a
     * press starts an action.
     */
    virtual AnnotationHit hitTest(const QPoint &globalPos) const = 0;

    /**
     * Start an annotation action at globalPos. Returning false means the
     * controller may fall back to selection interaction.
     */
    virtual bool beginInteraction(const QPoint &globalPos) = 0;
    virtual void updateInteraction(const QPoint &globalPos) = 0;
    virtual void endInteraction() = 0;
    virtual void cancelInteraction() = 0;
    virtual void undo() {}

    /**
     * Render annotations into the result image.
     *
     * The painter coordinate system is the result image. Implementations map
     * from globalSelection to image-local coordinates and apply pixelScale.
     */
    virtual void render(QPainter &painter,
                        const QRect &globalSelection,
                        qreal pixelScale) const = 0;

signals:
    // Emitted whenever committed or in-progress annotation pixels change.
    // contentBoundsChanged is intentionally geometry-only and may stay unchanged
    // when a new shape is drawn entirely inside existing content.
    void contentChanged();
    void contentBoundsChanged(const QRectF &globalBounds);
};

} // namespace Dsr::Screenshot

#endif // IANNOTATION_EDITOR_H
