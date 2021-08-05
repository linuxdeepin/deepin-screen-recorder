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

#ifndef PIXMERGETHREAD_H
#define PIXMERGETHREAD_H

#include <QObject>
#include <QThread>
#include <QPixmap>
#include <QImage>
#include <QQueue>
#include <QDebug>

#include<opencv2/opencv.hpp>


class PixMergeThread : public QThread
{
    Q_OBJECT
public:
    explicit PixMergeThread(QObject *parent = nullptr);
    ~PixMergeThread();
    void stopTask();
    void addShotImg(const QPixmap &picture);
    QImage getMerageResult() const;
    void run();
protected:
    cv::Mat qPixmapToCvMat( const QPixmap &inPixmap);
    bool mergeImageWork(const cv::Mat &image);
signals:
    void merageImgState(int state);
    void updatePreviewImg(QImage img);
private:
    bool m_loopTask = true;
    cv::Mat m_curImg;
    QQueue<QPixmap> m_pixImgs;
    unsigned int m_ImageCount = 0;
    unsigned int m_MeragerCount = 0;
    static const int LONG_IMG_MAX_HEIGHT;
};
#endif //
