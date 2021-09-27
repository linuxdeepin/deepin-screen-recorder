/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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
#include "utils/configsettings.h"
#include "utils.h"
#include "utils/audioutils.h"

#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QtDBus>
#include <QScreen>

#include <dlfcn.h>
#include <signal.h>

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

RecordProcess::RecordProcess(QObject *parent) : QObject(parent)
{
    settings = ConfigSettings::instance();
    m_framerate = RECORD_FRAMERATE_24;

    saveTempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
    saveDir = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first() + QDir::separator() + "Screen Recordings" + QDir::separator();
    displayNumber = QString(std::getenv("DISPLAY"));

    if((!QDir(saveDir).exists() && QDir().mkdir(saveDir) == false) ||   // 文件不存在，且创建失败
            (QDir(saveDir).exists() && !QFileInfo(saveDir).isWritable())) {  // 文件存在，且不能写
        saveDir = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first();
    }
    qDebug() << saveDir;
    if (settings->value("recordConfig", "lossless_recording").toString() == "") {
        settings->setValue("recordConfig", "lossless_recording", false);
    }
}

RecordProcess::~RecordProcess()
{
    /*
    if (m_pTranscodeProcess) {
        delete m_pTranscodeProcess;
        m_pTranscodeProcess = nullptr;
    }

    if (m_recorderProcess) {
        delete m_recorderProcess;
        m_recorderProcess = nullptr;
    }
    */
}

void RecordProcess::setRecordInfo(const QRect &recordRect, const QString &filename)
{
    m_recordRect = recordRect;
    saveAreaName = filename;
}
/*
void RecordProcess::setRecordType(int type)
{
    recordType = type;
}

void RecordProcess::setFrameRate(int framerate)
{
    m_framerate = framerate;
}
*/
void RecordProcess::setRecordAudioInputType(int inputType)
{
    recordAudioInputType = inputType;
}

void RecordProcess::onStartTranscode()
{
    m_pTranscodeProcess = new QProcess(this);
    connect(m_pTranscodeProcess, SIGNAL(finished(int)), this, SLOT(onTranscodeFinish()));
    connect(m_pTranscodeProcess, SIGNAL(finished(int)), m_pTranscodeProcess, SLOT(deleteLater()));
    QString path = savePath;
    QStringList arg;
    arg << "-i";
    arg << savePath;
    arg << "-r";
    arg << "24";
    arg << path.replace("mp4","gif");
    m_pTranscodeProcess->start("ffmpeg",arg);
}

void RecordProcess::onTranscodeFinish()
{
    QString path = savePath;
    QString gifOldPath = path.replace("mp4","gif");
    QString gifNewPath = QDir(saveDir).filePath(saveBaseName).replace(QString("mp4"),QString("gif"));
    qDebug() << "" << savePath << gifOldPath << gifNewPath;
    QFile::rename(gifOldPath, gifNewPath);
    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());
    QStringList actions;
    actions << "_open" << tr("View");
    QVariantMap hints;
    hints["x-deepin-action-_open"] = QString("xdg-open,%1").arg(gifNewPath);
    int timeout = -1;
    unsigned int id = 0;
    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName())                 // appname
        << id                                                    // id
        << QString("deepin-screen-recorder")                     // icon
        << tr("Recording finished")                              // summary
        << QString(tr("Saved to %1")).arg(gifNewPath)            // body
        << actions                                               // actions
        << hints                                                 // hints
        << timeout;                                              // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
    QFile::remove(savePath);
    QApplication::quit();
}

void RecordProcess::onRecordFinish()
{
    if (QProcess::ProcessState::NotRunning != m_recorderProcess->exitCode()) {
        qDebug() << "Error";
        foreach (auto line, (m_recorderProcess->readAllStandardError().split('\n'))) {
            qDebug() << line;
        }
    } else {
        qDebug() << "OK" << m_recorderProcess->readAllStandardOutput() << m_recorderProcess->readAllStandardError();
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
    int timeout = -1;
    unsigned int id = 0;

    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName())                 // appname
        << id                                                    // id
        << QString("deepin-screen-recorder")                     // icon
        << tr("Recording finished")                              // summary
        << QString(tr("Saved to %1")).arg(newSavePath)           // body
        << actions                                               // actions
        << hints                                                 // hints
        << timeout;                                              // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);

    QApplication::quit();
}

void RecordProcess::recordVideo()
{
    initProcess();
    //取系统音频的通道号
    AudioUtils audioUtils;
    QString t_currentAudioChannel = audioUtils.currentAudioChannel();

    t_currentAudioChannel = t_currentAudioChannel.left(t_currentAudioChannel.size() - 1);
    qDebug() << "current audio channel:" << t_currentAudioChannel;

    // FFmpeg need pass arugment split two part: -option value,
    // otherwise, it will report 'Unrecognized option' error.
    QStringList arguments;

    QString arch = QSysInfo::currentCpuArchitecture();

#if defined (__mips__) || defined (__sw_64__)
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
    arguments << QString("x11grab"); // 视频源
    arguments << QString("-framerate"); // 视频帧数
    arguments << QString("%1").arg(m_framerate);
    arguments << QString("-video_size"); // 视频分辨率
    arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height()); // 录制区域宽高
    if(!m_isRecordMouse) { // 不录制光标
        arguments << QString("-draw_mouse");
        arguments << QString("0");
    }
    arguments << QString("-thread_queue_size"); // 输入线程缓冲区大小
    arguments << QString("128");
    arguments << QString("-i");
    arguments << QString("%1+%2,%3").arg(displayNumber).arg(m_recordRect.x()).arg(m_recordRect.y()); // 录制区域左上角坐标
    if(recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO){
        arguments << QString("-thread_queue_size");
        arguments << QString("2048");
        arguments << QString("-fragment_size");
        arguments << QString("2048");
        arguments << QString("-f");
        arguments << QString("pulse");// 音频源
        arguments << QString("-ac"); // 输出通道数
        arguments << QString("2");
        arguments << QString("-ar"); // 音频采样率
        arguments << QString("44100");
        arguments << QString("-i"); // 系统音频id
        arguments << QString("%1").arg(t_currentAudioChannel);
    }
    if(recordAudioInputType == RECORD_AUDIO_INPUT_MIC || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO){
        arguments << QString("-thread_queue_size");
        arguments << QString("2048");
        arguments << QString("-fragment_size");
        arguments << QString("2048");
        arguments << QString("-f");
        arguments << QString("pulse");
        arguments << QString("-ac");
        arguments << QString("2");
        arguments << QString("-ar");
        arguments << QString("44100");
        arguments << QString("-i"); // 麦克风音频id，值为固定"default"
        arguments << QString("default");
    }
    if(recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO){
        arguments << QString("-filter_complex");
        arguments << QString("amerge");
    }

    arguments << QString("-pix_fmt"); // 像素格式
    arguments << QString("yuv420p");
    arguments << QString("-c:v"); // 视频编码器
    arguments << QString("libx264");
    //arguments << QString("mpeg2video");
    //arguments << QString("mpeg1video");
    //arguments << QString("libx264rgb");
    //arguments << QString("h263p");
    //arguments << QString("mjpeg");
    //arguments << QString("flv");
    //arguments << QString("-c:a"); // 音频编码器
    //arguments << QString("libmp3lame");
    arguments << QString("-crf"); // 视频质量，值越小，画质越好。
    arguments << QString("23");
    arguments << QString("-preset");
    arguments << QString("ultrafast");
    arguments << QString("-vsync");
    arguments << QString("passthrough");
    if(settings->value("recordConfig", "lossless_recording").toBool()){
        arguments << QString("-f");
        arguments << QString("matroska"); // mkv视频
    }else {
        arguments << QString("-f");
        arguments << QString("mp4"); // mp4视频
    }
    arguments << QString("-vf");
    arguments << QString("scale=trunc(iw/2)*2:trunc(ih/2)*2");
#else
    arguments << QString("-video_size");
    arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());
    if(!m_isRecordMouse) {
        arguments << QString("-draw_mouse");
        arguments << QString("0");
    }
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
    // baseline 算法，录制的视频在windos自带播放器不能播放
    //arguments << QString("-profile:v");
    //arguments << QString("baseline");
    if (settings->value("recordConfig", "lossless_recording").toBool()) {
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
#endif

    arguments << savePath;
    m_recorderProcess->start("ffmpeg", arguments);
}

void RecordProcess::initProcess()
{
    // Create process and handle finish signal.
    m_recorderProcess = new QProcess(this);

    // Disable scaling of byzanz-record (GTK3 based) here, because we pass subprocesses
    // absolute device geometry information, byzanz-record should not scale the information
    // one more time.
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GDK_SCALE", "1");
    m_recorderProcess->setProcessEnvironment(env);

    connect(m_recorderProcess, SIGNAL(finished(int)), m_recorderProcess, SLOT(deleteLater()));

    // Build temp save path.
    QDateTime date = QDateTime::currentDateTime();
    QString fileExtension;
    if (recordType == RECORD_TYPE_GIF) {
        if (settings->value("recordConfig", "lossless_recording").toBool()) {
            fileExtension = "flv";
        } else {
            //先录制mp4再转gif
            fileExtension = "mp4";
        }
    } else {
        if (settings->value("recordConfig", "lossless_recording").toBool()) {
            fileExtension = "mkv";
        } else {
            fileExtension = "mp4";
        }
    }
    saveBaseName = QString("%1_%2_%3.%4").arg(tr("Record")).arg(saveAreaName).arg(date.toString("yyyyMMddhhmmss")).arg(fileExtension);
    savePath = QDir(saveTempDir).filePath(saveBaseName);
    // Remove same cache file first.
    QFile file(savePath);
    file.remove();
}

void RecordProcess::setRecordMouse(const bool status)
{
    m_isRecordMouse = status;
}

void RecordProcess::startRecord()
{
    m_framerate = settings->value("recordConfig", "mkv_framerate").toString().toInt();
    if (settings->value("recordConfig", "save_as_gif").toBool()) {
        recordType = RECORD_TYPE_GIF;
        setRecordAudioInputType(RECORD_TYPE_GIF);
    } else {
        recordType = RECORD_TYPE_VIDEO;
    }
    recordVideo();
    if(Utils::isTabletEnvironment) {
        return;
    }
    if (Utils::isSysHighVersion1040() == false) {
        return;
    }
    QDBusMessage message = QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall("com.deepin.ScreenRecorder.time",
                                                                                             "/com/deepin/ScreenRecorder/time",
                                                                                             "com.deepin.ScreenRecorder.time",
                                                                                             "onStart"));
    if (QDBusMessage::ReplyMessage == message.type()){
        if(!message.arguments().takeFirst().toBool())
            qDebug() << "dde dock screen-recorder-plugin did not receive start message!";
    }
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
/*
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
//
//    char *error;
//    void *handle = dlopen("libprocps.so", RTLD_LAZY);
//    if(!handle){
//        qDebug() << "open libprocps.so failure";
//        return 0;
//    }

//    PROCTAB*(*openproc_handle)(int, ...);
//    openproc_handle = (PROCTAB*(*)(int, ...))dlsym(handle, "openproc");
//    if((error = dlerror()) != nullptr){
//        qDebug() << "get libprocps.so function openproc failure";
//        return 0;
//    }

//    PROCTAB *proc = openproc_handle(
//                PROC_FILLMEM |          // memory status: read information from /proc/#pid/statm
//                PROC_FILLSTAT |         // cpu status: read information from /proc/#pid/stat
//                PROC_FILLUSR            // user status: resolve user ids to names via /etc/passwd
//                );
//    static proc_t proc_info;
//    memset(&proc_info, 0, sizeof(proc_t));

//    proc_t* (*readproc_handle)(PROCTAB *__restrict const, proc_t *__restrict );
//    readproc_handle = (proc_t* (*)(PROCTAB *__restrict const, proc_t *__restrict ))dlsym(handle, "readproc");
//    if((error = dlerror()) != nullptr){
//        qDebug() << "get libprocps.so function readproc failure";
//        return 0;
//    }
//    StoredProcType processes;
//    while (readproc_handle(proc, &proc_info) != nullptr) {
//        processes[proc_info.tid] = proc_info;
//    }

//    void(*closeproc_handle)(PROCTAB*);
//    closeproc_handle = (void(*)(PROCTAB*))dlsym(handle, "closeproc");
//    if((error = dlerror()) != nullptr){
//        qDebug() << "get libprocps.so function closeproc failure";
//        return 0;
//    }
//    closeproc_handle(proc);

//    ProcessTree *processTree = new ProcessTree();
//    processTree->scanProcesses(processes);

//    dlclose(handle);
//
    return processTree->getAllChildPids(byzanzProcessId)[0];
}
*/

void RecordProcess::stopRecord()
{
    if (RECORD_TYPE_GIF == recordType) {
        connect(m_recorderProcess, SIGNAL(finished(int)), this, SLOT(onStartTranscode()));
    } else {
        connect(m_recorderProcess, SIGNAL(finished(int)), this, SLOT(onRecordFinish()));
    }
    m_recorderProcess->write("q");
    QDBusMessage message = QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall("com.deepin.ScreenRecorder.time",
                                                                                             "/com/deepin/ScreenRecorder/time",
                                                                                             "com.deepin.ScreenRecorder.time",
                                                                                             "onStop"));
    if (QDBusMessage::ReplyMessage == message.type()) {
        if(!message.arguments().takeFirst().toBool())
            qDebug() << "dde dock screen-recorder-plugin did not receive stop message!";
    }
}
