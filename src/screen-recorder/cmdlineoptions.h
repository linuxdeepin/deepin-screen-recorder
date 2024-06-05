// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCommandLineOption>
#include <QList>

struct CmdLineOptions {
    static CmdLineOptions *self();

    const QCommandLineOption delayOption = {{"d","delay"}, "Take a screenshot after NUM seconds.", "NUM"};
    const QCommandLineOption fullscreenOption = {{"f",  "fullscreen"}, "Take a screenshot the whole screen."};
    const QCommandLineOption topWindowOption = {{"w" ,  "top-window"}, "Take a screenshot of the most top window."};
    const QCommandLineOption savePathOption = {{"s",  "save-path"}, "Specify a path to save the screenshot.", "PATH"};
    const QCommandLineOption prohibitNotifyOption = {{"n",  "no-notification"}, "Don't send notifications."};
    const QCommandLineOption useGStreamer = {{"g",  "gstreamer"}, "Use GStreamer."};
    const QCommandLineOption dbusOption = {{"u",  "dbus"}, "Start  from dbus."};
    const QCommandLineOption screenRecordOption = {{"record",  "screenRecord"},  "start screen record"};
    const QCommandLineOption screenShotOption = {{"shot",  "screenShot"},  "start screen shot"};
    const QCommandLineOption screenOcrOption = {{"ocr",  "screenOcr"},  "start screen ocr"};
    const QCommandLineOption screenScrollOption = {{"scroll",  "screenScroll"},  "start screen scroll"};

    const QList<QCommandLineOption> allOptions = {
        delayOption, fullscreenOption, topWindowOption, savePathOption, prohibitNotifyOption, useGStreamer, 
        dbusOption, screenRecordOption, screenShotOption, screenOcrOption, screenScrollOption,
    };

    enum Option {
        DelayOption = 0,
        FullscreenOption,
        TopWindowOption,
        SavePathOption,
        ProhibitNotifyOption,
        UseGStreamer,
        DbusOption,
        ScreenRecordOption,
        ScreenShotOption,
        ScreenOcrOption,
        ScreenScrollOption,
        TotalOptions
    };

    const QCommandLineOption getOption(const Option &option);
};
