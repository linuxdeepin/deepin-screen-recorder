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
#include "screenshot/session/screenshotsessioncontroller.h"
#include "screenshot/session/screenshotsessionfactory.h"
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
    
    // 设置录屏状态变化回调
    m_window.setRecordingStateCallback([this](bool isRecording) {
        m_isRecording = isRecording;
        emit RecordingModeChanged(isRecording);
        qCDebug(dsrApp) << "Recording state updated via callback:" << isRecording;
    });
}

bool Screenshot::startTreelandScreenshot(int mode, bool enableWindowSnap,
                                         const QString &outputPath,
                                         bool notificationsEnabled,
                                         int delayMs)
{
    if (!Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported())
        return false;

    Dsr::Screenshot::Request request;
    request.mode = static_cast<Dsr::Screenshot::Mode>(mode);
    request.enableWindowSnap = enableWindowSnap;
    request.notificationsEnabled = notificationsEnabled;
    request.outputPath = outputPath.isEmpty() ? m_pendingSavePath : outputPath;
    request.delayMs = qMax(0, delayMs);
    qCInfo(dsrApp) << "Starting TreeLand screenshot session, mode:" << mode
                   << "snap:" << enableWindowSnap
                   << "delayMs:" << request.delayMs
                   << "output:" << request.outputPath;

    QString error;
    auto *session = Dsr::Screenshot::ScreenshotSessionFactory::createForCurrentPlatform(this,
                                                                                       &error);
    if (!session) {
        qWarning() << "Unable to create TreeLand screenshot session:" << error;
        return false;
    }

    m_treelandSession = session;
    connect(session, &Dsr::Screenshot::ScreenshotSessionController::saved,
            this, [this](const QString &path) {
        emit screenshotDone(path);
        if (m_isCustomScreenshot) {
            emit screenshotSaved(path);
            m_isCustomScreenshot = false;
        }
    });
    connect(session, &Dsr::Screenshot::ScreenshotSessionController::failed,
            this, [this](const QString &reason) {
        qWarning() << "TreeLand screenshot session failed:" << reason;
    });
    connect(session,
            &Dsr::Screenshot::ScreenshotSessionController::recordingStateChanged,
            this,
            [this, session] {
        const bool recording = session->isRecording();
        if (m_isRecording == recording)
            return;
        m_isRecording = recording;
        emit RecorderState(recording);
        emit RecordingModeChanged(recording);
        qCDebug(dsrApp) << "TreeLand recording state changed:" << recording;
    });
    connect(session, &Dsr::Screenshot::ScreenshotSessionController::finished,
            this, [this, session] {
        m_treelandSession.clear();
        session->deleteLater();
        emit screenshotFinished();
    });
    if (!session->start(request)) {
        qWarning() << "TreeLand screenshot session is already active";
        session->deleteLater();
        m_treelandSession.clear();
        return false;
    }
    m_pendingSavePath.clear();
    return true;
}

void Screenshot::startScreenshot()
{
    qCDebug(dsrApp) << "startScreenshot() called.";
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::Region), true))
            qWarning() << "Unable to start TreeLand region screenshot session";
        return;
    }
    m_window.initAttributes();
    m_window.initResource();
    m_window.initLaunchMode(m_launchMode);
#ifndef ENABLE_UNIT_TEST
    if (Utils::isWaylandMode){
        m_window.showNormal();
        qCDebug(dsrApp) << "Showing window in normal mode for Wayland.";
    }else {
#endif
        m_window.showFullScreen();
        qCDebug(dsrApp) << "Showing window in full screen mode for non-Wayland.";
#ifndef ENABLE_UNIT_TEST
    }
#endif
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
    m_isCustomScreenshot = true; // 标记为自定义截图
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::Region),
                                     true,
                                     QString(),
                                     notify))
            qWarning() << "Unable to start TreeLand custom screenshot session";
        return;
    }
    m_window.setToolbarVisable(hideToolbar);
    startScreenshot();
}

void Screenshot::delayScreenshot(double num)
{
    qCDebug(dsrApp) << "delayScreenshot() called with delay:" << num << "seconds.";
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        const qint64 requestedMs = qRound64(qMax(0.0, num) * 1000.0);
        const int delayMs = static_cast<int>(qMin<qint64>(requestedMs,
                                                          24LL * 60 * 60 * 1000));
        if (num >= 2) {
            const QString summary = QString(tr("Screen Capture will start in %1 seconds").arg(num));
            DBusNotify *notifyDBus = new DBusNotify(this);
            notifyDBus->Notify(Utils::appName, 0, "deepin-screen-recorder", "",
                               summary, QStringList(), QVariantMap(), 3000);
            QTimer::singleShot(int(500 * num), notifyDBus, [notifyDBus] {
                notifyDBus->CloseNotification(0);
            });
        }
        // Delay is owned by the session controller so cancel/fail can stop
        // it, and so D-Bus, CLI and shortcut paths share one implementation.
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::Region),
                                     true,
                                     QString(),
                                     true,
                                     delayMs)) {
            qWarning() << "Unable to start delayed TreeLand screenshot session";
        }
        return;
    }
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
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::FullScreen), false))
            qWarning() << "Unable to start TreeLand fullscreen screenshot session";
        return;
    }
    m_window.fullScreenshot();
}

void Screenshot::topWindowScreenshot()
{
    qCDebug(dsrApp) << "topWindowScreenshot() called.";
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::TopWindow), true))
            qWarning() << "Unable to start TreeLand top-window screenshot session";
        return;
    }
    m_window.topWindow();
}

void Screenshot::noNotifyScreenshot()
{
    qCDebug(dsrApp) << "noNotifyScreenshot() called.";
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::Region),
                                     true,
                                     QString(),
                                     false))
            qWarning() << "Unable to start TreeLand no-notify screenshot session";
        return;
    }
    m_window.noNotify();
}

void Screenshot::OcrScreenshot()
{
    qCDebug(dsrApp) << "OcrScreenshot() called.";
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::Ocr), true))
            qWarning() << "Unable to start TreeLand OCR screenshot session";
        return;
    }
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
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::Scroll), true))
            qWarning() << "Unable to start TreeLand scroll screenshot session";
        return;
    }

    //2d模式不支持滚动截图
    qCDebug(dsrApp) << "Whether to turn on window effects? " << (DWindowManagerHelper::instance()->hasBlurWindow() ? "yes" : "no") << ".";
    if (DWindowManagerHelper::instance()->hasBlurWindow()) {
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
#ifndef ENABLE_UNIT_TEST
        if (Utils::isWaylandMode) {
            _Exit(0);
            qCDebug(dsrApp) << "Exiting application for Wayland mode.";
        }
#endif
    }
#else
    qCDebug(dsrApp) << "OCR_SCROLL_FLAGE_ON is not defined, exiting scroll screenshot.";
    qApp->quit();
#ifndef ENABLE_UNIT_TEST
    if (Utils::isWaylandMode) {
        _Exit(0);
        qCDebug(dsrApp) << "Exiting application for Wayland mode.";
    }
#endif
#endif
}

void Screenshot::savePathScreenshot(const QString &path)
{
    qCDebug(dsrApp) << "savePathScreenshot() called with path:" << path << ".";
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::Region), true, path))
            qWarning() << "Unable to start TreeLand save-path screenshot session";
        return;
    }
    m_window.savePath(path);
}

void Screenshot::setSavePath(const QString &path)
{
    qCDebug(dsrApp) << "setSavePath() called with path:" << path << ".";
    m_pendingSavePath = path;
    m_window.setSavePath(path);
}

void Screenshot::startScreenshotFor3rd(const QString &path)
{
    qCDebug(dsrApp) << "startScreenshotFor3rd() called with path:" << path << ".";
    Utils::is3rdInterfaceStart = true;
    m_isCustomScreenshot = true;
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::Region),
                                     true,
                                     path,
                                     false))
            qWarning() << "Unable to start TreeLand third-party screenshot session";
        return;
    }
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
    if (Dsr::Screenshot::ScreenshotSessionFactory::isCurrentPlatformSupported()) {
        if (!startTreelandScreenshot(static_cast<int>(Dsr::Screenshot::Mode::RecordFullScreen),
                                     false,
                                     fileName))
            qWarning() << "Unable to start TreeLand fullscreen recording session";
        return;
    }
    m_window.fullScreenRecord(fileName);
}

void Screenshot::stopRecord()
{
    qCDebug(dsrApp) << "stopRecord() called.";
    if (m_treelandSession) {
        qCDebug(dsrApp) << "Forwarding stopRecord to TreeLand session.";
        m_treelandSession->stopRecord();
        return;
    }
    m_window.stopRecord();
}

void Screenshot::stopApp()
{
    qCDebug(dsrApp) << "stopApp() called.";
    if (m_treelandSession) {
        qCDebug(dsrApp) << "Forwarding stopApp to TreeLand session.";
        m_treelandSession->cancel();
        return;
    }
    m_window.stopApp();
}

QString Screenshot::getRecorderNormalIcon()
{
    qCDebug(dsrApp) << "getRecorderNormalIcon() called.";
    return RecorderTablet::getRecorderNormalIcon();
}

bool Screenshot::isRecording() const
{
    qCDebug(dsrApp) << "isRecording() called, returning:" << m_isRecording;
    return m_isRecording;
}

Screenshot::~Screenshot()
{
    qCDebug(dsrApp) << "Screenshot destructor called.";
}
