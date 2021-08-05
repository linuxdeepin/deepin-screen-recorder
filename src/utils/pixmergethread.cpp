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

const int PixMergeThread::LONG_IMG_MAX_HEIGHT = 10000;

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
    ++m_ImageCount;
    qDebug() << "==========" << m_ImageCount;
    m_pixImgs.enqueue(picture);
}

QImage PixMergeThread::getMerageResult() const
{
    return  QImage(m_curImg.data, m_curImg.cols, m_curImg.rows, static_cast<int>(m_curImg.step),QImage::Format_ARGB32);
}

void PixMergeThread::run()
{
    m_loopTask = true;
    while (m_loopTask) {
        if(!m_pixImgs.isEmpty()) {
            QPixmap temp = m_pixImgs.dequeue();
            cv::Mat matImg = qPixmapToCvMat(temp);
            if(m_curImg.empty()) {
                m_curImg = matImg;
                continue;
            }
            if (false == mergeImageWork(matImg)){
                break;
            }
            // 更新预览图
            emit updatePreviewImg(QImage(m_curImg.data, m_curImg.cols, m_curImg.rows, static_cast<int>(m_curImg.step),QImage::Format_ARGB32));
        }
        QThread::currentThread()->msleep(500);
    }
}

cv::Mat PixMergeThread::qPixmapToCvMat(const QPixmap &inPixmap)
{
    //qDebug() << inPixmap.toImage().format();
    if(QImage::Format_RGB32 == inPixmap.toImage().format()) {
        QImage   swapped = inPixmap.toImage();
        return cv::Mat(swapped.height(), swapped.width(), CV_8UC4, const_cast<uchar*>(swapped.bits()),
                       static_cast<size_t>(swapped.bytesPerLine())).clone();

    }
}

bool PixMergeThread::mergeImageWork(const cv::Mat &image)
{
    if(m_curImg.rows > LONG_IMG_MAX_HEIGHT) {
        // 拼接超过了最大限度
        emit merageImgState(3);
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
    cv::Mat temp = image2_gray(cv::Range(0, 35), cv::Range::all());

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
        image.copyTo(cv::Mat(result, cv::Rect(0, maxLoc.y - 1, image.cols, image.rows)));

        if (result.rows <= m_curImg.rows) {
            // 拼接前后图片高度不变，或减少了
            // 拼接到重复图片，拼接到低了
            // 往回滚动导致拼接比之前还低了
            emit merageImgState(2);
            return false;
        }
        m_curImg = result;
        return true;
    } else {
        // 拼接失败
        emit merageImgState(1);
        return false;
    }
}



