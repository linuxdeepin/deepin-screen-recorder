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

#ifndef BUTTONFEEDBACK_H
#define BUTTONFEEDBACK_H

#include <DWidget>
#include <QTimer>

DWIDGET_USE_NAMESPACE

class ButtonFeedback : public DWidget
{
    Q_OBJECT

    static const int FRAME_RATE;

public:
    explicit ButtonFeedback(DWidget *parent = nullptr);
    virtual ~ButtonFeedback();

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void showPressFeedback(int x, int y);
    void showDragFeedback(int x, int y);
    void showReleaseFeedback(int x, int y);
    void update();

private:
    QPixmap buttonFeedbackImg[10];

    QTimer *timer;

    int frameIndex;
    QPainter* m_painter = nullptr;
};

#endif
