#include "recordAdmin.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cstring>
#include <math.h>
#include <unistd.h> //sleep()
#include <qtimer.h>
#include <QDebug>

RecordAdmin* gpMainFrame = NULL;

//采集到的视频图像回调 CALLBACK
//int VideoCaptureCallback(AVStream * input_st, enum AVPixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp)
//{
////    if(gpMainFrame->IsPreview())
////    {
////       printf("XXX\n");
////    }
//    gpMainFrame->m_outputStream.writeVideoFrame(input_st, pix_fmt, pframe, lTimeStamp);
//    return 0;
//}

//采集到的音频数据回调 CALLBACK
//int AudioCaptureCallback(AVStream * input_st, AVFrame *pframe, int64_t lTimeStamp)
//{
//    gpMainFrame->m_pOutputStream->writeMicAudioFrame(input_st, pframe, lTimeStamp);
//    return 0;
//}
//int AudioScardCaptureCallback(AVStream * input_st, AVFrame *pframe, int64_t lTimeStamp)
//{
//    gpMainFrame->m_pOutputStream->writeSysAudioFrame(input_st, pframe, lTimeStamp);
//    return 0;
//}
//int writeAmixCallback()
//{
//    gpMainFrame->m_pOutputStream->startWriteMix();
//    return 0;
//}
RecordAdmin::RecordAdmin(WaylandIntegration::WaylandIntegrationPrivate *context, QObject *parent): QObject(parent),
    m_writeFrameThread(nullptr),
    m_context(context),
    m_pOutputStream(nullptr)
{
    m_pInputStream  = new CAVInputStream(context);
    m_pOutputStream = new CAVOutputStream(context);
    gpMainFrame = this;
    //m_isOverFlage = false;
    //m_outFilePath = nullptr;
    m_nChannelNum = -1;
    m_nFPS = 0;
    /* register all codecs, demux and protocols */
    avcodec_register_all();
    av_register_all();
    avdevice_register_all();
    m_writeFrameThread = new WaylandIntegration::WriteFrameThread(context);
}

RecordAdmin::~RecordAdmin()
{
    if(nullptr != m_writeFrameThread)
    {
        delete m_writeFrameThread;
        m_writeFrameThread = nullptr;
    }
    if(nullptr != m_pOutputStream)
    {
        delete m_pOutputStream;
        m_pOutputStream = nullptr;
    }
    if(nullptr != m_pInputStream)
    {
        delete m_pInputStream;
        m_pInputStream = nullptr;
    }
}

void RecordAdmin::setRecordAudioType(int audioType)
{
    switch (audioType)
    {
    case audioType::MIC:
        setMicAudioRecord(true);
        setSysAudioRecord(false);
        break;
    case audioType::SYS:
        setMicAudioRecord(false);
        setSysAudioRecord(true);
        break;
    case audioType::MIC_SYS:
        setMicAudioRecord(true);
        setSysAudioRecord(true);
        break;
    case audioType::NOS:
        setMicAudioRecord(false);
        setSysAudioRecord(false);
        break;
    default:{
        setMicAudioRecord(false);
        setSysAudioRecord(false);
    }
    }
}

void  RecordAdmin::setMicAudioRecord(bool bRecord)
{
    m_pInputStream->setMicAudioRecord(bRecord);
}

void  RecordAdmin::setSysAudioRecord(bool bRecord)
{
    m_pInputStream->setSysAudioRecord(bRecord);
}

void RecordAdmin::init(int screenWidth, int screenHeight, int fps, int audioType, int x, int y, int selectWidth, int selectHeight, const char* path)
{
    m_pInputStream->m_screenDW = screenWidth;
    m_pInputStream->m_screenDH = screenHeight;
    m_pInputStream->m_ipix_fmt = AV_PIX_FMT_RGB32;
    m_pInputStream->m_fps = fps;
    m_pOutputStream->m_left = m_pInputStream->m_left = x;
    m_pOutputStream->m_top = m_pInputStream->m_top = y;
    m_pOutputStream->m_right = m_pInputStream->m_right = screenWidth-x-selectWidth;
    m_pOutputStream->m_bottom = m_pInputStream->m_bottom = screenHeight-y-selectHeight;
    if(m_pInputStream->m_right<0)
    {
        selectWidth += m_pInputStream->m_right;
        m_pInputStream->m_selectWidth = selectWidth;
        m_pInputStream->m_right = 0;
    }
    if(m_pInputStream->m_bottom<0)
    {
        selectHeight += m_pInputStream->m_bottom;
        m_pInputStream->m_selectHeight = selectHeight;
        m_pInputStream->m_bottom = 0;
    }
    setRecordAudioType(audioType);
    m_pInputStream->setVidioOutPutType(OUTPUT_TYPE::MP4_MKV);
    m_outFilePath = new char[strlen(path)+1];
    strcpy(m_outFilePath,path);
    pthread_create(&m_mainThread, nullptr, stream,static_cast<void*>(this));
    pthread_detach(m_mainThread);
}

int RecordAdmin::startStream()
{
    //m_pInputStream->SetWirteAmixtCB(writeAmixCallback);
    bool bRet;
    bRet = m_pInputStream->openInputStream(); //初始化采集设备
    if(!bRet)
    {
        printf("打开采集设备失败\n");
        return 1;
    }
    int cx, cy, fps;
    AVPixelFormat pixel_fmt;
    //GetVideoInputInfo,获取相机capacity
    if(m_pInputStream->GetVideoInputInfo(cx, cy, fps, pixel_fmt)) //获取视频采集源的信息
    {
        //cx:width cy:height  //CBR（固定码率控制）, VBR是动态码率,  平均码率ABR,
        //视频编码器常用的码率控制方式: abr(平均码率)，crf（限制码率），cqp（固定质量）
        m_pOutputStream->SetVideoCodecProp(AV_CODEC_ID_H264, fps, 500000/*bps*/, 30/*GOP*/, cx, cy); //设置视频编码器属性
    }
    int sample_rate = 0, channels = 0,layout;
    AVSampleFormat  sample_fmt;
    if(m_pInputStream->GetAudioInputInfo(sample_fmt, sample_rate, channels,layout)) //获取音频采集源的信息
    {
        m_pOutputStream->SetAudioCodecProp(AV_CODEC_ID_MP3, sample_rate, channels,layout, 32000); //AV_CODEC_ID_MP3 AV_CODEC_ID_AAC设置音频编码器属性
    }
    if(m_pInputStream->GetAudioSCardInputInfo(sample_fmt, sample_rate, channels,layout)) //获取音频采集源的信息
    {
        m_pOutputStream->SetAudioCardCodecProp(AV_CODEC_ID_MP3, sample_rate, channels,layout, 32000); //设置音频编码器属性
    }
    bRet = m_pOutputStream->open(m_outFilePath); //设置输出路径
    if(!bRet)
    {
        printf("初始化输出失败\n");
        return 1;
    }
    //设置写视频帧
    m_writeFrameThread->setBWriteFrame(true);
    m_writeFrameThread->start();
    //采集音频
    m_pInputStream->audioCapture();
    return 0;
}

void* RecordAdmin::stream(void* param)
{
    RecordAdmin *recordAdmin = static_cast<RecordAdmin*>(param);
    recordAdmin->startStream();
    return nullptr;
}

int RecordAdmin::stopStream()
{
    //设置是否获取视频帧
    //m_context->setBGetFrame(false);
    //设置是否写视频帧
    m_writeFrameThread->setBWriteFrame(false);
    //设置是否写混音
    m_pInputStream->setbWriteAmix(false);
    //设置是否写音频帧
    m_pOutputStream->setIsWriteFrame(false);
    //设置是否运行线程
    m_pInputStream->setbRunThread(false);
    //关闭输出
    m_pOutputStream->close();
    return 0;
}





























