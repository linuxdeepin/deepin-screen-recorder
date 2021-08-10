/*

* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.

*

* Author:     wangcong <wangcong@uniontech.com>

*

* Maintainer: wangcong <wangcong@uniontech.com>

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
#include "scrollshottip.h"
#include "utils.h"
#include "constant.h"
#include "utils/configsettings.h"
#include "utils/tempfile.h"

#include <DWidget>
#include <DHiDPIHelper>

#include <QTimer>
#include <QApplication>
#include <QPen>
#include <QPainter>
#include <QDebug>
#include <QPainterPath>

const int ScrollShotTip::NUMBER_PADDING_Y = 30;

DWIDGET_USE_NAMESPACE

ScrollShotTip::ScrollShotTip(DWidget *parent) : DWidget(parent)
{
    installEventFilter(this);
    m_themeType = ConfigSettings::instance()->value("common", "themeType").toInt();
    if(m_themeType == 1) {
        m_warmingImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("warming.svg"));
    } else {
        m_warmingImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("warming.svg"));
    }
    m_scrollShotHelp = new DCommandLinkButton(tr("Get help."), this);
    connect(m_scrollShotHelp, &DCommandLinkButton::clicked, this, &ScrollShotTip::openScrollShotHelp);
}

ScrollShotTip::~ScrollShotTip()
{
    if(m_scrollShotHelp){
        delete  m_scrollShotHelp;
        m_scrollShotHelp = nullptr;
    }
}

//根据提示的类型选取相应的提示方法
void ScrollShotTip::showTip(TipType tipType)
{
    m_tipType = tipType;
    switch (m_tipType) {
    case TipType::StartScrollShotTip:
        showStartScrollShotTip();
        break;
    case TipType::ErrorScrollShotTip:
        showErrorScrollShotTip();
        break;
    case TipType::EndScrollShotTip:
        showEndScrollShotTip();
        break;
    case TipType::QuickScrollShotTip:
        break;
    case TipType::MaxLengthScrollShotTip:
        showMaxScrollShotTip();
        break;
    }

}

void ScrollShotTip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    int radius = 0;
    //不同的提示类型其背景的圆角矩形半径不同
    switch (m_tipType) {
    case TipType::StartScrollShotTip:
        radius = 20;
        break;
    case TipType::QuickScrollShotTip:
        radius = 20;
        break;
    case TipType::ErrorScrollShotTip:
        radius = 20;
        break;
    case TipType::EndScrollShotTip:
        radius = 20;
        break;
    case TipType::MaxLengthScrollShotTip:
        radius = 20;
        break;
    }
    //获取模糊背景的像素图
    QPixmap blurPixmap = getTooltipBackground();
    if(m_themeType == 1) {
        //判断是否获取到当前的模糊背景未获取到直接画一个矩形
        if (!blurPixmap.isNull()) {
            paintRect(painter,blurPixmap,radius);
            drawTooltipBackground(painter, rect(), "#EBEBEB", 0.3,radius);
        }
    } else {
        //判断是否获取到当前的模糊背景未获取到直接画一个矩形
        if (!blurPixmap.isNull()) {
            paintRect(painter,blurPixmap,radius);
            drawTooltipBackground(painter, rect(), "#191919", 0.8,radius);
        }
    }
    painter.setOpacity(1);
    //qDebug() << "m_tipText: " << m_tipText;
    //qDebug() << "rect().x(): " << rect().x() << "rect().y(): " << rect().y() <<"rect().width(): " << rect().width() <<"rect().height(): " << rect().height() ;
    qreal tipX = 0,tipY = 0,tipWidth = 0,tipHeight = 0;
    tipX = rect().x();
    tipY = rect().y();
    tipWidth = rect().width();
    tipHeight = rect().height();
    int warmingX = static_cast<int>(rect().x() + 5);
    int warmingY = static_cast<int>(rect().y() + 10 );
    if(m_tipType == TipType::EndScrollShotTip){
        painter.drawPixmap(QPoint(warmingX, warmingY), m_warmingImg);
    }else if(m_tipType == TipType::ErrorScrollShotTip){
        painter.drawPixmap(QPoint(warmingX, warmingY), m_warmingImg);
        tipWidth = rect().width() - Constant::RECTANGLE_PADDING  ;
    }
    //qDebug() << "tipX: " << tipX << "tipY: " << tipY << "tipWidth: " << tipWidth <<"tipHeight: " << tipHeight ;
    QRectF tooltipRect(tipX,tipY ,tipWidth,tipHeight);
    Utils::drawTooltipText(painter, m_tipText, "#000000", Constant::RECTANGLE_FONT_SIZE, tooltipRect);

}

//获取提示框的模糊背景图
QPixmap ScrollShotTip::getTooltipBackground()
{
    TempFile *tempFile = TempFile::instance();
    const int radius = 10;
    int imgWidth = 0;
    int imgHeight = 0;
    //qDebug() << "this->x(): " << this->x() << "this->y(): " << this->y() <<"this->width(): " << this->width() <<"this->height(): " << this->height() ;

    //用当前提示窗口的位置和提示框的大小构建一个矩形框
    QRect target( static_cast<int>(this->x()),
                      static_cast<int>(this->y()),
                      static_cast<int>(this->width() ),
                      static_cast<int>(this->height()));
    QPixmap tmpImg = tempFile->getFullscreenPixmap().copy(target);
    //qDebug() << "tmpImg.width1(),tmpImg.height1()" << tmpImg.width() << "," << tmpImg.height();

    if (!tmpImg.isNull()) {
        imgWidth = tmpImg.width();
        imgHeight = tmpImg.height();
        tmpImg = tmpImg.scaled(imgWidth / radius, imgHeight / radius, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        tmpImg = tmpImg.scaled(imgWidth, imgHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        //qDebug() << "tmpImg.width(),tmpImg.height()" << tmpImg.width() << "," << tmpImg.height();
    }
    return  tmpImg;
}

//画矩形
void ScrollShotTip::paintRect(QPainter &painter,QPixmap &blurPixmap,int radius)
{
    QPainterPath rectPath;
    painter.setPen(Qt::transparent);
    rectPath.addRoundedRect(QRectF(rect()), radius, radius);
    painter.drawPath(rectPath);
    painter.setClipPath(rectPath);
    painter.drawPixmap(0, 0,  width(), height(), blurPixmap);
    painter.drawPath(rectPath);
    painter.setClipping(false);
}

//画提示背景
void ScrollShotTip::drawTooltipBackground(QPainter &painter, QRect rect, QString textColor, qreal opacity,int radius)
{
    painter.setOpacity(opacity);
    QPainterPath path;
    path.addRoundedRect(QRectF(rect), radius, radius);
    painter.fillPath(path, QColor(textColor));

    QPen pen(QColor("#000000"));
    painter.setOpacity(0.04);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}

//开始滚动截图前的提示
void ScrollShotTip::showStartScrollShotTip()
{
    //m_tipText = "滚动鼠标滚轮或单击滚动区域";
    m_tipText = tr("Click to take a scrolling screenshot");
    QSize size = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, m_tipText);
    int width = size.width() + Constant::RECTANGLE_PADDING * 2;
    int height = size.height()+17;
    //qDebug() << "width: " << width << ",height: " << height;
    setFixedSize(width, height);
    repaint();

}

//滚动截图出现错误的提示
void ScrollShotTip::showErrorScrollShotTip()
{
    //m_tipText = tr("无法继续截图，查看帮助");
    m_tipText = tr("Failed to take a continuous screenshot.");
    QSize size = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, m_tipText);
    int width = size.width() + Constant::RECTANGLE_PADDING * 4 + m_warmingImg.width() ;
    int height = size.height() + 17;
    setFixedSize(width, height);
    repaint();

    QHBoxLayout *pHBoxLayout = new QHBoxLayout();
    size = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, "Get help.");

    m_scrollShotHelp->setMinimumSize(size.width(), size.height());
    m_scrollShotHelp->setStyleSheet("text-decoration: underline;");

    m_scrollShotHelp->show();

    pHBoxLayout->addSpacing(width);
    pHBoxLayout->addWidget(m_scrollShotHelp);
    this->setLayout(pHBoxLayout);

}

//滚动截图到底部出现的提示
void ScrollShotTip::showEndScrollShotTip()
{
    //m_tipText = tr("已到滚动区域底部");
    m_tipText = tr("Reached the bottom of the scroll area");
    QSize size = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, m_tipText);
    int width = size.width() + Constant::RECTANGLE_PADDING * 2 + m_warmingImg.height() ;
    int height = size.height() + 17;
    setFixedSize(width, height);
    repaint();
}

//图像拼接长度限制
void ScrollShotTip::showMaxScrollShotTip()
{
    //m_tipText = tr("已到最大长度");
    m_tipText = tr("Reached the maximum length");
    QSize size = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, m_tipText);
    int width = size.width() + Constant::RECTANGLE_PADDING * 2 + m_warmingImg.height() ;
    int height = size.height() + 17;
    setFixedSize(width, height);
    repaint();

}


