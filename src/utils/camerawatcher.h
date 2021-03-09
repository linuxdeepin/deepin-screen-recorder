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

#ifndef CAMERAWATCHER_H
#define CAMERAWATCHER_H

#include <QObject>
#include <QThread>
#include <QMutex>

class CameraWatcher : public QThread
{
    Q_OBJECT
public:
    explicit CameraWatcher(QObject *parent = nullptr);
    ~CameraWatcher();
    void setWatch(const bool &is);
    bool isWatch();
    //void setIsRecoding(bool value);
    void run();

signals:
    void sigCameraState(bool couldUse);


private:
    bool m_loopwatch;
    //bool m_isRecoding;
    bool m_coulduse;
    //多线程加锁
    QMutex m_mutex;
};

#endif // CAMERAWATCHER_H

