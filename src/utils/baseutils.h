// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
