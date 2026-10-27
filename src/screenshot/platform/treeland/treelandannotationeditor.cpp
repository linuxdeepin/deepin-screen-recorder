// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandannotationeditor.h"

#include "../../adapters/shapeswidgetannotationeditor.h"
#include "widgets/textedit.h"
#include "widgets/shapeswidget.h"

#include <QFont>
#include <QGuiApplication>
#include <QInputMethod>
#include <QIntValidator>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QTimer>
#include <QWidget>
#include <QWindow>

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
    m_documentWidget->setTextSelectionFrameEnabled(true);

    m_document = new ShapesWidgetAnnotationEditor(m_documentWidget, this);
    connect(m_documentWidget, &ShapesWidget::shapeClicked,
            this, &TreelandAnnotationEditor::shapeSelected);
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
    finishInlineEdit(false);
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

void TreelandAnnotationEditor::setEffectSourceImage(const QImage &image)
{
    if (m_document)
        m_document->setEffectSourceImage(image);
}

QWidget *TreelandAnnotationEditor::createView(QWidget *parent,
                                              const QRect &outputBounds)
{
    auto *view = new TreelandAnnotationView(m_documentWidget, outputBounds, parent);
    m_views.append(view);
    m_viewBounds.append(outputBounds);
    return view;
}

void TreelandAnnotationEditor::clearViews()
{
    finishInlineEdit(false);
    m_views.clear();
    m_viewBounds.clear();
}

bool TreelandAnnotationEditor::beginInlineEdit(const QPoint &globalPos)
{
    if (!m_document) {
        return false;
    }
    finishInlineEdit(true);
    ShapesWidget::InlineEditRequest request;
    if (!m_document->beginInlineEditAt(globalPos, &request)) {
        return false;
    }
    showInlineEditor(request);
    return true;
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
    finishInlineEdit(true);
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
    if (!m_document) {
        return false;
    }
    finishInlineEdit(true);
    const bool started = m_document->beginInteraction(globalPos);
    ShapesWidget::InlineEditRequest request;
    if (started && m_document->takePendingInlineEdit(&request)) {
        showInlineEditor(request);
    }
    return started;
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
    finishInlineEdit(false);
    if (m_document)
        m_document->cancelInteraction();
}

void TreelandAnnotationEditor::undo()
{
    finishInlineEdit(true);
    if (m_document)
        m_document->undo();
}

void TreelandAnnotationEditor::showInlineEditor(
        const ShapesWidget::InlineEditRequest &request)
{
    if (m_views.isEmpty() || m_views.size() != m_viewBounds.size()) {
        return;
    }

    int outputIndex = -1;
    const QPoint anchor = request.localRect.center().toPoint();
    for (int i = 0; i < m_viewBounds.size(); ++i) {
        if (m_viewBounds[i].contains(anchor)) {
            outputIndex = i;
            break;
        }
    }
    if (outputIndex < 0) {
        return;
    }

    QWidget *view = m_views[outputIndex];
    QWidget *parent = view ? view->parentWidget() : nullptr;
    if (!parent) {
        return;
    }

    const QRect outputBounds = m_viewBounds[outputIndex];
    QRect editorRect = request.localRect.translated(-outputBounds.topLeft()).toAlignedRect();
    QWidget *editor = nullptr;
    m_inlineShapeIndex = request.shapeIndex;
    m_inlineIsSequence = request.kind == ShapesWidget::InlineEditKind::Sequence;
    m_inlineOutputBounds = outputBounds;
    m_inlineOriginalText = request.text;
    m_inlineOriginalRect = request.localRect;
    m_finishingInlineEdit = false;
    m_document->setInlineEditingShape(request.shapeIndex);

    if (m_inlineIsSequence) {
        auto *sequenceEditor = new QLineEdit(parent);
        sequenceEditor->setObjectName(QStringLiteral("TreelandSequenceEditor"));
        sequenceEditor->setAccessibleName(QStringLiteral("TreelandSequenceEditor"));
        sequenceEditor->setInputMethodHints(Qt::ImhDigitsOnly
                                            | Qt::ImhNoPredictiveText);
        sequenceEditor->setAlignment(Qt::AlignCenter);
        sequenceEditor->setFrame(false);
        sequenceEditor->setMaxLength(2);
        sequenceEditor->setValidator(new QIntValidator(request.minValue,
                                                       request.maxValue,
                                                       sequenceEditor));
        sequenceEditor->setText(request.text);
        sequenceEditor->setFont(QFont(QString(), request.fontSize, QFont::Black));
        sequenceEditor->setStyleSheet(QStringLiteral(
                "QLineEdit { background: transparent; border: none; color: %1; }")
                .arg(request.foregroundColor.name(QColor::HexRgb)));
        const QSize editorSize(40, 28);
        editorRect = QRect(QPoint(editorRect.center().x() - editorSize.width() / 2,
                                  editorRect.center().y() - editorSize.height() / 2),
                           editorSize);
        editor = sequenceEditor;
    } else {
        auto *textEditor = new TextEdit(request.shapeIndex, parent);
        textEditor->setObjectName(QStringLiteral("TreelandTextEditor"));
        textEditor->setAccessibleName(QStringLiteral("TreelandTextEditor"));
        textEditor->setAttribute(Qt::WA_InputMethodEnabled, true);
        textEditor->setFocusPolicy(Qt::StrongFocus);
        textEditor->setFontSize(request.fontSize);
        textEditor->setColor(request.foregroundColor);
        textEditor->setPlainText(request.text);
        textEditor->setReadOnly(false);
        textEditor->setEditing(true);
        textEditor->setCursorVisible(true);
        textEditor->updateContentSize(request.text);
        editorRect.setSize(textEditor->size());
        editor = textEditor;
    }

    if (editorRect.width() <= 0 || editorRect.height() <= 0) {
        delete editor;
        return;
    }

    editor->setGeometry(editorRect);
    editor->installEventFilter(this);
    m_inlineEditor = editor;

    if (m_inlineIsSequence) {
        connect(qobject_cast<QLineEdit *>(editor), &QLineEdit::editingFinished, this, [this] {
            finishInlineEdit(true);
        });
    } else {
        auto *textEditor = qobject_cast<TextEdit *>(editor);
        auto syncText = [this, textEditor, outputBounds] {
            if (m_finishingInlineEdit || !textEditor || m_inlineEditor != textEditor) {
                return;
            }
            const QRect globalEditorRect = textEditor->geometry().translated(outputBounds.topLeft());
            const QRectF documentRect = globalEditorRect.translated(-m_desktopBounds.topLeft());
            m_document->updateInlineText(m_inlineShapeIndex,
                                         textEditor->toPlainText(),
                                         documentRect);
        };
        connect(textEditor, &QPlainTextEdit::textChanged, this, syncText);
        connect(textEditor, &TextEdit::repaintTextRect, this,
                [syncText](TextEdit *, const QRectF &) { syncText(); });
        syncText();
    }

    editor->show();
    editor->raise();
    parent->raise();
    if (QWindow *window = parent->windowHandle()) {
        window->requestActivate();
    }
    QTimer::singleShot(0, editor, [editor, selectAll = request.selectAll] {
        editor->setFocus(Qt::MouseFocusReason);
        if (auto *lineEdit = qobject_cast<QLineEdit *>(editor)) {
            if (selectAll)
                lineEdit->selectAll();
        } else if (auto *textEdit = qobject_cast<TextEdit *>(editor)) {
            if (selectAll)
                textEdit->selectAll();
        }
        if (QGuiApplication::inputMethod())
            QGuiApplication::inputMethod()->update(Qt::ImQueryAll);
    });
}

void TreelandAnnotationEditor::finishInlineEdit(bool commit)
{
    if (!m_inlineEditor || m_finishingInlineEdit) {
        return;
    }
    m_finishingInlineEdit = true;
    QWidget *editor = m_inlineEditor;
    editor->removeEventFilter(this);
    disconnect(editor, nullptr, this, nullptr);
    if (commit && QGuiApplication::inputMethod())
        QGuiApplication::inputMethod()->commit();

    if (m_inlineIsSequence) {
        auto *lineEdit = qobject_cast<QLineEdit *>(editor);
        if (lineEdit && commit && lineEdit->hasAcceptableInput()) {
            bool ok = false;
            const int value = lineEdit->text().toInt(&ok);
            if (ok) {
                m_document->updateInlineSequence(m_inlineShapeIndex, value);
            }
        }
    } else {
        auto *textEditor = qobject_cast<TextEdit *>(editor);
        if (textEditor) {
            if (commit) {
                const QRect globalEditorRect =
                        textEditor->geometry().translated(m_inlineOutputBounds.topLeft());
                m_document->updateInlineText(
                        m_inlineShapeIndex,
                        textEditor->toPlainText(),
                        globalEditorRect.translated(-m_desktopBounds.topLeft()));
            } else {
                m_document->updateInlineText(m_inlineShapeIndex,
                                             m_inlineOriginalText,
                                             m_inlineOriginalRect);
            }
            m_document->finishInlineText(m_inlineShapeIndex);
        }
    }

    QWidget *editorParent = editor->parentWidget();
    editor->hide();
    editor->deleteLater();
    m_document->setInlineEditingShape(-1);
    m_inlineEditor = nullptr;
    m_inlineShapeIndex = -1;
    m_inlineIsSequence = false;
    m_inlineOutputBounds = QRect();
    m_inlineOriginalText.clear();
    m_inlineOriginalRect = QRectF();
    m_finishingInlineEdit = false;

    if (editorParent) {
        editorParent->setFocus(Qt::OtherFocusReason);
    }
}

bool TreelandAnnotationEditor::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_inlineEditor || watched != m_inlineEditor) {
        return IAnnotationEditor::eventFilter(watched, event);
    }

    if (event->type() == QEvent::FocusOut && !m_finishingInlineEdit) {
        finishInlineEdit(true);
        return false;
    }
    if (event->type() == QEvent::ShortcutOverride) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape
                || (m_inlineIsSequence
                    && (keyEvent->key() == Qt::Key_Return
                        || keyEvent->key() == Qt::Key_Enter))) {
            event->accept();
            return true;
        }
    }
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            finishInlineEdit(false);
            event->accept();
            return true;
        }
        if (m_inlineIsSequence
                && (keyEvent->key() == Qt::Key_Return
                    || keyEvent->key() == Qt::Key_Enter)) {
            auto *lineEdit = qobject_cast<QLineEdit *>(m_inlineEditor);
            if (lineEdit) {
                // Numeric keys may still be handled by the active input method
                // as a preedit. Commit that preedit first, then finish after the
                // current key event so the QLineEdit contains the final value.
                if (QGuiApplication::inputMethod())
                    QGuiApplication::inputMethod()->commit();
                QTimer::singleShot(0, this, [this] {
                    auto *editor = qobject_cast<QLineEdit *>(m_inlineEditor);
                    if (editor && editor->hasAcceptableInput())
                        finishInlineEdit(true);
                });
            }
            event->accept();
            return true;
        }
    }
    return IAnnotationEditor::eventFilter(watched, event);
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
