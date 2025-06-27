// Copyright (C) 2020 ~ now Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gstrecordx.h"
#include "utils.h"
#include "../utils/log.h"


/**
 * @brief gstBusMessageCb
 * @param bus:总线
 * @param msg:消息
 * @param userdata:用户数据
 * @return
 */
static gboolean gstBusMessageCb(GstBus *bus, GstMessage *message, GstRecordX *gstRecord)
{
    qCDebug(dsrApp) << "gstBusMessageCb called.";
    Q_UNUSED(bus);
    if (!message) {
        qCWarning(dsrApp) << "Received null message from GstBus";
        qCDebug(dsrApp) << "Null message received.";
        return true;
    }
    switch (message->type) {
    case GST_MESSAGE_ERROR: {
        qCDebug(dsrApp) << "Received error message from GstBus";
        GError *error = nullptr;
        gchar *dbg = nullptr;
        gstInterface::m_gst_message_parse_error(message, &error, &dbg);
        if (dbg) {
            gstInterface::m_g_free(dbg);
            qCDebug(dsrApp) << "Debug message freed.";
        }
        if (error) {
            QString errMsg = error->message;
            qCritical() << "Gstreamer pipeline error: " << errMsg;
            gstInterface::m_g_error_copy(error);
            qCDebug(dsrApp) << "GStreamer pipeline error copied.";
        }
        break;
    }
    case GST_MESSAGE_EOS: {
        qCInfo(dsrApp) << "Received end of stream message";
        gstInterface::m_g_main_loop_quit(gstRecord->getGloop());
        qCDebug(dsrApp) << "End of stream, main loop quit.";
        break;
    }
    default:
        qCDebug(dsrApp) << "Received unknown GstBus message type:" << message->type;
        break;
    }
    qCDebug(dsrApp) << "gstBusMessageCb finished.";
    return true;
}

GstRecordX::GstRecordX(QObject *parent) : QObject(parent)
{
    qCDebug(dsrApp) << "GstRecordX constructor called.";
    initMemberVariables();
    qCDebug(dsrApp) << "GstRecordX constructor finished.";
}

//初始化成员变量
void GstRecordX::initMemberVariables()
{
    qCDebug(dsrApp) << "initMemberVariables called.";
    m_pipeline = nullptr;
    m_audioType = AudioType::None;
    m_videoType = VideoType::webm;
    m_sysDevcieName = "";
    m_micDeviceName = "";
    m_savePath = "";
    m_framerate = 24;
    m_recordArea = QRect(0, 0, 0, 0);
    m_channels = 2;
    m_rate = 44100;
    m_boardVendorType = 0;
    qCDebug(dsrApp) << "Member variables initialized.";
}

//x11协议下gstreamer录制视频
void GstRecordX::x11GstStartRecord()
{
    qCInfo(dsrApp) << "Starting X11 GStreamer recording";
    QStringList arguments;
    QStringList areaList;

    //设置录制区域的大小及位置 show-pointer:是否录制光标
    areaList << "ximagesrc"
             << "display-name=" + qgetenv("DISPLAY")
             << "use-damage=false"
             << "show-pointer=" + m_isRecordMouse //是否录制光标
             << "startx=" + QString::number(m_recordArea.x())
             << "starty=" + QString::number(m_recordArea.y())
             << "endx=" + QString::number(m_recordArea.x() + m_recordArea.width() - 1)
             << "endy=" + QString::number(m_recordArea.y() + m_recordArea.height() - 1);
    arguments << areaList.join(" ");
    qCDebug(dsrApp) << "Ximagesrc arguments set.";

    qCDebug(dsrApp) << "Recording area:" << m_recordArea;
    qCDebug(dsrApp) << "Frame rate:" << m_framerate;

    //设置录屏的帧率
    arguments << QString("video/x-raw, framerate=%1/1").arg(m_framerate);
    //设置视频转换器
    arguments << "videoconvert";
    arguments << "videorate";
    arguments << "queue max-size-bytes=1073741824 max-size-time=10000000000 max-size-buffers=1000";
    qCDebug(dsrApp) << "Video pipeline arguments set.";

    //创建管道
    if (createPipeline(arguments)) {
        qCDebug(dsrApp) << "Pipeline creation initiated.";
        if (nullptr == m_pipeline) {
            qCritical() << "Error: Gstreamer's Pipeline create failure!";
            return;
        }
        qCInfo(dsrApp) << "Gstreamer's Pipeline create successfully!";
        //启动Gstreamer录屏管道
        GstStateChangeReturn ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            qCWarning(dsrApp) << "Unable to set the pipeline to the playing state. Recording is failure";
            gstInterface::m_gst_object_unref(m_pipeline);
            return;
        }
        qCInfo(dsrApp) << "(x11) Gstreamer's Pipeline starup successfully!";
    } else {
        qCritical() << "Error: Gstreamer's Pipeline create failure!";
    }
    qCDebug(dsrApp) << "x11GstStartRecord finished.";
}

//x11协议下gstreamer停止录制视频
void GstRecordX::x11GstStopRecord()
{
    qCInfo(dsrApp) << "Stopping X11 GStreamer recording";
    if (!m_pipeline) {
        qCWarning(dsrApp) << "wayland Gstreamer 录屏未能正常结束！录屏管道被提前释放！";
        return;
    }
    stopPipeline();
    qCInfo(dsrApp) << "x11 Gstreamer 录屏结束！";
}

//wayland协议下gstreamer录制管道构建及启动录制视频
void GstRecordX::waylandGstStartRecord()
{
    qCInfo(dsrApp) << "Starting Wayland GStreamer recording";
    QStringList wlarguments;
    wlarguments << "appsrc name=videoSrc";
    
    qCDebug(dsrApp) << "Board vendor type:" << m_boardVendorType;
    qCDebug(dsrApp) << "Recording area:" << m_recordArea;
    qCDebug(dsrApp) << "Frame rate:" << m_framerate;

    //此处需特别注意视频帧的格式，由于在wayland协议上，采集的画面，hw机和普通机器的像素格式有差异
    if (m_boardVendorType) {
        qCDebug(dsrApp) << "Board vendor type is non-zero, using RGB format.";
        wlarguments << QString("video/x-raw, format=RGB, framerate=%1/1, width=%2, height=%3").arg(m_framerate).arg(m_recordArea.width()).arg(m_recordArea.height());
    } else {
        qCDebug(dsrApp) << "Board vendor type is zero, using RGBA format.";
        wlarguments << QString("video/x-raw, format=RGBA, framerate=%1/1, width=%2, height=%3").arg(m_framerate).arg(m_recordArea.width()).arg(m_recordArea.height());
    }
    wlarguments << "queue ! videoconvert primaries-mode=2 name=convert ! queue";
    qCDebug(dsrApp) << "Video raw format and conversion queue added.";

    //创建管道
    if (createPipeline(wlarguments)) {
        qCDebug(dsrApp) << "Pipeline creation initiated.";
        if (nullptr == m_pipeline) {
            qCritical() << "Error: Gstreamer's Pipeline create failure!";
            return;
        }
        qCInfo(dsrApp) << "GStreamer pipeline created successfully";
        //添加视频相关流信息
        GstElement *videoSrc = gstInterface::m_gst_bin_get_by_name(getGstBin(m_pipeline), "videoSrc");
        gstInterface::m_g_object_set(videoSrc, "format", GST_FORMAT_TIME, NULL);
        gstInterface::m_g_object_set(videoSrc, "is-live", TRUE, NULL);
        m_gloop = gstInterface::m_g_main_loop_new(NULL, TRUE);

        GstBus *bus = gstInterface::m_gst_pipeline_get_bus(reinterpret_cast<GstPipeline *>(m_pipeline));
        gstInterface::m_gst_bus_add_watch(bus, reinterpret_cast<GstBusFunc>(gstBusMessageCb), this);
        gstInterface::m_gst_object_unref(bus);

        //启动Gstreamer录屏管道
        GstStateChangeReturn ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            qCWarning(dsrApp) << "Unable to set the pipeline to the playing state. Recording failed";
            gstInterface::m_gst_object_unref(m_pipeline);
            return;
        }
        QtConcurrent::run(gstInterface::m_g_main_loop_run, m_gloop);
        qCInfo(dsrApp) << "(Wayland) GStreamer pipeline startup successful";
    } else {
        qCCritical(dsrApp) << "Error: GStreamer pipeline creation failed";
    }
    qCDebug(dsrApp) << "waylandGstStartRecord finished.";
}

//wayland协议下gstreamer停止录制视频
void GstRecordX::waylandGstStopRecord()
{
    qCInfo(dsrApp) << "Stopping Wayland GStreamer recording";
    if (!m_pipeline) {
        qCWarning(dsrApp) << "Wayland GStreamer recording could not end normally! Pipeline was released early!";
        return;
    }

    GstElement *videoSrc = gstInterface::m_gst_bin_get_by_name(getGstBin(m_pipeline), "videoSrc");
    GstFlowReturn ret = GST_FLOW_NOT_LINKED;
    if (videoSrc) {
        gstInterface::m_g_signal_emit_by_name(videoSrc, "end-of-stream", &ret);
        qCDebug(dsrApp) << "Video source end-of-stream signal emitted.";
    }
    if (GST_FLOW_OK == ret) {
        qCInfo(dsrApp) << "(wayland Gstreamer) Stop writing video data";
    } else {
        qCInfo(dsrApp) << "(wayland Gstreamer) Stopping video data writing failed! Gstreamer internal Error Code: " << ret;
    }
    stopPipeline();
    //发射wayland gstreamer录屏完成信号
    emit waylandGstRecrodFinish();
    qCInfo(dsrApp) << "Wayland GStreamer recording ended";
}

//wayland下写入视频帧
bool GstRecordX::waylandWriteVideoFrame(const unsigned char *frame, const int framewidth, const int frameheight)
{
    qCDebug(dsrApp) << "waylandWriteVideoFrame called with framewidth:" << framewidth << ", frameheight:" << frameheight;
    if (!m_pipeline) {
        qCWarning(dsrApp) << "Wayland GStreamer failed to write video frame! Recording pipeline not initialized!";
        return false;
    }
    qCDebug(dsrApp) << "Pipeline initialized, proceeding to write video frame.";

    //qDebug() << " ======= 取视频帧进行编码 ";

    GstFlowReturn ret;
    guint8 *ptr;
    int size = m_recordArea.width() * m_recordArea.height() * 4;
    ptr = (guint8 *)gstInterface::m_g_malloc(size * sizeof(uchar));
    if (nullptr == ptr) {
        qCWarning(dsrApp) << "GStreamer writeFrame malloc failed!";
        return false;
    } else {
        qCDebug(dsrApp) << "Memory allocated for video frame.";
        GstBuffer *buffer;
        QImage img(frame, framewidth, frameheight, QImage::Format::Format_RGBA8888);
        img = img.copy(m_recordArea);
        memcpy(ptr, img.bits(), size);
        buffer = gstInterface::m_gst_buffer_new_wrapped((void *)ptr, size);
        qCDebug(dsrApp) << "Video frame copied and wrapped into GstBuffer.";

//        QImage *img = new QImage(frame, m_recordArea.width(), m_recordArea.height(), QImage::Format::Format_RGBA8888);
//        img->save(QString("/home/uos/Desktop/test/image/test_%1.png").arg(globalCount));
//        globalCount++;

        //设置时间戳
        GST_BUFFER_PTS(buffer) = gstInterface::m_gst_clock_get_time(m_pipeline->clock) - m_pipeline->base_time;
        //获取视频源
        GstElement *videoSrc = gstInterface::m_gst_bin_get_by_name(getGstBin(m_pipeline), "videoSrc");
        //注入视频帧数据
        gstInterface::m_g_signal_emit_by_name(videoSrc, "push-buffer", buffer, &ret);
        //释放buffer
        //        gstInterface::m_gst_buffer_unref(buffer);
        gstInterface::m_gst_mini_object_unref(GST_MINI_OBJECT_CAST(buffer));
    }

    qCDebug(dsrApp) << "waylandWriteVideoFrame finished. Returning:" << (ret == GST_FLOW_OK);
    return ret == GST_FLOW_OK;
}

//设置输入设备名称
void GstRecordX::setInputDeviceName(const QString &device)
{
    qCDebug(dsrApp) << "setInputDeviceName called with device:" << device;
    //即使为空，也不影响功能，设备名称不会用作判断是否采集音频，只是存储设备名称，判断是否采集有另外的字段
//    if (device.isEmpty() || device == "") {
//        qCInfo(dsrApp) << "devcieType: " << type << " current audio device name is empty!";
//    }
    m_micDeviceName = device;
    qCDebug(dsrApp) << "Microphone device name set to:" << m_micDeviceName;
}

//设置输出设备名称
void GstRecordX::setOutputDeviceName(const QString &device)
{
    qCDebug(dsrApp) << "setOutputDeviceName called with device:" << device;
    //即使为空，也不影响功能，设备名称不会用作判断是否采集音频，只是存储设备名称，判断是否采集有另外的字段
//    if (device.isEmpty() || device == "") {
//        qCInfo(dsrApp) << "devcieType: " << type << " current audio device name is empty!";
//    }
    m_sysDevcieName = device;
    qCDebug(dsrApp) << "System device name set to:" << m_sysDevcieName;
}

//设置音频类型
void GstRecordX::setAudioType(GstRecordX::AudioType audioType)
{
    qCDebug(dsrApp) << "setAudioType called with audioType:" << audioType;
    m_audioType = audioType;
    qCDebug(dsrApp) << "Audio type set to:" << m_audioType;
}

//设置视频类型
void GstRecordX::setVidoeType(GstRecordX::VideoType videoType)
{
    qCDebug(dsrApp) << "setVidoeType called with videoType:" << videoType;
    m_videoType = videoType;
    qCDebug(dsrApp) << "Video type set to:" << m_videoType;
}

//设置视频帧率
void GstRecordX::setFramerate(const int &framerate)
{
    qCDebug(dsrApp) << "setFramerate called with framerate:" << framerate;
    m_framerate = framerate;
    qCDebug(dsrApp) << "Framerate set to:" << m_framerate;
}

//设置录制区域的位置及大小
void GstRecordX::setRecordArea(const QRect &recordArea)
{
    qCDebug(dsrApp) << "setRecordArea called with recordArea:" << recordArea;
    m_recordArea = recordArea;
    qCDebug(dsrApp) << "Record area set to:" << m_recordArea;
}

//设置保存路径
void GstRecordX::setSavePath(const QString &savePath)
{
    qCDebug(dsrApp) << "setSavePath called with savePath:" << savePath;
    m_savePath = savePath;
    qCDebug(dsrApp) << "Save path set to:" << m_savePath;
}

//设置x11录制光标
void GstRecordX::setX11RecordMouse(const bool recordMouse)
{
    qCDebug(dsrApp) << "setX11RecordMouse called with recordMouse:" << recordMouse;
    m_isRecordMouse =  recordMouse ? "true" : "false";
    qCDebug(dsrApp) << "X11 record mouse set to:" << m_isRecordMouse;
}

//设置主板供应商类型
void GstRecordX::setBoardVendorType(int boardVendorType)
{
    qCDebug(dsrApp) << "setBoardVendorType called with boardVendorType:" << boardVendorType;
    m_boardVendorType = boardVendorType;
    qCDebug(dsrApp) << "Board vendor type set to:" << m_boardVendorType;
}

//创建Gstreamer录屏管道，这部分x11和wayland可共用
bool GstRecordX::createPipeline(QStringList arguments)
{
    qCDebug(dsrApp) << "createPipeline called.";
    qCInfo(dsrApp) << "Creating GStreamer pipeline";
    //设置编码器
    // arguments << QString("vp8enc min_quantizer=20 max_quantizer=20 cpu-used=%1 deadline=1000000 threads=%2").arg(QThread::idealThreadCount()).arg(QThread::idealThreadCount());
    if (m_videoType == VideoType::webm) {
        qCDebug(dsrApp) << "Using VP8 encoder for WebM";
        //vp8编码
        arguments << "vp8enc min-quantizer=1 max-quantizer=50 undershoot=95 cpu-used=5 deadline=1 static-threshold=50 error-resilient=1";
    } else if (m_videoType == VideoType::ogg) {
        qCDebug(dsrApp) << "Using Theora encoder for OGG";
        //ogg不支持vp8编码，需使用theora编码，该编码器基于vp3,编码效率低，生成的视频容存在时长不正确的问题
        arguments << "theoraenc  bitrate=2200 drop-frames=false keyframe-auto=false keyframe-force=5 keyframe-freq=5";
    } else {
        qCWarning(dsrApp) << "Unknown video type, defaulting to VP8 encoder";
        arguments << "vp8enc min-quantizer=1 max-quantizer=50 undershoot=95 cpu-used=5 deadline=1 static-threshold=50 error-resilient=1";
    }
    arguments << "queue";
    arguments << "mux.";
    qCDebug(dsrApp) << "Video encoder and muxer configured.";

    //混音模式
    if (m_audioType == AudioType::Mix) {
        qCDebug(dsrApp) << "Configuring mixed audio pipeline";
        //为保证Gstreamer管道顺利执行，做了规避处理，遇到音频设备名称不存在的情况下，输出错误日志，不在管道命令中添加音频混流器
        if (m_sysDevcieName.isEmpty() && m_micDeviceName.isEmpty()) {
            qCritical() << "Error: The system audio device name and mic audio device name is empty in Mix audio!";
        } else {
            qCDebug(dsrApp) << "Adding audio mixer.";
            //音频的混流器
            arguments << "adder name=mix ! queue ! audiorate ! audioconvert ! vorbisenc ! mux.";

            //添加系统音频
            arguments << getAudioPipeline(m_sysDevcieName, "sys", "mix");
            qCDebug(dsrApp) << "System audio pipeline added.";
            //添加麦克风音频
            arguments << getAudioPipeline(m_micDeviceName, "mic", "mix");
            qCDebug(dsrApp) << "Microphone audio pipeline added.";
        }
    } else if (m_audioType == AudioType::Mic) { //单麦克风音频模式
        qCDebug(dsrApp) << "Configuring microphone-only audio pipeline";
        //为保证Gstreamer管道顺利执行，做了规避处理，遇到音频设备名称不存在的情况下，输出错误日志，不在管道命令中添加音频,录制无声视频
        arguments << getAudioPipeline(m_micDeviceName, "mic", "mux");
        qCDebug(dsrApp) << "Microphone-only audio pipeline added.";
    } else if (m_audioType == AudioType::Sys) { //单系统音频模式
        qCDebug(dsrApp) << "Configuring system-only audio pipeline";
        //为保证Gstreamer管道顺利执行，做了规避处理，遇到音频设备名称不存在的情况下，输出错误日志，不在管道命令中添加音频,录制无声视频
        arguments << getAudioPipeline(m_sysDevcieName, "sys", "mux");
        qCDebug(dsrApp) << "System-only audio pipeline added.";
    }
    qCDebug(dsrApp) << "Audio configuration complete.";

    if (m_videoType == VideoType::webm) {
        qCDebug(dsrApp) << "Using WebM muxer";
        //webmmux 复用器
        arguments << "webmmux name=mux";
    } else if (m_videoType == VideoType::ogg) {
        qCDebug(dsrApp) << "Using OGG muxer";
        arguments << "oggmux name=mux";
    } else {
        qCWarning(dsrApp) << "the recordType is Error! set default recordtype webm";
        arguments << "webmmux name=mux";
    }
    qCDebug(dsrApp) << "Muxer configured.";

    //文件路径
    arguments << "filesink location=" + m_savePath;
    arguments.removeAll("");
    qCDebug(dsrApp) << "Filesink location set and empty arguments removed.";
    //转换未字符串
    QString recordArg = arguments.join(" ! ");
    recordArg = recordArg.replace("mix. !", "mix.");
    recordArg = recordArg.replace("mux. !", "mux.");
    qCDebug(dsrApp) << "Record argument string prepared.";
    //格式化输出Gstreamer管道构建命令行
    pipelineStructuredOutput(recordArg);

    QByteArray byteArray = recordArg.toUtf8();
    const gchar *line = byteArray.constData();
    GError *error = Q_NULLPTR;

    //解析gstreamer管道命令
    m_pipeline = gstInterface::m_gst_parse_launch(line, &error);
    if (error != Q_NULLPTR) {
        qCritical() << "gstreamer pares error: " << error->code << " , " << error->message;
        return false;
    }
    qCInfo(dsrApp) << "GStreamer pipeline created successfully";
    return true;
}

//根据传入的参数获取，音频管道创建命令
QString GstRecordX::getAudioPipeline(const QString &audioDevName, const QString &audioType, const QString &arg)
{
    qCDebug(dsrApp) << "Creating audio pipeline for" << audioType << "device:" << audioDevName;
    QString result = "";
    //判断设备名称是否为空，为空不会生成管道命令
    if (audioDevName.isEmpty()) {
        qCritical() << "Error: The " << audioType << " audio device name is empty! (" << arg << ")";
    } else {
        qCDebug(dsrApp) << "Audio device name is not empty. Building pipeline.";
        //添加pulse源，采集设备的声音
        result = "pulsesrc device=" + audioDevName + " ! queue ! audioconvert ! audioresample ! ";
        //添加管道中流媒体格式
        result += QString("capsfilter caps=audio/x-raw,rate=%1,channels=%2").arg(m_rate).arg(m_channels);
        //根据传入的参数，生成不同的处理器（音频混流器或复用器）
        if (arg == "mix") {
            result += " ! queue ! mix.";
            qCDebug(dsrApp) << "Audio pipeline configured for mixing";
        } else if (arg == "mux") {
            result += " ! vorbisenc ! queue ! mux.";
            qCDebug(dsrApp) << "Audio pipeline configured for muxing";
        } else {
            qCritical() << "Error: Parameter that does not exist!";
            result = "";
        }
    }
    qCDebug(dsrApp) << "getAudioPipeline finished. Result:" << result;
    return result;
}

//停止管道，x11和wayland可共用
void GstRecordX::stopPipeline()
{
    qCInfo(dsrApp) << "Stopping GStreamer pipeline";
    bool a = gstInterface::m_gst_element_send_event(m_pipeline, gstInterface::m_gst_event_new_eos());
    Q_UNUSED(a);
    qCDebug(dsrApp) << "EOS event sent. Result:" << a;

    GstClockTime timeout = 5 * GST_SECOND;
    GstMessage *msg = gstInterface::m_gst_bus_timed_pop_filtered(GST_ELEMENT_BUS(m_pipeline), timeout, GST_MESSAGE_EOS);
    Q_UNUSED(msg);
    qCDebug(dsrApp) << "Bus timed pop filtered for EOS message.";

    GstStateChangeReturn ret ;
    Q_UNUSED(ret);
    ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
    qCDebug(dsrApp) << "Pipeline state set to PAUSED";
    Q_UNUSED(ret);
    ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_READY);
    qCDebug(dsrApp) << "Pipeline state set to READY";
    Q_UNUSED(ret);
    ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_NULL);
    qCDebug(dsrApp) << "Pipeline state set to NULL";
    Q_UNUSED(ret);
    gstInterface::m_gst_object_unref(m_pipeline);
    qCInfo(dsrApp) << "Pipeline stopped and cleaned up";
}


//格式化输出gstreamer命令
void GstRecordX::pipelineStructuredOutput(QString pipeline)
{
    qCDebug(dsrApp) << "pipelineStructuredOutput called with pipeline string.";
    QString string;
    QString nl;
    nl = "\\";
    pipeline.prepend("gst-launch-1.0 -e --gst-debug-level=2" + nl + "\n    ");
    pipeline.replace("mux.", "mux. " + nl + "\n   ");
    pipeline.replace("mix.", "mix. " + nl + "\n   ");
    string = pipeline.replace("!", nl + "\n        !");
    string.append("\n");
    //    printf("%s\n", string.toLocal8Bit().data());
    qCInfo(dsrApp) << "\n" << string.toLocal8Bit().data();
}

GstBin *GstRecordX::getGstBin(GstElement *element)
{
    qCDebug(dsrApp) << "getGstBin called with GstElement.";
    GstBin *bin = (GstBin *)gstInterface::m_g_type_check_instance_cast((GTypeInstance *)element, gstInterface::m_gst_bin_get_type());
    qCDebug(dsrApp) << "Returning GstBin from GstElement.";
    return bin;
}
GstRecordX::~GstRecordX()
{
    qCDebug(dsrApp) << "GstRecordX destructor called.";
    if (m_pipeline) {
        qCInfo(dsrApp) << "Cleaning up GStreamer pipeline";
        m_pipeline = nullptr;
        delete m_pipeline;
    }
    qCDebug(dsrApp) << "GstRecordX destructor finished.";
}
