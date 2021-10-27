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
#include <QTimer>
#include <QMutex>

class CameraWatcher : public QObject
{
    Q_OBJECT
public:
    explicit CameraWatcher(QObject *parent = nullptr);
    ~CameraWatcher();
    void setWatch(const bool isWatcher);
    // 设置Coulduse的值
    void setCoulduseValue(bool value);
public slots:
    // 将原来的run()方法改为定时器的槽函数，便于截图快速退出
    // 取消之前的线程方式，采用定时器监测
    void slotCameraWatcher();

signals:
    void sigCameraState(bool couldUse);


private:
    bool m_coulduse;
    QTimer *m_watchTimer = nullptr; //新增摄像头监视
};

#endif // CAMERAWATCHER_H

