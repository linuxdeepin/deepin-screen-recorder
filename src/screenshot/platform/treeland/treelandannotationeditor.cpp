// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandannotationeditor.h"

#include "../../adapters/shapeswidgetannotationeditor.h"
#include "widgets/shapeswidget.h"

#include <QPainter>
#include <QWidget>

namespace Dsr::Screenshot {

class TreelandAnnotationView final : public QWidget
{
public:
    TreelandAnnotationView(const ShapesWidget *document,
                           const QRect &outputBounds,
                           QWidget *parent)
        : QWidget(parent)
        , m_document(document)
        , m_outputBounds(outputBounds)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setStyleSheet(QStringLiteral(
                "QWidget { background: transparent; border: none; }"));
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        if (!m_document)
            return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setClipRect(rect());
        painter.translate(-m_outputBounds.left(), -m_outputBounds.top());
        m_document->paintEditingContent(painter);
    }

private:
    const ShapesWidget *m_document = nullptr;
    QRect m_outputBounds;
};

TreelandAnnotationEditor::TreelandAnnotationEditor(QObject *parent)
    : IAnnotationEditor(parent)
    , m_documentWidget(new ShapesWidget)
{
    m_documentWidget->hide();
    m_documentWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_documentWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    m_documentWidget->setAttribute(Qt::WA_NoSystemBackground, true);
    // The TreeLand mask owns the visible cursor. The detached document is
    // hidden and must never compete through QApplication::overrideCursor().
    m_documentWidget->setCursorManagementEnabled(false);
    m_documentWidget->setInteractionPlatform(
            ShapesWidget::InteractionPlatform::TreeLand);

    m_document = new ShapesWidgetAnnotationEditor(m_documentWidget, this);
    connect(m_document, &IAnnotationEditor::contentChanged, this, [this] {
        // The platform editor must forward every content mutation. Bounds can
        // stay identical when one of several nested annotations is undone, so
        // UI state such as undo availability cannot rely on bounds changes.
        emit contentChanged();
        refreshViews();
    });
    connect(m_document, &IAnnotationEditor::contentBoundsChanged, this, [this] {
        publishBounds();
    });
}

TreelandAnnotationEditor::~TreelandAnnotationEditor()
{
    delete m_documentWidget;
    m_documentWidget = nullptr;
}

void TreelandAnnotationEditor::setDesktopBounds(const QRect &globalDesktopBounds)
{
    m_desktopBounds = globalDesktopBounds.normalized();
    m_documentWidget->setFixedSize(m_desktopBounds.size());
    m_document->setGlobalOrigin(m_desktopBounds.topLeft());
    m_document->setDesktopBounds(m_desktopBounds);
    m_document->setTool(m_tool);
    publishBounds();
}

QWidget *TreelandAnnotationEditor::createView(QWidget *parent,
                                              const QRect &outputBounds)
{
    auto *view = new TreelandAnnotationView(m_documentWidget, outputBounds, parent);
    m_views.append(view);
    return view;
}

void TreelandAnnotationEditor::clearViews()
{
    m_views.clear();
}

QRectF TreelandAnnotationEditor::contentBoundsGlobal() const
{
    return m_document ? m_document->contentBoundsGlobal() : QRectF();
}

bool TreelandAnnotationEditor::isInteracting() const
{
    return m_document && m_document->isInteracting();
}

void TreelandAnnotationEditor::setTool(AnnotationTool tool)
{
    m_tool = tool;
    if (m_document)
        m_document->setTool(tool);
}

AnnotationTool TreelandAnnotationEditor::tool() const
{
    return m_tool;
}

AnnotationHit TreelandAnnotationEditor::hitTest(const QPoint &globalPos) const
{
    return m_document ? m_document->hitTest(globalPos) : AnnotationHit();
}

bool TreelandAnnotationEditor::beginInteraction(const QPoint &globalPos)
{
    return m_document && m_document->beginInteraction(globalPos);
}

void TreelandAnnotationEditor::updateInteraction(const QPoint &globalPos)
{
    if (m_document)
        m_document->updateInteraction(globalPos);
}

void TreelandAnnotationEditor::endInteraction()
{
    if (m_document)
        m_document->endInteraction();
}

void TreelandAnnotationEditor::cancelInteraction()
{
    if (m_document)
        m_document->cancelInteraction();
}

void TreelandAnnotationEditor::undo()
{
    if (m_document)
        m_document->undo();
}

void TreelandAnnotationEditor::render(QPainter &painter,
                                      const QRect &globalSelection,
                                      qreal pixelScale) const
{
    if (m_document)
        m_document->render(painter, globalSelection, pixelScale);
}

void TreelandAnnotationEditor::publishBounds()
{
    const QRectF bounds = contentBoundsGlobal();
    if (bounds != m_lastBounds) {
        m_lastBounds = bounds;
        emit contentBoundsChanged(bounds);
    }
}

void TreelandAnnotationEditor::refreshViews()
{
    for (const QPointer<QWidget> &view : m_views) {
        if (view)
            view->update();
    }
}

} // namespace Dsr::Screenshot
