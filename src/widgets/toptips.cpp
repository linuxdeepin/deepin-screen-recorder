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

DGUI_USE_NAMESPACE

TopTips::TopTips(DWidget *parent)
    : DLabel(parent)
{
    setFixedSize(90, 20);
//    this->setStyleSheet(" TopTips { background-color: transparent;"
//                        "border-image: url(:/resources/images/action/sizetip.png)  no-repeat;"
//                        "color: white;"
//                        "font-size: 12px;}");
//    this->setPixmap(QPixmap::fromImage(QImage(":/resources/images/action/sizetip.png")));
    DPalette pa = this->palette();
    pa.setColor(DPalette::Foreground, Qt::white);
    this->setPalette(pa);

}

void TopTips::setContent(QString widthXHeight)
{
    setText(widthXHeight);
    setAlignment(Qt::AlignCenter);
}

void TopTips::updateTips(QPoint pos, QString text)
{
    if (!this->isVisible())
        this->show();

    QPoint startPoint = pos;

    startPoint.setX(pos.x());

    if (pos.y() > this->height()) {
        startPoint.setY(pos.y() - this->height() - 3);
    } else {
        startPoint.setY(pos.y() + 3);
    }

    this->move(startPoint);
    setContent(text);
}

TopTips::~TopTips() {}
