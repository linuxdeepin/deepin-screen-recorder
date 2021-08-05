/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     He Mingyang<hemingyang@uniontech.com>
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

#ifndef SCROLLSCREENSHOT_H
#define SCROLLSCREENSHOT_H

#include "pixmergethread.h"
#include <QPixmap>
#include <QTimer>
#include <QRect>
#include<opencv2/opencv.hpp>

class ScrollScreenshot: public QObject
{
    Q_OBJECT
public:
    explicit ScrollScreenshot(QObject *parent = nullptr);
    void addPixmap(QPixmap piximg);
    QImage savePixmap();
    QRect getChangeArea(cv::Mat &img1, cv::Mat &img2);
    void calcHeadHeight();
signals:
    void getOneImg();
    void updatePreviewImg(QImage img);
public slots:
    void merageImgState(int state);
private:

    PixMergeThread *m_PicMerageThread = nullptr;

    QTimer *m_mouseWheelTimer = nullptr;
    // 长图顶部固定区域高度
    unsigned int m_headHeight = 0;
    // 截图实际高度
    unsigned int m_shotImgHeight = 0;
    // 截图宽度
    unsigned int m_shotImgWidth = 0;

    // 滚动事件发送m_shotFrequency次，截图一次。控制截图频率
    unsigned int m_shotFrequency = 1;
};

#endif // AUDIOUTILS_H
