//pts: （int64_t）显示时间，结合AVStream->time_base转换成时间戳
//dts: （int64_t）解码时间，结合AVStream->time_base转换成时间戳

#ifndef MAINLOOP_H
#define MAINLOOP_H

#include "AVInputStream.h"
#include "AVOutputStream.h"
#include <sys/time.h>
#include <map>
#include <qimage.h>
#include "waylandintegration.h"
#include "waylandintegration_p.h"
#define AUDIO_INPUT_DEVICE    "hw:0,0"        //hw:0,0
#define VIDEO_INPUT_DEVICE    "/dev/video0"   ///dev/video0

extern "C"
{
#include <libavdevice/avdevice.h>
}

using namespace std;
//namespace WaylandIntegration{
//}
enum StreamRecordAudioType{
    MIC,//只录制麦克风
    SYS,//之录制系统音频
    MIC_SYS,//录制麦克风+系统音频
     NOS //不录音
};

class MainLoop
{

public:
    MainLoop();
    virtual ~MainLoop();

public:
    void   CalculateFPS();
    bool   IsPreview() { return m_bPreview; }
    void   startInit(int img_width,int img_height,int fps,StreamRecordAudioType type,int cx,int cy,int cw,int ch,const char* outfile);
    void   addImage(QImage *img,int64_t time);

    void addImage(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame);

    int stopStream();
    void OnDestroy();
protected:
    void setRecordAudioType(StreamRecordAudioType type);
    void  setRecordAudioMic(bool isrecord);
    void  setRecordAudioSCard(bool isrecord);
     int OnStartStream();
    static void* preStartSteam(void* lParam);
    long              m_nChannelNum; //通道号

    uint64_t              m_frmCount;
    int              m_nFPS;
    bool              m_bPreview;
    char* m_outFilePath ;

    timeval p_start;
    timeval p_end;
    pthread_t  m_mainThread;

    int m_fps;
    bool m_isGif;
//    int m_iPWidth;
//    int m_iPHeight;
//     AVPixelFormat m_ipix_fmt;
public:
    CAVInputStream    m_InputStream;
    CAVOutputStream   m_OutputStream;
    bool m_isOverFlage;
    bool m_canAddImage; //是否正在结束
};


#endif // MAINLOOP_H
