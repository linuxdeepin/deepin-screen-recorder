/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
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
#include "utils/audioutils.h"
#include <signal.h>
#include <proc/sysinfo.h>
#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QtDBus>
#include <QScreen>

const int RecordProcess::RECORD_TYPE_VIDEO = 0;
const int RecordProcess::RECORD_TYPE_GIF = 1;
const int RecordProcess::RECORD_TYPE_MP4 = 2;
const int RecordProcess::RECORD_TYPE_MKV = 3;

const int RecordProcess::RECORD_AUDIO_INPUT_MIC = 2;
const int RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO = 3;
const int RecordProcess::RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO = 4;

const int RecordProcess::RECORD_FRAMERATE_5 = 5;
const int RecordProcess::RECORD_FRAMERATE_10 = 10;
const int RecordProcess::RECORD_FRAMERATE_20 = 20;
const int RecordProcess::RECORD_FRAMERATE_24 = 24;
const int RecordProcess::RECORD_FRAMERATE_30 = 30;





RecordProcess::RecordProcess(QObject *parent) : QThread(parent)
{
    settings = new Settings();
    m_framerate = RECORD_FRAMERATE_24;

    saveTempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
    defaultSaveDir = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();

    displayNumber = QString(std::getenv("DISPLAY"));

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

    if (settings->getOption("lossless_recording").toString() == "") {
        settings->setOption("lossless_recording", false);
    }
}

void RecordProcess::setRecordInfo(const QRect &recordRect, const QString &filename)
{
    m_recordRect = recordRect;
    saveAreaName = filename;
}

void RecordProcess::setRecordType(int type)
{
    recordType = type;
}

void RecordProcess::setFrameRate(int framerate)
{
    m_framerate = framerate;
}
void RecordProcess::setRecordAudioInputType(int inputType)
{
    recordAudioInputType = inputType;
}
void RecordProcess::run()
{
    if (m_info.waylandDectected()) {
        WaylandRecord();
    }else{
        // Start record.
        recordType == RECORD_TYPE_GIF ? recordGIF() : recordVideo();
    }

    // Got output or error.
    process->waitForFinished(-1);
    if (process->exitCode() != 0) {
        qDebug() << "Error";
        foreach (auto line, (process->readAllStandardError().split('\n'))) {
            qDebug() << line;
        }
    } else {
        qDebug() << "OK" << process->readAllStandardOutput() << process->readAllStandardError();
    }
}
void RecordProcess::WaylandRecord()
{
    // 启动wayland录屏
    qDebug() << "RecordProcess::WaylandRecord()";
    initProcess();
    QStringList arguments;
    arguments << QString("%1").arg(RECORD_TYPE_GIF);
    arguments << QString("%1").arg(m_recordRect.width()) << QString("%1").arg(m_recordRect.height());
    arguments << QString("%1").arg(m_recordRect.x()) << QString("%1").arg(m_recordRect.y());
    arguments << QString("%1").arg(m_framerate);

    QString fileName = savePath;
    if(recordType == RECORD_TYPE_GIF){
        fileName.replace(".gif", ".mp4");
    }
    arguments << QString("%1").arg(fileName);
    arguments << QString("%1").arg(recordAudioInputType);
    //arguments << QString("%d").arg(t_currentAudioChannel);
    qDebug() << arguments;
    process->start("xdg-desktop-portal-kde", arguments);
    return;
}
void RecordProcess::recordGIF()
{
//    if (!m_info.waylandDectected()) {
//        QRect target( m_recordRect.x(),
//                      m_recordRect.y(),
//                      m_recordRect.width(),
//                      m_recordRect.height() );
//        qDebug() << "m_backgroundRect" << m_recordRect;

//        QPixmap res;
//        QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
//                                     QStringLiteral("/Screenshot"),
//                                     QStringLiteral("org.kde.kwin.Screenshot"));
//        QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
//        res = QPixmap(reply.value());
//        if (!res.isNull()) {
//            QFile dbusResult(reply.value());
//            dbusResult.remove();
//        }

//        QPixmap t_resultPixmap = res.copy(target);

//        QDir saveTempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
//        QString t_saveFileName = saveTempDir.path() + "123.png";
//        t_resultPixmap.save(t_saveFileName,  "png");


//    } else {
        initProcess();

        // byzanz-record use command follow option --exec to stop recording gif.
        // So we use command "sleep 365d" (sleep 365 days, haha) to make byzanz-record keep recording.
        // We just need kill "sleep" process when we want stop recording gif.
        // NOTE: don't kill byzanz-record process directly, otherwise recording content in system memory can't flush to disk.
        QString sleepCommand = "sleep 365d";

        QStringList arguments;
        arguments << QString("--cursor");
        arguments << QString("--x=%1").arg(m_recordRect.x()) << QString("--y=%1").arg(m_recordRect.y());
        arguments << QString("--width=%1").arg(m_recordRect.width()) << QString(    "--height=%1").arg(m_recordRect.height());
        arguments << QString("--exec=%1").arg(sleepCommand);
        arguments << savePath;

        process->start("byzanz-record", arguments);

        byzanzProcessId = process->pid();

        qDebug() << "byzanz-record pid: " << byzanzProcessId;

//    }


}

void RecordProcess::recordVideo()
{
    initProcess();
    //取系统音频的通道号
    AudioUtils *audioUtils = new AudioUtils(this);
    QString t_currentAudioChannel = audioUtils->currentAudioChannel();

    t_currentAudioChannel = t_currentAudioChannel.left(t_currentAudioChannel.size() - 1);
    qDebug() << "current audio channel:" << t_currentAudioChannel;

    // FFmpeg need pass arugment split two part: -option value,
    // otherwise, it will report 'Unrecognized option' error.
    QStringList arguments;

    //if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxin == false) {
    //if (m_isZhaoxin == false) {
        if (settings->getOption("lossless_recording").toBool()) {
            //    if (settings->getOption("lossless_recording").toBool()) {
            //        int framerate = 30;
            //        if (!settings->getOption("mkv_framerate").isNull()) {
            //            framerate = settings->getOption("mkv_framerate").toInt();
            //        } else {
            //            qDebug() << "Not found mkv_framerate option in config file, mkv use framerate 30";
            //        }

            qDebug() << "mkv framerate " << m_framerate;

            arguments << QString("-video_size");
            arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());
            arguments << QString("-framerate");
            arguments << QString("%1").arg(m_framerate);
            arguments << QString("-probesize");
            arguments << QString("24M");
            arguments << QString("-thread_queue_size");
            arguments << QString("256");
            arguments << QString("-f");
            arguments << QString("x11grab");
            arguments << QString("-i");
            arguments << QString("%1+%2,%3").arg(displayNumber).arg(m_recordRect.x()).arg(m_recordRect.y());

            if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {

                lastAudioSink = audioUtils->currentAudioSink();

                arguments << QString("-thread_queue_size");
                arguments << QString("256");

                arguments << QString("-f");
                arguments << QString("pulse");

                arguments << QString("-ac");
                arguments << QString("2");

                arguments << QString("-i");
                arguments << QString("%1").arg(t_currentAudioChannel);

                arguments << QString("-thread_queue_size");
                arguments << QString("256");

                arguments << QString("-f");
                arguments << QString("pulse");

                arguments << QString("-ac");
                arguments << QString("2");

                arguments << QString("-i");
                arguments << QString("default");

                //arguments << QString("-filter_complex");
                //arguments << QString("amerge");

                arguments << QString("-filter_complex");
                arguments << QString("[1:a]aselect=concatdec_select,aresample=async=1[a1];[2:a]aselect=concatdec_select,aresample=async=1[a2];[a1][a2]amix=inputs=2:duration=first:dropout_transition=0[out]");
                //arguments << QString("[1:a]asetpts=N/SR/TB[a1];[2:a]asetpts=N/SR/TB[a2];[a1][a2]amerge=inputs=2[out]");

                arguments << QString("-map");
                arguments << QString("0:v");
                arguments << QString("-map");
                arguments << QString("[out]");

//                arguments << QString("-preset");
//                arguments << QString("ultrafast");
            } else if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC) {
                lastAudioSink = audioUtils->currentAudioSink();
                arguments << QString("-thread_queue_size");
                arguments << QString("256");
                arguments << QString("-f");
                arguments << QString("pulse");
                arguments << QString("-ac");
                arguments << QString("2");
                arguments << QString("-i");
                arguments << QString("default");
                arguments << QString("-af");
                arguments << QString("aselect=concatdec_select,aresample=async=1");
                //按当前采样率生成时间戳:
                //arguments << QString("-filter:a");
                //arguments << QString("asetpts=N/SR/TB");

//              arguments << QString("-preset");
//              arguments << QString("ultrafast");
            } else if (recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO) {
                lastAudioSink = audioUtils->currentAudioSink();
                arguments << QString("-thread_queue_size");
                arguments << QString("256");
                arguments << QString("-f");
                arguments << QString("pulse");
                arguments << QString("-ac");
                arguments << QString("2");
                arguments << QString("-i");
                arguments << QString("%1").arg(t_currentAudioChannel);
                arguments << QString("-af");
                arguments << QString("aselect=concatdec_select,aresample=async=1");
                //按当前采样率生成时间戳:
                //arguments << QString("-filter:a");
                //arguments << QString("asetpts=N/SR/TB");
            }


            arguments << QString("-c:v");
            arguments << QString("libx264");
            arguments << QString("-qp");
            arguments << QString("23");

            arguments << QString("-preset");
            arguments << QString("ultrafast");

            arguments << savePath;
        } else {
            qDebug() << "mp4 framerate" << m_framerate;

            arguments << QString("-video_size");
            arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());
            arguments << QString("-framerate");
            arguments << QString("%1").arg(m_framerate);
            arguments << QString("-probesize");
            arguments << QString("24M");
            arguments << QString("-thread_queue_size");
            arguments << QString("256");
            arguments << QString("-f");
            arguments << QString("x11grab");
            arguments << QString("-i");
            arguments << QString("%1+%2,%3").arg(displayNumber).arg(m_recordRect.x()).arg(m_recordRect.y());
            if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {

                lastAudioSink = audioUtils->currentAudioSink();
//                audioUtils->setupSystemAudioOutput();

//                arguments << QString("-thread_queue_size");
//                arguments << QString("1024");
//                arguments << QString("-f");
//                arguments << QString("alsa");
//                arguments << QString("-ac");
//                arguments << QString("2");
//                arguments << QString("-i");
//                arguments << QString("hw:Loopback,1,0");

                arguments << QString("-thread_queue_size");
                arguments << QString("256");

                arguments << QString("-f");
                arguments << QString("pulse");

                arguments << QString("-ac");
                arguments << QString("2");

                arguments << QString("-i");
                arguments << QString("%1").arg(t_currentAudioChannel);



                arguments << QString("-thread_queue_size");
                arguments << QString("256");

                arguments << QString("-f");
                arguments << QString("pulse");

                arguments << QString("-ac");
                arguments << QString("2");

                arguments << QString("-i");
                arguments << QString("default");

                //arguments << QString("-filter_complex");
                //arguments << QString("amerge");
                arguments << QString("-filter_complex");
                arguments << QString("[1:a]aselect=concatdec_select,aresample=async=1[a1];[2:a]aselect=concatdec_select,aresample=async=1[a2];[a1][a2]amix=inputs=2:duration=first:dropout_transition=0[out]");

                arguments << QString("-map");
                arguments << QString("0:v");
                arguments << QString("-map");
                arguments << QString("[out]");

//                arguments << QString("-preset");
//               arguments << QString("ultrafast");
//                arguments << QString("-filter_complex");
//                arguments << QString("amix=inputs=2:duration=first:dropout_transition=0");
            } else if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC) {
//                AudioUtils *audioUtils = new AudioUtils(this);
                lastAudioSink = audioUtils->currentAudioSink();

                arguments << QString("-thread_queue_size");
                arguments << QString("256");
                arguments << QString("-f");
                arguments << QString("pulse");
                arguments << QString("-ac");
                arguments << QString("2");
                arguments << QString("-i");
                arguments << QString("default");
                arguments << QString("-af");
                arguments << QString("aselect=concatdec_select,aresample=async=1");

//                arguments << QString("-preset");
//                arguments << QString("ultrafast");
            } else if (recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO) {
//                AudioUtils *audioUtils = new AudioUtils(this);
                lastAudioSink = audioUtils->currentAudioSink();
//                audioUtils->setupSystemAudioOutput();

//                arguments << QString("-thread_queue_size");
//                arguments << QString("1024");
//                arguments << QString("-f");
//                arguments << QString("alsa");
//                arguments << QString("-ac");
//                arguments << QString("2");
//                arguments << QString("-i");
//                arguments << QString("hw:Loopback,1,0");
                arguments << QString("-thread_queue_size");
                arguments << QString("256");
                arguments << QString("-f");
                arguments << QString("pulse");
                arguments << QString("-ac");
                arguments << QString("2");
                arguments << QString("-i");
                arguments << QString("%1").arg(t_currentAudioChannel);
                arguments << QString("-af");
                arguments << QString("aselect=concatdec_select,aresample=async=1");

//                arguments << QString("-crf");
//                arguments << QString("0");

//                arguments << QString("-preset");
//                arguments << QString("ultrafast");
            }

            // Most mobile mplayer can't decode yuv444p (ffempg default format) video, yuv420p looks good.
            arguments << QString("-pix_fmt");
            arguments << QString("yuv420p");

            // append crf parameter here
            arguments << QString("-crf");
            arguments << QString("23");

            arguments << QString("-preset");
            arguments << QString("ultrafast");


            arguments << QString("-vf");
            arguments << QString("scale=trunc(iw/2)*2:trunc(ih/2)*2");

//            arguments << QString("-threads");
//            arguments << QString("0");

            arguments << savePath;
        }

//    else {
//        if (settings->getOption("lossless_recording").toBool()) {
//            //    if (settings->getOption("lossless_recording").toBool()) {
//            //        int framerate = 30;
//            //        if (!settings->getOption("mkv_framerate").isNull()) {
//            //            framerate = settings->getOption("mkv_framerate").toInt();
//            //        } else {
//            //            qDebug() << "Not found mkv_framerate option in config file, mkv use framerate 30";
//            //        }

//            qDebug() << "mkv framerate " << m_framerate;

//            arguments << QString("-vaapi_device");
//            arguments << QString("/dev/dri/renderD128");
//            arguments << QString("-video_size");
//            arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());
//            arguments << QString("-framerate");
//            arguments << QString("%1").arg(m_framerate);
//            arguments << QString("-f");
//            arguments << QString("x11grab");
//            arguments << QString("-i");
//            arguments << QString("%1+%2,%3").arg(displayNumber).arg(m_recordRect.x()).arg(m_recordRect.y());
//            arguments << QString("-c:v");
//            arguments << QString("libx264");
//            arguments << QString("-qp");
//            arguments << QString("0");
//            arguments << QString("-preset");
//            arguments << QString("ultrafast");
//            arguments << savePath;
//        } else {
//            //        int framerate = 25;

//            //        if (!settings->getOption("mp4_framerate").isNull()) {
//            //            framerate = settings->getOption("mp4_framerate").toInt();
//            //        } else {
//            //            qDebug() << "Not found mp4_framerate option in config file, mp4 use framerate 25";
//            //        }

//            qDebug() << "mp4 framerate" << m_framerate;

//            arguments << QString("-vaapi_device");
//            arguments << QString("/dev/dri/renderD128");
//            arguments << QString("-video_size");
//            arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());
//            arguments << QString("-framerate");
//            arguments << QString("%1").arg(m_framerate);
//            arguments << QString("-f");
//            arguments << QString("x11grab");
//            arguments << QString("-i");
//            arguments << QString("%1+%2,%3").arg(displayNumber).arg(m_recordRect.x()).arg(m_recordRect.y());
//            if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
//                AudioUtils *audioUtils = new AudioUtils(this);
//                lastAudioSink = audioUtils->currentAudioSink();
//                audioUtils->setupSystemAudioOutput();

//                arguments << QString("-thread_queue_size");
//                arguments << QString("1024");
//                arguments << QString("-f");
//                arguments << QString("alsa");
//                arguments << QString("-ac");
//                arguments << QString("2");
//                arguments << QString("-i");
//                arguments << QString("hw:Loopback,1,0");

//                arguments << QString("-thread_queue_size");
//                arguments << QString("1024");
//                arguments << QString("-f");
//                arguments << QString("pulse");
//                arguments << QString("-ac");
//                arguments << QString("2");
//                arguments << QString("-i");
//                arguments << QString("default");
//                arguments << QString("-filter_complex");
//                arguments << QString("amix=inputs=2:duration=first:dropout_transition=0");
//            } else if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC) {
//                AudioUtils *audioUtils = new AudioUtils(this);
//                lastAudioSink = audioUtils->currentAudioSink();

//                arguments << QString("-thread_queue_size");
//                arguments << QString("1024");
//                arguments << QString("-f");
//                arguments << QString("pulse");
//                arguments << QString("-ac");
//                arguments << QString("2");
//                arguments << QString("-i");
//                arguments << QString("default");
//            } else if (recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO) {
//                AudioUtils *audioUtils = new AudioUtils(this);
//                lastAudioSink = audioUtils->currentAudioSink();
//                audioUtils->setupSystemAudioOutput();

//                arguments << QString("-thread_queue_size");
//                arguments << QString("1024");
//                arguments << QString("-f");
//                arguments << QString("alsa");
//                arguments << QString("-ac");
//                arguments << QString("2");
//                arguments << QString("-i");
//                arguments << QString("hw:Loopback,1,0");
//            }

//            // Most mobile mplayer can't decode yuv444p (ffempg default format) video, yuv420p looks good.
//            arguments << QString("-pix_fmt");
//            arguments << QString("yuv420p");

//            // append crf parameter here
//            arguments << QString("-crf");
//            arguments << QString("25");

//            arguments << QString("-vf");
//            arguments << QString("scale=trunc(iw/2)*2:trunc(ih/2)*2");

//            arguments << savePath;
//        }
//    }

    qDebug() << "arguments:" << arguments;

    process->start("ffmpeg", arguments);
}

void RecordProcess::initProcess()
{
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
        if (settings->getOption("lossless_recording").toBool()) {
            fileExtension = "mkv";
        } else {
            fileExtension = "mp4";
        }
    }
    saveBaseName = QString("%1_%2_%3.%4").arg(tr("Screen Capture")).arg(saveAreaName).arg(date.toString("yyyyMMddhhmmss")).arg(fileExtension);
    savePath = QDir(saveTempDir).filePath(saveBaseName);

    // Remove same cache file first.
    QFile file(savePath);
    file.remove();
}

void RecordProcess::startRecord()
{
    QThread::start();
}

void RecordProcess::setIsZhaoXinPlatform(bool status)
{
    if (status) {
        m_isZhaoxin = true;
    } else {
        m_isZhaoxin = false;
    }
}

int RecordProcess::readSleepProcessPid()
{
    // Read the list of open processes information.
    PROCTAB *proc = openproc(
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

void RecordProcess::waylandRecordOver()
{
    if(!m_info.waylandDectected()){
        return;
    }

    if(recordType == RECORD_TYPE_GIF){
        transformToGif();
    }

    // wayland录屏进程调用dbus通知保存完成。
    qDebug() << "RecordProcess::waylandRecordOver()";
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
        << tr("Recording finished")                              // summary
        << QString(tr("Saved to %1")).arg(newSavePath) // body
        << actions                                               // actions
        << hints                                                 // hints
        << (int) -1;                                             // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
    QApplication::quit();
}
void RecordProcess::stopRecord()
{

    if(m_info.waylandDectected()){
        // wayland 录屏通过dbus接口停止。
        qDebug() << "RecordProcess::stopRecord()";
        QDBusInterface waylandInterface("org.freedesktop.impl.portal.desktop",
                                    "/org/freedesktop/portal/desktop",
                                    "org.freedesktop.impl.portal.Settings",
                                    QDBusConnection::sessionBus());

        if(waylandInterface.isValid()) {
            waylandInterface.call("stopRecord");
        }
        return;
    }

    if (recordType == RECORD_TYPE_GIF) {
        int byzanzChildPid = readSleepProcessPid();
        kill(byzanzChildPid, SIGKILL);

        qDebug() << "Kill byzanz-record's child process (sleep) pid: " << byzanzChildPid;
    } else {
        process->terminate();
    }

// Wait thread.
//    if (QSysInfo::currentCpuArchitecture().startsWith("x86")) {
    wait();
//    }
//    else {
//        wait(2000);
//    }


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
        << tr("Recording finished")                              // summary
        << QString(tr("Saved to %1")).arg(newSavePath) // body
        << actions                                               // actions
        << hints                                                 // hints
        << (int) -1;                                             // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);

//    if (lastAudioSink.length() > 0) {
//        AudioUtils *audioUtils = new AudioUtils(this);
////        audioUtils->setupAudioSink(lastAudioSink);
//    }

    QApplication::quit();
}
void RecordProcess::transformToGif()
{
    QStringList options;
    options << "-i";
    QString inputFile = savePath;
    options << inputFile.replace("gif", "mp4");
    options << savePath;
    qDebug() << options;
    QProcess process;
    process.start("ffmpeg", options);
    process.waitForFinished();
    process.waitForReadyRead();
    QString str_output = process.readAllStandardOutput();
    process.close();
    QFile(inputFile).remove();
    qDebug() << str_output;
}
