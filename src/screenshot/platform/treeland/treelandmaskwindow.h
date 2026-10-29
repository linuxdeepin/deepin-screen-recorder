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
class QColor;
class QPainter;

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
    void setSelectionHintOwner(bool owner);
    void setRecordingMode(bool enabled, const QRect &globalSelection = QRect(),
                          bool dimOutside = false);
    void setInputRegion(const QRegion &region);
    void resetInputRegion();
    void setCursorIntent(CursorIntent intent);
    void setAnnotationView(QWidget *widget);
    bool prepareWindow();
    void showWindow();
    void closeSilently();

signals:
    void windowRoleCreated(QWindow *window);
    void pointerPressed(const QPoint &globalPos);
    void pointerMoved(const QPoint &globalPos);
    void pointerReleased(const QPoint &globalPos);
    void pointerDoubleClicked(const QPoint &globalPos);
    void confirmRequested();
    void copyRequested();
    void cancelRequested();
    void undoRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPoint toGlobal(const QPoint &localPos) const;
    void drawSelectionOutline(QPainter &painter, const QRect &globalRect,
                              const QColor &color, int width,
                              bool dashed = false) const;

    OutputDescriptor m_output;
    QImage m_sourceImage;
    QRect m_globalSelection;
    QRect m_selection;
    QRect m_globalSnapHint;
    QRect m_snapHint;
    QRect m_globalRecordingSelection;
    QRect m_recordingSelection;
    bool m_selectionHintOwner = false;
    bool m_finalized = false;
    bool m_recordingMode = false;
    bool m_recordingDimOutside = false;
    bool m_silentClose = false;
    CursorIntent m_cursorIntent = CursorIntent::Default;
    bool m_cancelEmitted = false;
    bool m_roleCreatedConnected = false;

    QWidget *m_annotationView = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_MASK_WINDOW_H
