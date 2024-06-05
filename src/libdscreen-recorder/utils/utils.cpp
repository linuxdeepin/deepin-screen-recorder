// Copyright (C) 2020 ~ 2024 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <QProcessEnvironment>
#include <QDir>
#include <QLibraryInfo>
#include <QStandardPaths>

static bool waylandMode = false;
static bool isFFmpegEnv = true;

bool Utils::checkWaylandMode()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    waylandMode = false;
    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) //是否开启wayland
        waylandMode = true;

    return waylandMode;
}

void Utils::setWaylandMode(bool mode)
{
    waylandMode = mode;
}

bool Utils::isWaylandMode()
{
    return waylandMode;
}

bool Utils::checkFFmpegEnv()
{
    bool flag = false;
    QDir dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    qDebug() <<  "where is libs? where is " << dir ;
    QStringList list = dir.entryList(QStringList() << (QString("libavcodec") + "*"), QDir::NoDotAndDotDot | QDir::Files);
    qDebug()  << "Is libavcodec in there?  there is :" << list ;

    if (list.contains("libavcodec.so.58")) {
        qInfo()  << "list contains libavcodec.so.58" ;
        flag = true;
    }

    //x11下需要检测ffmpeg应用是否存在
    if (!isWaylandMode()) {
        flag = !QStandardPaths::findExecutable("ffmpeg").isEmpty();
        qInfo() << "Is exists ffmpeg in PATH(" << qgetenv("PATH") << "):" << flag;
    }
    isFFmpegEnv = flag;
    return flag;
}

void Utils::setFFmpegMode(bool mode)
{
    isFFmpegEnv = mode;
}

bool Utils::isFFmpegMode()
{
    return isFFmpegEnv;
}
