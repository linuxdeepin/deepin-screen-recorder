#ifndef AVOUTPUTSTREAM_H
#define AVOUTPUTSTREAM_H

#include <string>
#include <assert.h>
#include <QMutex>
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
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include "libavdevice/avdevice.h"
#include "libavfilter/buffersink.h"
#include "libavutil/avutil.h"
#include "libavutil/fifo.h"
}

using namespace std;

class CAVOutputStream
{
public:
    CAVOutputStream(WaylandIntegration::WaylandIntegrationPrivate* context);
    ~CAVOutputStream(void);

    //初始化视频编码器
    void SetVideoCodecProp(AVCodecID codec_id, int framerate, int bitrate, int gopsize, int width, int height);

    //初始化音频编码器
    void SetAudioCodecProp(AVCodecID codec_id, int samplerate, int channels, int layout,int bitrate);
    void SetAudioCardCodecProp(AVCodecID codec_id, int samplerate, int channels,int layout, int bitrate);

    //创建编码器和混合器，打开输出
    bool open(QString path);

    /**
     * @brief close:关闭输出
     */
    void close();

    //写入一帧图像
    //int  writeVideoFrame(AVStream *st, enum AVPixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp);

    /**
     * @brief writeVideoFrame:写视频帧
     * @param frame:rgb帧
     * @return
     */
    int writeVideoFrame(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame);
    int writeMicAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp);
    int writeMicToMixAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp);
    int writeSysAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp);
    int writeSysToMixAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp);
    int write_filter_audio_frame(AVStream *&outst,AVCodecContext* &codecCtx_audio,AVFrame *&outframe);
    int init_filters();
    int init_context_amix(int channel, uint64_t channel_layout,int sample_rate,int64_t bit_rate);
    void writeMixAudio();
    void setIsOverWrite(bool isCOntinue);

    /**
     * @brief audioRead:读音频，自动锁
     * @param af
     * @param data
     * @param nb_samples
     * @return
     */
    int audioRead(AVAudioFifo *af, void **data, int nb_samples);

    /**
     * @brief audioWrite:写音频，自动锁
     * @param af
     * @param data
     * @param nb_samples
     * @return
     */

    int audioWrite(AVAudioFifo *af, void **data, int nb_samples);

    /**
     * @brief writeFrame:写视频帧，自动锁
     * @param s
     * @param pkt
     * @return
     */
    int writeFrame(AVFormatContext *s, AVPacket *pkt);

    /**
     * @brief writeTrailer:写视频文件尾，自动锁
     * @param s
     * @return
     */
    int writeTrailer(AVFormatContext *s);
    int audioFifoSpace(AVAudioFifo *af);
    int audioFifoSize(AVAudioFifo *af);
    int audioFifoRealloc(AVAudioFifo *af, int nb_samples);
    AVAudioFifo *audioFifoAlloc(enum AVSampleFormat sample_fmt, int channels,int nb_samples);
    void audioFifoFree(AVAudioFifo *af);
    bool isWriteFrame();
    void setIsWriteFrame(bool isWriteFrame);

public:
    //截图区域
    int m_left;
    int m_top;
    int m_right;
    int m_bottom;

private:
    char *m_path;
    QMutex m_audioReadWriteMutex;
    QMutex m_writeFrameMutex;
    bool m_isWriteFrame;
    QMutex m_isWriteFrameMutex;
    WaylandIntegration::WaylandIntegrationPrivate* m_context;
    AVStream* m_videoStream;
    AVStream* m_micAudioStream;
    AVStream* m_sysAudioStream;
    AVStream* audio_amix_st;
    AVFormatContext *m_videoFormatContext;
    AVCodecContext* pCodecCtx;  //video
    AVCodecContext* m_pMicCodecContext;  //audio
    AVCodecContext* m_pSysCodecContext;
    AVCodecContext* pCodecCtx_amix;
    AVCodec* pCodec;  //videos
    AVCodec* pCodec_a;  //audio
    AVCodec* pCodec_aCard;
    AVCodec* pCodec_amix;
    AVFrame *pFrameYUV;   ///转换为YUV420P保存的图像
    struct SwsContext *m_pVideoSwsContext;
    struct SwrContext *m_pMicAudioSwrContext;
    struct SwrContext *m_pSysAudioSwrContext;
    AVAudioFifo * m_micAudioFifo;
    AVAudioFifo * m_sysAudioFifo;
    int is_fifo_scardinit;
    int  m_micAudioFrame;
    int  m_sysAudioFrame;
    int  m_nb_samples;
    //int64_t m_first_vid_time1, m_first_vid_time2; //前者是采集视频的第一帧的时间，后者是编码器输出的第一帧的时间
    int64_t m_start_mix_time; //第一个音频帧的时间
    int64_t m_next_vid_time;
    int64_t m_next_aud_time;
    int64_t  m_nLastAudioPresentationTime; //记录上一帧的音频时间戳
    int64_t  m_nLastAudioCardPresentationTime;
    int64_t  m_nLastAudioMixPresentationTime;
    int64_t  m_mixCount;
    uint8_t ** m_convertedMicSamples;
    uint8_t ** m_convertedSysSamples;
    uint8_t * m_out_buffer;
    AVFilterGraph *filter_graph;
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx1;
    AVFilterContext *buffersrc_ctx2;
    int tmpFifoFailed;
    bool m_isOverWrite;
    AVCodecID  m_videoCodecID;
    AVCodecID  m_micAudioCodecID;
    AVCodecID  m_sysAudioCodecID;
    bool m_bMix;
    AVFrame * mMic_frame;
    AVFrame * mSpeaker_frame;
    int m_width, m_height;
    int m_framerate;
    int m_video_bitrate;
    int m_gopsize;
    int m_samplerate;
    int m_channels;
    int m_channels_layout;
    int m_audio_bitrate;
    int m_samplerate_card;
    int m_channels_card;
    int m_channels_card_layout;
    int m_audio_bitrate_card;
};

#endif //AVOUTPUTSTREAM_H
