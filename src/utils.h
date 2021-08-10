/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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
#include <DImageButton>

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
    static void drawTooltipBackground(QPainter &painter, QRect rect, QString textColor, qreal opacity = 0.4);
    static void drawTooltipText(QPainter &painter, QString text, QString textColor, int textSize, QRectF rect);
    static void passInputEvent(int wid);
    static void setFontSize(QPainter &painter, int textSize);
//    static void warnNoComposite();
//    static void notSupportWarn();
//    static QString getRecordingSaveDirectory();
    static void setAccessibility(DPushButton *button,const QString name);
    static void setAccessibility(DImageButton *button,const QString name);
    static void setAccessibility(QAction *action, const QString name);
    static bool is3rdInterfaceStart;
    static bool isTabletEnvironment;

    /**
     * @brief 对目标区域做穿透处理
     * @param 窗口id
     * @param 区域位置x坐标
     * @param 区域位置y坐标
     * @param 区域宽
     * @param 区域高
     */
    static void getInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height);
    /**
     * @brief 取消对目标区域的穿透处理
     * @param wid  窗口id
     * @param x  区域位置x坐标
     * @param y  区域位置y坐标
     * @param width  区域宽
     * @param height  区域高
     */
    static void cancelInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height);

    /**
     * @brief 取消对目标区域的穿透处理
     * @param wid  窗口id
     * @param x  区域位置x坐标
     * @param y  区域位置y坐标
     * @param width  区域宽
     * @param height  区域高
     */
    static void cancelInputEvent1(const int wid, const short x, const short y, const unsigned short width, const unsigned short height);

};

#endif //UTILS_H
