// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewwidget.h"
#include "../utils/log.h"

#include <QPainter>
#include <QApplication>
#include <QScreen>

PreviewWidget::PreviewWidget(const QRect &rect, QWidget *parent) : QWidget(parent), m_previewRect(rect)
{
    qCDebug(dsrApp) << "PreviewWidget constructor entered with rect:" << rect;
    m_recordHeight = rect.height();
    m_recordWidth = rect.width();
    m_recordX = rect.x();
    m_recordY = rect.y();
    QRect mainRect = QGuiApplication::primaryScreen()->geometry();;
    m_maxHeight = mainRect.height() * 7 / 10 ; //最大预览高度
    m_maxWidth = mainRect.width() / 4 ; //最大预览宽度
    qCDebug(dsrApp) << "Max preview height:" << m_maxHeight << ", max preview width:" << m_maxWidth;
}

//初始化位置，大小
void PreviewWidget::initPreviewWidget()
{
    qCDebug(dsrApp) << "initPreviewWidget called";
    m_previewRect = previewGeomtroy();
    setGeometry(m_previewRect);
    qCDebug(dsrApp) << "Preview widget geometry set to:" << m_previewRect;
}

//更新捕捉区域的大小及位置
void PreviewWidget::updatePreviewSize(const QRect &rect)
{
    qCDebug(dsrApp) << "updatePreviewSize called with rect:" << rect;
    m_recordHeight = rect.height();
    m_recordWidth = rect.width();
    m_recordX = rect.x();
    m_recordY = rect.y();
    initPreviewWidget();
    qCDebug(dsrApp) << "Preview size updated and re-initialized.";
}

PreviewWidget::PostionStatus PreviewWidget::getPreviewPostion()
{
    qCDebug(dsrApp) << "getPreviewPostion called, returning:" << m_StatusPos;
    return m_StatusPos;
}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    qCDebug(dsrApp) << "PreviewWidget paintEvent called";
    //paint
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawImage(rect(), m_currentPix);
}
//设置位置状态
void PreviewWidget::setPreviewWidgetStatusPos(PostionStatus statusPos)
{
    qCDebug(dsrApp) << "setPreviewWidgetStatusPos called with status:" << statusPos;
    m_StatusPos = statusPos;
}
//更新图片
void PreviewWidget::updateImage(const QImage &image)
{
    qCDebug(dsrApp) << "updateImage called with image size:" << image.size();
    int previewHeight = 0; //预览高度
    int previewWidth = 0; //预览宽度
    int imageHight = int(image.height() / m_screenRatio);
    int imageWidth = int(image.width() / m_screenRatio);
    bool unchanged = false; //是否高宽不变
    qCDebug(dsrApp) << "Scaled image dimensions: height=" << imageHight << ", width=" << imageWidth;
    //计算图片缩放后的预览宽高
    if (imageHight <= m_maxHeight && imageWidth <= m_maxWidth) {
        qCDebug(dsrApp) << "Image fits within max dimensions";
        previewHeight = imageHight;
        previewWidth = imageWidth;
        unchanged = true;
    } else if (imageHight <= m_maxHeight && imageWidth > m_maxWidth) {
        qCDebug(dsrApp) << "Image width exceeds max width, scaling based on width";
        previewHeight = m_maxWidth * imageHight / imageWidth;
        previewWidth = m_maxWidth;
    } else if (imageHight > m_maxHeight && imageWidth <= m_maxWidth) {
        qCDebug(dsrApp) << "Image height exceeds max height, scaling based on height";
        previewHeight = m_maxHeight;
        previewWidth = m_maxHeight * imageWidth / imageHight;
    } else if (imageHight > m_maxHeight && imageWidth > m_maxWidth) {
        qCDebug(dsrApp) << "Image exceeds both max height and width, calculating best fit";
        previewHeight = m_maxWidth * imageHight / imageWidth; //假设宽度为最大
        previewWidth = m_maxHeight * imageWidth / imageHight;//假设高度为最大
        if (previewHeight <= m_maxHeight) {
            qCDebug(dsrApp) << "Adjusting preview width to max width";
            previewWidth = m_maxWidth;
        } else if (previewWidth <= m_maxWidth) {
            qCDebug(dsrApp) << "Adjusting preview height to max height";
            previewHeight = m_maxHeight;
        }
    }
    qCDebug(dsrApp) << "Calculated preview dimensions: height=" << previewHeight << ", width=" << previewWidth;
    int hightDiff_t = previewHeight - m_previewRect.height(); //高度差
    //判断向上是否超出屏幕外
    int previewY = m_previewRect.y() - hightDiff_t;
    if (previewY <= 0) {
        qCDebug(dsrApp) << "Preview Y position is <= 0, adjusting";
        previewY = 0;
        if (m_StatusPos == INSIDE)
            previewHeight = m_recordY + m_recordHeight - 20; //设置此时的预览高度=捕捉区域的高度+y轴坐标
        else
            previewHeight = m_recordY + m_recordHeight;//设置此时的预览高度=捕捉区域的高度+y轴坐标
    }
    m_previewRect.setY(previewY); //重新设置y坐标
    m_previewRect.setHeight(previewHeight);//重新设置预览高度
    int widthDiff_t = m_previewRect.width() - previewWidth; //宽度差
    if (m_StatusPos == LEFT) {//预览框在左
        qCDebug(dsrApp) << "Preview status is LEFT, adjusting X position";
        m_previewRect.setX(m_previewRect.x() + widthDiff_t); //重新设置x坐标
    } else if (m_StatusPos == RIGHT) {//预览框在右
        qCDebug(dsrApp) << "Preview status is RIGHT, keeping X position";
        m_previewRect.setX(m_previewRect.x());
    } else {//预览框在内部
        qCDebug(dsrApp) << "Preview status is INSIDE, adjusting X position";
        m_previewRect.setX(m_previewRect.x() + widthDiff_t);
    }
    m_previewRect.setWidth(previewWidth);//重新设置预览宽度
    setGeometry(m_previewRect);
    if (true == unchanged) {//高宽不变,不进行宽高等比缩放
        qCDebug(dsrApp) << "Image unchanged, setting current pixmap directly";
        m_currentPix = image;
    } else {
        qCDebug(dsrApp) << "Image changed, scaling to preview dimensions";
        QImage tempImage = image.scaled(previewWidth, previewHeight, Qt::
                                        KeepAspectRatioByExpanding, Qt::SmoothTransformation);//以预览框的宽高等比例缩放
        m_currentPix = image;
    }
    update();
}

//根据捕捉区域大小初始化预览框位置大小
QRect PreviewWidget::previewGeomtroy()
{
    qCDebug(dsrApp) << "previewGeomtroy called";
    QRect rt;
    int previewHeight = 0; //预览高度
    int previewWidth = 0; //预览宽度
    if (m_recordHeight <= m_maxHeight && m_recordWidth <= m_maxWidth) {
        qCDebug(dsrApp) << "Record area fits within max dimensions";
        previewHeight = m_recordHeight;
        previewWidth = m_recordWidth;
        rt = calculatePreviewPosition(previewWidth, previewHeight);
    } else if (m_recordHeight <= m_maxHeight && m_recordWidth > m_maxWidth) {
        qCDebug(dsrApp) << "Record width exceeds max width, scaling based on width";
        previewHeight = m_maxWidth * m_recordHeight / m_recordWidth;;
        previewWidth = m_maxWidth;
        rt = calculatePreviewPosition(previewWidth, previewHeight);
    } else if (m_recordHeight > m_maxHeight && m_recordWidth <= m_maxWidth) {
        qCDebug(dsrApp) << "Record height exceeds max height, scaling based on height";
        previewHeight = m_maxHeight;
        previewWidth = m_maxHeight * m_recordWidth / m_recordHeight;
        rt = calculatePreviewPosition(previewWidth, previewWidth);
    } else if (m_recordHeight > m_maxHeight && m_recordWidth > m_maxWidth) {
        qCDebug(dsrApp) << "Record area exceeds both max height and width, calculating best fit";
        previewHeight = m_maxWidth * m_recordHeight / m_recordWidth; //假设宽度为最大
        previewWidth = m_maxHeight * m_recordWidth / m_recordHeight;//假设高度为最大
        if (previewHeight <= m_maxHeight) {
            qCDebug(dsrApp) << "Adjusting preview width to max width";
            previewWidth = m_maxWidth;
            rt = calculatePreviewPosition(previewWidth, previewHeight);
        } else if (previewWidth <= m_maxWidth) {
            qCDebug(dsrApp) << "Adjusting preview height to max height";
            previewHeight = m_maxHeight;
            rt = calculatePreviewPosition(previewWidth, previewHeight);
        }
    }
    qCDebug(dsrApp) << "Calculated preview geometry:" << rt;
    return rt;
}
//判定在左还是在右
QRect PreviewWidget::calculatePreviewPosition(int previewWidth, int previewHeight)
{
    qCDebug(dsrApp) << "calculatePreviewPosition called with previewWidth:" << previewWidth << ", previewHeight:" << previewHeight;
    QRect rt;
    int previewY = m_recordY + m_recordHeight - previewHeight;
    int rightX = m_recordX + m_recordWidth + 20; //根据捕捉区域计算预览向右的起始x
    int leftX = m_recordX - previewWidth - 20; //根据捕捉区域计算预览向左的起始x
    int rightMargin = m_screenWidth - rightX; //右边距
    int leftMargin = m_recordX; //左边距
    qCDebug(dsrApp) << "Calculated positions and margins: rightX=" << rightX << ", leftX=" << leftX << ", rightMargin=" << rightMargin << ", leftMargin=" << leftMargin;
    //判断位置是左还是右
    if (rightMargin >= leftMargin && rightMargin >= previewWidth + 1) {
        qCDebug(dsrApp) << "Positioning preview to the RIGHT";
        rt.setX(rightX);
        rt.setY(previewY);
        rt.setWidth(previewWidth);
        rt.setHeight(previewHeight);
        setPreviewWidgetStatusPos(RIGHT);//设置位置状态
    } else if (rightMargin < leftMargin && leftMargin >= previewWidth + 1) {
        qCDebug(dsrApp) << "Right margin is less than left margin";
        if (rightMargin >= previewWidth + 1) { //默认优先显示在右边
            qCDebug(dsrApp) << "Right margin allows, positioning to the RIGHT (default)";
            rt.setX(rightX);
            rt.setY(previewY);
            rt.setWidth(previewWidth);
            rt.setHeight(previewHeight);
            setPreviewWidgetStatusPos(RIGHT);
        } else {
            qCDebug(dsrApp) << "Positioning preview to the LEFT";
            rt.setX(leftX);
            rt.setY(previewY);
            rt.setWidth(previewWidth);
            rt.setHeight(previewHeight);
            setPreviewWidgetStatusPos(LEFT);
        }
    } else {
        qCDebug(dsrApp) << "Positioning preview INSIDE";
        int nX = m_recordX + m_recordWidth - previewWidth - 20; //根据捕捉区域计算内部预览的起始x
        int ny = m_recordY + m_recordHeight - previewHeight - 20;//根据捕捉区域计算内部预览的起始y
        rt.setX(nX);
        rt.setY(ny);
        rt.setWidth(previewWidth);
        rt.setHeight(previewHeight);
        setPreviewWidgetStatusPos(INSIDE);
    }
    qCDebug(dsrApp) << "Returning calculated preview rectangle:" << rt;
    return rt;
}
//设置屏幕宽度
void PreviewWidget::setScreenInfo(int screenWidth, double screenRatio)
{
    qCDebug(dsrApp) << "setScreenInfo called with screenWidth:" << screenWidth << ", screenRatio:" << screenRatio;
    m_screenWidth = screenWidth;
    m_screenRatio = screenRatio;
}
