// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_MASK_OVERLAY_HOST_H
#define TREELAND_MASK_OVERLAY_HOST_H

#include "../../ports/iscreenshotoverlayhost.h"

class ToolBar;
class SideBar;
class ToolBarHost;
class QWidget;
class CountdownTooltip;

namespace Dsr::Screenshot {

class IWindowTagger;
class TreelandMaskWindow;
class TreelandAnnotationEditor;

class TreelandMaskOverlayHost : public IScreenshotOverlayHost
{
    Q_OBJECT
public:
    explicit TreelandMaskOverlayHost(IWindowTagger *windowTagger,
                                     QObject *parent = nullptr);
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
    bool startRecordingCountdown(const QRect &globalSelection) override;
    void stopRecordingCountdown() override;
    bool beginRecording(const QRect &globalSelection) override;
    void finishRecording() override;
    bool isRecordingActive() const override;
    void setRecordingControlsActive(bool active) override;
    void close() override;

private:
    friend class TreelandToolBarHost;

    TreelandMaskWindow *selectionOwnerWindow() const;
    void ensureEffectSource();
    void ensureToolBar();
    void ensureRecordingToolBar(TreelandMaskWindow *owner);
    void destroyRecordingToolBar();
    void destroyRecordingCountdown();
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
    ToolBar *m_recordingToolBar = nullptr;
    ToolBarHost *m_recordingToolBarHost = nullptr;
    TreelandMaskWindow *m_recordingToolBarOwner = nullptr;
    CountdownTooltip *m_countdown = nullptr;
    QRect m_selection;
    FrozenFrameSet m_frozenFrames;
    bool m_finalized = false;
    bool m_annotationToolActive = false;
    bool m_prepared = false;
    bool m_effectSourceReady = false;
    bool m_inputReadyPending = false;
    bool m_recordingActive = false;
    int m_inputReadyChecks = 0;
    IWindowTagger *m_windowTagger = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_MASK_OVERLAY_HOST_H
