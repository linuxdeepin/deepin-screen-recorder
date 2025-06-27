// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pixmergethread.h"
#include "log.h"

#include <QMutexLocker>
#include <QDateTime>
#include <opencv2/imgproc/types_c.h>
const int PixMergeThread::LONG_IMG_MAX_HEIGHT = 10000;
const int PixMergeThread::TEMPLATE_HEIGHT = 50;

PixMergeThread::PixMergeThread(QObject *parent) : QThread(parent)
{
#if (QT_VERSION_MAJOR == 5)
    m_lastTime = int(QDateTime::currentDateTime().toTime_t());
#elif (QT_VERSION_MAJOR == 6)
    m_lastTime = int(QDateTime::currentDateTime().toSecsSinceEpoch());
#endif
    qCDebug(dsrApp) << "PixMergeThread initialized";
}

PixMergeThread::~PixMergeThread()
{
    qCDebug(dsrApp) << "PixMergeThread destructor called.";
}

void PixMergeThread::stopTask()
{
    m_loopTask = false;
    qCDebug(dsrApp) << "Stopping task loop.";
}

//滚动向上时添加
void PixMergeThread::ScrollUpAddImg(const QPixmap &picture)
{
    qCDebug(dsrApp) << "Adding image for scroll up.";
    if (m_bottomHeight == -1) {
        cv::Mat tempData = qPixmapToCvMat(picture);
        m_bottomHeight = getBottomFixedHigh(m_curImg, tempData);
        qDebug() << "计算出底部固定高度:" << m_bottomHeight;
    }
    QPair<QPixmap, PictureDirection> pair;
    QMutexLocker locker(&m_Mutex);
    if (m_bottomHeight > 0) {
        // 将图片底部固定区域裁剪掉后，加入拼接队列
        pair.first = picture.copy(0, 0, picture.width(), m_bottomHeight);
        pair.second = ScrollUp;
        m_pixImgs.enqueue(pair);
        qCDebug(dsrApp) << "Added cropped image to queue, height:" << m_bottomHeight;
    } else {
        pair.first = picture;
        pair.second = ScrollUp;
        m_pixImgs.enqueue(pair);
        qCDebug(dsrApp) << "Added full image to queue";
    }
}

//滚动向下时添加
void PixMergeThread::ScrollDwonAddImg(const QPixmap &picture)
{
    qCDebug(dsrApp) << "Adding image for scroll down.";
    if (m_headHeight == -1) {
        cv::Mat tempData = qPixmapToCvMat(picture);
        m_headHeight = getTopFixedHigh(m_curImg, tempData);
        qCDebug(dsrApp) << "Calculated top fixed height:" << m_headHeight;
    }
    QPair<QPixmap, PictureDirection> pair;
    QMutexLocker locker(&m_Mutex);
    if (m_headHeight > 0) {
        // 将图片顶部固定区域裁剪掉后，加入拼接队列
        pair.first = picture.copy(0, m_headHeight, picture.width(), picture.height() - m_headHeight);
        pair.second = ScrollDown;
        m_pixImgs.enqueue(pair);
        qCDebug(dsrApp) << "Added cropped image to queue, head height:" << m_headHeight;
    } else {
        pair.first = picture;
        pair.second = ScrollDown;
        m_pixImgs.enqueue(pair);
        qCDebug(dsrApp) << "Added full image to queue for ScrollDown.";
    }
}

void PixMergeThread::addShotImg(const QPixmap &picture, PictureDirection direction)
{
    qCDebug(dsrApp) << "Adding shot image with direction:" << direction;
    if (m_loopTask == false) {
        qCDebug(dsrApp) << "Task stopped, image not added";
        return;
    }

    // 计数调试
    ++m_ImageCount;
    qDebug() << "==========" << m_ImageCount;


    if (m_curImg.empty()) {
        m_curImg = qPixmapToCvMat(picture);
        qCDebug(dsrApp) << "First image initialized.";
        return;
    }
    //将图片去除顶部或者底部的固定区域，放入图片队列
    if (direction == ScrollDown) {
        qCDebug(dsrApp) << "Calling ScrollDwonAddImg.";
        ScrollDwonAddImg(picture);
    } else {
        qCDebug(dsrApp) << "Calling ScrollUpAddImg.";
        ScrollUpAddImg(picture);
    }
}

QImage PixMergeThread::getMerageResult() const
{
    qCDebug(dsrApp) << "Getting merged image result.";
    return  QImage(m_curImg.data, m_curImg.cols, m_curImg.rows, static_cast<int>(m_curImg.step), QImage::Format_ARGB32).copy();
}

void PixMergeThread::run()
{
    m_loopTask = true;
    qCInfo(dsrApp) << "PixMergeThread started running";
    while (m_loopTask) {
        while (!m_pixImgs.isEmpty()) {
            QPair<QPixmap, PictureDirection> pair;
            {
                QMutexLocker locker(&m_Mutex);
                pair = m_pixImgs.dequeue();
            }
            cv::Mat matImg = qPixmapToCvMat(pair.first);
            if (mergeImageWork(matImg, pair.second)) {
                // 更新预览图
                emit updatePreviewImg(QImage(m_curImg.data, m_curImg.cols, m_curImg.rows,
                                             static_cast<int>(m_curImg.step), QImage::Format_ARGB32));
                qCDebug(dsrApp) << "Preview image updated";
            }
        }
        QThread::currentThread()->msleep(300);
    }
    qCDebug(dsrApp) << "Thread run loop finished.";
}
//设置是否为手动模式
void PixMergeThread::setScrollModel(bool isManualScrollMode)
{
    m_isManualScrollModel = isManualScrollMode;
    qCDebug(dsrApp) << "Setting scroll mode to manual:" << isManualScrollMode;
    //m_lastTime = QDateTime::currentDateTime().toTime_t();
}

void PixMergeThread::clearCurImg()
{
    m_curImg.release();
    qCDebug(dsrApp) << "Cleared current image.";
}

//计算时间差
void PixMergeThread::calculateTimeDiff(int time)
{
    m_curTimeDiff = (time - m_lastTime) * 100;
    qCDebug(dsrApp) << "Calculated time difference: " << m_curTimeDiff << " from time: " << time << " last time: " << m_lastTime;
    m_lastTime = time;
}

bool PixMergeThread::isOneWay()
{
    qCDebug(dsrApp) << "Checking if scroll is one way.";
    if (m_upCount > 0 && m_downCount > 0)
        return false;
    return true;
}

// 设置最后一张图片标记
void PixMergeThread::setIsLastImg(bool isLastImg)
{
    m_isLastPixmap = isLastImg;
    qCDebug(dsrApp) << "Setting isLastImg flag to:" << isLastImg;
}

cv::Mat PixMergeThread::qPixmapToCvMat(const QPixmap &inPixmap)
{
    qCDebug(dsrApp) << "Converting QPixmap to cv::Mat.";
    //qDebug() << inPixmap.toImage().format();
    //if(QImage::Format_RGB32 == inPixmap.toImage().format()) {
    QImage   swapped = inPixmap.toImage();
    return cv::Mat(swapped.height(), swapped.width(), CV_8UC4, const_cast<uchar *>(swapped.bits()),
                   static_cast<size_t>(swapped.bytesPerLine())).clone();

    //}
}

bool PixMergeThread::mergeImageWork(const cv::Mat &image, int imageStatus)
{
    qCDebug(dsrApp) << "Merging image with status:" << imageStatus;
    if (image.empty()) {
        qCWarning(dsrApp) << "Input image is empty for merge operation";
        return false;
    }

    bool isSucess = false;
    switch (imageStatus) {
    case ScrollDown:
        qCDebug(dsrApp) << "Merging image for ScrollDown.";
        isSucess = splicePictureDown(image);
        return isSucess;
    case ScrollUp:
        qCDebug(dsrApp) << "Merging image for ScrollUp.";
        isSucess = splicePictureUp(image);
        return isSucess;
    default:
        qCWarning(dsrApp) << "Invalid image status for merge operation:" << imageStatus;
        return isSucess;
    }
}

int PixMergeThread::getTopFixedHigh(cv::Mat &img1, cv::Mat &img2)
{
    qCDebug(dsrApp) << "Calculating top fixed height.";
    // 计算变化部分
    for (int i = 0; i < img1.rows; ++i) {
        for (int j = 0; j < img1.cols; ++j) {
            if (img1.at<cv::Vec3b>(i, j) != img2.at<cv::Vec3b>(i, j)) {
                qCDebug(dsrApp) << "Found top fixed height:" << i;
                return i;
            }
        }
    }
    qCDebug(dsrApp) << "No top fixed height found, returning 0.";
    return 0;
}
//裁剪底部固定区域
int PixMergeThread::getBottomFixedHigh(cv::Mat &img1, cv::Mat &img2)
{
    qCDebug(dsrApp) << "Calculating bottom fixed height.";
    int rowsCount = img2.rows - 1;
    // 计算变化部分
    for (int i = img1.rows - 1; i > 0; i--) {
        for (int j = 0; j < img1.cols; ++j) {
            if (img1.at<cv::Vec3b>(i, j) != img2.at<cv::Vec3b>(rowsCount, j)) {
                qCDebug(dsrApp) << "Found bottom fixed height:" << rowsCount;
                return rowsCount;
            }
        }
        --rowsCount;
    }
    qCDebug(dsrApp) << "No bottom fixed height found, returning 0.";
    return 0;
}

//向上拼接图片
bool PixMergeThread::splicePictureUp(const cv::Mat &image)
{
    qCDebug(dsrApp) << "Splicing picture upwards.";
    // 保存后的最后一张图片不做长度检查
    if (!m_isLastPixmap && m_curImg.rows > LONG_IMG_MAX_HEIGHT) {
        // 拼接超过了最大限度
        qCWarning(dsrApp) << "Image height exceeds maximum limit:" << LONG_IMG_MAX_HEIGHT;
        emit merageError(MaxHeight);
        return false;
    }

    if (image.rows <= TEMPLATE_HEIGHT) {
        qCWarning(dsrApp) << "Image height is too small for splicing, minimum required:" << TEMPLATE_HEIGHT;
        emit merageError(Failed);
        return false;
    }

    ++m_MeragerCount;
    qCDebug(dsrApp) << "Merge count increased to:" << m_MeragerCount;
    m_upCount++;
    /*转灰度图像*/
    cv::Mat image1_gray, image2_gray;
    cvtColor(image, image1_gray, CV_BGR2GRAY);
    cvtColor(m_curImg, image2_gray, CV_BGR2GRAY);
    /*
     * 取图像2的全部行，1到35列作为模板
     * 这样image1作为原图，temp作为模板图像
     */
    cv::Mat temp = image1_gray(cv::Range(image.rows - TEMPLATE_HEIGHT, image.rows), cv::Range::all());
    /*结果矩阵图像,大小，数据类型*/
    cv::Mat res(image2_gray.rows - temp.rows + 1, image2_gray.cols - temp.cols + 1, CV_32FC1);
    /*模板匹配，采用归一化相关系数匹配*/
    matchTemplate(image2_gray, temp, res, CV_TM_CCOEFF_NORMED);
    /*结果矩阵阈值化处理*/
    threshold(res, res, 0.8, 1, CV_THRESH_TOZERO);
    double minVal, maxVal, thresholdv = 0.8;

    /*查找最大值及位置*/
    cv::Point minLoc, maxLoc;
    minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc);
    /*图像拼接*/
    cv::Mat temp1, result;
    if (maxVal >= thresholdv && maxLoc.y > 0) { //只有度量值大于阈值才认为是匹配
        result = cv::Mat::zeros(cvSize(image.cols, image.rows + m_curImg.rows - maxLoc.y - TEMPLATE_HEIGHT), image.type());
        temp1 = m_curImg(cv::Rect(0, maxLoc.y + TEMPLATE_HEIGHT, m_curImg.cols, m_curImg.rows - maxLoc.y - TEMPLATE_HEIGHT));
        if (temp1.empty()) {
            qCWarning(dsrApp) << "Failed to extract template region for upward splicing";
            return false;
        }

        image.copyTo(cv::Mat(result, cv::Rect(0, 0, image.cols, image.rows)));
        temp1.copyTo(cv::Mat(result, cv::Rect(0, image.rows, temp1.cols, temp1.rows)));
        if (result.rows == m_curImg.rows) {// 拼接前后图片高度不变
            if (m_MeragerCount == 1) {
                cv::Mat curImg = m_curImg;//拷贝不影响m_curImg
                QRect rect = getScrollChangeRectArea(curImg, image);
                if (rect.width() < 0 || rect.height() < 0) {
                    qDebug() << "1 拼接失败了";
                    emit merageError(Failed);
                } else {
                    m_bottomHeight = -1;
                    qDebug() << "1 无效区域，点击调整捕捉区域";
                    emit invalidAreaError(InvalidArea, rect); //无效区域，点击调整捕捉区域
                }
            } else {
                if (0 <= m_curTimeDiff && m_curTimeDiff < 200) {
                    qDebug() << "=======1=滚动速度过快";
                    emit merageError(RoollingTooFast);
                } /*else {
                    qDebug() << "2 拼接失败了";
                    emit merageError(Failed);
                }*/
            }
            return false;
        } else if (result.rows < m_curImg.rows) {
            qCWarning(dsrApp) << "Result image height is smaller than current image";
            return false;
        }
        qDebug() << "拼接成功了";
        m_downCount = 0;
        m_curImg = result;
        return true;
    } else {
        if (m_MeragerCount == 1) {
            cv::Mat curImg = m_curImg;
            QRect rect = getScrollChangeRectArea(curImg, image);
            if (rect.width() < 0 || rect.height() < 0) {
                qDebug() << "3 拼接失败了";
                emit merageError(Failed);
            } else {
                m_bottomHeight = -1;
                qDebug() << "2 无效区域，点击调整捕捉区域";
                emit invalidAreaError(InvalidArea, rect); //无效区域，点击调整捕捉区域
            }
        } else {
            if (0 <= m_curTimeDiff && m_curTimeDiff < 200) {
                qDebug() << "=======2=滚动速度过快";
                emit merageError(RoollingTooFast);
            } else {
                if (isOneWay() == true) {
                    qDebug() << "1-------m_upCount: " << m_upCount << " m_downCount: " << m_downCount;
                    qDebug() << "4 拼接失败了";
                    emit merageError(Failed);
                }
            }
        }
        return false;
    }
}

//向下拼接图片
bool PixMergeThread::splicePictureDown(const cv::Mat &image)
{
    // 保存后的最后一张图片不做长度检查
    if (!m_isLastPixmap && m_curImg.rows > LONG_IMG_MAX_HEIGHT) {
        // 拼接超过了最大限度
        qCWarning(dsrApp) << "Image height exceeds maximum limit:" << LONG_IMG_MAX_HEIGHT;
        emit merageError(MaxHeight);
        return false;
    }
    if (image.rows <= TEMPLATE_HEIGHT) {
        qCWarning(dsrApp) << "Image height is too small for splicing, minimum required:" << TEMPLATE_HEIGHT;
        emit merageError(Failed);
        return false;
    }
    ++m_MeragerCount;
    qDebug() << "**************m_MeragerCount: " << m_MeragerCount;
    m_downCount++;
    /*转灰度图像*/
    cv::Mat image1_gray, image2_gray;
    cvtColor(m_curImg, image1_gray, CV_BGR2GRAY);
    cvtColor(image, image2_gray, CV_BGR2GRAY);
    //imwrite("m_curImg.png",m_curImg);
    //imwrite("image.png",image);
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
    if (maxVal >= thresholdv && maxLoc.y > 0) { //只有度量值大于阈值才认为是匹配
        result = cv::Mat::zeros(cvSize(m_curImg.cols, maxLoc.y + image.rows), m_curImg.type());
        temp1 = m_curImg(cv::Rect(0, 0, m_curImg.cols, maxLoc.y));
        if (temp1.empty()) {
            qCWarning(dsrApp) << "Failed to extract template region for downward splicing";
            return false;
        }

        /*将图1的非模板部分和图2拷贝到result*/
        temp1.copyTo(cv::Mat(result, cv::Rect(0, 0, m_curImg.cols, maxLoc.y)));
        image.copyTo(cv::Mat(result, cv::Rect(0, maxLoc.y, image.cols, image.rows)));
        if (result.rows == m_curImg.rows) {// 拼接前后图片高度不变
            cv::Mat curImg = m_curImg;
            QRect rect = getScrollChangeRectArea(curImg, image);
            if (m_isManualScrollModel == false) { //自动滚动时异常处理
                if (m_MeragerCount == 1) {
                    if (rect.width() < 0 || rect.height() < 0) {
                        qDebug() << "1 拼接失败了";
                        emit merageError(Failed);
                    } else {
                        m_headHeight = -1;
                        qDebug() << "1 无效区域，点击调整捕捉区域";
                        emit invalidAreaError(InvalidArea, rect); //无效区域，点击调整捕捉区域
                    }
                } else {
                    qDebug() << "======1==拼接到重复图片，拼接到低了=====";
                    emit merageError(ReachBottom);
                }
            } else {//手动滚动
                if (m_MeragerCount == 1) {
                    if (rect.width() < 0 || rect.height() < 0) {
                        qDebug() << "1 拼接失败了";
                        emit merageError(Failed);
                    } else {
                        m_headHeight = -1;
                        qDebug() << "1 无效区域，点击调整捕捉区域";
                        emit invalidAreaError(InvalidArea, rect); //无效区域，点击调整捕捉区域
                    }
                } else {
                    if (0 <= m_curTimeDiff && m_curTimeDiff < 200) {
                        qDebug() << "====1====滚动速度过快";
                        emit merageError(RoollingTooFast);
                    } /*else {
                        qDebug() << "2 拼接失败了";
                        emit merageError(Failed);
                    }*/
                }
            }
            return false;
        } else if (result.rows < m_curImg.rows) {
            qDebug() << "===result.rows < m_curImg.rows";
            return false;
        }
        qDebug() << "拼接成功了";
        m_upCount = 0;
        m_curImg = result;
        return true;
    } else {
        cv::Mat curImg = m_curImg;//拷贝不影响m_curImg
        QRect rect = getScrollChangeRectArea(curImg, image);
        if (m_isManualScrollModel == false) { //自动滚动异常处理
            if (m_MeragerCount == 1) {
                if (rect.width() < 0 || rect.height() < 0) {
                    qDebug() << "2 拼接到重复图片，拼接到低了";
                    emit merageError(ReachBottom);
                } else {
                    m_headHeight = -1;
                    qDebug() << "2 无效区域，点击调整捕捉区域";
                    emit invalidAreaError(InvalidArea, rect); //无效区域，点击调整捕捉区域
                }
            } else {
                qDebug() << "2 拼接失败了";
                emit merageError(Failed);
            }
        } else { //手动滚动
            if (m_MeragerCount == 1) {
                if (rect.width() < 0 || rect.height() < 0) {
                    qDebug() << "3 拼接失败了";
                    emit merageError(Failed);
                } else {
                    m_headHeight = -1;
                    qDebug() << "2 无效区域，点击调整捕捉区域";
                    emit invalidAreaError(InvalidArea, rect); //无效区域，点击调整捕捉区域
                }
            } else {
                if (0 <= m_curTimeDiff && m_curTimeDiff < 200) {
                    qDebug() << "=====2===滚动速度过快";
                    emit merageError(RoollingTooFast);
                } else {
                    if (isOneWay() == true) {
                        qDebug() << "1-------m_upCount: " << m_upCount << " m_downCount: " << m_downCount;
                        qDebug() << "4 拼接失败了";
                        emit merageError(Failed);
                    }
                }
            }
        }
        return false;
    }
}

//计算可以滚动的区域
QRect PixMergeThread::getScrollChangeRectArea(cv::Mat &img1, const cv::Mat &img2)
{
    QImage tempImg = QImage(img1.data, img1.cols, img1.rows, static_cast<int>(img1.step), QImage::Format_ARGB32);
    if (m_headHeight > 0) {
        tempImg = tempImg.copy(0, m_headHeight, tempImg.width(), tempImg.height() - m_headHeight);
    }
    if (m_bottomHeight > 0) {
        tempImg = tempImg.copy(0, 0, tempImg.width(), m_bottomHeight);
    }
    img1 = qPixmapToCvMat(QPixmap::fromImage(tempImg));

    int minI = img1.rows;
    int minJ = img1.cols;
    int maxI = 0;
    int maxJ = 0;
    // 计算变化部分
    for (int i = 0; i < img1.rows; ++i) {
        for (int j = 0; j < img1.cols; ++j) {
            //if(img1.at<Vec3b>(i, j)[0] != img2.at<Vec3b>(i, j)[0] || img1.at<Vec3b>(i, j)[1] != img2.at<Vec3b>(i, j)[1] || img1.at<Vec3b>(i, j)[2] != img2.at<Vec3b>(i, j)[2]) {
            if (img1.at<cv::Vec4b>(i, j) != img2.at<cv::Vec4b>(i, j)) {
                if (i < minI) minI = i;
                if (j < minJ) minJ = j;
                if (i > maxI) maxI = i;
                if (j > maxJ) maxJ = j;
            }
        }
    }
    qDebug() << "minJ: " << minJ << "minI:" << minI << "maxJ - minJ:" << maxJ - minJ << "maxI - minI" << maxI - minI;
    return  QRect(minJ, minI + m_headHeight, maxJ - minJ, maxI - minI);
}



