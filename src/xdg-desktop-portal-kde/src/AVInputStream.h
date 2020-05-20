#ifndef AVINPUTSTREAM_H
#define AVINPUTSTREAM_H

#include <string>
#include <assert.h>
#include <qimage.h>
#include <qqueue.h>
#include "waylandintegration.h"
#include "waylandintegration_p.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h> ////av_gettime()
}

using namespace std;

//typedef double (*VideoCaptureCB)(); //OK
typedef int (*VideoCaptureCB)(AVStream * input_st,  enum AVPixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp);
typedef int (*AudioCaptureCB)(AVStream *input_st, AVFrame *pframe, int64_t lTimeStamp);
typedef int (*AudioMixCB)();
//typedef LRESULT (CALLBACK* VideoCaptureCB)(AVStream * input_st, enum PixelFormat pix_fmt, AVFrame *pframe, INT64 lTimeStamp);
//typedef LRESULT (CALLBACK* AudioCaptureCB)(AVStream * input_st, AVFrame *pframe, INT64 lTimeStamp);

//struct ScreenData{
//    QImage *image;
//    int64_t timeStamp;
//};
enum OUTPUT_TYPE{
    Nomal = 0,
    Gif
};

class CAVInputStream
{
public:
    CAVInputStream(void);
    ~CAVInputStream(void);

public:
//    void  SetVideoCaptureDevice(string device_name);
//    void  SetAudioCaptureDevice(string device_name);
    void  setRecordAudioMic(bool isrecord);
    void  setRecordAudioSCard(bool isrecord);
    bool  OpenInputStream();
    void  CloseInputStream();
    void  onsFinisheStream();
    void onFInishCleanImage();
    bool  StartCapture();
    void writeToFrame(QImage *img,int64_t time);

    void writeToFrame(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame);

    void  SetVideoCaptureCB(VideoCaptureCB pFuncCB);
    void  SetAudioCaptureCB(AudioCaptureCB pFuncCB);
    void  SetAudioScardCaptureCB(AudioCaptureCB pFuncCB);
    void  SetWirteAmixtCB(AudioMixCB pFuncCB);
    bool  GetVideoInputInfo(int & width, int & height, int & framerate, AVPixelFormat & pixFmt);
    bool  GetAudioInputInfo(AVSampleFormat & sample_fmt, int & sample_rate, int & channels,int &layout);
    bool  GetAudioSCardInputInfo(AVSampleFormat & sample_fmt, int & sample_rate, int & channels,int &layout);
    void  setVidioOutPutType(OUTPUT_TYPE outType);
    OUTPUT_TYPE getVidioOutPutType();
    QString currentAudioChannel();
protected:
//    static /*unsigned long*/void  CaptureVideoThreadFunc(void* lParam);
//    static /*unsigned long*/void  CaptureAudioThreadFunc(void* lParam);
//    static void  *CaptureVideoThreadFunc(void* lParam);
    static void  *CaptureAudioThreadFunc(void* lParam);
    static void  *CaptureAudioSCardThreadFunc(void* lParam);
    static void  *writeAmixThreadFunc(void* lParam);
    int  beginWriteMixAudio();
    void doWritAmixAudio();
    int  ReadAudioPackets();
    int  ReadAudioSCardPackets();
    void initScreenData();
//protected:
public:
//    string  m_video_device;
//    string b;
    string  m_audio_device;
    string  m_audio_device_scard;
    OUTPUT_TYPE m_outPutType;
//    int     m_videoindex;
    int     m_audioindex;
    int     m_audioindex_scard;
    int m_cl;
    int m_ct;
    int m_cr;
    int m_cb;
    int m_cw;
    int m_ch;
    AVPixelFormat m_ipix_fmt; //输入图像格式
    int m_fps;
    int m_screenDW; //输入图像宽
    int m_screenDH; //输入图像高
//     QQueue<ScreenData> m_ScreenDatas;//输入图像集合

//    AVFormatContext *m_pVidFmtCtx;
    AVFormatContext *m_pAudFmtCtx; //音频context
    AVFormatContext *m_pAudFmtCtx_scard; //声卡音频context
//    AVInputFormat  *m_pInputFormat;
    AVInputFormat  *m_pAudioInputFormat;
    AVInputFormat  *m_pAudioCardInputFormat;

    AVPacket *dec_pkt;

    pthread_t  m_hCapAudioThread ,m_hCapAudioScardThread; //线程句柄
    pthread_t  m_hReadMixThread;
    bool   m_exit_thread; //退出线程的标志变量

    VideoCaptureCB  m_pVideoCBFunc; //视频数据回调函数指针
    AudioCaptureCB  m_pAudioCBFunc; //音频数据回调函数指针
    AudioCaptureCB  m_pAudioScardCBFunc; //声卡音频数据回调函数指针
    AudioMixCB   m_mixCBFunc;
///    CCritSec     m_WriteLock;
    //pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    //pthread_mutex_t mutexScreenD = PTHREAD_MUTEX_INITIALIZER;

/// pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

    int64_t     m_start_time; //采集的起点时间
    bool m_isMerge;
    bool m_isWriting;
};

#endif //AVINPUTSTREAM_H
