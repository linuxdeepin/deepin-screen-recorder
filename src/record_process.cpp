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

#include "record_process.h"
#include "settings.h"
#include "process_tree.h"
#include "utils.h"
#include <signal.h>
#include <proc/sysinfo.h>
#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QtDBus>

const int RecordProcess::RECORD_TYPE_VIDEO = 0;
const int RecordProcess::RECORD_TYPE_GIF = 1;
const int RecordProcess::RECORD_GIF_SLEEP_TIME = 2000;

RecordProcess::RecordProcess(QObject *parent) : QThread(parent)
{
    settings = new Settings();

    saveTempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
    defaultSaveDir = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();

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

    qreal devicePixelRatio = qApp->devicePixelRatio();
    recordX = int(recordX * devicePixelRatio);
    recordY = int(recordY * devicePixelRatio);
    recordWidth = int(recordWidth * devicePixelRatio);
    recordHeight = int(recordHeight * devicePixelRatio);

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

    // byzanz-record use command follow option --exec to stop recording gif.
    // So we use command "sleep 365d" (sleep 365 days, haha) to make byzanz-record keep recording.
    // We just need kill "sleep" process when we want stop recording gif.
    // NOTE: don't kill byzanz-record process directly, otherwise recording content in system memory can't flush to disk.
    QString sleepCommand = "sleep 365d";

    QStringList arguments;
    arguments << QString("--cursor");
    arguments << QString("--x=%1").arg(recordX) << QString("--y=%1").arg(recordY);
    arguments << QString("--width=%1").arg(recordWidth) << QString("--height=%1").arg(recordHeight);
    arguments << QString("--exec=%1").arg(sleepCommand);
    arguments << savePath;

    process->start("byzanz-record", arguments);

    byzanzProcessId = process->pid();

    qDebug() << "byzanz-record pid: " << byzanzProcessId;

}

void RecordProcess::recordVideo()
{
    initProcess();

    // FFmpeg need pass arugment split two part: -option value,
    // otherwise, it will report 'Unrecognized option' error.
    QStringList arguments;

    if (settings->getOption("lossless_recording").toBool() || !QSysInfo::currentCpuArchitecture().startsWith("x86")) {
        int framerate = 30;
        if (!settings->getOption("mkv_framerate").isNull()) {
            framerate = settings->getOption("mkv_framerate").toInt();
        } else {
            qDebug() << "Not found mkv_framerate option in config file, mkv use framerate 30";
        }

        qDebug() << "mkv framerate " << framerate;

        arguments << QString("-video_size");
        arguments << QString("%1x%2").arg(recordWidth).arg(recordHeight);
        arguments << QString("-framerate");
        arguments << QString("%1").arg(framerate);
        arguments << QString("-f");
        arguments << QString("x11grab");
        arguments << QString("-i");
        arguments << QString(":0.0+%1,%2").arg(recordX).arg(recordY);
        arguments << QString("-c:v");
        arguments << QString("libx264");
        arguments << QString("-qp");
        arguments << QString("0");
        arguments << QString("-preset");
        arguments << QString("ultrafast");
        arguments << savePath;
    } else {
        int framerate = 25;

        if (!settings->getOption("mp4_framerate").isNull()) {
            framerate = settings->getOption("mp4_framerate").toInt();
        } else {
            qDebug() << "Not found mp4_framerate option in config file, mp4 use framerate 25";
        }

        qDebug() << "mp4 framerate " << framerate;

        arguments << QString("-video_size");
        arguments << QString("%1x%2").arg(recordWidth).arg(recordHeight);
        arguments << QString("-framerate");
        arguments << QString("%1").arg(framerate);
        arguments << QString("-f");
        arguments << QString("x11grab");
        arguments << QString("-i");
        arguments << QString(":0.0+%1,%2").arg(recordX).arg(recordY);

        // Most mobile mplayer can't decode yuv444p (ffempg default format) video, yuv420p looks good.
        arguments << QString("-pix_fmt");
        arguments << QString("yuv420p");

        arguments << QString("-vf");
        arguments << QString("scale=trunc(iw/2)*2:trunc(ih/2)*2");

        arguments << savePath;
    }

    process->start("ffmpeg", arguments);
}

void RecordProcess::initProcess() {
    // Create process and handle finish signal.
    process = new QProcess();

    // Disable scaling of byzanz-record (GTK3 based) here, because we pass subprocesses
    // absolute device geometry information, byzanz-record should not scale the information
    // one more time.
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GDK_SCALE", "1");
    process->setProcessEnvironment(env);

    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));

    // Build temp save path.
    QDateTime date = QDateTime::currentDateTime();
    QString fileExtension;
    if (recordType == RECORD_TYPE_GIF) {
        if (settings->getOption("lossless_recording").toBool()) {
            fileExtension = "flv";
        } else {
            fileExtension = "gif";
        }
    } else {
        if (settings->getOption("lossless_recording").toBool() || !QSysInfo::currentCpuArchitecture().startsWith("x86")) {
            fileExtension = "mkv";
        } else {
            fileExtension = "mp4";
        }
    }
    saveBaseName = QString("%1_%2_%3.%4").arg(tr("deepin-screen-recorder")).arg(saveAreaName).arg(date.toString("yyyyMMddhhmmss")).arg(fileExtension);
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

int RecordProcess::readSleepProcessPid()
{
    // Read the list of open processes information.
    PROCTAB* proc = openproc(
        PROC_FILLMEM |          // memory status: read information from /proc/#pid/statm
        PROC_FILLSTAT |         // cpu status: read information from /proc/#pid/stat
        PROC_FILLUSR            // user status: resolve user ids to names via /etc/passwd
        );
    static proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_t));

    StoredProcType processes;
    while (readproc(proc, &proc_info) != NULL) {
        processes[proc_info.tid] = proc_info;
    }
    closeproc(proc);

    ProcessTree *processTree = new ProcessTree();
    processTree->scanProcesses(processes);

    return processTree->getAllChildPids(byzanzProcessId)[0];
}


void RecordProcess::stopRecord()
{
    if (recordType == RECORD_TYPE_GIF) {
        int elapsedTime = recordTime->elapsed();
        if (elapsedTime < RECORD_GIF_SLEEP_TIME) {
            qDebug() << QString("Record time too short (%1), wait 1 second make sure generate gif file correctly.").arg(elapsedTime);
            msleep(RECORD_GIF_SLEEP_TIME);
        }

        int byzanzChildPid = readSleepProcessPid();
        kill(byzanzChildPid, SIGKILL);

        qDebug() << "Kill byzanz-record's child process (sleep) pid: " << byzanzChildPid;
    } else {
        process->terminate();
    }

    // Wait thread.
    wait();

    // Add end char ';' to gif file, avoid browser or gif-player can't play it.
    if (recordType == RECORD_TYPE_GIF) {
        FILE *gifFile = fopen(savePath.toUtf8().constData(), "ab");
        putc(0x3B, gifFile);
        fclose(gifFile);
    }

    // Move file to save directory.
    QString newSavePath = QDir(saveDir).filePath(saveBaseName);
    QFile::rename(savePath, newSavePath);

    // Popup notify.
    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());

    QStringList actions;
    actions << "_open" << tr("View");

    QVariantMap hints;
    hints["x-deepin-action-_open"] = QString("xdg-open,%1").arg(newSavePath);


    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName())                 // appname
        << ((unsigned int) 0)                                    // id
        << QString("deepin-screen-recorder")                     // icon
        << tr("Record finished")                                 // summary
        << QString("%1 %2").arg(tr("Saved to")).arg(newSavePath) // body
        << actions                                               // actions
        << hints                                                 // hints
        << (int) -1;                                             // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);

    QApplication::quit();
}
