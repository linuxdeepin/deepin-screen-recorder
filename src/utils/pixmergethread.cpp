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

#include "pixmergethread.h"

#include <QMutexLocker>

const int PixMergeThread::LONG_IMG_MAX_HEIGHT = 10000;
const int PixMergeThread::TEMPLATE_HEIGHT = 35;

PixMergeThread::PixMergeThread(QObject *parent) : QThread(parent)
{

}

PixMergeThread::~PixMergeThread()
{

}

void PixMergeThread::stopTask()
{
    m_loopTask = false;
}

void PixMergeThread::addShotImg(const QPixmap &picture)
{
    if (m_loopTask == false)
        return;

    // 计数调试
    ++m_ImageCount;
    qDebug() << "==========" << m_ImageCount;


    if (m_curImg.empty()) {
        m_curImg = qPixmapToCvMat(picture);
        return;
    }


    if (m_headHeight == -1) {
        cv::Mat tempData = qPixmapToCvMat(picture);
        m_headHeight = getFixedHigh(m_curImg, tempData);
        qDebug() << "计算出顶部固定高度:"<< m_headHeight;
    }

    QMutexLocker locker(&m_Mutex);
    if(m_headHeight > 0) {
        // 将图片顶部固定区域裁剪掉后，加入拼接队列
        m_pixImgs.enqueue(picture.copy(0, m_headHeight, picture.width(), picture.height() - m_headHeight));
    } else {
        m_pixImgs.enqueue(picture);
    }
}

QImage PixMergeThread::getMerageResult() const
{
    return  QImage(m_curImg.data, m_curImg.cols, m_curImg.rows, static_cast<int>(m_curImg.step),QImage::Format_ARGB32);
}

void PixMergeThread::run()
{
    m_loopTask = true;
    while (m_loopTask) {
        while (!m_pixImgs.isEmpty()) {
            QPixmap temp;
            {
                QMutexLocker locker(&m_Mutex);
                temp = m_pixImgs.dequeue();
            }
            cv::Mat matImg = qPixmapToCvMat(temp);
            if (!mergeImageWork(matImg)) {
                //拼接失败
                qDebug() << __FUNCTION__ << __LINE__ << "Merge ERROR!!!!!!!";
                return;
            }
            // 更新预览图
            emit updatePreviewImg(QImage(m_curImg.data, m_curImg.cols, m_curImg.rows, static_cast<int>(m_curImg.step),QImage::Format_ARGB32));
        }
        QThread::currentThread()->msleep(300);
    }
}

cv::Mat PixMergeThread::qPixmapToCvMat(const QPixmap &inPixmap)
{
    //qDebug() << inPixmap.toImage().format();
    //if(QImage::Format_RGB32 == inPixmap.toImage().format()) {
    QImage   swapped = inPixmap.toImage();
    return cv::Mat(swapped.height(), swapped.width(), CV_8UC4, const_cast<uchar*>(swapped.bits()),
                   static_cast<size_t>(swapped.bytesPerLine())).clone();

    //}
}

bool PixMergeThread::mergeImageWork(const cv::Mat &image)
{
    if(m_curImg.rows > LONG_IMG_MAX_HEIGHT) {
        // 拼接超过了最大限度
        emit merageError(MaxHeight);
        return false;
    }

    if(image.rows <= TEMPLATE_HEIGHT) {
        emit merageError(Failed);
        return false;
    }

    ++m_MeragerCount;
    qDebug() << "**************" << m_MeragerCount;
    /*转灰度图像*/
    cv::Mat image1_gray, image2_gray;
    cvtColor(m_curImg, image1_gray, CV_BGR2GRAY);
    cvtColor(image, image2_gray, CV_BGR2GRAY);
    /*
     * 取图像2的全部行，1到35列作为模板
     * 这样image1作为原图，temp作为模板图像
     */
    cv::Mat temp = image2_gray(cv::Range(0, TEMPLATE_HEIGHT), cv::Range::all());
    /*结果矩阵图像,大小，数据类型*/
    cv::Mat res(image1_gray.rows - temp.rows + 1, image2_gray.cols - temp.cols + 1, CV_32FC1);
    /*模板匹配，采用归一化相关系数匹配*/
    matchTemplate(image1_gray, temp, res, CV_TM_CCOEFF_NORMED);
    /*结果矩阵阈值化处理*/
    threshold(res, res, 0.8, 1, CV_THRESH_TOZERO);
    double minVal, maxVal, thresholdv = 0.8;

    /*查找最大值及位置*/
    cv::Point minLoc, maxLoc;
    minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc);
    /*图像拼接*/
    cv::Mat temp1, result;
    if (maxVal >= thresholdv && maxLoc.y > 0)//只有度量值大于阈值才认为是匹配
    {
        result = cv::Mat::zeros(cvSize(m_curImg.cols, maxLoc.y + image.rows), m_curImg.type());
        temp1 = m_curImg(cv::Rect(0, 0, m_curImg.cols, maxLoc.y));
        /*将图1的非模板部分和图2拷贝到result*/
        temp1.copyTo(cv::Mat(result, cv::Rect(0, 0, m_curImg.cols, maxLoc.y)));
        //image.copyTo(cv::Mat(result, cv::Rect(0, maxLoc.y - 1, image.cols, image.rows)));
        image.copyTo(cv::Mat(result, cv::Rect(0, maxLoc.y, image.cols, image.rows)));

        if (result.rows == m_curImg.rows) {
            // 拼接前后图片高度不变
            // 拼接到重复图片，拼接到低了
            emit merageError(ReachBottom);
            return false;
        } else if (result.rows < m_curImg.rows) {
            // 拼接前后图片高度减少了
            // 往回滚动导致拼接比之前还低了
            emit merageError(Failed);
            return false;
        }
        m_curImg = result;
        return true;
    } else {
        // 测试代码
        //imwrite("curImg.png", m_curImg);
        //imwrite("image.png", image);
        emit merageError(Failed);
        return false;
    }
}

int PixMergeThread::getFixedHigh(cv::Mat &img1, cv::Mat &img2)
{
    // 计算变化部分
    for(int i = 0; i < img1.rows; ++i) {
        for(int j = 0; j < img1.cols; ++j) {
            if(img1.at<cv::Vec3b>(i, j) != img2.at<cv::Vec3b>(i, j)){
                return i;
            }
        }
    }
    return 0;
}



