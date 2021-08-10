/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
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

#ifndef SCREENSHOTEVENT_H
#define SCREENSHOTEVENT_H

#include <QThread>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/extensions/Xfixes.h>

class ScreenShotEvent : public QThread
{
    Q_OBJECT

public:
    explicit ScreenShotEvent(QObject *parent = nullptr);

    static void callback(XPointer trash, XRecordInterceptData *data);
    void handleRecordEvent(XRecordInterceptData *);
    XFixesCursorImage* getCursorImage();

signals:
    void activateWindow();
    void shotKeyPressEvent(const unsigned char &keyCode);
    /**
     * @brief 通过x11从底层获取鼠标点击事件
     * @param 鼠标按下的x坐标
     * @param 鼠标按下的y坐标
     */
    void buttonedPress(int x, int y);

    /**
     * @brief 通过x11从底层获取鼠标移动事件
     * @param 鼠标移动的x坐标
     * @param 鼠标移动的y坐标
     */
    void moveMouse(int x , int y);

protected:
    void run();
};

#endif // SCREENSHOTEVENT_H
