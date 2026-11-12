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
class FineAdjustTips;
class RoundedCornerWidget;
class ZoomIndicator;
class PreviewWidget;
class ScrollShotTip;

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
    void setSelectionHints(bool visible,
                           bool selectionMode,
                           const QPoint &globalCursor,
                           bool hasGlobalCursor) override;
    void setRecordModeActive(bool active) override;
    void setInitialMode(Mode mode) override;
    QString magnifierColorHex() const override;
    bool startRecordingCountdown(const QRect &globalSelection) override;
    void stopRecordingCountdown() override;
    bool beginRecording(const QRect &globalSelection) override;
    void finishRecording() override;
    bool isRecordingActive() const override;
    void setRecordingControlsActive(bool active) override;
    bool beginScroll(const QRect &globalSelection) override;
    void setScrollPreview(const QImage &image) override;
    void beginScrollFrameCapture() override;
    void endScrollFrameCapture() override;
    void showScrollTip(ScrollTipKind kind) override;
    void hideScrollTip() override;
    void finishScroll(bool restoreSelectionUi = true) override;
    bool isScrollActive() const override;
    void close() override;

private:
    friend class TreelandToolBarHost;

    TreelandMaskWindow *selectionOwnerWindow() const;
    TreelandMaskWindow *windowForGlobalPoint(const QPoint &globalPos) const;
    void ensureEffectSource();
    void ensureToolBar();
    void ensureSelectionWidgets();
    void destroyRecordingCountdown();
    void applyScrollInputRegions();
    void ensureScrollPreview();
    void destroyScrollPreview();
    void ensureScrollTip();
    void destroyScrollTip();
    QPoint scrollTipPosition(const QSize &tipSize,
                             TreelandMaskWindow *owner) const;
    void applyRoundedCornerRadiusToWindows();
    QRect widgetGlobalRect(QWidget *widget) const;
    void updateToolBar();
    void updateRoundedCornerWidget();
    void updateFineAdjustTips();
    void updateMagnifier();
    void hideMagnifier();
    qreal selectionPixelScale() const;
    void handleToolChanged(const QString &tool);
    void checkInputReady();
    void undoAnnotation();
    void destroyWindows();

    QList<TreelandMaskWindow *> m_windows;
    TreelandAnnotationEditor *m_annotationEditor = nullptr;
    ToolBar *m_toolBar = nullptr;
    SideBar *m_sideBar = nullptr;
    ToolBarHost *m_toolBarHost = nullptr;
    RoundedCornerWidget *m_roundedCornerWidget = nullptr;
    FineAdjustTips *m_fineAdjustTips = nullptr;
    ZoomIndicator *m_magnifier = nullptr;
    CountdownTooltip *m_countdown = nullptr;
    PreviewWidget *m_scrollPreview = nullptr;
    ScrollShotTip *m_scrollTip = nullptr;
    QRect m_selection;
    QRect m_scrollSelection;
    QPoint m_globalCursor = QPoint(-1, -1);
    FrozenFrameSet m_frozenFrames;
    bool m_finalized = false;
    bool m_annotationToolActive = false;
    bool m_prepared = false;
    bool m_effectSourceReady = false;
    bool m_inputReadyPending = false;
    bool m_recordingActive = false;
    bool m_scrollActive = false;
    bool m_scrollFrameCaptureInProgress = false;
    bool m_scrollPreviewHiddenForCapture = false;
    bool m_scrollTipHiddenForCapture = false;
    bool m_scrollToolBarHiddenForCapture = false;
    bool m_recordModeActive = false;
    bool m_hintsVisible = false;
    bool m_hintsSelectionMode = false;
    bool m_hasGlobalCursor = false;
    bool m_toolBarManuallyMoved = false;
    int m_inputReadyChecks = 0;
    IWindowTagger *m_windowTagger = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_MASK_OVERLAY_HOST_H
