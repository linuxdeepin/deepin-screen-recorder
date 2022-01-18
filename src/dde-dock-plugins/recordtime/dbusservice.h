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

#ifndef DBUSSERVICE_H
#define DBUSSERVICE_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class DBusService : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.ScreenRecorder.time")

public:
    explicit DBusService(QObject *parent = nullptr);

public slots:
    /**
     * @brief onStart:主程序通知插件开始计时
     * @return
     */
    Q_SCRIPTABLE bool onStart();

    /**
     * @brief onStop:主程序通知插件结束计时
     * @return
     */
    Q_SCRIPTABLE bool onStop();

    /**
     * @brief onRecording:主程序通知插件正在进行录屏
     * @return
     */
    Q_SCRIPTABLE bool onRecording();

    /**
     * @brief onRecording:主程序通知插件暂停录屏
     * @return
     */
    Q_SCRIPTABLE bool onPause();
signals:
    void start();
    void stop();
    void recording();
    void pause();
};

#endif // DBUSSERVICE_H
