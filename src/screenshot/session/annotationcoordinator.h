// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANNOTATION_COORDINATOR_H
#define ANNOTATION_COORDINATOR_H

#include <QObject>

namespace Dsr::Screenshot {

class IAnnotationEditor;
class SelectionModel;

/**
 * Keeps annotation geometry and selection constraints in sync.
 */
class AnnotationCoordinator : public QObject
{
    Q_OBJECT
public:
    AnnotationCoordinator(SelectionModel *selection,
                          IAnnotationEditor *editor,
                          QObject *parent = nullptr);

    void setEditor(IAnnotationEditor *editor);
    IAnnotationEditor *editor() const;
    void syncBounds();

private:
    SelectionModel *m_selection = nullptr;
    IAnnotationEditor *m_editor = nullptr;
};

} // namespace Dsr::Screenshot

#endif // ANNOTATION_COORDINATOR_H
