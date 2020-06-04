//pts: （int64_t）显示时间，结合AVStream->time_base转换成时间戳
//dts: （int64_t）解码时间，结合AVStream->time_base转换成时间戳
#ifndef RECORDADMIN_H
#define RECORDADMIN_H
#include "AVInputStream.h"
#include "AVOutputStream.h"
#include "writeFrameThread.h"
#include <sys/time.h>
#include <map>
#include <qimage.h>
#include "AVInputStream.h"
#include "AVInputStream.h"
#include "waylandintegration.h"
#include "waylandintegration_p.h"
#include "writeFrameThread.h"
#define AUDIO_INPUT_DEVICE    "hw:0,0"        //hw:0,0
#define VIDEO_INPUT_DEVICE    "/dev/video0"   ///dev/video0
#include <QThread>

extern "C"
{
#include <libavdevice/avdevice.h>
}

using namespace std;
class RecordAdmin :public QObject
{
    Q_OBJECT

public:
    RecordAdmin(WaylandIntegration::WaylandIntegrationPrivate* context,QObject *parent = nullptr);
    virtual ~RecordAdmin();

public:
    void   CalculateFPS();
    //bool   IsPreview() { return m_bPreview; }

    /**
     * @brief init:初始化录屏管理
     * @param screenWidth:原图宽度
     * @param screenHeight:原图高度
     * @param fps:帧率
     * @param audioType:音频类型
     * @param x:x坐标
     * @param y:y坐标
     * @param selectWidth:选择的宽度
     * @param selectHeight:选择的高度
     * @param path:视频保存路径
     */
    void init(int screenWidth, int screenHeight, int fps, int audioType, int x, int y, int selectWidth, int selectHeight, const char* path);

    //void addImage(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame);
    int stopStream();
    //void OnDestroy();
protected:
    void  setRecordAudioType(int audioType);
    void  setMicAudioRecord(bool bRecord);
    void  setSysAudioRecord(bool bRecord);

    int startStream();
    static void* stream(void* param);
    long              m_nChannelNum; //通道号
    uint64_t              m_frmCount;
    int              m_nFPS;
    //bool              m_bPreview;
    char* m_outFilePath ;
    //timeval p_start;
    //timeval p_end;
    pthread_t  m_mainThread;
    int m_fps;
    //bool m_isGif;
public:
    CAVInputStream    *m_pInputStream;
    CAVOutputStream   *m_pOutputStream;
    WaylandIntegration::WriteFrameThread *m_writeFrameThread;
    //bool m_isOverFlage;
    //bool m_canAddImage; //是否正在结束

private:
    WaylandIntegration::WaylandIntegrationPrivate* m_context;
};

#endif // RECORDADMIN_H
