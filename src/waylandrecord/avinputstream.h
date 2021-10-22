#ifndef AVINPUTSTREAM_H
#define AVINPUTSTREAM_H

#include <string>
#include <assert.h>
#include <qimage.h>
#include <qqueue.h>
#include <QMutex>
//#include "waylandintegration.h"
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
#include <libavutil/time.h>
}

using namespace std;

class CAVInputStream
{
public:
    explicit CAVInputStream(WaylandIntegration::WaylandIntegrationPrivate *context);
    ~CAVInputStream(void);

public:
    void  setMicAudioRecord(bool bRecord);
    void  setSysAudioRecord(bool bRecord);
    bool  openInputStream();
    void  onsFinisheStream();
    bool  audioCapture();
    bool  GetVideoInputInfo(int &width, int &height, int &framerate, AVPixelFormat &pixFmt);
    bool  GetAudioInputInfo(AVSampleFormat &sample_fmt, int &sample_rate, int &channels, int &layout);
    bool  GetAudioSCardInputInfo(AVSampleFormat &sample_fmt, int &sample_rate, int &channels, int &layout);
    QString currentAudioChannel();

public:
    bool m_bMicAudio;
    bool m_bSysAudio;
    //OUTPUT_TYPE m_outPutType;
    int     m_micAudioindex;
    int     m_sysAudioindex;
    int m_left;
    int m_top;
    int m_right;
    int m_bottom;
    int m_selectWidth;
    int m_selectHeight;
    AVPixelFormat m_ipix_fmt; //输入图像格式
    int m_fps;
    int m_screenDW; //输入图像宽
    int m_screenDH; //输入图像高
    //麦克风音频上下文
    AVFormatContext *m_pMicAudioFormatContext;
    //系统声卡音频上下文
    AVFormatContext *m_pSysAudioFormatContext;
    /**
     * @brief m_pAudioInputFormat 麦克风音频输入格式
     */
    AVInputFormat  *m_pAudioInputFormat;
    /**
     * @brief m_pAudioCardInputFormat 系统音频输入格式
     */
    AVInputFormat  *m_pAudioCardInputFormat;
    AVPacket *dec_pkt;
    pthread_t  m_hMicAudioThread, m_hSysAudioThread; //线程句柄
    pthread_t  m_hMixThread;
    //VideoCaptureCB  m_pVideoCBFunc; //视频数据回调函数指针
    //AudioCaptureCB  m_pAudioCBFunc; //音频数据回调函数指针
    //AudioCaptureCB  m_pAudioScardCBFunc; //声卡音频数据回调函数指针
    //AudioMixCB   m_mixCBFunc;
    int64_t     m_start_time; //采集的起点时间
    bool m_bMix;
    bool bWriteMix();
    void setbWriteAmix(bool bWriteMix);
    bool bRunThread() ;
    void setbRunThread(bool bRunThread);

protected:
    static void *captureMicAudioThreadFunc(void *param);
    static void *captureMicToMixAudioThreadFunc(void *param);
    static void *captureSysAudioThreadFunc(void *param);
    static void *captureSysToMixAudioThreadFunc(void *param);
    static void  *writeMixThreadFunc(void *param);
    void writMixAudio();
    int  readMicAudioPacket();
    int  readMicToMixAudioPacket();
    int  readSysAudioPacket();
    int  readSysToMixAudioPacket();
    void initScreenData();

private:
    bool m_bWriteMix;
    QMutex m_bWriteMixMutex;
    bool m_bRunThread;
    QMutex m_bRunThreadMutex;
    WaylandIntegration::WaylandIntegrationPrivate *m_context;
};

#endif //AVINPUTSTREAM_H
