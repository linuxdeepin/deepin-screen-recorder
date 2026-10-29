// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_ANNOTATION_EDITOR_H
#define TREELAND_ANNOTATION_EDITOR_H

#include "../../ports/iannotationeditor.h"
#include "widgets/shapeswidget.h"

#include <QList>
#include <QPointer>
#include <QRect>
#include <QRectF>

class QWidget;
class QEvent;

namespace Dsr::Screenshot {

class ShapesWidgetAnnotationEditor;

/**
 * One logical annotation document shared by all TreeLand output surfaces.
 *
 * ShapesWidget remains the editing implementation, but it is no longer
 * instantiated once per output. A hidden document widget owns global logical
 * coordinates; each output only paints a translated, clipped view of that
 * document. This is what allows a rectangle to begin on one output and end on
 * another without moving shapes between independent widgets.
 */
class TreelandAnnotationEditor final : public IAnnotationEditor
{
    Q_OBJECT
public:
    explicit TreelandAnnotationEditor(QObject *parent = nullptr);
    ~TreelandAnnotationEditor() override;

    void setDesktopBounds(const QRect &globalDesktopBounds);
    void setEffectSourceImage(const QImage &image);
    QWidget *createView(QWidget *parent, const QRect &outputBounds);
    void clearViews();
    bool beginInlineEdit(const QPoint &globalPos);

    QRectF contentBoundsGlobal() const override;
    bool isInteracting() const override;
    void setTool(AnnotationTool tool) override;
    AnnotationTool tool() const override;
    bool hasSelectedContent() const override;
    AnnotationHit hitTest(const QPoint &globalPos) const override;
    bool beginInteraction(const QPoint &globalPos) override;
    void updateInteraction(const QPoint &globalPos) override;
    void endInteraction() override;
    void cancelInteraction() override;
    void undo() override;
    void render(QPainter &painter,
                const QRect &globalSelection,
                qreal pixelScale) const override;

signals:
    void shapeSelected(const QString &shape);

private:
    void showInlineEditor(const ShapesWidget::InlineEditRequest &request);
    void finishInlineEdit(bool commit);
    void publishBounds();
    void refreshViews();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:

    ShapesWidget *m_documentWidget = nullptr;
    ShapesWidgetAnnotationEditor *m_document = nullptr;
    QList<QPointer<QWidget>> m_views;
    QList<QRect> m_viewBounds;
    QPointer<QWidget> m_inlineEditor;
    int m_inlineShapeIndex = -1;
    bool m_inlineIsSequence = false;
    bool m_finishingInlineEdit = false;
    QRect m_inlineOutputBounds;
    QString m_inlineOriginalText;
    QRectF m_inlineOriginalRect;
    QRect m_desktopBounds;
    QRectF m_lastBounds;
    AnnotationTool m_tool = AnnotationTool::Pointer;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_ANNOTATION_EDITOR_H
