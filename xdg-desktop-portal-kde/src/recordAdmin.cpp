#include "recordAdmin.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cstring>
#include <math.h>
#include <unistd.h>
#include <qtimer.h>
#include "qgifimage.h"
#include <QDebug>

RecordAdmin::RecordAdmin(int &argc, char **argv, WaylandIntegration::WaylandIntegrationPrivate *context, QObject *parent): QObject(parent),
    m_pInputStream(nullptr),
    m_pOutputStream(nullptr),
    m_writeFrameThread(nullptr),
    m_pGifWrite(nullptr),
    //m_pGifCreator(nullptr),
    m_context(context)
{
    if(argc > 7)
    {
        QString tempStr;
        QString str;
        for(int i=0;i<argc;i++)
        {
            str = QString::fromUtf8(argv[i]);
            argvList.append(str);
            switch (i)
            {
            case 1:
                m_videoType = str.toInt();
                break;
            case 2:
                //录屏不支持奇数，转偶数
                m_selectWidth = str.toInt()/2*2;
                break;
            case 3:
                m_selectHeight = str.toInt()/2*2;
                break;
            case 4:
                m_x =  str.toInt();
                break;
            case 5:
                m_y = str.toInt();
                break;
            case 6:
                m_fps = str.toInt();
                break;
            case 7:
                m_filePath = str;
                break;
            case 8:
                m_audioType = str.toInt();
                break;
            }
        }
    }

    switch (m_videoType)
    {
    case videoType::GIF:
        m_gifBuffersize = 60;
        //m_pGifCreator = new GifCreator(context);
        for(int i=0;i<1;i++)
        {
            m_pGifRecord[i] = nullptr;
            m_pGifRecord[i] = new GifRecord(context,i);
        }
        m_pGifWrite = new GifWrite(context);
        break;
    case videoType::MP4:
    {
        m_pInputStream  = new CAVInputStream(context);
        m_pOutputStream = new CAVOutputStream(context);
        avcodec_register_all();
        av_register_all();
        avdevice_register_all();
        m_writeFrameThread = new WriteFrameThread(context);
    }
        break;
    case videoType::MKV:
    {
        m_pInputStream  = new CAVInputStream(context);
        m_pOutputStream = new CAVOutputStream(context);
        avcodec_register_all();
        av_register_all();
        avdevice_register_all();
        m_writeFrameThread = new WriteFrameThread(context);
    }
        break;
    default:
        break;
    }
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
    for (int i=0;i<1;i++)
    {
        if(nullptr != m_pGifRecord[i])
        {
            delete m_pGifRecord[i];
            m_pGifRecord[i] = nullptr;
        }
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

void RecordAdmin::init(int screenWidth, int screenHeight)
{
    switch (m_videoType)
    {
    case videoType::GIF:
    {
        qDebug()  <<  QThread::currentThread();
        m_pGifRecord[0]->init(screenWidth,screenHeight,m_x,m_y,m_selectWidth,m_selectHeight,m_fps,m_filePath);
        m_pGifRecord[0]->start();
//                QByteArray pathArry = m_filePath.toLocal8Bit();
//                char *pathCh = new char[strlen(pathArry.data())+1];
//                strcpy(pathCh,pathArry.data());
//                m_delay = 15;
//        m_pGifCreator->init(screenWidth,screenHeight,m_x,m_y,m_selectWidth,m_selectHeight,m_fps,m_filePath);
//        GifBegin(m_context->m_recordAdmin->m_pGifWriter,pathCh,static_cast<uint32_t>(m_selectWidth),static_cast<uint32_t>(m_selectHeight),static_cast<uint32_t>(m_delay));
//        for (int i=0;i<1;i++)
//        {
//            m_pGifRecord[i]->init(screenWidth,screenHeight,m_x,m_y,m_selectWidth,m_selectHeight,m_fps,m_filePath);
//            m_pGifRecord[i]->setBWriteFrame(true);
//            m_pGifRecord[i]->start();
//        }
//        m_pGifWrite->start();
    }
        break;
    case videoType::MP4:
    {
        m_pInputStream->m_screenDW = screenWidth;
        m_pInputStream->m_screenDH = screenHeight;
        m_pInputStream->m_ipix_fmt = AV_PIX_FMT_RGB32;
        m_pInputStream->m_fps = m_fps;
        m_pOutputStream->m_left = m_pInputStream->m_left = m_x;
        m_pOutputStream->m_top = m_pInputStream->m_top = m_y;
        m_pOutputStream->m_right = m_pInputStream->m_right = screenWidth-m_x-m_selectWidth;
        m_pOutputStream->m_bottom = m_pInputStream->m_bottom = screenHeight-m_y-m_selectHeight;
        if(m_pInputStream->m_right<0)
        {
            m_selectWidth += m_pInputStream->m_right;
            m_pInputStream->m_selectWidth = m_selectWidth;
            m_pInputStream->m_right = 0;
        }
        if(m_pInputStream->m_bottom<0)
        {
            m_selectHeight += m_pInputStream->m_bottom;
            m_pInputStream->m_selectHeight = m_selectHeight;
            m_pInputStream->m_bottom = 0;
        }
        setRecordAudioType(m_audioType);
        pthread_create(&m_mainThread, nullptr, stream,static_cast<void*>(this));
        pthread_detach(m_mainThread);
    }
        break;
    case videoType::MKV:
    {
        m_pInputStream->m_screenDW = screenWidth;
        m_pInputStream->m_screenDH = screenHeight;
        m_pInputStream->m_ipix_fmt = AV_PIX_FMT_RGB32;
        m_pInputStream->m_fps = m_fps;
        m_pOutputStream->m_left = m_pInputStream->m_left = m_x;
        m_pOutputStream->m_top = m_pInputStream->m_top = m_y;
        m_pOutputStream->m_right = m_pInputStream->m_right = screenWidth-m_x-m_selectWidth;
        m_pOutputStream->m_bottom = m_pInputStream->m_bottom = screenHeight-m_y-m_selectHeight;
        if(m_pInputStream->m_right<0)
        {
            m_selectWidth += m_pInputStream->m_right;
            m_pInputStream->m_selectWidth = m_selectWidth;
            m_pInputStream->m_right = 0;
        }
        if(m_pInputStream->m_bottom<0)
        {
            m_selectHeight += m_pInputStream->m_bottom;
            m_pInputStream->m_selectHeight = m_selectHeight;
            m_pInputStream->m_bottom = 0;
        }
        setRecordAudioType(m_audioType);
        pthread_create(&m_mainThread, nullptr, stream,static_cast<void*>(this));
        pthread_detach(m_mainThread);
    }
        break;
    default:
        break;
    }
}

int RecordAdmin::startStream()
{
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
    bRet = m_pOutputStream->open(m_filePath);
    if(!bRet)
    {
        printf("初始化输出失败\n");
        return 1;
    }
    //设置写mp4/mkv视频帧
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
    switch (m_videoType)
    {
    case videoType::GIF:
        //设置是否写gif视频帧
        for(int i =0;i<1;i++)
        {
            m_pGifRecord[i]->setBWriteFrame(false);
        }
        //设置是否缓存区排序
        //m_pGifWrite->setBCache(false);
        break;
    case videoType::MP4:
    {
        //设置是否获取视频帧
        //m_context->setBGetFrame(false);
        //设置是否写MP4/MKV视频帧
        m_writeFrameThread->setBWriteFrame(false);
        //设置是否写混音
        m_pInputStream->setbWriteAmix(false);
        //设置是否写音频帧
        m_pOutputStream->setIsWriteFrame(false);
        //设置是否运行线程
        m_pInputStream->setbRunThread(false);
        //关闭输出
        m_pOutputStream->close();
    }
        break;
    case videoType::MKV:
    {
        //设置是否获取视频帧
        //m_context->setBGetFrame(false);
        //设置是否写MP4/MKV视频帧
        m_writeFrameThread->setBWriteFrame(false);
        //设置是否写混音
        m_pInputStream->setbWriteAmix(false);
        //设置是否写音频帧
        m_pOutputStream->setIsWriteFrame(false);
        //设置是否运行线程
        m_pInputStream->setbRunThread(false);
        //关闭输出
        m_pOutputStream->close();
    }
        break;
    default:
        break;
    }
    return 0;
}

//void RecordAdmin::insertOldFrame(GifFrame frame)
//{
//    QMutexLocker locker(&m_oldFrameMutex);
//    if(m_oldFrameMap.size() >= m_gifBuffersize)
//    {
//        m_oldFrameMap.remove(m_oldFrameMap.firstKey());
//    }
//    m_oldFrameMap.insert(frame.index,frame);
//}

//GifFrame RecordAdmin::getOldFrame(int index)
//{
//    QMutexLocker locker(&m_oldFrameMutex);
//    if(m_oldFrameMap.contains(index))
//    {
//        return m_oldFrameMap.value(index);
//    }
//    else
//    {
//        GifFrame frame;
//        frame.data = nullptr;
//        return frame;
//    }
//}
