/* -*- Mode: Vala; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2017 Deepin, Inc.
 *               2011 ~ 2017 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
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

#include <QString>
#include <QDir>
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include "utils.h"
#include "window_manager.h"

QString Utils::getImagePath(QString imageName)
{
    QDir dir(qApp->applicationDirPath());
    dir.cdUp();

    return QDir(DSR_ICON_PATH).filePath(imageName);
}

QString Utils::getQrcPath(QString imageName)
{
    return QString(":/image/%1").arg(imageName);
}

QSize Utils::getRenderSize(int fontSize, QString string)
{
    QFont font;
    font.setPointSize(fontSize);
    QFontMetrics fm(font);

    int width = 0;
    int height = 0;
    foreach (auto line, string.split("\n")) {
        int lineWidth = fm.width(line);
        int lineHeight = fm.height();

        if (lineWidth > width) {
            width = lineWidth;
        }
        height += lineHeight;
    }

    return QSize(width, height);
}

void Utils::setFontSize(QPainter &painter, int textSize)
{
    QFont font = painter.font() ;
    font.setPointSize(textSize);
    painter.setFont(font);
}

void Utils::blurWidget(WindowManager *windowManager, int widgetId, QRectF &rect)
{
    QVector<uint32_t> data;
    data << rect.x() << rect.y() << rect.width() << rect.height() << RECTANGLE_RAIUDS << RECTANGLE_RAIUDS;
    windowManager->setWindowBlur(widgetId, data);
}    

void Utils::clearBlur(WindowManager *windowManager, int widgetId)
{
    QVector<uint32_t> data;
    data << 0 << 0 << 0 << 0 << 0 << 0;
    windowManager->setWindowBlur(widgetId, data);
}

