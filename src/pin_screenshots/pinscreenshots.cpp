/*

* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.

*

* Author:     wangcong <wangcong@uniontech.com>

*

* Maintainer: wangcong <wangcong@uniontech.com>

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
#include "pinscreenshots.h"
#include "mainwindow.h"
#include <DWidgetUtil>
#include <QDebug>

PinScreenShots::PinScreenShots(QObject *parent) : QObject(parent)
{
    m_loadingCount = 0;
}

bool PinScreenShots::openFile(QString filePath)
{
    qDebug() << __FUNCTION__ << __LINE__ << filePath;
    bool bRet = false;
    MainWindow *win = new MainWindow();
    //增加判断，空图片不会启动
    bRet = win->openFile(filePath);
    if (bRet) {
        win->show();
        //第一次启动才居中
        if (m_loadingCount == 0) {
            m_loadingCount++;
        }
    }
    return bRet;
}

void PinScreenShots::openImage(QImage image)
{
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        qDebug() << __FUNCTION__ << __LINE__ << image.size();
        MainWindow *win = new MainWindow();
        win->openImage(image);
        win->show();
        //第一次启动才居中
        if (m_loadingCount == 0) {
            m_loadingCount++;
        }
    }
}

void PinScreenShots::openImageAndName(QImage image, QString imageName, QPoint point)
{
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        MainWindow *win = new MainWindow();
        win->openImageAndName(image, imageName, point);
        // 防止双屏模式下，贴图尺寸被改变
        Dtk::Widget::moveToCenter(win);
        win->move(win->getShowPosition());
        win->show();
        //第一次启动才居中
        if (m_loadingCount == 0) {
            m_loadingCount++;
        }
    }
}
