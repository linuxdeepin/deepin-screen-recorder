// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "record_process.h"
#include "utils/configsettings.h"
#include "utils.h"
#include "utils/audioutils.h"
#include "gstrecord/gstinterface.h"
#include "utils/eventlogutils.h"
#ifdef KF5_WAYLAND_FLAGE_ON
#    include "waylandrecord/avlibinterface.h"
#endif
#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QtDBus>
#include <QScreen>
#include <QMetaType>
#include <QClipboard>
#include <QUrl>
#include <dlfcn.h>
#include <signal.h>

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

// 用于任务栏图标DBus通信调用的函数
const QString DBUS_FUNC_ON_START = "onStart";
const QString DBUS_FUNC_ON_STOP = "onStop";
const QString DBUS_FUNC_ON_RECORDING = "onRecording";
const QString DBUS_FUNC_ON_PAUSE = "onPause";

/**
   @brief 调用托盘图标"录制中"的对应函数，改变显示状态
 */
static QDBusMessage callTrayTimeIcon(const QString &function)
{
    QDBusInterface timeInterface("com.deepin.ScreenRecorder.time",
                             "/com/deepin/ScreenRecorder/time",
                             "com.deepin.ScreenRecorder.time",
                             QDBusConnection::sessionBus());
    return timeInterface.call(function);
}

/**
   @brief 调用托盘图标"截图"的对应函数，改变显示状态
 */
static QDBusMessage callTrayShotIcon(const QString &function)
{
    QDBusInterface shotInterface("com.deepin.ShotRecorder.PanelStatus",
                             "/com/deepin/ShotRecorder/PanelStatus",
                             "com.deepin.ShotRecorder.PanelStatus",
                             QDBusConnection::sessionBus());
    return shotInterface.call(function);
}

/**
   @brief 调用托盘图标"录屏"的对应函数，改变显示状态
 */
static QDBusMessage callTrayRecorderIcon(const QString &function)
{
    QDBusInterface recorderInterface("com.deepin.ShotRecorder.Recorder.PanelStatus",
                             "/com/deepin/ShotRecorder/Recorder/PanelStatus",
                             "com.deepin.ShotRecorder.Recorder.PanelStatus",
                             QDBusConnection::sessionBus());
    return recorderInterface.call(function);
}


RecordProcess::RecordProcess(QObject *parent)
    : QObject(parent)
{
    qDebug() << "录屏控制类初始化！";
    settings = ConfigSettings::instance();
    m_framerate = RECORD_FRAMERATE_24;

    saveTempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
    displayNumber = QString(std::getenv("DISPLAY"));

    if (settings->value("recordConfig", "lossless_recording").toString() == "") {
        settings->setValue("recordConfig", "lossless_recording", false);
    }
    qRegisterMetaType<QProcess::ProcessState>("ProcessState");
    m_recordingFlag = false;
    m_gstRecordX = nullptr;
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
    if (m_gstRecordX) {
        m_gstRecordX = nullptr;
        delete m_gstRecordX;
    }
}
//设置录屏的基础信息
void RecordProcess::setRecordInfo(const QRect &recordRect, const QString &filename)
{
    m_recordRect = recordRect;
    saveAreaName = filename;
}

//开始将mp4视频转码成gif
void RecordProcess::onStartTranscode()
{
    qInfo() << __LINE__ << __func__ << "正在转码视频(mp4 to gif)...";
    QProcess *transcodeProcess = new QProcess(this);
    connect(transcodeProcess, QOverload<QProcess::ProcessError>::of(&QProcess::error),
    [ = ](QProcess::ProcessError processError) {
        qDebug() << "processError: " << processError;
    });
    connect(transcodeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    [ = ](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "exitCode: " << exitCode << "  exitStatus: " << exitStatus;
        //转换进程是否正常退出
        if (exitStatus == QProcess::ExitStatus::NormalExit) {
            onTranscodeFinish();
        } else {
            qDebug() << "m_pTranscodeProcess is CrashExit:!";
        }
    });
    //connect(m_pTranscodeProcess, SIGNAL(finished(int)), this, SLOT(onTranscodeFinish()));
    //connect(m_pTranscodeProcess, SIGNAL(finished(int)), m_pTranscodeProcess, SLOT(deleteLater()));
    QString path = savePath;
    QStringList arg;
    arg << "-i";
    arg << savePath;
    arg << "-r";
    arg << "12";
    arg << path.replace("mp4", "gif");
    transcodeProcess->start("ffmpeg", arg);
    //部分hw arm架构的机型需要这样设置
#if defined(__aarch64__)
    if (Utils::isWaylandMode) {
        qDebug() << "watting transcode gif end!";
        transcodeProcess->waitForFinished();
    }
#endif
}

//转码完成后通知栏弹出提示
void RecordProcess::onTranscodeFinish()
{
    qInfo() << __LINE__ << __func__ << "已完成转码";
    QString path = savePath;
    QString gifOldPath = path.replace("mp4", "gif");
    QString gifNewPath = QDir(saveDir).filePath(saveBaseName).replace(QString("mp4"), QString("gif"));
    qDebug() << "" << savePath << gifOldPath << gifNewPath;
    QFile::rename(gifOldPath, gifNewPath);
    exitRecord(gifNewPath);
}

//录屏结束后弹出通知
void RecordProcess::onRecordFinish()
{
    //x11录屏结束
    if (!Utils::isWaylandMode) {
        if (QProcess::ProcessState::NotRunning != m_recorderProcess->exitCode()) {
            qDebug() << "Error";
            foreach (auto line, (m_recorderProcess->readAllStandardError().split('\n'))) {
                qDebug() << line;
            }
        } else {
            qDebug() << "OK" << m_recorderProcess->readAllStandardOutput() << m_recorderProcess->readAllStandardError();
        }
    }

    // Move file to save directory.
    QString newSavePath = QDir(saveDir).filePath(saveBaseName);
    QFile::remove(newSavePath);
    QFile::rename(savePath, newSavePath);
    exitRecord(newSavePath);
}

//x11录制视频
void RecordProcess::recordVideo()
{
    qDebug() << "x11 FFmpeg 录屏！";
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

#if defined(__mips__) || defined(__sw_64__) || defined(__loongarch_64__) || defined(__loongarch__)
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
    arguments << QString("x11grab");   // 视频源
    arguments << QString("-framerate");   // 视频帧数
    arguments << QString("%1").arg(m_framerate);
    arguments << QString("-video_size");   // 视频分辨率
    arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());   // 录制区域宽高
    if (!m_isRecordMouse) {   // 不录制光标
        arguments << QString("-draw_mouse");
        arguments << QString("0");
    }
    arguments << QString("-thread_queue_size");   // 输入线程缓冲区大小
    arguments << QString("128");
    arguments << QString("-i");
    arguments << QString("%1+%2,%3").arg(displayNumber).arg(m_recordRect.x()).arg(m_recordRect.y());   // 录制区域左上角坐标
    if (recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
        arguments << QString("-thread_queue_size");
        arguments << QString("2048");
        arguments << QString("-fragment_size");
        arguments << QString("2048");
        arguments << QString("-f");
        arguments << QString("pulse");   // 音频源
        arguments << QString("-ac");   // 输出通道数
        arguments << QString("2");
        arguments << QString("-ar");   // 音频采样率
        arguments << QString("44100");
        arguments << QString("-i");   // 系统音频id
        arguments << QString("%1").arg(t_currentAudioChannel);
    }
    if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
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
        arguments << QString("-i");   // 麦克风音频id，值为固定"default"
        arguments << QString("default");
        arguments << QString("-ac");
        arguments << QString("2");
    }
    if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
        arguments << QString("-filter_complex");
        arguments << QString("amerge");
    }

    arguments << QString("-pix_fmt");   // 像素格式
    arguments << QString("yuv420p");
    arguments << QString("-c:v");   // 视频编码器
    arguments << QString("libx264");
    //arguments << QString("mpeg2video");
    //arguments << QString("mpeg1video");
    //arguments << QString("libx264rgb");
    //arguments << QString("h263p");
    //arguments << QString("mjpeg");
    //arguments << QString("flv");
    //arguments << QString("-c:a"); // 音频编码器
    //arguments << QString("libmp3lame");
    arguments << QString("-crf");   // 视频质量，值越小，画质越好。
    arguments << QString("23");
    arguments << QString("-preset");
    arguments << QString("ultrafast");
    arguments << QString("-vsync");
    arguments << QString("passthrough");
    if (settings->value("recordConfig", "lossless_recording").toBool()) {
        arguments << QString("-f");
        arguments << QString("matroska");   // mkv视频
    } else {
        arguments << QString("-f");
        arguments << QString("mp4");   // mp4视频
    }
    arguments << QString("-vf");
    arguments << QString("scale=trunc(iw/2)*2:trunc(ih/2)*2");
#else

    if (recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
        //lastAudioSink = audioUtils->currentAudioSink();
        arguments << QString("-thread_queue_size");
//        if ((arch.startsWith("ARM", Qt::CaseInsensitive))) {
            arguments << QString("2048");
//        } else {
//            arguments << QString("32");
//        }
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

    }
    if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC || recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
        arguments << QString("-thread_queue_size");
//        if ((arch.startsWith("ARM", Qt::CaseInsensitive))) {
            arguments << QString("2048");
//        } else {
//            arguments << QString("32");
//        }
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
        arguments << QString("-ac");
        arguments << QString("2");
    }


    arguments << QString("-video_size");
    arguments << QString("%1x%2").arg(m_recordRect.width()).arg(m_recordRect.height());
    if (!m_isRecordMouse) {
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
    } else {
        arguments << QString("-crf");
        arguments << QString("23");
    }
    arguments << QString("-preset");
    arguments << QString("ultrafast");
    arguments << QString("-vsync");
    arguments << QString("passthrough");
    arguments << QString("-vf");
    arguments << QString("scale=trunc(iw/2)*2:trunc(ih/2)*2");

    if (recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO) {
        if ((arch.startsWith("ARM", Qt::CaseInsensitive))) {
            arguments << QString("-af");
            arguments << QString("volume=20dB");
        }
    }
    if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
        arguments << QString("-filter_complex");
        if ((arch.startsWith("ARM", Qt::CaseInsensitive))) {
            arguments << QString("[0:a]volume=30dB[a1];[a1][1:a]amix=inputs=2:duration=first:dropout_transition=0[out]");
            arguments << QString("-map");
            arguments << QString("2:v");
            arguments << QString("-map");
            arguments << QString("[out]");
        } else {
            arguments << QString("amerge");
        }
    }
#endif

    arguments << savePath;
    qDebug() << " ffmpeg "  << arguments;
    m_recorderProcess->start("ffmpeg", arguments);
}

//初始化x11 FFmpeg录屏进程
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
    if(saveAreaName.isEmpty()){
        saveBaseName = QString("%1_%2.%3").arg(tr("Record")).arg(date.toString("yyyyMMddhhmmss")).arg(fileExtension);
    }else{
        if(m_isFullScreenRecord){
            saveBaseName = QString("%1.%2").arg(saveAreaName).arg(fileExtension);
        }else {
            saveBaseName = QString("%1_%2_%3.%4").arg(tr("Record")).arg(saveAreaName).arg(date.toString("yyyyMMddhhmmss")).arg(fileExtension);
        }
    }
    savePath = QDir(saveTempDir).filePath(saveBaseName);
    // Remove same cache file first.
    QFile::remove(savePath);
}
void RecordProcess::getScreenRecordSavePath()
{
    if (settings->value("recordConfig", "savepath").toString() == "") {
        qWarning() << "配置文件中录屏保存路径为空,采用默认路径";
        saveDir = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first() + QDir::separator() + "Screen Recordings" + QDir::separator();
    } else {
        int t_saveIndex = ConfigSettings::instance()->value("recordConfig", "save_op_record").toInt();
        switch (t_saveIndex) {
        case 0: {
            //保存到桌面
            saveDir = settings->value("recordConfig", "savepath").toString() + QDir::separator();
            break;
        }
        default:
            //保存到视频目录中的Screen Recordings目录
            saveDir = settings->value("recordConfig", "savepath").toString() + QDir::separator() + "Screen Recordings" + QDir::separator();
            break;
        }
    }
    if ((!QDir(saveDir).exists() && QDir().mkdir(saveDir) == false) ||   // 文件不存在，且创建失败
            (QDir(saveDir).exists() && !QFileInfo(saveDir).isWritable())) {   // 文件存在，且不能写
        saveDir = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first();
    }
    qInfo() << "录屏保存目录: " << saveDir;
}

void RecordProcess::save2Clipboard(QString file)
{
    //录屏文件保存到剪切板
    QClipboard *cb = qApp->clipboard();
    // Ownership of the new data is transferred to the clipboard.
    QMimeData *newMimeData = new QMimeData();

    QByteArray gnomeFormat = QByteArray("copy\n");
    QString text;
    QList<QUrl> dataUrls;

    dataUrls << QUrl::fromLocalFile(file);
    gnomeFormat.append(QUrl::fromLocalFile(file).toEncoded()).append("\n");

    newMimeData->setText(text.endsWith('\n') ? text.left(text.length() - 1) : text);
    newMimeData->setUrls(dataUrls);

    gnomeFormat.remove(gnomeFormat.length() - 1, 1);

    newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

    cb->setMimeData(newMimeData, QClipboard::Clipboard);
}
//wayland录制视频
void RecordProcess::waylandRecord()
{
#ifdef KF5_WAYLAND_FLAGE_ON
    avlibInterface::initFunctions();
    qDebug() << "wayland 录屏！";
    // 启动wayland录屏
    initProcess();
    AudioUtils audioUtils;
    QStringList arguments;
    arguments << QString("%1").arg(recordType);
    arguments << QString("%1").arg(m_recordRect.width()) << QString("%1").arg(m_recordRect.height());
    arguments << QString("%1").arg(m_recordRect.x()) << QString("%1").arg(m_recordRect.y());
    arguments << QString("%1").arg(m_framerate);
    arguments << QString("%1").arg(savePath);
    arguments << QString("%1").arg(recordAudioInputType);
    arguments << QString(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Source));
    arguments << QString(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Sink));
    qDebug() << arguments;
    WaylandIntegration::init(arguments);
#endif
    return;
}
//gstreamer录制视频
void RecordProcess::GstStartRecord()
{
    int argc = 1;
    //    char *mock[1] = {QString("empty").toLatin1().data()};
    //    char **argv[1];
    //    *argv = mock;
    //gstreamer接口初始化
    gstInterface::initFunctions();
    gstInterface::m_gst_init(&argc, nullptr);
    qDebug() << "Gstreamer 录屏开始！";
    GstRecordX::VideoType videoType = GstRecordX::VideoType::webm;
    GstRecordX::AudioType audioType = GstRecordX::AudioType::None;
    m_gstRecordX = new GstRecordX(this);
    //设置参数
    m_gstRecordX->setFramerate(m_framerate);
    m_gstRecordX->setRecordArea(m_recordRect);
    //这里设置音频设备名称（输入和输出），即使名称为空也不影响。
    AudioUtils audioUtils;
    m_gstRecordX->setInputDeviceName(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Source));
    m_gstRecordX->setOutputDeviceName(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Sink));
    //这里才会设置究竟采集哪些音频设备的音频数据
    if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO) {
        audioType = GstRecordX::AudioType::Mix;
    } else if (recordAudioInputType == RECORD_AUDIO_INPUT_MIC) {
        audioType = GstRecordX::AudioType::Mic;
    } else if (recordAudioInputType == RECORD_AUDIO_INPUT_SYSTEMAUDIO) {
        audioType = GstRecordX::AudioType::Sys;
    }
    m_gstRecordX->setAudioType(audioType);
    QDateTime date = QDateTime::currentDateTime();
    QString fileExtension = "webm";
    if (recordType == RECORD_TYPE_VIDEO) {
        if (settings->value("recordConfig", "lossless_recording").toBool()) {
            videoType = GstRecordX::VideoType::ogg;
            fileExtension = "ogg";
        } else {
            videoType = GstRecordX::VideoType::webm;
            fileExtension = "webm";
        }
    }
    if(saveAreaName.isEmpty()){
        saveBaseName = QString("%1_%2.%3").arg(tr("Record")).arg(date.toString("yyyyMMddhhmmss")).arg(fileExtension);
    }else{
        if(m_isFullScreenRecord){
            saveBaseName = QString("%1.%2").arg(saveAreaName).arg(fileExtension);
        }else {
            saveBaseName = QString("%1_%2_%3.%4").arg(tr("Record")).arg(saveAreaName).arg(date.toString("yyyyMMddhhmmss")).arg(fileExtension);
        }
    }

    savePath = QDir(saveTempDir).filePath(saveBaseName);
    // Remove same cache file first.
    QFile::remove(savePath);
    m_gstRecordX->setVidoeType(videoType);
    m_gstRecordX->setSavePath(savePath);
    m_gstRecordX->setX11RecordMouse(m_isRecordMouse);
    //开始录制
    if (Utils::isWaylandMode) {
#ifdef KF5_WAYLAND_FLAGE_ON
        //wayland下停止录屏需要通过信号槽控制，避免Gstreamer管道数据未写完程序就被退出了
        connect(m_gstRecordX, &GstRecordX::waylandGstRecrodFinish, this, &RecordProcess::onExitGstRecord);
        //wayland模式需打开视频画面采集
        QStringList arguments;
        arguments << QString("%1").arg(videoType);
        arguments << QString("%1").arg(m_recordRect.width()) << QString("%1").arg(m_recordRect.height());
        arguments << QString("%1").arg(m_recordRect.x()) << QString("%1").arg(m_recordRect.y());
        arguments << QString("%1").arg(m_framerate);
        arguments << QString("%1").arg(savePath);
        arguments << QString("%1").arg(recordAudioInputType);
        qDebug() << arguments;
        WaylandIntegration::init(arguments, m_gstRecordX);
#endif
    } else {
        m_gstRecordX->x11GstStartRecord();
    }
}

//gstreamer停止录制视频
void RecordProcess::GstStopRecord()
{
    if (Utils::isWaylandMode) {
#ifdef KF5_WAYLAND_FLAGE_ON
        WaylandIntegration::stopStreaming();
#endif
    } else {
        //x11 gstreamer录屏
        m_gstRecordX->x11GstStopRecord();
        onExitGstRecord();
    }
}

//退出gstreamer wayland录屏，需要Gstreamer录屏类触发
void RecordProcess::onExitGstRecord()
{
    QString newSavePath = QDir(saveDir).filePath(saveBaseName);
    QFile::remove(newSavePath);
    QFile::rename(savePath, newSavePath);
    //注销gstreamer相关库加载
    gstInterface::unloadFunctions();
    qDebug() << "Gstreamer 录屏结束！";
    exitRecord(newSavePath);
}

void RecordProcess::onRecordMouse(const bool status)
{
    m_isRecordMouse = status;
}

void RecordProcess::setMicrophone(const bool status)
{
    m_selectedMic = status;
}

void RecordProcess::setSystemAudio(const bool status)
{
    m_selectedSystemAudio = status;
}

//开始录屏
void RecordProcess::startRecord()
{
    getScreenRecordSavePath();
    //使用QtConcurrent::run受cpu核心线程数的影响，线程池默认大小为CPU核心线程数大小，由于程序中通过此方法启动的线程数超出4个，故再次设置线程池大小
    QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount() > 6 ? QThreadPool::globalInstance()->maxThreadCount() : 10);
    m_framerate = settings->value("recordConfig", "mkv_framerate").toString().toInt();
    qDebug() << "m_selectedMic: " << m_selectedMic;
    qDebug() << "m_selectedSystemAudio: " << m_selectedSystemAudio;
    if (settings->value("recordConfig", "save_as_gif").toBool()) {
        recordType = RECORD_TYPE_GIF;
        recordAudioInputType = RECORD_TYPE_GIF;
    } else {
        recordType = RECORD_TYPE_VIDEO;
        if (m_selectedMic && m_selectedSystemAudio) {
            recordAudioInputType = RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO;
        } else if (m_selectedMic) {
            recordAudioInputType = RECORD_AUDIO_INPUT_MIC;
        } else if (m_selectedSystemAudio) {
            recordAudioInputType = RECORD_AUDIO_INPUT_SYSTEMAUDIO;
        }
    }
    if (!Utils::isFFmpegEnv) {
        GstStartRecord();
    } else {
        //x11下的录屏
        if (!Utils::isWaylandMode) {
            recordVideo();
            if (Utils::isTabletEnvironment) {
                return;
            }
        } else {
            //wayland下的录屏
            if (recordType != RECORD_TYPE_GIF) {
                if (settings->value("recordConfig", "lossless_recording").toBool()) {
                    recordType = RECORD_TYPE_MKV;
                } else {
                    recordType = RECORD_TYPE_MP4;
                }
            }
            waylandRecord();
        }
    }

    QJsonObject obj {
        { "tid", EventLogUtils::StartRecording },
        { "version", QCoreApplication::applicationVersion() },
        { "type", recordType == RECORD_TYPE_GIF ? "gif" : (settings->value("recordConfig", "lossless_recording").toBool() || recordType == RECORD_TYPE_MKV ? "mkv" : "mp4") }
    };
    EventLogUtils::get().writeLogs(obj);

    if (Utils::isSysHighVersion1040() == false) {
        return;
    }
    //1040及以上的版本可通过此方式启动状态栏图标闪烁
    qDebug() << "通知录屏插件开始录屏! currentTime: " << QTime::currentTime();
    QDBusMessage message = callTrayTimeIcon(DBUS_FUNC_ON_START);
    callTrayShotIcon(DBUS_FUNC_ON_START);
    callTrayRecorderIcon(DBUS_FUNC_ON_START);

    qDebug() << "已通知录屏插件开始录屏! currentTime: " << QTime::currentTime();
    m_recordingFlag = true;
    QtConcurrent::run(this, &RecordProcess::emitRecording);
    if (QDBusMessage::ReplyMessage == message.type()) {
        if (!message.arguments().takeFirst().toBool())
            qDebug() << "dde dock screen-recorder-plugin did not receive start message!";
    }
}
void RecordProcess::emitRecording()
{
    while (m_recordingFlag) {
        //qDebug() << "录屏正在进行中! currentTime: " << QTime::currentTime();
        callTrayTimeIcon(DBUS_FUNC_ON_RECORDING);
        callTrayShotIcon(DBUS_FUNC_ON_RECORDING);
        callTrayRecorderIcon(DBUS_FUNC_ON_RECORDING);
        QThread::msleep(1000);
    }
}

void RecordProcess::setFullScreenRecord(bool flag)
{
    m_isFullScreenRecord = flag;
}

void RecordProcess::stopRecord()
{
    //    QJsonObject obj{
    //        {"tid", EventLogUtils::EndRecording},
    //        {"version", QCoreApplication::applicationVersion()},
    //        {"type", recordType == RECORD_TYPE_GIF ? "gif" : (recordType == RECORD_TYPE_MKV ? "mkv" : "mp4")}
    //    };
    //    EventLogUtils::get().writeLogs(obj);

    if (Utils::isSysHighVersion1040() == true) {
        qInfo() << __FUNCTION__ << __LINE__ << "正在暂停录屏计时...";
        qDebug() << "Pause the screen recording timer!";

        //系统托盘图标停止闪烁，时间暂停，但还没有结束
        QDBusMessage message = callTrayTimeIcon(DBUS_FUNC_ON_PAUSE);
        if (QDBusMessage::ReplyMessage == message.type()) {
            if (!message.arguments().takeFirst().toBool())
                qDebug() << "dde dock screen-recorder-plugin did not receive stop message!";
        }
        callTrayShotIcon(DBUS_FUNC_ON_PAUSE);
        callTrayRecorderIcon(DBUS_FUNC_ON_PAUSE);

        qInfo() << __FUNCTION__ << __LINE__ << "录屏计时已暂停";
    }
    if (Utils::isFFmpegEnv) {
        //停止wayland录屏
        if (Utils::isWaylandMode) {
#ifdef KF5_WAYLAND_FLAGE_ON
            WaylandIntegration::stopStreaming();
            if (RECORD_TYPE_GIF == recordType) {
                onStartTranscode();
            } else {
                onRecordFinish();
            }
#endif
        }
        //停止x11录屏
        else {
            //录制的视频类型是否是gif格式，是gif的话需要进行转码
            if (RECORD_TYPE_GIF == recordType) {
                connect(m_recorderProcess, SIGNAL(finished(int)), this, SLOT(onStartTranscode()));
            } else {
                connect(m_recorderProcess, SIGNAL(finished(int)), this, SLOT(onRecordFinish()));
            }
            m_recorderProcess->write("q");
        }
    } else {
        GstStopRecord();
    }
}
//退出录屏（先停止，再弹提示，最后退出）
void RecordProcess::exitRecord(QString newSavePath)
{
    if (!Utils::isRootUser && !m_isFullScreenRecord) {
        qInfo() << __LINE__ << __func__ << "正在弹出保存完成的通知...";
        // Popup notify.
        QDBusInterface notification("org.freedesktop.Notifications",
                                    "/org/freedesktop/Notifications",
                                    "org.freedesktop.Notifications",
                                    QDBusConnection::sessionBus());

        QStringList actions;
        actions << "_open" << tr("View");
        actions << "_open1" << tr("Open Folder");
        QVariantMap hints;
        hints["x-deepin-action-_open"] = QString("xdg-open,%1").arg(newSavePath);
        QString savepathcommand = QString("dde-file-manager,--show-item,%1").arg(newSavePath);
        hints["x-deepin-action-_open1"] = savepathcommand;
        int timeout = -1;
        unsigned int id = 0;

        QList<QVariant> arg;
        arg << (QCoreApplication::applicationName())   // appname
            << id   // id
            << QString("deepin-screen-recorder")   // icon
            << tr("Recording finished")   // summary
            << QString(tr("Saved to %1")).arg(newSavePath)   // body
            << actions   // actions
            << hints   // hints
            << timeout;   // timeout
        notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
        qInfo() << __LINE__ << __func__ << "已弹出通知消息";
    }
    if (recordType == RECORD_TYPE_GIF) {
        QFile::remove(savePath);
    }
    qInfo() << __LINE__ << __func__ << "正在保存到剪切板...";
    //保存到剪切板
    save2Clipboard(newSavePath);
    qInfo() << __LINE__ << __func__ << "已保存到剪切板";
    if (Utils::isWaylandMode) {
#ifdef KF5_WAYLAND_FLAGE_ON
        avlibInterface::unloadFunctions();
#endif
    }

    m_recordingFlag = false;
    if (Utils::isSysHighVersion1040() == true) {
        qInfo() << __LINE__ << __func__ << "正在退出录屏计时图标...";
        qDebug() << __LINE__ << ": Stop the screen recording timer!";
        //系统托盘图标结束并退出
        callTrayTimeIcon(DBUS_FUNC_ON_STOP);
        callTrayShotIcon(DBUS_FUNC_ON_STOP);
        callTrayRecorderIcon(DBUS_FUNC_ON_STOP);
        qInfo() << __LINE__ << __func__ << "录屏计时图标已退出";
    }
    qInfo() << __LINE__ << __func__ << "录屏结束!!!";
    QApplication::quit();
    if (Utils::isWaylandMode) {
        qInfo() << "wayland record exit! (_Exit(0))";
        _Exit(0);
    }
}
