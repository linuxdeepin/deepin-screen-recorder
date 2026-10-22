// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINEADJUSTTIPS_H
#define FINEADJUSTTIPS_H

#include <DBlurEffectWidget>
#include <DWidget>

#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>

DWIDGET_USE_NAMESPACE

/**
 * @brief 像素级微调快捷键提示面板。
 *
 * 对应 Sketch 中的 “提示语/截图时” 与 “提示语/选区完成时” 两个控件：
 * 未框选时提示移动鼠标指针，框选完成后提示移动、扩大、缩小选区。
 */
class FineAdjustTips : public DBlurEffectWidget
{
    Q_OBJECT
public:
    enum Mode {
        CursorMode,     // 尚未框选，提示移动鼠标指针
        SelectionMode   // 框选完成，提示移动/扩大/缩小选区
    };

    explicit FineAdjustTips(DWidget *parent = nullptr);
    ~FineAdjustTips() override;

    /**
     * @brief 切换提示内容（移动指针 / 调整选区）
     */
    void setMode(Mode mode);
    Mode mode() const;

    /**
     * @brief 固定比例激活时，扩大/缩小两行置灰且不可用
     */
    void setResizeEnabled(bool enabled);
    bool resizeEnabled() const;

    /**
     * @brief 依据当前内容重新计算尺寸，并停靠到父窗口左下角
     */
    void dockToBottomLeft();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QSize panelSize() const;
    int contentWidth() const;
    void drawKeyCap(QPainter &painter, const QRect &rect, const QString &text, bool enabled);
    void drawSeparator(QPainter &painter, const QRect &rect, bool enabled);
    void drawPlus(QPainter &painter, const QRect &rect, bool enabled);
    void drawArrowKeys(QPainter &painter, int x, int y, bool enabled);
    void drawDescription(QPainter &painter, const QPoint &topLeft, const QString &text, bool enabled);

    Mode m_mode = CursorMode;
    bool m_resizeEnabled = true;
};

#endif // FINEADJUSTTIPS_H
