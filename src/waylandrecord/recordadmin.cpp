// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordadmin.h"
#include <QtConcurrent>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cstring>
#include <math.h>
#include <unistd.h>
#include <qtimer.h>
#include <QDebug>
#include "../utils/log.h"

#include "utils.h"

RecordAdmin::RecordAdmin(QStringList list, WaylandIntegration::WaylandIntegrationPrivate *context, QObject *parent): QObject(parent),
    m_pInputStream(nullptr),
    m_pOutputStream(nullptr),
    m_writeFrameThread(nullptr),
    m_context(context)
{
    qCDebug(dsrApp) << "Initializing RecordAdmin with parameters";
    m_delay = 0;
    m_gifBuffersize = 0;
    if (list.size() > 9) {
        m_videoType = list[0].toInt();
        //录屏不支持奇数，转偶数
        m_selectWidth = list[1].toInt() / 2 * 2;
        m_selectHeight = list[2].toInt() / 2 * 2;
        m_x = list[3].toInt();
        m_y = list[4].toInt();
        m_fps = list[5].toInt();
        m_filePath = list[6];
        m_audioType = list[7].toInt();
        m_inputDeviceName = list[8];
        m_outputDeviceName = list[9];
        qCDebug(dsrApp) << "Recording parameters set - Size:" << m_selectWidth << "x" << m_selectHeight 
                      << "Position:" << m_x << "," << m_y 
                      << "FPS:" << m_fps 
                      << "Audio type:" << m_audioType;
    }
    if (Utils::kGIF == m_videoType) {
        m_fps = 24;
        m_audioType = Utils::kNoAudio;
        m_filePath = m_filePath.replace("gif", "mp4");
        qCDebug(dsrApp) << "GIF mode detected, adjusted settings - FPS:" << m_fps << "File:" << m_filePath;
    }
    m_pInputStream  = new CAVInputStream(context);
    m_pOutputStream = new CAVOutputStream(context);
    avlibInterface::m_avcodec_register_all();
    avlibInterface::m_av_register_all();
    avlibInterface::m_avdevice_register_all();
    m_writeFrameThread = new WriteFrameThread(context);
    qCDebug(dsrApp) << "RecordAdmin initialization completed";
}

RecordAdmin::~RecordAdmin()
{
    if (nullptr != m_writeFrameThread) {
        delete m_writeFrameThread;
        m_writeFrameThread = nullptr;
    }
    if (nullptr != m_pOutputStream) {
        delete m_pOutputStream;
        m_pOutputStream = nullptr;
    }
    if (nullptr != m_pInputStream) {
        delete m_pInputStream;
        m_pInputStream = nullptr;
    }

}

void RecordAdmin::setRecordAudioType(int audioType)
{
    qCDebug(dsrApp) << "Setting audio type to:" << audioType;
    switch (audioType) {
    case Utils::kMic:
        setMicAudioRecord(true);
        setSysAudioRecord(false);
        break;
    case Utils::kSystemAudio:
        setMicAudioRecord(false);
        setSysAudioRecord(true);
        break;
    case Utils::kMicAndSystemAudio:
        setMicAudioRecord(true);
        setSysAudioRecord(true);
        break;
    case Utils::kNoAudio:
        setMicAudioRecord(false);
        setSysAudioRecord(false);
        break;
    default: {
        setMicAudioRecord(false);
        setSysAudioRecord(false);
    }
    }
}

void  RecordAdmin::setMicAudioRecord(bool bRecord)
{
    qCDebug(dsrApp) << "Setting mic audio record:" << bRecord;
    m_pInputStream->setMicAudioRecord(bRecord);
}

void  RecordAdmin::setSysAudioRecord(bool bRecord)
{
    qCDebug(dsrApp) << "Setting system audio record:" << bRecord;
    m_pInputStream->setSysAudioRecord(bRecord);
}

void RecordAdmin::init(int screenWidth, int screenHeight)
{
    qCDebug(dsrApp) << "Initializing RecordAdmin with screen size:" << screenWidth << "x" << screenHeight;
    m_pInputStream->m_screenDW = screenWidth;
    m_pInputStream->m_screenDH = screenHeight;
    m_pInputStream->m_sysDeviceName = m_outputDeviceName;
    m_pInputStream->m_micDeviceName = m_inputDeviceName;
    m_pInputStream->m_ipix_fmt = AV_PIX_FMT_RGB32;
    m_pInputStream->m_fps = m_fps;
    m_pOutputStream->m_left = m_pInputStream->m_left = m_x;
    m_pOutputStream->m_top = m_pInputStream->m_top = m_y;
    m_pOutputStream->m_right = m_pInputStream->m_right = screenWidth - m_x - m_selectWidth;
    m_pOutputStream->m_bottom = m_pInputStream->m_bottom = screenHeight - m_y - m_selectHeight;
    m_pOutputStream->m_videoType = m_videoType;
    m_pOutputStream->setBoardVendor(m_boardVendorType);
    if (m_pInputStream->m_right < 0) {
        m_selectWidth += m_pInputStream->m_right;
        m_pInputStream->m_selectWidth = m_selectWidth;
        m_pInputStream->m_right = 0;
        qCDebug(dsrApp) << "Adjusted width due to right boundary:" << m_selectWidth;
    }
    if (m_pInputStream->m_bottom < 0) {
        m_selectHeight += m_pInputStream->m_bottom;
        m_pInputStream->m_selectHeight = m_selectHeight;
        m_pInputStream->m_bottom = 0;
        qCDebug(dsrApp) << "Adjusted height due to bottom boundary:" << m_selectHeight;
    }
    setRecordAudioType(m_audioType);
    QtConcurrent::run(this, &RecordAdmin::startStream);

//    pthread_create(&m_mainThread, nullptr, stream, static_cast<void *>(this));
//    pthread_detach(m_mainThread);
}

int RecordAdmin::startStream()
{
    qCInfo(dsrApp) << "Starting recording stream";
    bool bRet;
    qInfo() << "打开音频采集设备!";
    bRet = m_pInputStream->openInputStream(); //初始化采集设备
    if (!bRet) {
        qCCritical(dsrApp) << "Failed to open capture device";
    }
    int cx, cy, fps;
    AVPixelFormat pixel_fmt;
    //GetVideoInputInfo,获取相机capacity
    if (m_pInputStream->GetVideoInputInfo(cx, cy, fps, pixel_fmt)) { //获取视频采集源的信息
        //cx:width cy:height  //CBR（固定码率控制）, VBR是动态码率,  平均码率ABR,
        //视频编码器常用的码率控制方式: abr(平均码率)，crf（限制码率），cqp（固定质量）
        qCDebug(dsrApp) << "Video input info - Size:" << cx << "x" << cy << "FPS:" << fps;
        m_pOutputStream->SetVideoCodecProp(AV_CODEC_ID_H264, fps, 500000/*bps*/, 30/*GOP*/, cx, cy); //设置视频编码器属性
    }
    int sample_rate = 0, channels = 0, layout;
    AVSampleFormat  sample_fmt;
    if (m_pInputStream->GetAudioInputInfo(sample_fmt, sample_rate, channels, layout)) { //获取音频采集源的信息
        qCDebug(dsrApp) << "Audio input info - Sample rate:" << sample_rate << "Channels:" << channels;
        m_pOutputStream->SetAudioCodecProp(AV_CODEC_ID_MP3, sample_rate, channels, layout, 32000); //AV_CODEC_ID_MP3 AV_CODEC_ID_AAC设置音频编码器属性
    } else {
        qCWarning(dsrApp) << "Failed to get audio input information";
    }
    
    if (m_pInputStream->GetAudioSCardInputInfo(sample_fmt, sample_rate, channels, layout)) { //获取音频采集源的信息
        qCDebug(dsrApp) << "Audio card info - Sample rate:" << sample_rate << "Channels:" << channels;
        m_pOutputStream->SetAudioCardCodecProp(AV_CODEC_ID_MP3, sample_rate, channels, layout, 32000); //设置音频编码器属性
    } else {
        qCWarning(dsrApp) << "Failed to get audio card information";
    }
    
    qInfo() << "打开输出!";
    bRet = m_pOutputStream->open(m_filePath);
    if (!bRet) {
        printf("初始化输出失败\n");
        return 1;
    }
    qCInfo(dsrApp) << "Output initialized successfully to:" << m_filePath;
    
    qInfo() << "采集画面!";
    //设置写mp4/mkv视频帧
    m_writeFrameThread->setBWriteFrame(true);
    m_writeFrameThread->start();
    qCInfo(dsrApp) << "Video frame writing thread started";
    
    qInfo() << "采集音频!";
    //采集音频
    m_pInputStream->audioCapture();
    qCInfo(dsrApp) << "Audio capture started";
    return 0;
}

void RecordAdmin::setBoardVendor(int boardVendorType)
{
    qCDebug(dsrApp) << "Setting board vendor type:" << boardVendorType;
    m_boardVendorType = boardVendorType;
}

void *RecordAdmin::stream(void *param)
{
    qCDebug(dsrApp) << "Starting stream thread";
    RecordAdmin *recordAdmin = static_cast<RecordAdmin *>(param);
    recordAdmin->startStream();
    return nullptr;
}

int RecordAdmin::stopStream()
{
    qCInfo(dsrApp) << "Stopping video stream";
    qInfo() << __LINE__ << __func__ << "正在停止视频数据写入...";
    //设置是否运行线程，用来关闭采集音频数据的线程
    m_pInputStream->setbRunThread(false);
    qCDebug(dsrApp) << "Audio capture thread stopped";
    
    //设置关闭视频数据采集，将视频数据采集到队列中
    m_context->setBGetFrame(false);
    qCDebug(dsrApp) << "Video frame capture stopped";
    
    //设置是否写MP4/MKV视频帧，将数据从队列中取出
    m_writeFrameThread->setBWriteFrame(false);
    qCDebug(dsrApp) << "Video frame writing stopped";
    
    m_cacheMutex.lock();
    //设置是否写混音,此时采集音频流并将音频流写入音频fifo缓冲区
    m_pInputStream->setbWriteAmix(false);
    qCDebug(dsrApp) << "Audio mixing stopped";
    
    //设置是否写音频帧，此时将音频缓冲区的数据写入到输出媒体文件
    m_pOutputStream->setIsWriteFrame(false);
    qCDebug(dsrApp) << "Audio frame writing stopped";
    
    //关闭输出
    m_pOutputStream->close();
    qCDebug(dsrApp) << "Output stream closed";
    
    m_cacheMutex.unlock();
    qInfo() << __LINE__ << __func__ << "已停止视频数据写入";
    qCInfo(dsrApp) << "Stream stopping completed";
    return 0;
}
