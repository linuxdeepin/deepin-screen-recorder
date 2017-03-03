/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
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
#include "settings.h"

const int RecordProcess::RECORD_TYPE_VIDEO = 0;
const int RecordProcess::RECORD_TYPE_GIF = 1;
const int RecordProcess::RECORD_GIF_SLEEP_TIME = 1000;

RecordProcess::RecordProcess(QObject *parent) : QThread(parent)
{
    saveTempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
    defaultSaveDir = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();

    Settings *settings = new Settings();
    QVariant saveDirectoryOption = settings->getOption("save_directory");
    if (saveDirectoryOption.isNull()) {
        saveDir = defaultSaveDir;
    } else {
        // Make save directory as user's setting if directory exists and writable.
        QString saveDirectory = saveDirectoryOption.toString();
        if (QDir(saveDirectory).exists()) {
            if (QFileInfo(saveDirectory).isWritable()) {
                saveDir = saveDirectory;
            } else {
                saveDir = defaultSaveDir;
                qDebug() << QString("Directory %1 is not writable, save to %2").arg(saveDirectory).arg(defaultSaveDir);
            }
        } else {
            saveDir = defaultSaveDir;
            qDebug() << QString("Directory %1 not exists, save to %2").arg(saveDirectory).arg(defaultSaveDir);
        }
    }

    settings->setOption("save_directory", saveDir);
}

void RecordProcess::setRecordInfo(int rx, int ry, int rw, int rh, QString name, int sw, int sh)
{
    recordX = rx;
    recordY = ry;
    recordWidth = rx + rw <= sw ? rw : sw - rx;
    recordHeight = ry + rh <= sh ? rh : sh - ry;
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
    // Create process and handle finish signal.
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

void RecordProcess::startRecord()
{
    recordTime = new QTime();
    recordTime->start();
    QThread::start();
}

void RecordProcess::stopRecord()
{
    // If record time less than 1 second, wait 1 second make sure generate gif file correctly.
    int elapsedTime = recordTime->elapsed();
    if (elapsedTime < RECORD_GIF_SLEEP_TIME && recordType == RECORD_TYPE_GIF) {
        msleep(RECORD_GIF_SLEEP_TIME);
        qDebug() << QString("Record time too short (%1), wait 1 second make sure generate gif file correctly.").arg(elapsedTime);
    }
    
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
        << ((unsigned int) 0)                    // id
        << QString("deepin-screen-recorder") // icon
        << tr("Record finished")    // summary
        << QString("%1 %2").arg(tr("Saved to")).arg(newSavePath) // body
        << actions              // actions
        << hints                // hints
        << (int) -1;            // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);

    QApplication::quit();
}
