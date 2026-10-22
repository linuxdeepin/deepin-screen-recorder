// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_MASK_OVERLAY_HOST_H
#define TREELAND_MASK_OVERLAY_HOST_H

#include "../../ports/iscreenshotoverlayhost.h"

namespace Dsr::Screenshot {

class TreelandMaskWindow;

class TreelandMaskOverlayHost : public IScreenshotOverlayHost
{
    Q_OBJECT
public:
    explicit TreelandMaskOverlayHost(QObject *parent = nullptr);
    ~TreelandMaskOverlayHost() override;

    bool create(const QList<OutputDescriptor> &outputs) override;
    QList<QWindow *> maskWindows() const override;
    void setFrozenFrames(const FrozenFrameSet &frames) override;
    void show() override;
    void setSelection(const QRect &globalSelection) override;
    void setSnapHint(const QRect &globalRegion) override;
    void setSelectionFinalized(bool finalized) override;
    void setCursorIntent(CursorIntent intent) override;
    void close() override;

private:
    TreelandMaskWindow *toolbarWindow() const;
    void updateToolbarVisibility();
    void destroyWindows();

    QList<TreelandMaskWindow *> m_windows;
    QRect m_selection;
    bool m_finalized = false;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_MASK_OVERLAY_HOST_H
