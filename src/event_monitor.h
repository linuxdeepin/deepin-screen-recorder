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

#ifndef EVENTMONITOR_H
#define EVENTMONITOR_H

#include "utils.h"

#include <QThread>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QtDBus/QtDBus>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusError>
#include <QDBusMessage>

#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/extensions/Xfixes.h>

/**
 * @brief The EventMonitor class
 * 录屏与截屏的事件监听类
 */
class EventMonitor : public QThread
{
    Q_OBJECT

public:
    explicit EventMonitor(QObject *parent = nullptr);
    ~EventMonitor();
    static void callback(XPointer trash, XRecordInterceptData *data);
    void handleEvent(XRecordInterceptData *);

    /**
     * @brief x11截屏下获取光标的图片
     * @return
     */
    XFixesCursorImage *getCursorImage();

    /**
     * @brief wayland截屏下获取光标的图片
     * @return
     */
    QImage getCursorImageWayland();
    /**
     * @brief 初始化wayland录屏事件监听
     */
    void initWaylandEventMonitor();

    /**
     * @brief 释放X11相关资源
     */
    void releaseRes();

public slots:
    /**
     * @brief wayland通过dbus接口接收鼠标按钮按下事件
     * @param type
     * @param x
     * @param y
     * @param str
     */
    void ButtonPressEvent(int type, int x, int y, QString str);
    /**
     * @brief wayland通过dbus接口接收鼠标按钮释放事件
     * @param type
     * @param x
     * @param y
     * @param str
     */
    void ButtonReleaseEvent(int type, int x, int y, QString str);
    /**
     * @brief wayland通过dbus接口接收鼠标移动事件
     * @param x
     * @param y
     * @param str
     */
    void CursorMoveEvent(int x, int y, QString str);

signals:
    /**
     * @brief 通过x11从底层获取鼠标拖动事件
     * @param 鼠标按下的x坐标
     * @param 鼠标按下的y坐标
     */
    void mouseDrag(int x, int y);

    /**
     * @brief 通过x11从底层获取鼠标按压事件
     * @param 鼠标按下的x坐标
     * @param 鼠标按下的y坐标
     */
    void mousePress(int x, int y);

    /**
     * @brief 通过x11从底层获取鼠标释放事件
     * @param x
     * @param y
     */
    void mouseRelease(int x, int y);

    /**
     * @brief 通过x11从底层获取鼠标移动事件
     * @param 鼠标移动的x坐标
     * @param 鼠标移动的y坐标
     */
    void mouseMove(int x, int y);

    /**
     * @brief 滚动鼠标滚轮,此处需区分是由模拟滚动触发的还是通过真实的鼠标事件触发的效果
     * @param direction 鼠标滚动的方向： 1：向上滚动； 0：向下滚动
     * @param 鼠标移动的x坐标
     * @param 鼠标移动的y坐标
    */
    void mouseScroll(int mouseTime, int direction, int x, int y);

    /**
     * @brief 通过x11从底层获取键盘按下事件
     * @param keyCode: 按下的键盘按键代号
     */
    void keyboardPress(unsigned char keyCode);

    /**
     * @brief 通过x11从底层获取键盘释放事件
     * @param keyCode: 释放的键盘按键代号
     */
    void keyboardRelease(unsigned char keyCode);

    /**
     * @brief 进行操作后发射活动窗口
     */
    void activateWindow();

protected:
    void run();

private:
    bool isPress;
    Display *m_display = nullptr;
    Display *m_display_datalink = nullptr;
    XRecordContext m_context;
};

#endif
