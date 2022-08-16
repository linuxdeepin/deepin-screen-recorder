/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ZOOMINDICATOR_H
#define ZOOMINDICATOR_H

#include "zoomIndicatorGL.h"
#include <DLabel>
#include <DWidget>
#include <QPainter>
#include <QPaintEvent>

DWIDGET_USE_NAMESPACE

class ZoomIndicator : public DLabel
{
    Q_OBJECT
public:
    explicit ZoomIndicator(DWidget *parent = 0);
    ~ZoomIndicator();

    void showMagnifier(QPoint pos);
    void hideMagnifier();

    void setCursorPos(QPoint pos);
protected:
    void paintEvent(QPaintEvent *);

private:
    QRect m_globalRect;
    QRect m_centerRect;
    QBrush m_lastCenterPosBrush;
    ZoomIndicatorGL *m_zoomIndicatorGL = nullptr;
    QPoint m_cursorPos;

    /**
     * @brief 是否打开多任务视图界面。
     * 在某些hw机型上，打开多任务视图界面后，
     * 通过opengl绘制的widget，在执行move函数卡顿，因此做了规避。
     */
    bool m_isOpenWM = false;
};

#endif // MAGNIFIER_H
