/* -*- Mode: Vala; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2017 Deepin, Inc.
 *               2011 ~ 2017 Wang Yong
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

#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include <QtNetwork/QLocalServer>

class SingleApplication : public QApplication {
    Q_OBJECT

public:
    SingleApplication(int &argc, char **argv);
    ~SingleApplication() {
        delete localServer;
    };

    bool isRunning();
	void loadTranslations();

private slots:
    void newLocalConnection();
    
signals:
    void secondInstanceStart();

private:
    void initLocalConnection();
    void newLocalServer();

    bool appIsRunning;
    QLocalServer *localServer;
    QString serverName;
};

#endif // SINGLEAPPLICATION_H
