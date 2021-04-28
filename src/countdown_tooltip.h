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

#ifndef COUNTDOWNTOOLTIP_H
#define COUNTDOWNTOOLTIP_H

#include <DWidget>
#include <QTimer>

DWIDGET_USE_NAMESPACE

class CountdownTooltip : public DWidget
{
    Q_OBJECT

    static const int NUMBER_PADDING_Y;

public:
    explicit CountdownTooltip(DWidget *parent = 0);
    void start();

signals:
    void finished();

public slots:
    void update();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPixmap countdown1Img;
    QPixmap countdown2Img;
    QPixmap countdown3Img;

    int showCountdownCounter;

    QTimer *showCountdownTimer;
    int m_themeType = 0;
    QString text;
};

#endif // COUNTDOWNTOOLTIP_H
