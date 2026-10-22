// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_MASK_WINDOW_H
#define TREELAND_MASK_WINDOW_H

#include "../../core/screenshottypes.h"

#include <QImage>
#include <QRect>
#include <QWidget>

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QPushButton;

namespace Dsr::Screenshot {

class TreelandMaskWindow : public QWidget
{
    Q_OBJECT
public:
    explicit TreelandMaskWindow(const OutputDescriptor &output, QWidget *parent = nullptr);
    ~TreelandMaskWindow() override;

    OutputDescriptor output() const;
    void setFrozenFrame(const QImage &image);
    void setSelection(const QRect &globalSelection);
    void setSnapHint(const QRect &globalRegion);
    void setSelectionFinalized(bool finalized);
    void setToolbarVisible(bool visible);
    void setCursorIntent(CursorIntent intent);
    void prepareWindow();
    void showWindow();
    void closeSilently();

signals:
    void pointerPressed(const QPoint &globalPos);
    void pointerMoved(const QPoint &globalPos);
    void pointerReleased(const QPoint &globalPos);
    void confirmRequested();
    void copyRequested();
    void cancelRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void updateToolbar();
    QPoint toGlobal(const QPoint &localPos) const;

    OutputDescriptor m_output;
    QImage m_sourceImage;
    QRect m_selection;
    QRect m_snapHint;
    bool m_finalized = false;
    bool m_toolbarVisible = true;
    bool m_silentClose = false;
    bool m_cancelEmitted = false;

    QWidget *m_toolbar = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_copyButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QPushButton *m_closeButton = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_MASK_WINDOW_H
