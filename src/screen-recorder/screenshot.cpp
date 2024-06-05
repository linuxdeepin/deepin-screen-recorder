// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshot.h"
#include "cmdlineoptions.h"
#include "dbusscreenshotservice.h"
#include "config.h"

#include "utils.h"
#include "global.h"
#include "eventlogutils.h"
#include "recorderwindow.h"
#include "imageplatform.h"
#include "videoplatform.h"
#include "platformloader.h"
#include "savemanager.h"

#include <QScreen>
#include <QCommandLineParser>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QGuiApplication>

Screenshot *Screenshot::s_self = nullptr;

//DWM_USE_NAMESPACE
Screenshot::Screenshot(QObject *parent)
    : QObject(parent)
{
    m_imagePlatform = loadImagePlatform();
    m_videoPlatform = loadVideoPlatform();

    connect(m_imagePlatform.get(), &ImagePlatform::newScreenshotTaken, this, [this](const QImage &image){
        setExportImage(image);
    });
}

Screenshot::~Screenshot()
{
}

Screenshot *Screenshot::instance()
{
    if (!s_self) {
        s_self = new Screenshot();
    }
    return s_self;
}

void Screenshot::setExportImage(const QImage &image)
{
    SaveManager::instance()->setImage(image);
}

void Screenshot::parser(const QStringList &arguments)
{
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("deepin-screen-recorder");
    cmdParser.addHelpOption();
    cmdParser.addOptions(CmdLineOptions::self()->allOptions);
    cmdParser.process(arguments);

    QVector<bool> cliOptions(CmdLineOptions::TotalOptions,false);
    int optionsToCheck = cmdParser.optionNames().size();
    for (int i = 0; optionsToCheck > 0 && i < CmdLineOptions::self()->allOptions.size(); ++i) {
        cliOptions[i] = cmdParser.isSet(CmdLineOptions::self()->allOptions[i]);
        if (cliOptions[i]) {
            --optionsToCheck;
        }
    }

    if (cliOptions[CmdLineOptions::UseGStreamer]) {
        Utils::setFFmpegMode(false);
    }

    m_launchMode = "screenShot";
    if (cliOptions[CmdLineOptions::ScreenRecordOption]) {
        m_launchMode = "screenRecord";
    } else if (cliOptions[CmdLineOptions::ScreenOcrOption]) {
        m_launchMode = "screenOcr";
    } else if (cliOptions[CmdLineOptions::ScreenScrollOption]) {
        m_launchMode = "screenScroll";
    }

    DBusScreenshotService *dbusService = new DBusScreenshotService(this);
    if (!cliOptions[CmdLineOptions::DbusOption]) {
        QJsonObject obj{
                {"tid", EventLogUtils::Start},
                {"version", QCoreApplication::applicationVersion()},
                {"mode", 1},
                {"startup_mode", "A"}
            };
        if (!cliOptions[CmdLineOptions::ScreenRecordOption])
            EventLogUtils::get().writeLogs(obj);

        dbusService->setSingleInstance(true);
        if (cliOptions[CmdLineOptions::DelayOption]) {
            qDebug() << "cmd delay screenshot";
            delayScreenshot(cmdParser.value(CmdLineOptions::self()->getOption(CmdLineOptions::DelayOption)).toInt());
        } else if (cliOptions[CmdLineOptions::FullscreenOption]) {
            fullscreenScreenshot();
        } else if (cliOptions[CmdLineOptions::TopWindowOption]) {
            qDebug() << "cmd topWindow screenshot";
            topWindowScreenshot();
        } else if (cliOptions[CmdLineOptions::SavePathOption]) {
            qDebug() << "cmd savepath screenshot";
            savePathScreenshot(cmdParser.value(CmdLineOptions::self()->getOption(CmdLineOptions::SavePathOption)));
        } else if (cliOptions[CmdLineOptions::ProhibitNotifyOption]) {
            qDebug() << "screenshot no notify!";
            noNotifyScreenshot();
        } else {
            startScreenshot();
        }
    }
}

QQmlEngine *Screenshot::getQmlEngine()
{
    if (m_engine == nullptr) {
        m_engine = std::make_unique<QQmlEngine>(this);

        qmlRegisterSingletonInstance(SCREENRECORDERCORE_QML_URI, 1, 0, "Screenshot", this);
        qmlRegisterSingletonInstance(SCREENRECORDERCORE_QML_URI, 1, 0, "ImagePlatform", m_imagePlatform.get());
        qmlRegisterSingletonInstance(SCREENRECORDERCORE_QML_URI, 1, 0, "VideoPlatform", m_videoPlatform.get());
        qmlRegisterSingletonInstance(SCREENRECORDERCORE_QML_URI, 1, 0, "Global", Global::instance());
    }
    return m_engine.get();
}

void Screenshot::startScreenshot()
{
    m_imagePlatform->grab(ImagePlatform::AllScreens,false);

    //test
    RecorderWindow *window = new RecorderWindow(RecorderWindow::shot,QGuiApplication::screens().first(),getQmlEngine());
    m_recorderWindows.push_back(window);
    window->setVisible(true);
}

void Screenshot::delayScreenshot(double num)
{
    
}

void Screenshot::fullscreenScreenshot()
{
    
}

void Screenshot::topWindowScreenshot()
{
    
}

void Screenshot::noNotifyScreenshot()
{
    
}

void Screenshot::ocrScreenshot()
{

}

void Screenshot::scrollScreenshot()
{

}

void Screenshot::savePathScreenshot(const QString &path)
{
   
}

void Screenshot::startScreenshotFor3rd(const QString &path)
{
    
}

void Screenshot::stopRecord()
{
    
}

QString Screenshot::getRecorderNormalIcon()
{
    return QString();
}
