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

#include <QApplication>
#include <QDebug>
#include <QDate>
#include <QtDBus>
#include <QDir>
#include <QStandardPaths>
#include "record_process.h"
#include "utils.h"

RecordProcess::RecordProcess(QObject *parent) : QThread(parent)
{
    saveTempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
    saveDir = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
}

void RecordProcess::setRecordInfo(int rx, int ry, int rw, int rh, QString name)
{
    recordX = rx;
    recordY = ry;
    recordWidth = rw;
    recordHeight = rh;
    saveAreaName = name;
}

void RecordProcess::setRecordType(int type)
{
    recordType = type;
}

void RecordProcess::run()
{
    // Start record.
    recordType == RECORD_TYPE_GIF ? recordGIF() : recordVideo();

    // Got output or error.
    process->waitForFinished(-1);
    if (process->exitCode() !=0) {
        qDebug() << "Error";
        foreach (auto line, (process->readAllStandardError().split('\n'))) {
            qDebug() << line;
        }
    } else{
        qDebug() << "OK" << process->readAllStandardOutput() << process->readAllStandardError();
    }
}

void RecordProcess::recordGIF()
{
    initProcess();

    QStringList arguments;
    arguments << QString("--duration=%1").arg(864000);
    arguments << QString("--x=%1").arg(recordX) << QString("--y=%1").arg(recordY);
    arguments << QString("--width=%1").arg(recordWidth) << QString("--height=%1").arg(recordHeight);
    arguments << savePath;

    process->start("byzanz-record", arguments);
}

void RecordProcess::recordVideo()
{
    initProcess();

    // FFmpeg need pass arugment split two part: -option value,
    // otherwise, it will report 'Unrecognized option' error.
    QStringList arguments;
    arguments << QString("-video_size");
    arguments << QString("%1x%2").arg(recordWidth).arg(recordHeight);
    arguments << QString("-framerate");
    arguments << QString("25");
    arguments << QString("-f");
    arguments << QString("x11grab");
    arguments << QString("-i");
    arguments << QString(":0.0+%1,%2").arg(recordX).arg(recordY);
    arguments << savePath;

    process->start("ffmpeg", arguments);
}

void RecordProcess::initProcess() {
    // Create process and handle finish singal.
    process = new QProcess();
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));

    // Build temp save path.
    QDateTime date = QDateTime::currentDateTime();
    saveBaseName = QString("%1_%2_%3.%4").arg(tr("deepin-screen-recorder")).arg(saveAreaName).arg(date.toString("yyyyMMddhhmmss")).arg(recordType == RECORD_TYPE_GIF ? "gif" : "mp4");
    savePath = QDir(saveTempDir).filePath(saveBaseName);

    // Remove same cache file first.
    QFile file(savePath);
    file.remove();
}

void RecordProcess::stopRecord()
{
    // Exit record process.
    process->terminate();

    // Wait thread.
    wait();

    // Popup notify.
    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());

    QStringList actions;
    actions << "_open" << tr("View");

    QString newSavePath = QDir(saveDir).filePath(saveBaseName);
    QFile::rename(savePath, newSavePath);

    QVariantMap hints;
    hints["x-deepin-action-_open"] = QString("xdg-open,%1").arg(newSavePath);


    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName()) // appname
        << ((unsigned int) 0)					 // id
        << Utils::getImagePath("deepin-screen-recorder.svg") // icon
        << tr("Recording finished")	// summary
        << QString("%1 %2").arg(tr("Save in")).arg(newSavePath) // body
        << actions				// actions
        << hints				// hints
        << (int) -1;			// timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);

    QApplication::quit();
}
