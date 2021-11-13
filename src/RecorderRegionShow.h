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

#ifndef RECORDERREGION_H
#define RECORDERREGION_H

#include "widgets/camerawidget.h"
#include "widgets/keybuttonwidget.h"

#include <QPainter>
#include <QDesktopWidget>
#include <QPaintEvent>


class RecorderRegionShow : public DWidget
{
    Q_OBJECT
public:
    RecorderRegionShow();
    virtual ~RecorderRegionShow();
    void initCameraInfo(const CameraWidget::Position position, const QSize size);
    void showKeyBoardButtons(const QString &key);
    void updateKeyBoardButtonStyle();
    void setCameraShow(); //设置Camera窗口显示
protected:
    void paintEvent(QPaintEvent *event);
    void updateMultiKeyBoardPos();


private:
    QPainter *m_painter;
    CameraWidget *m_cameraWidget;
    QList<KeyButtonWidget *> m_keyButtonList;
};
#endif
