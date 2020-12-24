/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Maintainer: Peng Hui<penghui@deepin.com>
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

#include "toptips.h"
#include "../utils/baseutils.h"

#include <DPalette>
#include <QImage>
#include <QSizePolicy>
#include <QDebug>

#include <cmath>

DGUI_USE_NAMESPACE

TopTips::TopTips(DWidget *parent)
    : QLabel(parent)
{
    setFixedSize(500, 30);
//    this->setStyleSheet(" TopTips { background-color: transparent;"
//                        "border-image: url(:/resources/images/action/sizetip.png)  no-repeat;"
//                        "color: white;"
//                        "font-size: 12px;}");
//    this->setPixmap(QPixmap::fromImage(QImage(":/resources/images/action/sizetip.png")));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    DPalette pa = this->palette();
    pa.setColor(DPalette::Foreground, Qt::white);
    this->setPalette(pa);
    this->setAlignment(Qt::AlignLeft | Qt::AlignBottom);


}

void TopTips::setContent(const QSize &size)
{
    QString text = QString("%1X%2").arg(size.width()).arg(size.height());
    if(m_showRecorderTips && size.width() * size.height() > 1920 * 1080 && size.width() != m_width && size.height() != m_height) {
        // 1920 / 1080 = w / h
        // w h 等比缩放
        int h = static_cast<int>(sqrt(1920.0 * 1080 * size.height() / size.width()));
        int w = static_cast<int>(sqrt(1920.0 * 1080 * size.width() / size.height()));
        QString recorderTips = tr(" Adjust the recording area within %1*%2 to get better video effect");
        setText(text + recorderTips.arg(w).arg(h));
    } else {
        setText(text);
    }
}

void TopTips::updateTips(QPoint pos, const QSize &size)
{
    if (!this->isVisible())
        this->show();

    QPoint startPoint = pos;
    setContent(size);
    startPoint.setX(pos.x());

    if (pos.y() > this->height()) {
        startPoint.setY(pos.y() - this->height() - 3);
    } else {
        startPoint.setY(pos.y() + 3);
    }

    this->move(startPoint);
}

void TopTips::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    return;
}


TopTips::~TopTips() {}

void TopTips::setRecorderTipsInfo(const bool showState)
{
    if(QSysInfo::currentCpuArchitecture().startsWith(QString("mips"))){
        m_showRecorderTips = showState;
    }
}
