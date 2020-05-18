#include "MainLoop.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cstring>
#include <math.h>
#include <unistd.h> //sleep()
#include <qtimer.h>
MainLoop* gpMainFrame = NULL;

//采集到的视频图像回调 CALLBACK
int VideoCaptureCallback(AVStream * input_st, enum AVPixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp)
{
    if(gpMainFrame->IsPreview())
    {
       printf("XXX\n");
    }
    gpMainFrame->m_OutputStream.write_video_frame(input_st, pix_fmt, pframe, lTimeStamp);
    return 0;
}

//采集到的音频数据回调 CALLBACK
int AudioCaptureCallback(AVStream * input_st, AVFrame *pframe, int64_t lTimeStamp)
{
    gpMainFrame->m_OutputStream.write_audio_frame(input_st, pframe, lTimeStamp);
    return 0;
}
int AudioScardCaptureCallback(AVStream * input_st, AVFrame *pframe, int64_t lTimeStamp)
{
    gpMainFrame->m_OutputStream.write_audio_card_frame(input_st, pframe, lTimeStamp);
    return 0;
}
int writeAmixCallback()
{
    gpMainFrame->m_OutputStream.startWriteAmixData();
    return 0;
}
MainLoop::MainLoop()
{
    gpMainFrame = this;
    m_isOverFlage = false;
    m_outFilePath = NULL;
    m_nChannelNum = -1;
    m_nFPS = 0;
     /* register all codecs, demux and protocols */
    avcodec_register_all();
    av_register_all();

    avdevice_register_all();
}

MainLoop::~MainLoop()
{
    if(m_outFilePath){
        delete m_outFilePath;
        m_outFilePath = NULL;
    }
}
void MainLoop::setRecordAudioType(StreamRecordAudioType type)
{
    switch (type) {
    case StreamRecordAudioType::MIC:
        setRecordAudioMic(true);
        setRecordAudioSCard(false);
        break;
    case StreamRecordAudioType::SYS:
        setRecordAudioMic(false);
        setRecordAudioSCard(true);
        break;
    case StreamRecordAudioType::MIC_SYS:
        setRecordAudioMic(true);
        setRecordAudioSCard(true);
        break;
    case StreamRecordAudioType::NOS:
        setRecordAudioMic(false);
        setRecordAudioSCard(false);
        break;
    default:{
        setRecordAudioMic(false);
        setRecordAudioSCard(false);
    }
    }
}
void  MainLoop::setRecordAudioMic(bool isrecord)
{
    gpMainFrame->m_InputStream.setRecordAudioMic(isrecord);
}
void  MainLoop::setRecordAudioSCard(bool isrecord)
{
    gpMainFrame->m_InputStream.setRecordAudioSCard(isrecord);
}
/**
  *img_width 原始图片宽
  *img_height 原始图片高
  *cx 切图开始x坐标
  *cy 切图开始y坐标
  * cw 切图宽
  * ch 切图高
  * */
void MainLoop::startInit(int img_width,int img_height,int fps,StreamRecordAudioType type,int cx,int cy,int cw,int ch,const char* outfile){
    m_canAddImage = true;
    m_isOverFlage = false;
    gpMainFrame->m_InputStream.m_screenDW = img_width;
    gpMainFrame->m_InputStream.m_screenDH = img_height;
    gpMainFrame->m_InputStream.m_ipix_fmt = AV_PIX_FMT_RGB32;
    m_fps = fps;
    gpMainFrame->m_InputStream.m_fps = fps;
    gpMainFrame->m_InputStream.m_cl = cx;
    gpMainFrame->m_InputStream.m_ct = cy;
    gpMainFrame->m_InputStream.m_cr = img_width-cx-cw;
    gpMainFrame->m_InputStream.m_cb = img_height-cy-ch;
    if(gpMainFrame->m_InputStream.m_cr<0){
        cw += gpMainFrame->m_InputStream.m_cr;
        gpMainFrame->m_InputStream.m_cw = cw;
        gpMainFrame->m_InputStream.m_cr = 0;
    }

    if(gpMainFrame->m_InputStream.m_cb<0){
        ch += gpMainFrame->m_InputStream.m_cb;
        gpMainFrame->m_InputStream.m_ch = ch;
        gpMainFrame->m_InputStream.m_cb = 0;
    }
    setRecordAudioType(type);
    if(m_outFilePath){
        delete m_outFilePath;
    }
    m_isGif = false;
    int fileLenght = strlen(outfile);
    m_outFilePath = new char[fileLenght+1];
    strcpy(m_outFilePath,outfile);
    if(fileLenght>4){
        char dest[4] = {""};
        char gifChar[] = {"gif"};
        strncpy(dest,m_outFilePath+(fileLenght-3),3);
        if(strcmp(dest,gifChar) == 0){
            gpMainFrame->m_InputStream.setVidioOutPutType(OUTPUT_TYPE::Gif);
           gpMainFrame->m_OutputStream.m_isGif = m_isGif = true;
        }
    }else{
        gpMainFrame->m_InputStream.setVidioOutPutType(OUTPUT_TYPE::Nomal);
       gpMainFrame->m_OutputStream.m_isGif = m_isGif = false;
    }
    int rc = pthread_create(&m_mainThread, NULL, preStartSteam,(void*)this);
    pthread_detach(m_mainThread);
}
void  MainLoop::addImage(QImage *img)
{
    if(m_canAddImage){
        gpMainFrame->m_InputStream.writeToFrame(img);
    }

}
int MainLoop:: OnStartStream()
{
    m_InputStream.SetVideoCaptureCB(VideoCaptureCallback);
    m_InputStream.SetAudioCaptureCB(AudioCaptureCallback);
    m_InputStream.SetAudioScardCaptureCB(AudioScardCaptureCallback);
    m_InputStream.SetWirteAmixtCB(writeAmixCallback);
    bool bRet;

    bRet = m_InputStream.OpenInputStream(); //初始化采集设备
    if(!bRet)
    {
        printf("打开采集设备失败\n");
        return 1;
    }

    int cx, cy, fps;
    AVPixelFormat pixel_fmt;
    //GetVideoInputInfo,获取相机capacity
    if(m_InputStream.GetVideoInputInfo(cx, cy, fps, pixel_fmt)) //获取视频采集源的信息
    {
        //cx:width cy:height  //CBR（固定码率控制）, VBR是动态码率,  平均码率ABR,
        //视频编码器常用的码率控制方式: abr(平均码率)，crf（限制码率），cqp（固定质量）
        m_OutputStream.SetVideoCodecProp(AV_CODEC_ID_H264, fps, 500000/*bps*/, 30/*GOP*/, cx, cy); //设置视频编码器属性
    }

    int sample_rate = 0, channels = 0,layout;
    AVSampleFormat  sample_fmt;
     AVFormatContext *ifmt_ctx;
     int istream_index;
    if(m_InputStream.GetAudioInputInfo(sample_fmt, sample_rate, channels,layout)) //获取音频采集源的信息
    {
        m_OutputStream.SetAudioCodecProp(AV_CODEC_ID_MP3, sample_rate, channels,layout, 32000); //AV_CODEC_ID_MP3 AV_CODEC_ID_AAC设置音频编码器属性
    }
    if(m_InputStream.GetAudioSCardInputInfo(sample_fmt, sample_rate, channels,layout)) //获取音频采集源的信息
    {
        m_OutputStream.SetAudioCardCodecProp(AV_CODEC_ID_MP3, sample_rate, channels,layout, 32000); //设置音频编码器属性
    }
    bRet  = m_OutputStream.OpenOutputStream(m_outFilePath); //设置输出路径
    if(!bRet)
    {
        printf("初始化输出失败\n");
        return 1;
    }

    //开始计时
    gettimeofday(&p_start, NULL );

    bRet = m_InputStream.StartCapture(); //开始采集


    if(!m_isGif){
        OnDestroy();
    }

    return 0;
}
void MainLoop::OnDestroy()
{
    m_frmCount = 0;
    m_nFPS = 0;
    m_OutputStream.CloseOutput();
    //结束计时
    gettimeofday(&p_end, NULL );
    long timeuse = ((p_end.tv_sec - p_start.tv_sec)*1000) + ((p_end.tv_usec - p_start.tv_usec)/1000);
//    printf("---采集用时：%ld ms\n", timeuse);
    m_isOverFlage = true;
}
void* MainLoop::preStartSteam(void* lParam){
    MainLoop * pThis = (MainLoop*)lParam;

    printf("mainThread\n");
    pThis->OnStartStream();
    return NULL;
}
int MainLoop:: stopStream()
{
    m_canAddImage = false;
    if(!m_isGif){
        sleep(2);
    }

    m_InputStream.CloseInputStream();
     m_OutputStream.setIsOverWrite(true);
     if(m_isGif){
         OnDestroy();
     }
//    m_InputStream.onFInishCleanImage();
    int times = 0;
    while(!m_isOverFlage){
        if(times>5){
            return 1;
        }
        sleep(1);
        times++;
    }
    return 0;
}



