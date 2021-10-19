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

#include "screenshot.h"
#include "dbusinterface/dbusnotify.h"
#include "utils/configsettings.h"
#include "utils.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QWindow>

//#include <dscreenwindowsutil.h>

//DWM_USE_NAMESPACE
Screenshot::Screenshot(QObject *parent)
    : QObject(parent)
{

}

void Screenshot::startScreenshot()
{
    m_window.initAttributes();
    m_window.initResource();
    m_window.initLaunchMode(m_launchMode);
    m_window.showFullScreen();
    //平板模式截图录屏
    if (Utils::isTabletEnvironment) {
        if (QString("screenRecord") == m_launchMode) {
            m_window.tableRecordSet();
        } else {
            m_window.initPadShot();
        }
    }
}

void Screenshot::delayScreenshot(double num)
{
    qDebug() << "init with delay";
    QString summary = QString(tr("Screen Capture will start in %1 seconds").arg(num));
    QStringList actions = QStringList();
    QVariantMap hints;
    DBusNotify *notifyDBus = new DBusNotify(this);
    if (num >= 2) {
        notifyDBus->Notify(QCoreApplication::applicationName(), 0,  "deepin-screen-recorder", "",
                           summary, actions, hints, 3000);
    }

    QTimer *timerNoti = new QTimer;
    timerNoti->setSingleShot(true);
    timerNoti->start(int(500 * num));
    connect(timerNoti, &QTimer::timeout, this, [ = ] {
        notifyDBus->CloseNotification(0);
    });

    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    timer->start(int(1000 * num));
    connect(timer, &QTimer::timeout, this, [ = ] {
        m_window.initAttributes();
        m_window.initLaunchMode("screenShot");
        m_window.showFullScreen();
        m_window.initResource();
    });
}

void Screenshot::fullscreenScreenshot()
{
    m_window.fullScreenshot();
}

void Screenshot::topWindowScreenshot()
{
    m_window.topWindow();
}

void Screenshot::noNotifyScreenshot()
{
    m_window.noNotify();
}

void Screenshot::OcrScreenshot()
{
    m_window.initAttributes();
    m_window.initResource();
    m_window.initLaunchMode("screenOcr");
    m_window.showFullScreen();
}

void Screenshot::ScrollScreenshot()
{
    m_window.initAttributes();
    m_window.initResource();
    m_window.initLaunchMode("screenScroll");
    m_window.showFullScreen();

}

void Screenshot::savePathScreenshot(const QString &path)
{
    m_window.savePath(path);
}

void Screenshot::startScreenshotFor3rd(const QString &path)
{
    Utils::is3rdInterfaceStart = true;
    m_window.startScreenshotFor3rd(path);
}

void Screenshot::initLaunchMode(const QString &launchmode)
{
    m_launchMode = launchmode;
}

void Screenshot::stopRecord()
{
    m_window.stopRecord();
}

QString Screenshot::getRecorderNormalIcon()
{
    return RecorderTablet::getRecorderNormalIcon();
}

Screenshot::~Screenshot() 
{
}
