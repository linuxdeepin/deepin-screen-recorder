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

#ifndef BASEUTILS_H
#define BASEUTILS_H

#include <QCursor>
#include <QFont>
#include <QLayout>
#include <QFontMetrics>
#include <QPainter>

enum ResizeDirection {
    Rotate,
    Moving,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Top,
    Bottom,
    Left,
    Right,
    Outting,
};

class BaseUtils : public QObject
{
    Q_OBJECT
public:
    static QCursor setCursorShape(QString cursorName, int colorIndex = 0);
    static int stringWidth(const QFont &f, const QString &str);
    static QColor       colorIndexOf(int index);
    static int                colorIndex(QColor color);
    /**
     * @brief 判断后缀是否是"bmp" "jpg" "jpeg" "png" "pbm" "pgm" "xbm" "xpm"
     * @param suffix:后缀
     * @return true:是 false:否
     */
    static bool          isValidFormat(QString suffix);
    static bool          isCommandExist(QString command);
    static QMap<QString, QCursor> m_shapesCursor;
};
#endif // BASEUTILS_H
