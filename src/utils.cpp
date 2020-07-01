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

#include <QString>
#include <QDir>
#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QDBusInterface>
#include <QtX11Extras/QX11Info>
#include <X11/extensions/shape.h>
#include "utils.h"
//#include <dwindowmanager.h>
#include "constant.h"
#include <QStandardPaths>

//DWM_USE_NAMESPACE

static const QString WarningDialogService = "com.deepin.dde.WarningDialog";
static const QString WarningDialogPath = "/com/deepin/dde/WarningDialog";
static const QString WarningDialogInterface = "com.deepin.dde.WarningDialog";

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

void Utils::warnNoComposite()
{
    QDBusInterface iface(WarningDialogService,
                         WarningDialogPath,
                         WarningDialogService);
    iface.call("RaiseWindow");
}

//void Utils::blurRect(DWindowManager *windowManager, int widgetId, QRectF rect)
//{
//    QVector<uint32_t> data;

//    qreal devicePixelRatio = qApp->devicePixelRatio();
//    data << rect.x() * devicePixelRatio << rect.y() * devicePixelRatio << rect.width() * devicePixelRatio << rect.height() * devicePixelRatio << Constant::RECTANGLE_RADIUS << Constant::RECTANGLE_RADIUS;
//    windowManager->setWindowBlur(widgetId, data);
//}

//void Utils::blurRects(DWindowManager *windowManager, int widgetId, QList<QRectF> rects)
//{
//    QVector<uint32_t> data;
//    qreal devicePixelRatio = qApp->devicePixelRatio();
//    foreach (auto rect, rects) {
//        data << rect.x() * devicePixelRatio << rect.y() * devicePixelRatio << rect.width() * devicePixelRatio << rect.height() * devicePixelRatio << Constant::RECTANGLE_RADIUS << Constant::RECTANGLE_RADIUS;
//    }
//    windowManager->setWindowBlur(widgetId, data);
//}

//void Utils::clearBlur(DWindowManager *windowManager, int widgetId)
//{
//    QVector<uint32_t> data;
//    data << 0 << 0 << 0 << 0 << 0 << 0;
//    windowManager->setWindowBlur(widgetId, data);
//}

void Utils::drawTooltipBackground(QPainter &painter, QRect rect, qreal opacity)
{
    painter.setOpacity(opacity);
    QPainterPath path;
    path.addRoundedRect(QRectF(rect), Constant::RECTANGLE_RADIUS, Constant::RECTANGLE_RADIUS);
    painter.fillPath(path, QColor("#F5F5F5"));

    QPen pen(QColor("#000000"));
    painter.setOpacity(0.04);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}

void Utils::drawTooltipText(QPainter &painter, QString text, QString textColor, int textSize, QRectF rect)
{
    Utils::setFontSize(painter, textSize);
    painter.setOpacity(1);
    painter.setPen(QPen(QColor(textColor)));
    painter.drawText(rect, Qt::AlignCenter, text);
}

void Utils::passInputEvent(int wid)
{
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = 0;
    reponseArea->y = 0;
    reponseArea->width = 0;
    reponseArea->height = 0;

    XShapeCombineRectangles(QX11Info::display(), wid, ShapeInput, 0, 0, reponseArea, 1, ShapeSet, YXBanded);

    delete reponseArea;
}

QString Utils::getRecordingSaveDirectory()
{
    QDir musicDirectory = QDir(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first());
    QString subDirectory = tr("Recordings");
    musicDirectory.mkdir(subDirectory);
    return musicDirectory.filePath(subDirectory);
}

