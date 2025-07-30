// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshot.h"
#include "dbusinterface/dbusnotify.h"
#include "utils/configsettings.h"
#include "utils.h"
#include "utils/eventlogutils.h"
#include "utils/log.h"
#include <QApplication>
#include <QScreen>
#include <QWindow>

//#include <dscreenwindowsutil.h>

//DWM_USE_NAMESPACE
Screenshot::Screenshot(QObject *parent)
    : QObject(parent)
{
    qCDebug(dsrApp) << "Screenshot constructor called.";
    connect(&m_window, &MainWindow::screenshotSaved, this, [this](const QString &savePath) {
        if (m_isCustomScreenshot) {
            emit screenshotSaved(savePath);
            m_isCustomScreenshot = false; // 重置标记
        }
    });
}

void Screenshot::startScreenshot()
{
    qCDebug(dsrApp) << "startScreenshot() called.";
    m_window.initAttributes();
    m_window.initResource();
    m_window.initLaunchMode(m_launchMode);
    if (Utils::isWaylandMode){
        m_window.showNormal();
        qCDebug(dsrApp) << "Showing window in normal mode for Wayland.";
    }else {
        m_window.showFullScreen();
        qCDebug(dsrApp) << "Showing window in full screen mode for non-Wayland.";
    }
    m_window.createWinId();
    qCDebug(dsrApp) << "Window ID created.";
    //平板模式截图录屏
    if (Utils::isTabletEnvironment) {
        qCDebug(dsrApp) << "Tablet environment detected.";
        if (QString("screenRecord") == m_launchMode) {
            m_window.tableRecordSet();
            qCDebug(dsrApp) << "Setting table record for screenRecord launch mode.";
        } else {
            m_window.initPadShot();
            qCDebug(dsrApp) << "Initializing pad shot for other launch modes.";
        }
    }
}

void Screenshot::customScreenshot(bool hideToolbar, bool notify)
{
    m_window.setToolbarVisable(hideToolbar);
    m_isCustomScreenshot = true; // 标记为自定义截图
    startScreenshot();
}

void Screenshot::delayScreenshot(double num)
{
    qCDebug(dsrApp) << "delayScreenshot() called with delay:" << num << "seconds.";
    QString summary = QString(tr("Screen Capture will start in %1 seconds").arg(num));
    QStringList actions = QStringList();
    QVariantMap hints;
    DBusNotify *notifyDBus = new DBusNotify(this);
    if (num >= 2) {
        notifyDBus->Notify(Utils::appName /*QCoreApplication::applicationName()*/, 0,  "deepin-screen-recorder", "",
                           summary, actions, hints, 3000);
        qCDebug(dsrApp) << "Notification sent for delayed screenshot.";
    }

    QTimer *timerNoti = new QTimer(this);
    timerNoti->setSingleShot(true);
    timerNoti->start(int(500 * num));
    qCDebug(dsrApp) << "Notification timer started.";
    connect(timerNoti, &QTimer::timeout, this, [ = ] {
        notifyDBus->CloseNotification(0);
        qCDebug(dsrApp) << "Notification closed.";
    });

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(int(1000 * num));
    qCDebug(dsrApp) << "Main delay timer started.";
    connect(timer, &QTimer::timeout, this, [ = ] {
        m_window.initAttributes();
        m_window.initLaunchMode("screenShot");
        m_window.showFullScreen();
        m_window.initResource();
        m_window.createWinId();
        qCDebug(dsrApp) << "Screenshot window initialized and shown after delay.";
    });
}

void Screenshot::fullscreenScreenshot()
{
    qCDebug(dsrApp) << "fullscreenScreenshot() called.";
    m_window.fullScreenshot();
}

void Screenshot::topWindowScreenshot()
{
    qCDebug(dsrApp) << "topWindowScreenshot() called.";
    m_window.topWindow();
}

void Screenshot::noNotifyScreenshot()
{
    qCDebug(dsrApp) << "noNotifyScreenshot() called.";
    m_window.noNotify();
}

void Screenshot::OcrScreenshot()
{
    qCDebug(dsrApp) << "OcrScreenshot() called.";
#ifdef OCR_SCROLL_FLAGE_ON
    m_window.initAttributes();
    m_window.initResource();
    m_window.initLaunchMode("screenOcr");
    m_window.showFullScreen();
    m_window.createWinId();
    qCDebug(dsrApp) << "OCR screenshot window initialized and shown.";
#else
    qCDebug(dsrApp) << "OCR_SCROLL_FLAGE_ON is not defined, skipping OCR screenshot initialization.";
#endif
}

void Screenshot::ScrollScreenshot()
{
    qCDebug(dsrApp) << "ScrollScreenshot() called.";
#ifdef OCR_SCROLL_FLAGE_ON
    //2d模式不支持滚动截图
    qCDebug(dsrApp) << "Whether to turn on window effects? " << (DWindowManagerHelper::instance()->hasComposite() ? "yes" : "no") << ".";
    if (DWindowManagerHelper::instance()->hasComposite()) {
        qCDebug(dsrApp) << "Starting scroll shot.";
        m_window.initAttributes();
        m_window.initResource();
        m_window.initLaunchMode("screenScroll");
        m_window.showFullScreen();
        m_window.createWinId();
        qCDebug(dsrApp) << "Scroll screenshot window initialized and shown.";
    } else {
        qCDebug(dsrApp) << "Scroll shot exit. Window effects not supported.";
        qApp->quit();
        if (Utils::isWaylandMode) {
            _Exit(0);
            qCDebug(dsrApp) << "Exiting application for Wayland mode.";
        }
    }
#else
    qCDebug(dsrApp) << "OCR_SCROLL_FLAGE_ON is not defined, exiting scroll screenshot.";
    qApp->quit();
    if (Utils::isWaylandMode) {
        _Exit(0);
        qCDebug(dsrApp) << "Exiting application for Wayland mode.";
    }
#endif
}

void Screenshot::savePathScreenshot(const QString &path)
{
    qCDebug(dsrApp) << "savePathScreenshot() called with path:" << path << ".";
    m_window.savePath(path);
}

void Screenshot::startScreenshotFor3rd(const QString &path)
{
    qCDebug(dsrApp) << "startScreenshotFor3rd() called with path:" << path << ".";
    Utils::is3rdInterfaceStart = true;
    m_window.startScreenshotFor3rd(path);
}

void Screenshot::initLaunchMode(const QString &launchmode)
{
    qCDebug(dsrApp) << "initLaunchMode() called with mode:" << launchmode << ".";
    m_launchMode = launchmode;
}

void Screenshot::fullScreenRecord(QString fileName)
{
    qCDebug(dsrApp) << "Start Full Screen Record! File name:" << fileName << ".";
     m_window.fullScreenRecord(fileName);
}

void Screenshot::stopRecord()
{
    qCDebug(dsrApp) << "stopRecord() called.";
    m_window.stopRecord();
}

void Screenshot::stopApp()
{
    qCDebug(dsrApp) << "stopApp() called.";
    m_window.stopApp();
}

QString Screenshot::getRecorderNormalIcon()
{
    qCDebug(dsrApp) << "getRecorderNormalIcon() called.";
    return RecorderTablet::getRecorderNormalIcon();
}

Screenshot::~Screenshot()
{
    qCDebug(dsrApp) << "Screenshot destructor called.";
}
