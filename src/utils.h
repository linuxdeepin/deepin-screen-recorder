/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
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

#ifndef UTILS_H
#define UTILS_H
//#include <dwindowmanager.h>
#include <DPushButton>

#include <QObject>
#include <QPainter>
#include <QAction>
#include <QString>

DWIDGET_USE_NAMESPACE

class Utils : public QObject
{
    Q_OBJECT
public:
    static QSize getRenderSize(int fontSize, QString string);
    static QString getQrcPath(QString imageName);
//    static void blurRect(DWindowManager *windowManager, int widgetId, QRectF rect);
//    static void blurRects(DWindowManager *windowManager, int widgetId, QList<QRectF> rects);
//    static void clearBlur(DWindowManager *windowManager, int widgetId);
    static void drawTooltipBackground(QPainter &painter, QRect rect, qreal opacity = 0.4);
    static void drawTooltipText(QPainter &painter, QString text, QString textColor, int textSize, QRectF rect);
    static void passInputEvent(int wid);
    static void setFontSize(QPainter &painter, int textSize);
    static void warnNoComposite();
//    static void notSupportWarn();
    static QString getRecordingSaveDirectory();
    static void setAccessibility(DPushButton *button,const QString name);
    static void setAccessibility(QAction *action, const QString name);
    static bool is3rdInterfaceStart;
};

#endif //UTILS_H
