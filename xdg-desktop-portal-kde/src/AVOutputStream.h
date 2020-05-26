#ifndef AVOUTPUTSTREAM_H
#define AVOUTPUTSTREAM_H

#include <string>
#include <assert.h>
#include <QMutex>


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
#include <libavutil/time.h> ////av_gettime()


#include "libavdevice/avdevice.h"
#include "libavfilter/buffersink.h"
//#include "libavfilter/avfilter.h"
//#include "libavfilter/buffersink.h"
//#include "libavfilter/buffersrc.h"
//#include "libavutil/audio_fifo.h"
#include "libavutil/avutil.h"
#include "libavutil/fifo.h"
#include "gif_encoder.h"
}

using namespace std;

class CAVOutputStream
{
public:
    CAVOutputStream(void);
    ~CAVOutputStream(void);

public:

    //初始化视频编码器
    void SetVideoCodecProp(AVCodecID codec_id, int framerate, int bitrate, int gopsize, int width, int height);

    //初始化音频编码器
    void SetAudioCodecProp(AVCodecID codec_id, int samplerate, int channels, int layout,int bitrate);
    void SetAudioCardCodecProp(AVCodecID codec_id, int samplerate, int channels,int layout, int bitrate);
    //创建编码器和混合器，打开输出
    bool  OpenOutputStream(const char* out_path);

    //写入一帧图像
    int  write_video_frame(AVStream *st, enum AVPixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp);

    //写入一帧音频
    int  write_audio_frame(AVStream *st, AVFrame *pframe, int64_t lTimeStamp);
    int write_filter_audio_frame(AVStream *&outst,AVCodecContext* &codecCtx_audio,AVFrame *&outframe);
    int  write_audio_card_frame(AVStream *st, AVFrame *pframe, int64_t lTimeStamp);

    //关闭输出
    void  CloseOutput();
    int init_filters();
    int init_context_amix(int channel, uint64_t channel_layout,int sample_rate,int64_t bit_rate);
    void startWriteAmixData();
    void setIsOverWrite(bool isCOntinue);

public:

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

    AVAudioFifo *audioFifoAlloc(enum AVSampleFormat sample_fmt, int channels,
                                int nb_samples);


    void audioFifoFree(AVAudioFifo *af);

    bool isWriteFrame();
    void setIsWriteFrame(bool isWriteFrame);

private:

    QMutex m_audioReadWriteMutex;
    QMutex m_writeFrameMutex;
    bool m_isWriteFrame;
    QMutex m_isWriteFrameMutex;


protected:
    AVStream* video_st;
    AVStream* audio_st;
    AVStream* audio_scard_st;
    AVStream* audio_amix_st;
    AVFormatContext *ofmt_ctx;
    AVCodecContext* pCodecCtx;  //video
    AVCodecContext* pCodecCtx_a;  //audio
    AVCodecContext* pCodecCtx_aCard;
    AVCodecContext* pCodecCtx_amix;
    AVCodec* pCodec;  //video
    AVCodec* pCodec_a;  //audio
    AVCodec* pCodec_aCard;
    AVCodec* pCodec_amix;
    AVFrame *pFrameYUV;   ///转换为YUV420P保存的图像
    struct SwsContext *img_convert_ctx;
    struct SwrContext *aud_convert_ctx;
    struct SwrContext *aud_card_convert_ctx;

    AVAudioFifo * m_fifo;
    AVAudioFifo * m_fifo_scard;
    int is_fifo_scardinit;
    int  m_vid_framecnt;
    int  m_aud_framecnt;
    int  m_aud_card_framecnt;
    int  m_nb_samples;

    int64_t m_first_vid_time1, m_first_vid_time2; //前者是采集视频的第一帧的时间，后者是编码器输出的第一帧的时间
    int64_t m_first_aud_time,m_first_aud_card_time,m_start_mix_time; //第一个音频帧的时间

    int64_t m_next_vid_time;
    int64_t m_next_aud_time;

    int64_t  m_nLastAudioPresentationTime; //记录上一帧的音频时间戳
    int64_t  m_nLastAudioCardPresentationTime;
    int64_t  m_nLastAudioMixPresentationTime;
    int64_t  m_mixCount;
    uint8_t ** m_converted_input_samples;
    uint8_t * m_out_buffer;
    uint8_t ** m_converted_input_samples_scard;
    AVFilterGraph *filter_graph;
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx1;
    AVFilterContext *buffersrc_ctx2;
    int tmpFifoFailed;
    bool m_isOverWrite;
public:
    string     m_output_path; //输出路径
    bool m_isGif;
    AVCodecID  m_video_codec_id;
    AVCodecID  m_audio_codec_id;
    AVCodecID  m_audio_card_codec_id;
    bool m_isMerge;
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
    GifInfo *gifInfo;
};

#endif //AVOUTPUTSTREAM_H
