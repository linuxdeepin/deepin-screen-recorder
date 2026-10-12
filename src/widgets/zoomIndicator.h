// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZOOMINDICATOR_H
#define ZOOMINDICATOR_H

#include "zoomIndicatorGL.h"

#include <DLabel>
#include <QColor>
#include <QImage>
#include <QPoint>
#include <QString>

DWIDGET_USE_NAMESPACE

class QPainter;

/**
 * @brief The ZoomIndicator class
 * 截图选区阶段跟随光标的像素放大镜与取色浮窗。
 * 视觉参数取自 Sketch 画板「截图录屏-光标指针」。
 */
class ZoomIndicator : public DLabel
{
    Q_OBJECT
public:
    // 浮窗尺寸（逻辑像素），取自 Sketch 画板「截图录屏-光标指针」
    static constexpr int PANEL_WIDTH = 150;
    static constexpr int PANEL_HEIGHT = 170;

    explicit ZoomIndicator(DWidget *parent = nullptr, bool isRecord = false);
    ~ZoomIndicator() override;

    void showMagnifier(QPoint pos);
    void hideMagnifier();

    // 屏幕物理像素坐标，用于放大取样与取色
    void setCursorPos(QPoint pos);
    // 屏幕逻辑坐标，用于浮窗上的坐标文案
    void setScreenPos(QPoint pos);

    QString currentColorHex() const;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QColor sampleColor() const;
    const QImage &cachedFullscreenImage() const;
    void drawPreview(QPainter &painter);
    void drawCrosshair(QPainter &painter);
    void drawInfoPanel(QPainter &painter);

    ZoomIndicatorGL *m_zoomIndicatorGL = nullptr;
    // 物理像素坐标（全屏截图坐标系）
    QPoint m_cursorPos;
    // 逻辑屏幕坐标（坐标文案显示）
    QPoint m_screenPos;

    /**
     * @brief 是否打开多任务视图界面。
     * 在某些hw机型上，打开多任务视图界面后，
     * 通过opengl绘制的widget，在执行move函数卡顿，因此做了规避。
     */
    bool m_isOpenWM = false;
    bool m_isRecord = false;

    mutable QImage m_cachedImage;
    mutable qint64 m_cachedPixmapKey = 0;
};

#endif // ZOOMINDICATOR_H
