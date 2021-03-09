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

#ifndef RECORDERTABLET_H
#define RECORDERTABLET_H

#include <QObject>
#include <QTimer>

class RecorderTablet : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.TabletScreenRecorder")
public:
    explicit RecorderTablet(QObject *parent = nullptr);
    ~RecorderTablet();

    void start();
    void stop();
    void startStatusBar();
    static const QString getRecorderNormalIcon();
signals:
    Q_SCRIPTABLE void StatusBarIconChange(const QString &in0);
    Q_SCRIPTABLE void DropDownIconChange(const QString &in0);
    void finished();
public slots:
    void update();
    void updateStatusBar();
private:
    void sendRecorderState(const bool state);
public:
    // 平板模式下 资源配置路径
    static const QString RESOURCES_PATH;


private:
    int showCountdownCounter = 0;
    int flashTrayIconCounter = 0;
    QTimer *m_statusBarTimer = nullptr;
    QTimer *m_changeTimer = nullptr;
};

#endif // RECORDERTABLET_H
