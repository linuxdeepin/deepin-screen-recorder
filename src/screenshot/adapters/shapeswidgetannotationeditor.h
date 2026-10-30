// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAPES_WIDGET_ANNOTATION_EDITOR_H
#define SHAPES_WIDGET_ANNOTATION_EDITOR_H

#include "../ports/iannotationeditor.h"
#include "widgets/shapeswidget.h"

#include <QPointer>
#include <QRect>

namespace Dsr::Screenshot {

/**
 * Adapt the legacy ShapesWidget editing technology to the screenshot session.
 *
 * ShapesWidget keeps local geometry. This adapter is the only place that
 * translates it to/from global logical coordinates, so SelectionModel and the
 * controller never depend on Toolshape internals.
 */
class ShapesWidgetAnnotationEditor : public IAnnotationEditor
{
    Q_OBJECT
public:
    explicit ShapesWidgetAnnotationEditor(ShapesWidget *widget,
                                          QObject *parent = nullptr);

    void setDesktopBounds(const QRect &globalDesktopBounds);
    void setGlobalOrigin(const QPoint &globalOrigin);
    void setEffectSourceImage(const QImage &image);
    void setTool(AnnotationTool tool) override;
    AnnotationTool tool() const override;
    bool hasSelectedContent() const override;

    QRectF contentBoundsGlobal() const override;
    bool isInteracting() const override;
    AnnotationHit hitTest(const QPoint &globalPos) const override;
    bool beginInteraction(const QPoint &globalPos) override;
    void updateInteraction(const QPoint &globalPos) override;
    void endInteraction() override;
    void cancelInteraction() override;
    void undo() override;

    bool takePendingInlineEdit(ShapesWidget::InlineEditRequest *request);
    bool beginInlineEditAt(const QPoint &globalPos,
                           ShapesWidget::InlineEditRequest *request);
    void updateInlineText(int shapeIndex,
                          const QString &text,
                          const QRectF &globalRect);
    void finishInlineText(int shapeIndex);
    bool updateInlineSequence(int shapeIndex, int sequenceNumber);
    void setInlineEditingShape(int shapeIndex);

    void render(QPainter &painter,
                const QRect &globalSelection,
                qreal pixelScale) const override;

private:
    QPoint localPosition(const QPoint &globalPos) const;
    QRectF publishBounds();

    QPointer<ShapesWidget> m_widget;
    QRect m_desktopBounds;
    QPoint m_globalOrigin;
    QRectF m_lastBounds;
    AnnotationTool m_tool = AnnotationTool::Pointer;
};

} // namespace Dsr::Screenshot

#endif // SHAPES_WIDGET_ANNOTATION_EDITOR_H
