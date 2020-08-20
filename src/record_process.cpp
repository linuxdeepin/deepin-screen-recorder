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
    // Start record.
    QVariant t_saveGifVar = settings->getOption("save_as_gif");
    QVariant t_frameRateVar = settings->getOption("mkv_framerate");
    //保持帧数的配置文件判断
    int t_frameRate = t_frameRateVar.toString().toInt();
    m_framerate = t_frameRate;
    if (t_saveGifVar.toString() == "true") {
        recordType = RECORD_TYPE_GIF;
    } else {
        recordType = RECORD_TYPE_VIDEO;
    }

    recordType == RECORD_TYPE_GIF ? recordGIF() : recordVideo();

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
    arguments << QString("--x=%1").arg(m_recordRect.x()) << QString("--y=%1").arg(m_recordRect.y());
    arguments << QString("--width=%1").arg(m_recordRect.width()) << QString(    "--height=%1").arg(m_recordRect.height());
    arguments << QString("--exec=%1").arg(sleepCommand);
    arguments << savePath;

    process->start("byzanz-record", arguments);

    byzanzProcessId = static_cast<int>(process->pid());

    qDebug() << "byzanz-record pid: " << byzanzProcessId;
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

    QString arch = QSysInfo::currentCpuArchitecture();
    if(arch.startsWith("mips", Qt::CaseInsensitive) || arch.startsWith("sw", Qt::CaseInsensitive)){
        // mips sw 视频编码 mpeg4 音频编码 mp3
        /*
         * mkv
         ffmpeg -f x11grab -draw_mouse 1 -framerate 25 -video_size 1920x1080 -i :0+0,0 -f pulse -i vokoscreenMix.monitor
        -pix_fmt yuv420p -c:v mpeg4 -c:a libmp3lame -q:v 1 -s 1920x1080 -f matroska ./out.mkv
        */
        /*
         * mp4
         ffmpeg -f x11grab -draw_mouse 1 -framerate 25 -video_size 1920x1080 -i :0+0,0 -f pulse -i vokoscreenMix.monitor
        -pix_fmt yuv420p -c:v mpeg4 -c:a libmp3lame -q:v 1 -s 1920x1080 -f mp4 ./out.mp4
        */
        arguments << QString("-f");
        arguments << QString("x11grab");
        arguments << QString("-framerate");
        arguments << QString("%1").arg(m_framerate);
        arguments << QString("-video_size");
        arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());
        arguments << QString("-thread_queue_size");
        arguments << QString("64");
        arguments << QString("-i");
        arguments << QString("%1+%2,%3").arg(displayNumber).arg(m_recordRect.x()).arg(m_recordRect.y());
        if(recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO){
            arguments << QString("-thread_queue_size");
            arguments << QString("32");
            arguments << QString("-fragment_size");
            arguments << QString("2048");
            arguments << QString("-f");
            arguments << QString("pulse");
            arguments << QString("-ac");
            arguments << QString("2");
            arguments << QString("-ar");
            arguments << QString("44100");
            arguments << QString("-i");
            arguments << QString("%1").arg(t_currentAudioChannel);
        }
        if(recordAudioInputType == RECORD_AUDIO_INPUT_MIC || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO){
            arguments << QString("-thread_queue_size");
            arguments << QString("32");
            arguments << QString("-fragment_size");
            arguments << QString("2048");
            arguments << QString("-f");
            arguments << QString("pulse");
            arguments << QString("-ac");
            arguments << QString("2");
            arguments << QString("-ar");
            arguments << QString("44100");
            arguments << QString("-i");
            arguments << QString("default");
        }
        if(recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO){
            arguments << QString("-filter_complex");
            arguments << QString("amerge");
        }

        arguments << QString("-pix_fmt");
        arguments << QString("yuv420p");
        arguments << QString("-c:v");
        arguments << QString("mpeg4");
        //arguments << QString("mpeg2video");
        //arguments << QString("mpeg1video");
        //arguments << QString("libx264rgb");
        //arguments << QString("h263p");
        //arguments << QString("mjpeg");
        //arguments << QString("flv");
        arguments << QString("-c:a");
        arguments << QString("libmp3lame");
        arguments << QString("-q:v");
        arguments << QString("20");
        //arguments << QString("31"); // 视频质量
        arguments << QString("-s");
        arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());
        if(settings->getOption("lossless_recording").toBool()){
            arguments << QString("-f");
            arguments << QString("matroska");
        }else {
            arguments << QString("-f");
            arguments << QString("mp4");
        }
    }else{
        arguments << QString("-video_size");
        arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());
        arguments << QString("-framerate");
        arguments << QString("%1").arg(m_framerate);
        arguments << QString("-probesize");
        arguments << QString("24M");
        arguments << QString("-thread_queue_size");
        arguments << QString("64");
        arguments << QString("-f");
        arguments << QString("x11grab");
        arguments << QString("-i");
        arguments << QString("%1+%2,%3").arg(displayNumber).arg(m_recordRect.x()).arg(m_recordRect.y());


        if (recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
            //lastAudioSink = audioUtils->currentAudioSink();
            arguments << QString("-thread_queue_size");
            arguments << QString("32");
            arguments << QString("-fragment_size");
            arguments << QString("4096");
            arguments << QString("-f");
            arguments << QString("pulse");
            arguments << QString("-ac");
            arguments << QString("2");
            arguments << QString("-ar");
            arguments << QString("44100");
            arguments << QString("-i");
            arguments << QString("%1").arg(t_currentAudioChannel);
            if(recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO){
                if((arch.startsWith("ARM", Qt::CaseInsensitive))) {
                    arguments << QString("-af");
                    arguments << QString("volume=20dB");
                }
            }
        }
        if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
            arguments << QString("-thread_queue_size");
            arguments << QString("32");
            arguments << QString("-fragment_size");
            arguments << QString("4096");
            arguments << QString("-f");
            arguments << QString("pulse");
            arguments << QString("-ac");
            arguments << QString("2");
            arguments << QString("-ar");
            arguments << QString("44100");
            arguments << QString("-i");
            arguments << QString("default");
        }
        if(recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO){
            arguments << QString("-filter_complex");
            if((arch.startsWith("ARM", Qt::CaseInsensitive))) {
                arguments << QString("[1:a]volume=30dB[a1];[a1][2:a]amix=inputs=2:duration=first:dropout_transition=0[out]");
                arguments << QString("-map");
                arguments << QString("0:v");
                arguments << QString("-map");
                arguments << QString("[out]");
            }else {
                arguments << QString("amerge");
            }
        }
        arguments << QString("-c:v");
        arguments << QString("libx264");
        arguments << QString("-pix_fmt");
        arguments << QString("yuv420p");
        arguments << QString("-profile:v");
        arguments << QString("baseline");
        if (settings->getOption("lossless_recording").toBool()) {
            arguments << QString("-qp");
            arguments << QString("23");
        }else {
            arguments << QString("-crf");
            arguments << QString("23");
        }
        arguments << QString("-preset");
        arguments << QString("ultrafast");
        arguments << QString("-vf");
        arguments << QString("scale=trunc(iw/2)*2:trunc(ih/2)*2");
    }

    arguments << savePath;
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
/*
void RecordProcess::setIsZhaoXinPlatform(bool status)
{
    if (status) {
        m_isZhaoxin = true;
    } else {
        m_isZhaoxin = false;
    }
}
*/
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
    while (readproc(proc, &proc_info) != nullptr) {
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
        int byzanzChildPid = readSleepProcessPid();
        kill(byzanzChildPid, SIGKILL);

        qDebug() << "Kill byzanz-record's child process (sleep) pid: " << byzanzChildPid;
    } else {
        //process->terminate();
        process->write("q");
    }

    wait();
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
    int timeout = -1;
    unsigned int id = 0;

    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName())                 // appname
        << id                                    // id
        << QString("deepin-screen-recorder")                     // icon
        << tr("Recording finished")                              // summary
        << QString(tr("Saved to %1")).arg(newSavePath) // body
        << actions                                               // actions
        << hints                                                 // hints
        << timeout;                                              // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);

    QApplication::quit();
}
