// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "annotationcoordinator.h"

#include "../core/selectionmodel.h"
#include "../ports/iannotationeditor.h"

namespace Dsr::Screenshot {

AnnotationCoordinator::AnnotationCoordinator(SelectionModel *selection,
                                             IAnnotationEditor *editor,
                                             QObject *parent)
    : QObject(parent)
    , m_selection(selection)
{
    Q_ASSERT(m_selection);
    setEditor(editor);
}

void AnnotationCoordinator::setEditor(IAnnotationEditor *editor)
{
    if (m_editor == editor) {
        syncBounds();
        return;
    }

    if (m_editor) {
        disconnect(m_editor, &IAnnotationEditor::contentBoundsChanged,
                   m_selection, &SelectionModel::setAnnotationBounds);
    }
    m_editor = editor;
    if (m_editor) {
        connect(m_editor, &IAnnotationEditor::contentBoundsChanged,
                m_selection, &SelectionModel::setAnnotationBounds);
    }
    syncBounds();
}

IAnnotationEditor *AnnotationCoordinator::editor() const
{
    return m_editor;
}

void AnnotationCoordinator::syncBounds()
{
    m_selection->setAnnotationBounds(m_editor ? m_editor->contentBoundsGlobal() : QRectF());
}

} // namespace Dsr::Screenshot
