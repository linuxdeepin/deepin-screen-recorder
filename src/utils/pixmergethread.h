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
#include <QMutex>
#include <QThread>
#include <QPixmap>
#include <QImage>
#include <QQueue>
#include <QDebug>

#include<opencv2/opencv.hpp>


class PixMergeThread : public QThread
{
    Q_OBJECT

    Q_ENUMS(MergeErrorValue)
public:

    enum MergeErrorValue{
        Failed = 1,     // 拼接失败， 拼接出现不重叠区域
        ReachBottom,    // 图片长度不再曾涨(滚动到底)，甚至减少（反向滚动出现）
        MaxHeight,      // 最大高度
    };
    explicit PixMergeThread(QObject *parent = nullptr);
    ~PixMergeThread();
    void stopTask();
    void addShotImg(const QPixmap &picture);
    QImage getMerageResult() const;
    void run();
protected:
    cv::Mat qPixmapToCvMat( const QPixmap &inPixmap);
    bool mergeImageWork(const cv::Mat &image);
    int getFixedHigh(cv::Mat &img1, cv::Mat &img2);
signals:
    void merageError(MergeErrorValue state);
    void updatePreviewImg(QImage img);
private:
    QMutex m_Mutex;
    bool m_loopTask = true;
    cv::Mat m_curImg;
    QQueue<QPixmap> m_pixImgs;
    unsigned int m_ImageCount = 0;
    unsigned int m_MeragerCount = 0;
    // 长图顶部固定区域高度
    int m_headHeight = -1;
    static const int LONG_IMG_MAX_HEIGHT;
    static const int TEMPLATE_HEIGHT;
};

Q_DECLARE_METATYPE(PixMergeThread::MergeErrorValue);
#endif //
