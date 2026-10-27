// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_MASK_OVERLAY_HOST_H
#define TREELAND_MASK_OVERLAY_HOST_H

#include "../../ports/iscreenshotoverlayhost.h"

class ToolBar;
class SideBar;
class ToolBarHost;

namespace Dsr::Screenshot {

class TreelandMaskWindow;
class TreelandAnnotationEditor;

class TreelandMaskOverlayHost : public IScreenshotOverlayHost
{
    Q_OBJECT
public:
    explicit TreelandMaskOverlayHost(QObject *parent = nullptr);
    ~TreelandMaskOverlayHost() override;

    bool create(const QList<OutputDescriptor> &outputs) override;
    IAnnotationEditor *annotationEditor() const override;
    QList<QWindow *> maskWindows() const override;
    void setFrozenFrames(const FrozenFrameSet &frames) override;
    bool prepare() override;
    void show() override;
    void setSelection(const QRect &globalSelection) override;
    void setSnapHint(const QRect &globalRegion) override;
    void setSelectionFinalized(bool finalized) override;
    void setCursorIntent(CursorIntent intent) override;
    void close() override;

private:
    friend class TreelandToolBarHost;

    TreelandMaskWindow *selectionOwnerWindow() const;
    void ensureToolBar();
    void updateToolBar();
    void handleToolChanged(const QString &tool);
    void checkInputReady();
    void undoAnnotation();
    void destroyWindows();

    QList<TreelandMaskWindow *> m_windows;
    TreelandAnnotationEditor *m_annotationEditor = nullptr;
    ToolBar *m_toolBar = nullptr;
    SideBar *m_sideBar = nullptr;
    ToolBarHost *m_toolBarHost = nullptr;
    QRect m_selection;
    bool m_finalized = false;
    bool m_annotationToolActive = false;
    bool m_prepared = false;
    bool m_inputReadyPending = false;
    int m_inputReadyChecks = 0;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_MASK_OVERLAY_HOST_H
