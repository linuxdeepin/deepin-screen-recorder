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
#include <QDebug>
#include <DPalette>
#include <QImage>
#include <QSizePolicy>

DGUI_USE_NAMESPACE

TopTips::TopTips(DWidget *parent)
    : QLabel(parent)
{
    setFixedSize(90, 20);
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

void TopTips::setContent(QString widthXHeight)
{
    setText(widthXHeight);
}

void TopTips::updateTips(QPoint pos, QString text)
{
    if (!this->isVisible())
        this->show();

    QPoint startPoint = pos;
    setContent(text);
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
