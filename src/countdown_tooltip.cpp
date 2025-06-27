// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "countdown_tooltip.h"
#include "utils.h"
#include "constant.h"
#include "utils/configsettings.h"
#include "utils/tempfile.h"
#include "utils/log.h"

#include <DWidget>
#include <DHiDPIHelper>

#include <QTimer>
#include <QApplication>
#include <QPen>
#include <QPainter>
#include <QDebug>
#include <QPainterPath>

const int CountdownTooltip::NUMBER_PADDING_Y = 30;

DWIDGET_USE_NAMESPACE

CountdownTooltip::CountdownTooltip(DWidget *parent) : DWidget(parent),
    showCountdownTimer(nullptr)
{
    qCInfo(dsrApp) << Q_FUNC_INFO << "Entry.";
    installEventFilter(this);

    if(Utils::themeType == 1) {
        qCInfo(dsrApp) << Q_FUNC_INFO << "Theme type is 1. Loading light theme countdown images.";
        countdown1Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("recorder/countdown_1.svg"));
        countdown2Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("recorder/countdown_2.svg"));
        countdown3Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("recorder/countdown_3.svg"));
    } else {
        qCInfo(dsrApp) << Q_FUNC_INFO << "Theme type is not 1. Loading dark theme countdown images.";
        countdown1Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("recorder/countdown_1-01.svg"));
        countdown2Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("recorder/countdown_2-01.svg"));
        countdown3Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("recorder/countdown_3-01.svg"));
    }

    showCountdownCounter = 0;

    text = tr("Click the tray icon \nor press the shortcut again to stop recording");
    if (Utils::isTabletEnvironment) {
        qCInfo(dsrApp) << Q_FUNC_INFO << "Tablet environment detected. Changing tooltip text.";
        text = tr("Do not rotate your screen during recording");
    }
    QSize size = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, text);
    int width = size.width() + Constant::RECTANGLE_PADDING * 2;
    int height = size.height() + Constant::RECTANGLE_PADDING * 2 + countdown1Img.height() + NUMBER_PADDING_Y;
    setFixedSize(width, height);
    qCInfo(dsrApp) << Q_FUNC_INFO << "Exit. Initial size:" << size << ", Width:" << width << ", Height:" << height;
}

//2021.6.24注释
//void CountdownTooltip::paintEvent(QPaintEvent *)
//{
//    if (showCountdownCounter > 0) {
//        QPainter painter(this);
//        if(m_themeType == 1) {
//            Utils::drawTooltipBackground(painter, rect(), "#EBEBEB", 0.3);
//        } else {
//            Utils::drawTooltipBackground(painter, rect(), "#191919", 0.8);
//        }

//        qreal devicePixelRatio = qApp->devicePixelRatio();
//        painter.setOpacity(1);
//        int countdownX = static_cast<int>(rect().x() + (rect().width() - countdown1Img.width() / devicePixelRatio) / 2);
//        int countdownY = static_cast<int>(rect().y() + NUMBER_PADDING_Y * devicePixelRatio);
//        if (showCountdownCounter == 1) {
//            painter.drawPixmap(QPoint(countdownX, countdownY), countdown1Img);
//        } else if (showCountdownCounter == 2) {
//            painter.drawPixmap(QPoint(countdownX, countdownY), countdown2Img);
//        } else if (showCountdownCounter == 3) {
//            painter.drawPixmap(QPoint(countdownX, countdownY), countdown3Img);
//        }

//        QRectF tooltipRect(rect().x(),
//                           rect().y() + countdown1Img.height() / devicePixelRatio + NUMBER_PADDING_Y,
//                           rect().width(),
//                           rect().height() - countdown1Img.height() / devicePixelRatio - NUMBER_PADDING_Y);
//        Utils::drawTooltipText(painter, text, "#000000", Constant::RECTANGLE_FONT_SIZE, tooltipRect);
//    }
//}

void CountdownTooltip::paintEvent(QPaintEvent *)
{
    if (showCountdownCounter > 0) {
        QPainter painter(this);

        //获取模糊背景的像素图
        QPixmap blurPixmap = getTooltipBackground();

        if(Utils::themeType == 1) {
            //判断是否获取到当前的模糊背景未获取到直接画一个矩形
            if (!blurPixmap.isNull()) {
                paintRect(painter,blurPixmap);
                Utils::drawTooltipBackground(painter, rect(), "#EBEBEB", 0.3);
            }
        } else {
            //判断是否获取到当前的模糊背景未获取到直接画一个矩形
            if (!blurPixmap.isNull()) {
                paintRect(painter,blurPixmap);
                Utils::drawTooltipBackground(painter, rect(), "#191919", 0.8);
            }
        }

        qreal devicePixelRatio = qApp->devicePixelRatio();
        painter.setOpacity(1);
        int countdownX = static_cast<int>(rect().x() + (rect().width() - countdown1Img.width() / devicePixelRatio) / 2);
        int countdownY = static_cast<int>(rect().y() + NUMBER_PADDING_Y * devicePixelRatio);
        if (showCountdownCounter == 1) {
            painter.drawPixmap(QPoint(countdownX, countdownY), countdown1Img);
        } else if (showCountdownCounter == 2) {
            painter.drawPixmap(QPoint(countdownX, countdownY), countdown2Img);
        } else if (showCountdownCounter == 3) {
            painter.drawPixmap(QPoint(countdownX, countdownY), countdown3Img);
        }

        QRectF tooltipRect(rect().x(),
                           rect().y() + countdown1Img.height() / devicePixelRatio + NUMBER_PADDING_Y,
                           rect().width(),
                           rect().height() - countdown1Img.height() / devicePixelRatio - NUMBER_PADDING_Y);
        Utils::drawTooltipText(painter, text, "#000000", Constant::RECTANGLE_FONT_SIZE, tooltipRect);
    }
}

QPixmap CountdownTooltip::getTooltipBackground()
{
    qCInfo(dsrApp) << Q_FUNC_INFO << "Entry."; // Log function entry
    TempFile *tempFile = TempFile::instance();
    const int radius = 50;

    //用当前提示窗口的位置和提示框的大小构建一个矩形框
    auto ratio = qApp->devicePixelRatio();
    QRect target( static_cast<int>(this->x() * ratio),
                      static_cast<int>(this->y() * ratio),
                      static_cast<int>(this->width() * ratio),
                      static_cast<int>(this->height() * ratio));

    QPixmap tmpImg = tempFile->getFullscreenPixmap().copy(target);
    if (!tmpImg.isNull()) {
        qCInfo(dsrApp) << Q_FUNC_INFO << "Temporary image is not null. Scaling image."; // Log branch
        int imgWidth = tmpImg.width();
        int imgHeight = tmpImg.height();
        tmpImg = tmpImg.scaled(imgWidth / radius, imgHeight / radius, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        tmpImg = tmpImg.scaled(imgWidth, imgHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

#ifdef QT_DEBUG
    qCInfo(dsrApp) << "rect().x(),rect().y()" << rect().x() << "," << rect().y();
    qCInfo(dsrApp) << "rect().width(),rect().height()" << rect().width() << "," << rect().height();
    qCInfo(dsrApp) << "this->x(),this->y()" << this->x() << "," << this->y();
    qCInfo(dsrApp) << "this->width(),this->height()" << this->width() << "," << this->height();
    qCInfo(dsrApp) << "tmpImg.width(),tmpImg.height()" << tmpImg.width() << "," << tmpImg.height();
#endif
    qCInfo(dsrApp) << Q_FUNC_INFO << "Exit."; // Log function exit
    return  tmpImg;
}

void CountdownTooltip::paintRect(QPainter &painter,QPixmap &blurPixmap)
{
    QPainterPath rectPath;
    painter.setPen(Qt::transparent);
    rectPath.addRoundedRect(QRectF(rect()), 8, 8);
    painter.drawPath(rectPath);
    painter.setClipPath(rectPath);
    painter.drawPixmap(0, 0,  width(), height(), blurPixmap);
    painter.drawPath(rectPath);
    painter.setClipping(false);
}

void CountdownTooltip::start()
{
    qCInfo(dsrApp) << Q_FUNC_INFO << "Entry."; // Log function entry
    showCountdownCounter = 3;
    if(nullptr == showCountdownTimer){
        qCInfo(dsrApp) << Q_FUNC_INFO << "showCountdownTimer is nullptr, creating new QTimer."; // Log branch
        showCountdownTimer = new QTimer(this);
    }
    if(nullptr != showCountdownTimer){
        qCInfo(dsrApp) << Q_FUNC_INFO << "Connecting timeout signal and starting timer."; // Log branch
        connect(showCountdownTimer, SIGNAL(timeout()), this, SLOT(update()));
        showCountdownTimer->start(1000);
    }
    qCInfo(dsrApp) << Q_FUNC_INFO << "Exit."; // Log function exit
}

void CountdownTooltip::startAtOnce()
{
    qCInfo(dsrApp) << Q_FUNC_INFO << "Entry."; // Log function entry
    emit finished();
    qCInfo(dsrApp) << Q_FUNC_INFO << "Exit."; // Log function exit
}

void CountdownTooltip::update()
{
    qCInfo(dsrApp) << Q_FUNC_INFO << "Entry."; // Log function entry
    showCountdownCounter--;

    if (showCountdownCounter <= 0 && nullptr != showCountdownTimer) {
        qCInfo(dsrApp) << Q_FUNC_INFO << "showCountdownCounter <= 0, stopping timer."; // Log branch
        showCountdownTimer->stop();

        emit finished();
    }

    repaint();
}
