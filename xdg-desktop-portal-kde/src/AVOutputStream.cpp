/*
时间戳，DTS(decoding time stamp)，PTS(presention time stamp)，CTS（current time stamp）。
---
ffmepg中的时间戳，是以微秒为单位，关乎timebase变量，它是作为dts、pts的时间基准粒度，数值会很大。
---
其中函数av_rescale_q()是很多的，AV_ROUND_NEAR_INF是就近、中间从零，av_rescale_rnd它是计算a*b/c，
传入参数为八字节，为避免溢出，里面做了与INT_MAX的比较，分开计算。将以 "时钟基c" 表示的 数值a 转换成以 "时钟基b" 来表示。
---
FFmpeg中用AVPacket结构体来描述解码前或编码后的压缩包，用AVFrame结构体来描述解码后或编码前的信号帧。
对于视频来说，AVFrame就是视频的一帧图像。这帧图像什么时候显示给用户，就取决于它的PTS。
DTS是AVPacket里的一个成员，表示这个压缩包应该什么时候被解码。
如果视频里各帧的编码是按输入顺序（也就是显示顺序）依次进行的，那么解码和显示时间应该是一致的。
可事实上，在大多数编解码标准（如H.264或HEVC）中，编码顺序和输入顺序并不一致。 于是才会需要PTS和DTS这两种不同的时间戳。
*/

#include "AVOutputStream.h"
#include <unistd.h>
#include <QTime>
#include <QDebug>
#include <QThread>

CAVOutputStream::CAVOutputStream(WaylandIntegration::WaylandIntegrationPrivate *context):
    m_context(context),
    m_sysAudioFifo(nullptr),
    m_micAudioFifo(nullptr),
    m_pSysAudioSwrContext(nullptr)
{
    m_videoCodecID  = AV_CODEC_ID_NONE;
    m_micAudioCodecID = AV_CODEC_ID_NONE;
    m_sysAudioCodecID = AV_CODEC_ID_NONE;
    m_bMix = false;
    m_out_buffer = NULL;
    m_width = 320;
    m_height = 240;
    m_framerate = 25;
    m_video_bitrate = 500000;
    m_samplerate = 0;
    m_channels = 1;
    m_audio_bitrate = 32000;
    m_samplerate_card = 0;
    is_fifo_scardinit = 0;
    m_channels_card = 1;
    m_audio_bitrate_card = 32000;
    m_videoStream = NULL;
    m_micAudioStream = NULL;
    m_videoFormatContext = nullptr;
    pCodecCtx = NULL;
    m_pMicCodecContext = NULL;
    pCodec = NULL;
    pCodec_a = NULL;
    pCodec_aCard = NULL;
    pCodec_amix = NULL;
    pFrameYUV = NULL;
    m_pVideoSwsContext = nullptr;
    m_pMicAudioSwrContext = NULL;
    m_nb_samples = 0;
    m_convertedMicSamples = NULL;
    m_convertedSysSamples = NULL;
    m_path = "";
    m_micAudioFrame = 0;
    m_sysAudioFrame = 0;
    m_next_vid_time = 0;
    m_next_aud_time = 0;
    audio_amix_st =NULL;
    m_nLastAudioPresentationTime = 0;
    m_nLastAudioCardPresentationTime = 0;
    m_nLastAudioMixPresentationTime = 0;
    m_mixCount = 0;
    avcodec_register_all();
    av_register_all();
}

CAVOutputStream::~CAVOutputStream(void)
{
    printf("Desctruction Onput!\n");
    if (m_micAudioFifo)
    {
        audioFifoFree(m_micAudioFifo);
        m_micAudioFifo = NULL;
    }
    if (nullptr != m_sysAudioFifo)
    {
        audioFifoFree(m_sysAudioFifo);
        m_sysAudioFifo = nullptr;
    }
}



//初始化视频编码器
void CAVOutputStream::SetVideoCodecProp(AVCodecID codec_id, int framerate, int bitrate, int gopsize, int width, int height)
{
    m_videoCodecID  = codec_id;
    m_width = width;
    m_height = height;
    m_framerate = ((framerate == 0) ? 10 : framerate);
    m_video_bitrate = bitrate;
    m_gopsize = gopsize;
}

//初始化音频编码器
void CAVOutputStream::SetAudioCodecProp(AVCodecID codec_id, int samplerate, int channels,int layout ,int bitrate)
{
    m_micAudioCodecID = codec_id;
    m_samplerate = samplerate;
    m_channels = channels;
    m_channels_layout = layout;
    m_audio_bitrate = bitrate;
}
void CAVOutputStream::SetAudioCardCodecProp(AVCodecID codec_id, int samplerate, int channels, int layout ,int bitrate)
{
    m_sysAudioCodecID = codec_id;
    m_samplerate_card = samplerate;
    m_channels_card = channels;
    m_channels_card_layout = layout;
    m_audio_bitrate_card = bitrate;
}
int CAVOutputStream::init_filters()
{
    static const char *filter_descr = "[in0][in1]amix=inputs=2[out]";//"[in0][in1]amix=inputs=2[out]";amerge//"aresample=8000,aformat=sample_fmts=s16:channel_layouts=mono";
    char args1[512];
    char args2[512];
    int ret = 0;
    string formatStr = "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%";
    formatStr.append(PRIx64);
    AVFilterInOut* filter_outputs[2];
    const AVFilter *abuffersrc1  = avfilter_get_by_name("abuffer");
    const AVFilter *abuffersrc2  = avfilter_get_by_name("abuffer");
    const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    AVFilterInOut *outputs1 = avfilter_inout_alloc();
    AVFilterInOut *outputs2 = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    static  enum AVSampleFormat out_sample_fmts[] ={ pCodecCtx_amix->sample_fmt, AV_SAMPLE_FMT_NONE }; //{ AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    static const int64_t out_channel_layouts[] = { pCodecCtx_amix->channel_layout, -1 };
    static const int out_sample_rates[] = { pCodecCtx_amix->sample_rate, -1 };
    const AVFilterLink *outlink;
    AVRational time_base_1 = m_pMicCodecContext->time_base;
    AVRational time_base_2 = m_pSysCodecContext->time_base;
    filter_graph = avfilter_graph_alloc();
    if (!outputs1 || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        avfilter_inout_free(&inputs);

        avfilter_inout_free(&outputs1);
        return 1;
    }


    AVCodecContext *dec_ctx1;
    AVCodecContext *dec_ctx2;
    /* buffer audio source: the decoded frames from the decoder will be inserted here. */
    dec_ctx1 = m_pMicCodecContext;
    if (!dec_ctx1->channel_layout)
        dec_ctx1->channel_layout = av_get_default_channel_layout(dec_ctx1->channels);
    snprintf(args1, sizeof(args1),
             formatStr.c_str(),
             time_base_1.num, time_base_1.den, dec_ctx1->sample_rate,
             av_get_sample_fmt_name(dec_ctx1->sample_fmt), dec_ctx1->channel_layout);
    ret = avfilter_graph_create_filter(&buffersrc_ctx1, abuffersrc1, "in0",
                                       args1, NULL, filter_graph);

    if (ret < 0) {

        av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
        avfilter_inout_free(&inputs);

        avfilter_inout_free(&outputs1);
        return 1;
    }
    /* buffer audio source: the decoded frames from the decoder will be inserted here. */
    dec_ctx2 =m_pSysCodecContext;


    snprintf(args2, sizeof(args2),
             formatStr.c_str(),
             time_base_2.num, time_base_2.den, dec_ctx2->sample_rate,
             av_get_sample_fmt_name(dec_ctx2->sample_fmt), dec_ctx2->channel_layout);
    ret = avfilter_graph_create_filter(&buffersrc_ctx2, abuffersrc2, "in1",
                                       args2, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");

        avfilter_inout_free(&inputs);

        avfilter_inout_free(&outputs1);
        return 1;

    }

    /* buffer audio sink: to terminate the filter chain. */

    ret = avfilter_graph_create_filter(&buffersink_ctx, abuffersink, "out",

                                       NULL, NULL, filter_graph);

    if (ret < 0) {

        av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");

        avfilter_inout_free(&inputs);

        avfilter_inout_free(&outputs1);
        return 1;

    }



    ret = av_opt_set_int_list(buffersink_ctx, "sample_fmts", out_sample_fmts, -1,

                              AV_OPT_SEARCH_CHILDREN);

    if (ret < 0) {

        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");

        avfilter_inout_free(&inputs);

        avfilter_inout_free(&outputs1);
        return 1;

    }


    ret = av_opt_set_int_list(buffersink_ctx, "channel_layouts", out_channel_layouts, -1,

                              AV_OPT_SEARCH_CHILDREN);

    if (ret < 0) {

        av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");

        avfilter_inout_free(&inputs);

        avfilter_inout_free(&outputs1);
        return 1;

    }

    ret = av_opt_set_int_list(buffersink_ctx, "sample_rates", out_sample_rates, -1,

                              AV_OPT_SEARCH_CHILDREN);

    if (ret < 0) {

        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");

        avfilter_inout_free(&inputs);

        avfilter_inout_free(&outputs1);
        return 1;

    }

    /*

     * Set the endpoints for the filter graph. The filter_graph will

     * be linked to the graph described by filters_descr.

     */



    /*

     * The buffer source output must be connected to the input pad of

     * the first filter described by filters_descr; since the first

     * filter input label is not specified, it is set to "in" by

     * default.

     */

    outputs1->name       = av_strdup("in0");
    outputs1->filter_ctx = buffersrc_ctx1;
    outputs1->pad_idx    = 0;
    outputs1->next       = outputs2;
    outputs2->name       = av_strdup("in1");
    outputs2->filter_ctx = buffersrc_ctx2;
    outputs2->pad_idx    = 0;
    outputs2->next       = NULL;

    /*

     * The buffer sink input must be connected to the output pad of

     * the last filter described by filters_descr; since the last

     * filter output label is not specified, it is set to "out" by

     * default.

     */
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;


    filter_outputs[0] = outputs1;

    filter_outputs[1] = outputs2;




    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr,

                                        &inputs, filter_outputs, NULL)) < 0)//filter_outputs

    {

        av_log(NULL, AV_LOG_ERROR, "parse ptr fail, ret: %d\n", ret);

        avfilter_inout_free(&inputs);

        avfilter_inout_free(&outputs1);
        return 1;

    }



    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)

    {

        av_log(NULL, AV_LOG_ERROR, "config graph fail, ret: %d\n", ret);

        avfilter_inout_free(&inputs);

        avfilter_inout_free(&outputs1);
        return 1;
    }
    /* Print summary of the sink buffer

     * Note: args buffer is reused to store channel layout string */
    outlink = buffersink_ctx->inputs[0];

    av_get_channel_layout_string(args1, sizeof(args1), -1, outlink->channel_layout);

    av_log(NULL, AV_LOG_INFO, "Output: srate:%dHz fmt:%s chlayout:%s\n",

           (int)outlink->sample_rate,

           (char *)av_x_if_null(av_get_sample_fmt_name((AVSampleFormat)outlink->format), "?"),

           args1);
    mMic_frame = av_frame_alloc();
    mSpeaker_frame = av_frame_alloc();
    avfilter_inout_free(&inputs);
    avfilter_inout_free(filter_outputs);

    return ret;

}
int CAVOutputStream::init_context_amix(int channel, uint64_t channel_layout,int sample_rate,int64_t bit_rate){
    pCodec_amix = avcodec_find_encoder(m_sysAudioCodecID);
    if (!pCodec_amix)
    {
        printf("Can not find output audio encoder! (没有找到合适的编码器！)\n");
        return false;
    }

    pCodecCtx_amix = avcodec_alloc_context3(pCodec_amix);
    pCodecCtx_amix = avcodec_alloc_context3(pCodec_a);
    pCodecCtx_amix->channels = m_channels;
    pCodecCtx_amix->channel_layout = m_channels_layout;
    if (pCodecCtx_amix->channel_layout == 0)
    {
        pCodecCtx_amix->channel_layout = AV_CH_LAYOUT_STEREO;
        pCodecCtx_amix->channels = av_get_channel_layout_nb_channels(pCodecCtx_amix->channel_layout);
    }

    pCodecCtx_amix->sample_rate = m_samplerate;
    pCodecCtx_amix->sample_fmt = pCodec_amix->sample_fmts[0];
    pCodecCtx_amix->bit_rate = m_audio_bitrate;
    pCodecCtx_amix->time_base.num = 1;
    pCodecCtx_amix->time_base.den = pCodecCtx_amix->sample_rate;

    if(m_sysAudioCodecID == AV_CODEC_ID_AAC)
    {
        /** Allow the use of the experimental AAC encoder */
        pCodecCtx_amix->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    }

    /* Some formats want stream headers to be separate. */
    if (m_videoFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        pCodecCtx_amix->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(pCodecCtx_amix, pCodec_amix, NULL) < 0)
    {
        printf("Failed to open ouput audio encoder! (编码器打开失败！)\n");
        return false;
    }
    audio_amix_st = avformat_new_stream(m_videoFormatContext, pCodec_amix);
    //Add a new stream to output,should be called by the user before avformat_write_header() for muxing
    if(audio_amix_st)
    {
        //                audio_amix_st->index = 1;
        if (audio_amix_st == NULL)
        {
            return false;
        }
        audio_amix_st->time_base.num = 1;
        audio_amix_st->time_base.den = pCodecCtx_amix->sample_rate;//48000
        audio_amix_st->codec = pCodecCtx_amix;
    }
    return true;
}
//创建编码器和混合器
bool CAVOutputStream::open(const char* out_path)
{
    m_path = out_path;

    //output initialize
    avformat_alloc_output_context2(&m_videoFormatContext, NULL, NULL, out_path);

    if(m_videoCodecID  != 0)
    {
        printf("FLQQ,video encoder initialize\n\n");
        pCodec = avcodec_find_encoder(m_videoCodecID );

        if (!pCodec)
        {
            printf("Can not find output video encoder! (没有找到合适的编码器！)\n");
            return false;
        }

        pCodecCtx = avcodec_alloc_context3(pCodec);
        pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
#ifdef VIDEO_RESCALE
        pCodecCtx->width = m_width/2;
        pCodecCtx->height = m_height/2;
#else
        pCodecCtx->width = m_width;
        pCodecCtx->height = m_height;

#endif
        pCodecCtx->time_base.num = 1;
        pCodecCtx->time_base.den = m_framerate;
        //pCodecCtx->bit_rate = m_video_bitrate;
        //added by flq
        //pCodecCtx->flags |= CODEC_FLAG_QSCALE; //VBR（可变率控制）
        //added end
        pCodecCtx->gop_size = m_gopsize;
        /* Some formats want stream headers to be separate. */
        if (m_videoFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


        AVDictionary *param = 0;

        //set H264 codec param
        if(m_videoCodecID  == AV_CODEC_ID_H264)
        {
            pCodecCtx->qmin = 10;
            //low
            pCodecCtx->qmax = 51;
            pCodecCtx->max_b_frames = 0;

            // Set H264 preset and tune
            av_dict_set(&param, "preset", "ultrafast", 0);
            //                av_dict_set(&param, "tune", "zerolatency", 0);

        }

        if (avcodec_open2(pCodecCtx, pCodec, &param) < 0)
        {
            printf("Failed to open output video encoder! (编码器打开失败！)\n");
            return false;
        }

        //Add a new stream to output,should be called by the user before avformat_write_header() for muxing
        m_videoStream = avformat_new_stream(m_videoFormatContext, pCodec);
        if (m_videoStream == NULL)
        {
            return false;
        }
        m_videoStream->time_base.num = 1;
        m_videoStream->time_base.den = m_framerate;
        m_videoStream->codec = pCodecCtx;
        //Initialize the buffer to store YUV frames to be encoded.
        //
        pFrameYUV = av_frame_alloc();
        m_out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
        avpicture_fill((AVPicture *)pFrameYUV, m_out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

    }
    if(m_sysAudioCodecID && m_micAudioCodecID)
    {
        m_bMix = true;
        bool initSccess = init_context_amix(m_channels,0,0,m_audio_bitrate);
        if(!initSccess)
        {
            printf("Can not init_context_amix\n");
            return 1;
        }
    }
    ///音频
    if(m_micAudioCodecID != 0)
    {
        printf("FLQQ,audio encoder initialize\n\n");
        //output audio encoder initialize
        pCodec_a = avcodec_find_encoder(m_micAudioCodecID);
        if (!pCodec_a)
        {
            printf("Can not find output audio encoder! (没有找到合适的编码器！)\n");
            return false;
        }
        m_pMicCodecContext = avcodec_alloc_context3(pCodec_a);
        m_pMicCodecContext->channels = m_channels;
        //        pCodecCtx_a->channel_layout = av_get_default_channel_layout(m_channels);
        m_pMicCodecContext->channel_layout =m_channels_layout;
        //      pCodecCtx_a->channels = av_get_channel_layout_nb_channels(pAudioStream->codec->channel_layout);
        if (m_pMicCodecContext->channel_layout == 0)
        {
            m_pMicCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
            m_pMicCodecContext->channels = av_get_channel_layout_nb_channels(m_pMicCodecContext->channel_layout);
        }

        m_pMicCodecContext->sample_rate = m_samplerate;
        m_pMicCodecContext->sample_fmt = pCodec_a->sample_fmts[0];
        m_pMicCodecContext->bit_rate = m_audio_bitrate;
        m_pMicCodecContext->time_base.num = 1;
        m_pMicCodecContext->time_base.den = m_pMicCodecContext->sample_rate;

        if(m_micAudioCodecID == AV_CODEC_ID_AAC)
        {
            /** Allow the use of the experimental AAC encoder */
            m_pMicCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
        }

        /* Some formats want stream headers to be separate. */
        if (m_videoFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            m_pMicCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        if (avcodec_open2(m_pMicCodecContext, pCodec_a, NULL) < 0)
        {
            printf("Failed to open ouput audio encoder! (编码器打开失败！)\n");
            return false;
        }

        //Add a new stream to output,should be called by the user before avformat_write_header() for muxing
        if(!m_bMix)
        {
            m_micAudioStream = avformat_new_stream(m_videoFormatContext, pCodec_a);
            if (nullptr == m_micAudioStream)
                return false;
            m_micAudioStream->time_base.num = 1;
            m_micAudioStream->time_base.den = m_pMicCodecContext->sample_rate;//48000
            m_micAudioStream->codec = m_pMicCodecContext;
        }


        //Initialize the FIFO buffer to store audio samples to be encoded.

        //        m_fifo = av_audio_fifo_alloc(pCodecCtx_a->sample_fmt, pCodecCtx_a->channels, 30*pCodecCtx_a->frame_size);

        //Initialize the buffer to store converted samples to be encoded.
        m_convertedMicSamples = NULL;
        /**
        * Allocate as many pointers as there are audio channels.
        * Each pointer will later point to the audio samples of the corresponding
        * channels (although it may be NULL for interleaved formats).
        */
        if (!(m_convertedMicSamples = (uint8_t**)calloc(m_pMicCodecContext->channels, sizeof(**m_convertedMicSamples))))
        {
            printf("Could not allocate converted input sample pointers\n");
            return false;
        }
        m_convertedMicSamples[0] = NULL;
    }
    if(m_sysAudioCodecID != 0)
    {
        printf("FLQQ,audio encoder initialize\n\n");
        //output audio encoder initialize
        pCodec_aCard = avcodec_find_encoder(m_sysAudioCodecID);
        if (!pCodec_aCard)
        {
            printf("Can not find output audio encoder! (没有找到合适的编码器！)\n");
            return false;
        }
        m_pSysCodecContext = avcodec_alloc_context3(pCodec_aCard);
        m_pSysCodecContext->channels = m_channels_card;
        //        pCodecCtx_aCard->channel_layout = av_get_default_channel_layout(m_channels_card);
        m_pSysCodecContext->channel_layout = m_channels_card_layout;
        //                pCodecCtx_aCard->channels = av_get_channel_layout_nb_channels(pAudioStream->codec->channel_layout);
        if (m_pSysCodecContext->channel_layout == 0)
        {
            m_pSysCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
            m_pSysCodecContext->channels = av_get_channel_layout_nb_channels(m_pSysCodecContext->channel_layout);
        }
        m_pSysCodecContext->sample_rate = m_samplerate_card;
        m_pSysCodecContext->sample_fmt = pCodec_aCard->sample_fmts[0];
        m_pSysCodecContext->bit_rate = m_audio_bitrate_card;
        m_pSysCodecContext->time_base.num = 1;
        m_pSysCodecContext->time_base.den = m_pSysCodecContext->sample_rate;

        if(m_sysAudioCodecID == AV_CODEC_ID_AAC)
        {
            /** Allow the use of the experimental AAC encoder */
            m_pSysCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
        }

        /* Some formats want stream headers to be separate. */
        if (m_videoFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            m_pSysCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        if (avcodec_open2(m_pSysCodecContext, pCodec_aCard, NULL) < 0)
        {
            printf("Failed to open ouput audio encoder! (编码器打开失败！)\n");
            return false;
        }

        //Add a new stream to output,should be called by the user before avformat_write_header() for muxing
        if(!m_bMix)
        {
            m_sysAudioStream = avformat_new_stream(m_videoFormatContext, pCodec_aCard);
            if (m_sysAudioStream == NULL)
            {
                return false;
            }
            m_sysAudioStream->time_base.num = 1;
            m_sysAudioStream->time_base.den = m_pSysCodecContext->sample_rate;//48000
            m_sysAudioStream->codec = m_pSysCodecContext;
        }
        m_convertedSysSamples = NULL;
        if (!(m_convertedSysSamples = (uint8_t**)calloc(m_pSysCodecContext->channels, sizeof(**m_convertedSysSamples))))
        {
            printf("Could not allocate converted input sample pointers\n");
            return false;
        }
        m_convertedSysSamples[0] = NULL;
    }
    if(m_bMix)
    {
        if(init_filters()!=0)
        {
            return false;
        }
    }

    //Open output URL,set before avformat_write_header() for muxing
    if (avio_open(&m_videoFormatContext->pb, out_path, AVIO_FLAG_READ_WRITE) < 0)
    {
        printf("Failed to open output file! (输出文件打开失败！)\n");
        return false;
    }
    //Show some Information
    av_dump_format(m_videoFormatContext, 0, out_path, 1);

    //Write File Header
    avformat_write_header(m_videoFormatContext, NULL);

    //m_vid_framecnt = 0;
    m_micAudioFrame = 0;
    m_sysAudioFrame = 0;
    m_nb_samples = 0;
    m_nLastAudioPresentationTime = 0;
    m_nLastAudioMixPresentationTime = 0;
    m_mixCount = 0;
    m_next_vid_time = 0;
    m_next_aud_time = 0;
    //m_first_vid_time1 = m_first_vid_time2 = -1;
    //m_first_aud_time = -1;
    m_start_mix_time = -1;
    //m_isOverWrite = false;
    setIsWriteFrame(true);
    fflush(stdout);
    return true;
}

//input_st -- 输入流的信息
//input_frame -- 输入视频帧的信息
//lTimeStamp -- 时间戳，时间单位为1/1000000
//int CAVOutputStream::writeVideoFrame(AVStream * input_st, enum AVPixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp)
//{
//   if(m_first_vid_time1 == -1)
//   {
//       printf("First Video timestamp: %ld \n", lTimeStamp);
//       m_first_vid_time1 = lTimeStamp;
//   }

////    AVRational time_base_q = { 1, AV_TIME_BASE };

//    if(m_pVideoSwsContext == NULL)
//    {
//        if(m_isGif){
//            /*
//            img_convert_ctx = sws_getCachedContext(
//                            nullptr, m_width, m_height, AV_PIX_FMT_RGB32,
//                            m_width, m_height, video_st->codec->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
//                            */

//            //AV_PIX_FMT_ARGB,
//            //AV_PIX_FMT_RGBA,
//            //AV_PIX_FMT_ABGR,
//            //AV_PIX_FMT_BGRA,
//            m_pVideoSwsContext = sws_getContext(
//                            m_width, m_width, AV_PIX_FMT_ARGB,
//                            m_width, m_height,  AV_PIX_FMT_RGBA, SWS_BICUBIC, nullptr, nullptr, nullptr);

//        }else{
//            m_pVideoSwsContext = sws_getContext(m_width, m_height,
//                pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
//        }

//    }
//    if(av_frame_apply_cropping(pframe,AV_FRAME_CROP_UNALIGNED)<0){
//        AVERROR(ERANGE);
//        return 2;
//    }
//    if(m_isGif){

//        int bufferSize = avpicture_get_size(AV_PIX_FMT_RGBA, m_width, m_height);
//        uint8_t* buffer = (uint8_t*)malloc(bufferSize);
//        AVFrame* pFrameRGBA;
//        pFrameRGBA = av_frame_alloc();
//        if(pFrameRGBA == NULL){
//            return  2;
//        }

//        avpicture_fill((AVPicture*)pFrameRGBA, buffer, AV_PIX_FMT_RGBA, m_width, m_height);

//        sws_scale(m_pVideoSwsContext, pframe->data, pframe->linesize, 0, pframe->height, pFrameRGBA->data, pFrameRGBA->linesize);

//        basicReduceColor(gifInfo, (uint32_t*)buffer);
//        writeNetscapeExt(gifInfo);
//        graphicsControlExtension(gifInfo, 0);
//        imageDescriptor(gifInfo);
//        imageData(gifInfo, buffer);

//        av_free(pFrameRGBA);
//        free(buffer);
//        /*
//        sws_scale(img_convert_ctx, pframe->data, pframe->linesize, 0, pframe->height, pFrameYUV->data, pFrameYUV->linesize);
//        pFrameYUV->pts = m_mixCount++;

//        // encode video frame
//        AVPacket pkt;
//        pkt.data = nullptr;
//        pkt.size = 0;
//        int got_pkt;
//        av_init_packet(&pkt);
//        int ret = avcodec_encode_video2(video_st->codec, &pkt,  pFrameYUV, &got_pkt);

//        if (ret < 0) {
//            printf("fail to avcodec_encode_video2: ret=%d\n" ,ret);
//            return 1;
//        }
//        // rescale packet timestamp
//        pkt.duration = 1;
//        av_packet_rescale_ts(&pkt, video_st->codec->time_base, video_st->time_base);
//        av_write_frame(ofmt_ctx, &pkt);
////        std::cout << nb_frames << '\r' << std::flush;  // dump progress
////        ++nb_frames;
//        av_free_packet(&pkt);
//        */
//        return 0;
//    }
//    sws_scale(m_pVideoSwsContext, (const uint8_t* const*)pframe->data, pframe->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
//    pFrameYUV->height = pframe->height;
//    pFrameYUV->format = AV_PIX_FMT_YUV420P;

//    AVPacket enc_pkt;
//    enc_pkt.data = NULL;
//    enc_pkt.size = 0;
//    av_init_packet(&enc_pkt);

//    int ret;
//    int enc_got_frame = 0;

//    //tqq
//    //Param1: pCodecCtx, codec context
//    //Param2: enc_pkt,  output AVPacket
//    //Param3: AVFrame containing the raw video data to be encoded
//    ret = avcodec_encode_video2(pCodecCtx, &enc_pkt, pFrameYUV, &enc_got_frame);
////    enc_pkt.size();
//    if (enc_got_frame == 1)
//    {
//#ifdef DEBUG
//        printf("Succeed to encode frame: %5d\tsize:%5d\n", m_vid_framecnt, enc_pkt.size);
//#endif

//        if(m_first_vid_time2 == -1)
//        {
//            m_first_vid_time2 = lTimeStamp;
//        }

//        enc_pkt.stream_index = video_st->index;

//        //enc_pkt.pts= av_rescale_q(lTimeStamp, time_base_q, video_st->time_base);
//        enc_pkt.pts = (int64_t)video_st->time_base.den * lTimeStamp/AV_TIME_BASE;

//        m_vid_framecnt++;

//        /* write the compressed frame in the media file */

//        ret = writeFrame(ofmt_ctx, &enc_pkt);
//        if(ret < 0)
//        {
//            char tmpErrString[128] = {0};
//            printf("Could not write video frame, error: %s\n", av_make_error_string(tmpErrString, AV_ERROR_MAX_STRING_SIZE, ret));
//            av_packet_unref(&enc_pkt);
//            return ret;
//        }
//        av_free_packet(&enc_pkt);
//    }
//    else if(ret == 0)
//    {
//            printf("Buffer video frame, timestamp: %I64d.\n", lTimeStamp); //编码器缓冲帧
//    }
//    fflush(stdout);
//    return 0;
//}

int CAVOutputStream::writeVideoFrame(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame)
{
    AVFrame * pRgbFrame = av_frame_alloc();
    pRgbFrame->width = frame._width;
    pRgbFrame->height = frame._height;
    pRgbFrame->format = AV_PIX_FMT_RGB32;
    if(0 == av_frame_get_buffer(pRgbFrame,32))
    {
        pRgbFrame->width  = frame._width;
        pRgbFrame->height = frame._height;
        //pRgbFrame->format = AV_PIX_FMT_RGB32;
        pRgbFrame->crop_left   = m_left;
        pRgbFrame->crop_top    = m_top;
        pRgbFrame->crop_right  = m_right;
        pRgbFrame->crop_bottom = m_bottom;
        pRgbFrame->linesize[0] = frame._stride;
        pRgbFrame->data[0]     = frame._frame;
    }
    if(nullptr == m_pVideoSwsContext)
    {
        m_pVideoSwsContext = sws_getContext(m_width, m_height,
                                            AV_PIX_FMT_RGB32,
                                            pCodecCtx->width,
                                            pCodecCtx->height,
                                            AV_PIX_FMT_YUV420P,
                                            SWS_BICUBIC,
                                            nullptr,
                                            nullptr,
                                            nullptr);
    }
    if(av_frame_apply_cropping(pRgbFrame,AV_FRAME_CROP_UNALIGNED)<0)
    {
        AVERROR(ERANGE);
        return 2;
    }
    sws_scale(m_pVideoSwsContext, pRgbFrame->data, pRgbFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
    pFrameYUV->width  = pRgbFrame->width;//test
    pFrameYUV->height = pRgbFrame->height;
    pFrameYUV->format = AV_PIX_FMT_YUV420P;
    AVPacket packet;
    packet.data = nullptr;
    packet.size = 0;
    av_init_packet(&packet);
    int enc_got_frame = 0;
    avcodec_encode_video2(pCodecCtx, &packet, pFrameYUV, &enc_got_frame);
    if (1 == enc_got_frame)
    {
        packet.stream_index = m_videoStream->index;
        packet.pts = (int64_t)m_videoStream->time_base.den * frame._time/AV_TIME_BASE;
        int ret = writeFrame(m_videoFormatContext, &packet);
        if(ret < 0)
        {
            char tmpErrString[128] = {0};
            printf("Could not write video frame, error: %s\n", av_make_error_string(tmpErrString, AV_ERROR_MAX_STRING_SIZE, ret));
            av_packet_unref(&packet);
            return ret;
        }
    }
    av_free_packet(&packet);
    av_frame_free(&pRgbFrame);
    fflush(stdout);
    return 0;
}

//input_st -- 输入流的信息
//input_frame -- 输入音频帧的信息
//lTimeStamp -- 时间戳，时间单位为1/1000000
//
int CAVOutputStream::writeMicAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp)
{
    if(nullptr == m_micAudioStream)
        return -1;

    const int frameSize = m_pMicCodecContext->frame_size;
    int ret;
    if(nullptr == m_pMicAudioSwrContext)
    {
        // Initialize the resampler to be able to convert audio sample formats
        m_pMicAudioSwrContext = swr_alloc_set_opts(nullptr,
                                                   av_get_default_channel_layout(m_pMicCodecContext->channels),
                                                   m_pMicCodecContext->sample_fmt,
                                                   m_pMicCodecContext->sample_rate,
                                                   av_get_default_channel_layout(stream->codec->channels),
                                                   stream->codec->sample_fmt,
                                                   stream->codec->sample_rate,
                                                   0,
                                                   nullptr);
        /**
        * Perform a sanity check so that the number of converted samples is
        * not greater than the number of samples to be converted.
        * If the sample rates differ, this case has to be handled differently
        */
        assert(m_pMicCodecContext->sample_rate == stream->codec->sample_rate);
        swr_init(m_pMicAudioSwrContext);
        if(nullptr == m_micAudioFifo)
        {
            m_micAudioFifo = audioFifoAlloc(m_pMicCodecContext->sample_fmt, m_pMicCodecContext->channels, 20*inputFrame->nb_samples);
        }
        is_fifo_scardinit++;
    }

    /**
    * Allocate memory for the samples of all channels in one consecutive
    * block for convenience.
    */

    if ((ret = av_samples_alloc(m_convertedMicSamples, nullptr, m_pMicCodecContext->channels, inputFrame->nb_samples, m_pMicCodecContext->sample_fmt, 0)) < 0)
    {
        printf("Could not allocate converted input samples\n");
        av_freep(&(*m_convertedMicSamples)[0]);
        free(*m_convertedMicSamples);
        return ret;
    }

    /**
    * Convert the input samples to the desired output sample format.
    * This requires a temporary storage provided by converted_input_samples.
    */
    /** Convert the samples using the resampler. */
    //extended_data -> m_convertedMicSamples
    if ((ret = swr_convert(m_pMicAudioSwrContext,m_convertedMicSamples, inputFrame->nb_samples,(const uint8_t**)inputFrame->extended_data,inputFrame->nb_samples)) < 0)
    {
        printf("Could not convert input samples\n");
        return ret;
    }
    AVRational rational;
    int audioSize = audioFifoSize(m_micAudioFifo);
    //因为Fifo里有之前未读完的数据，所以从Fifo队列里面取出的第一个音频包的时间戳等于当前时间减掉缓冲部分的时长
    int64_t timeshift = (int64_t)audioSize * AV_TIME_BASE /(int64_t)(stream->codec->sample_rate);
    m_micAudioFrame += inputFrame->nb_samples;
    /** Add the converted input samples to the FIFO buffer for later processing. */

    /**
    * Make the FIFO as large as it needs to be to hold both,
    * the old and the new samples.
    */
    if ((ret = audioFifoRealloc(m_micAudioFifo, audioFifoSize(m_micAudioFifo) + inputFrame->nb_samples)) < 0)
    {
        printf("Could not reallocate FIFO\n");
        return ret;
    }

    /** Store the new samples in the FIFO buffer. */
    //write m_convertedMicSamples
    if (audioWrite(m_micAudioFifo, (void **)m_convertedMicSamples, inputFrame->nb_samples) < inputFrame->nb_samples)
    {
        printf("Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    int64_t timeinc = (int64_t)m_pMicCodecContext->frame_size * AV_TIME_BASE /(int64_t)(stream->codec->sample_rate);
    //当前帧的时间戳不能小于上一帧的值
    if(lTimeStamp - timeshift > m_nLastAudioPresentationTime )
    {
        m_nLastAudioPresentationTime = lTimeStamp - timeshift;
    }
    /**
    * Take one frame worth of audio samples from the FIFO buffer,
    * encode it and write it to the output file.
    */
    while (audioFifoSize(m_micAudioFifo) >= frameSize)
    {
        /** Temporary storage of the output samples of the frame written to the file. */
        AVFrame *outputFrame = av_frame_alloc();
        if (!outputFrame)
        {
            return AVERROR(ENOMEM);
        }
        /**
        * Use the maximum number of possible samples per frame.
        * If there is less than the maximum possible frame size in the FIFO
        * buffer use this number. Otherwise, use the maximum possible frame size
        */
        const int frame_size = FFMIN(audioFifoSize(m_micAudioFifo), m_pMicCodecContext->frame_size);
        /** Initialize temporary storage for one output frame. */
        /**
        * Set the frame's parame-ters, especially its size and format.
        * av_frame_get_buffer needs this to allocate memory for the
        * audio samples of the frame.
        * Default channel layouts based on the number of channels
        * are assumed for simplicity.
        */
        outputFrame->nb_samples = frame_size;
        outputFrame->channel_layout = m_pMicCodecContext->channel_layout;
        outputFrame->format = m_pMicCodecContext->sample_fmt;
        outputFrame->sample_rate = m_pMicCodecContext->sample_rate;
        outputFrame->pts = av_rescale_q(m_nLastAudioPresentationTime, rational, m_micAudioStream->time_base);
        /**
        * Allocate the samples of the created frame. This call will make
        * sure that the audio frame can hold as many samples as specified.
        */
        if ((ret = av_frame_get_buffer(outputFrame, 0)) < 0)
        {
            printf("Could not allocate output frame samples\n");
            av_frame_free(&outputFrame);
            return ret;
        }

        /**
        * Read as many samples from the FIFO buffer as required to fill the frame.
        * The samples are stored in the frame temporarily.
        */
        //read -> outputFrame->data
        if (audioRead(m_micAudioFifo, (void **)outputFrame->data, frame_size) < frame_size)
        {
            printf("Could not read data from FIFO\n");
            return AVERROR_EXIT;
        }
        //int ret = 0;
        /** Encode one frame worth of audio samples. */
        /** Packet used for temporary storage. */
        AVPacket outputPacket;
        av_init_packet(&outputPacket);
        outputPacket.data = nullptr;
        outputPacket.size = 0;
        int enc_got_frame_a = 0;
        /**
            * Encode the audio frame and store it in the temporary packet.
            * The output audio stream encoder is used to do this.
            */
        //outputFrame -> outputPacket
        if ((ret = avcodec_encode_audio2(m_pMicCodecContext, &outputPacket, outputFrame, &enc_got_frame_a)) < 0)
        {
            printf("Could not encode frame\n");
            av_packet_unref(&outputPacket);
            return ret;
        }
        /** Write one audio frame from the temporary packet to the output file. */
        if (enc_got_frame_a)
        {
            outputPacket.stream_index = m_micAudioStream->index;
            printf("output_packet.stream_index1  audio_st =%d\n", outputPacket.stream_index);
            outputPacket.pts = av_rescale_q(m_nLastAudioPresentationTime, rational, m_micAudioStream->time_base);
            //write -> outputPacket
            if ((ret = writeFrame(m_videoFormatContext, &outputPacket)) < 0)
            {
                char tmpErrString[128] = {0};
                printf("Could not write audio frame, error: %s\n", av_make_error_string(tmpErrString, AV_ERROR_MAX_STRING_SIZE, ret));
                av_packet_unref(&outputPacket);
                return ret;
            }
            av_packet_unref(&outputPacket);
        }
        m_nb_samples += outputFrame->nb_samples;
        m_nLastAudioPresentationTime += timeinc;
        av_frame_free(&outputFrame);
    }// -- while end
    return 0;
}

int CAVOutputStream::writeMicToMixAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp)
{
    int ret;
    if(nullptr == m_pMicAudioSwrContext)
    {
        // Initialize the resampler to be able to convert audio sample formats
        m_pMicAudioSwrContext = swr_alloc_set_opts(nullptr,
                                                   av_get_default_channel_layout(m_pMicCodecContext->channels),
                                                   m_pMicCodecContext->sample_fmt,
                                                   m_pMicCodecContext->sample_rate,
                                                   av_get_default_channel_layout(stream->codec->channels),
                                                   stream->codec->sample_fmt,
                                                   stream->codec->sample_rate,
                                                   0,
                                                   nullptr);
        assert(m_pMicCodecContext->sample_rate == stream->codec->sample_rate);
        swr_init(m_pMicAudioSwrContext);
        if(nullptr == m_micAudioFifo)
        {
            m_micAudioFifo = audioFifoAlloc(m_pMicCodecContext->sample_fmt, m_pMicCodecContext->channels, 20*inputFrame->nb_samples);
        }
        is_fifo_scardinit++;
    }
    if ((ret = av_samples_alloc(m_convertedMicSamples, nullptr, m_pMicCodecContext->channels, inputFrame->nb_samples, m_pMicCodecContext->sample_fmt, 0)) < 0)
    {
        printf("Could not allocate converted input samples\n");
        av_freep(&(*m_convertedMicSamples)[0]);
        free(*m_convertedMicSamples);
        return ret;
    }
    if ((ret = swr_convert(m_pMicAudioSwrContext,m_convertedMicSamples, inputFrame->nb_samples,(const uint8_t**)inputFrame->extended_data,inputFrame->nb_samples)) < 0)
    {
        printf("Could not convert input samples\n");
        return ret;
    }
    int fifoSpace = audioFifoSpace(m_micAudioFifo);
    int checkTime = 0;
    while(fifoSpace < inputFrame->nb_samples
          && isWriteFrame()
          && checkTime <= 1000)
    {
        fifoSpace = audioFifoSpace(m_micAudioFifo);
        usleep(10*1000);
        checkTime ++;
        //printf("_fifo_spk full  m_fifo!\n");
    }
    if (fifoSpace >= inputFrame->nb_samples)
    {
        //write m_convertedMicSamples
        if (audioWrite(m_micAudioFifo, (void **)m_convertedMicSamples, inputFrame->nb_samples) < inputFrame->nb_samples)
        {
            printf("Could not write data to FIFO\n");
            return AVERROR_EXIT;
        }
        printf("_fifo_spk write secceffull  m_fifo!\n");
    }
    return 0;
}
int CAVOutputStream::write_filter_audio_frame(AVStream *&outst,AVCodecContext* &codecCtx_audio,AVFrame *&output_frame)
{
    int ret = 0;
    /** Encode one frame worth of audio samples. */
    /** Packet used for temporary storage. */
    AVPacket output_packet;
    av_init_packet(&output_packet);
    output_packet.data = NULL;
    output_packet.size = 0;

    int enc_got_frame_a = 0;

    /**
     * Encode the audio frame and store it in the temporary packet.
     * The output audio stream encoder is used to do this.
     */
    if ((ret = avcodec_encode_audio2(codecCtx_audio, &output_packet, output_frame, &enc_got_frame_a)) < 0)
    {
        printf("Could not encode frame\n");
        av_packet_unref(&output_packet);
        return ret;
    }




    /** Write one audio frame from the temporary packet to the output file. */
    if (enc_got_frame_a)
    {
        //output_packet.flags |= AV_PKT_FLAG_KEY;
        output_packet.stream_index = outst->index;
        printf("output_packet.stream_index2  audio_st =%d\n", output_packet.stream_index);
#if 0
        AVRational r_framerate1 = { input_st->codec->sample_rate, 1 };// { 44100, 1};
        //int64_t_t calc_duration = (double)(AV_TIME_BASE)*(1 / av_q2d(r_framerate1));  //内部时间戳
        int64_t_t calc_pts = (double)m_nb_samples * (AV_TIME_BASE)*(1 / av_q2d(r_framerate1));

        output_packet.pts = av_rescale_q(calc_pts, time_base_q, audio_st->time_base);
        //output_packet.dts = output_packet.pts;
        //output_packet.duration = output_frame->nb_samples;
#else
        //         output_packet.pts = av_rescale_q(lastAudioPresentationTime, time_base_q, outst->time_base);

#endif


        if ((ret = writeFrame(m_videoFormatContext, &output_packet)) < 0)
        {
            char tmpErrString[128] = {0};
            printf("Could not write audio frame, error: %s\n", av_make_error_string(tmpErrString, AV_ERROR_MAX_STRING_SIZE, ret));
            av_packet_unref(&output_packet);
            return ret;
        }

        av_packet_unref(&output_packet);
    }//if (enc_got_frame_a)


    //     m_nb_samples += output_frame->nb_samples;

    av_frame_unref(output_frame);
    //     av_frame_free(&output_frame);
    return ret;
}

void CAVOutputStream::writeMixAudio()
{
    if(is_fifo_scardinit <2 || nullptr == m_sysAudioFifo || nullptr == m_micAudioFifo)
        return;
    AVFrame* pFrame_scard = av_frame_alloc();
    AVFrame* pFrame_temp = av_frame_alloc();
    int sysFifosize = audioFifoSize(m_sysAudioFifo);
    int micFifoSize = audioFifoSize(m_micAudioFifo);
    int minMicFrameSize = m_pMicCodecContext->frame_size;
    int minSysFrameSize = m_pSysCodecContext->frame_size;
    int ret;
    qDebug() << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << micFifoSize << minMicFrameSize << sysFifosize << minSysFrameSize;
    if (micFifoSize >= minMicFrameSize && sysFifosize >= minSysFrameSize)
    {
        tmpFifoFailed = 0;
        AVRational time_base_q;
        pFrame_scard->nb_samples = minSysFrameSize;
        pFrame_scard->channel_layout = m_pSysCodecContext->channel_layout;
        pFrame_scard->format = m_pSysCodecContext->sample_fmt;
        pFrame_scard->sample_rate = m_pSysCodecContext->sample_rate;
        av_frame_get_buffer(pFrame_scard, 0);
        pFrame_temp->nb_samples = minMicFrameSize;
        pFrame_temp->channel_layout = m_pMicCodecContext->channel_layout;
        pFrame_temp->format = m_pMicCodecContext->sample_fmt;
        pFrame_temp->sample_rate = m_pMicCodecContext->sample_rate;
        av_frame_get_buffer(pFrame_temp, 0);
        ret = audioRead(m_sysAudioFifo, (void**)pFrame_scard->data, minSysFrameSize);
        ret = audioRead(m_micAudioFifo, (void**)pFrame_temp->data, minMicFrameSize);
        int nFifoSamples = pFrame_scard->nb_samples;
        if(m_start_mix_time == -1)
        {
            printf("First Audio timestamp: %ld \n", av_gettime());
            m_start_mix_time = av_gettime();
        }
        int64_t lTimeStamp = av_gettime() - m_start_mix_time;
        int64_t timeshift = (int64_t)nFifoSamples * AV_TIME_BASE /(int64_t)(audio_amix_st->codec->sample_rate);
        if(lTimeStamp - timeshift > m_nLastAudioMixPresentationTime )
        {
            m_nLastAudioMixPresentationTime = lTimeStamp - timeshift;
        }
        pFrame_scard->pts = av_rescale_q(m_nLastAudioMixPresentationTime, time_base_q, audio_amix_st->time_base);
        pFrame_temp->pts = pFrame_scard->pts;
        int64_t timeinc = (int64_t)pCodecCtx_amix->frame_size * AV_TIME_BASE /(int64_t)(audio_amix_st->codec->sample_rate);
        m_nLastAudioMixPresentationTime += timeinc;
        ret = av_buffersrc_add_frame_flags(buffersrc_ctx1, pFrame_temp,0);
        if (ret < 0)
        {
            printf("Mixer: failed to call av_buffersrc_add_frame (speaker)\n");
            return;
        }
        ret = av_buffersrc_add_frame_flags(buffersrc_ctx2, pFrame_scard,0);
        if (ret < 0)
        {
            printf("Mixer: failed to call av_buffersrc_add_frame (microphone)\n");
            return;
        }
        while (isWriteFrame())
        {
            AVFrame* pFrame_out = av_frame_alloc();
            //AVERROR(EAGAIN) 返回这个表示还没转换完成既 不存在帧，则返回AVERROR（EAGAIN）
            ret = av_buffersink_get_frame(buffersink_ctx, pFrame_out);
            if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                //printf("%d %d \n",AVERROR(EAGAIN),AVERROR_EOF);
            }
            if (ret < 0)
            {
                printf("Mixer: failed to call av_buffersink_get_frame_flags ret : %d \n",ret);
                break;
            }
            //            printf("-------audio pFrame_out->channel_layout=%d\n",pFrame_out->channel_layout);
            //            printf("-------audio pFrame_out->nb_samples=%d\n",pFrame_out->nb_samples);
            //            printf("-------audio pFrame_out.format=%d\n",pFrame_out->format);
            //            printf("-------audio pFrame_out.sample_rate=%d\n",pFrame_out->sample_rate);
            //            printf("-------audio pFrame_out.pts=%d\n",pFrame_out->pts);
            //            printf("-------audio pFrame_out.size=%d\n",pFrame_out->pkt_size);
            fflush(stdout);
            if (pFrame_out->data[0] != NULL)
            {
                AVPacket packet_out;
                av_init_packet(&packet_out);
                packet_out.data = NULL;
                packet_out.size = 0;
                int got_packet_ptr;
                ret = avcodec_encode_audio2(pCodecCtx_amix, &packet_out, pFrame_out, &got_packet_ptr);
                if (ret < 0)
                {
                    printf("Mixer: failed to call avcodec_decode_audio4\n");
                    break;
                }
                if (got_packet_ptr)
                {
                    packet_out.stream_index = audio_amix_st->index;
                    packet_out.pts = m_mixCount * pCodecCtx_amix->frame_size;
                    packet_out.dts = packet_out.pts;
                    packet_out.duration = pCodecCtx_amix->frame_size;
                    packet_out.pts = av_rescale_q_rnd(packet_out.pts,
                                                      pCodecCtx_amix->time_base,
                                                      pCodecCtx_amix->time_base,
                                                      (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                    packet_out.dts = packet_out.pts;
                    packet_out.duration = av_rescale_q_rnd(packet_out.duration,
                                                           pCodecCtx_amix->time_base,
                                                           audio_amix_st->time_base,
                                                           (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                    m_mixCount++;
                    ret = writeFrame(m_videoFormatContext, &packet_out);
                    if (ret < 0)
                    {
                        printf("Mixer: failed to call av_interleaved_write_frame\n");
                    }
                    printf("-------Mixer: write frame to file got_packet_ptr =%d\n",got_packet_ptr);
                }
                av_free_packet(&packet_out);
            }
            av_frame_free(&pFrame_out);
        }
    }
    else
    {
        tmpFifoFailed++;
        usleep(20*1000);
        if (tmpFifoFailed > 300)
        {
            usleep(30*1000);
            return;
        }
    }
    av_frame_free(&pFrame_scard);
    av_frame_free(&pFrame_temp);
}

int  CAVOutputStream::writeSysAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp)
{
    if(nullptr == m_sysAudioStream)
        return -1;
    const int output_frame_size = m_pSysCodecContext->frame_size;
    int ret;
    //因为Fifo里有之前未读完的数据，所以从Fifo队列里面取出的第一个音频包的时间戳等于当前时间减掉缓冲部分的时长
    if(nullptr == m_pSysAudioSwrContext)
    {
        // Initialize the resampler to be able to convert audio sample formats
        m_pSysAudioSwrContext = swr_alloc_set_opts(nullptr,
                                                   av_get_default_channel_layout(m_pSysCodecContext->channels),
                                                   m_pSysCodecContext->sample_fmt,
                                                   m_pSysCodecContext->sample_rate,
                                                   av_get_default_channel_layout(stream->codec->channels),
                                                   stream->codec->sample_fmt,
                                                   stream->codec->sample_rate,
                                                   0,
                                                   nullptr);

        /**
        * Perform a sanity check so that the number of converted samples is
        * not greater than the number of samples to be converted.
        * If the sample rates differ, this case has to be handled differently
        */
        assert(m_pSysCodecContext->sample_rate == stream->codec->sample_rate);
        swr_init(m_pSysAudioSwrContext);
        if(nullptr == m_sysAudioFifo)
        {
            m_sysAudioFifo = audioFifoAlloc(m_pSysCodecContext->sample_fmt, m_pSysCodecContext->channels, 20*inputFrame->nb_samples);
        }
        is_fifo_scardinit ++;
    }
    /**
    * Allocate memory for the samples of all channels in one consecutive
    * block for convenience.
    */

    if ((ret = av_samples_alloc(m_convertedSysSamples, nullptr, m_pSysCodecContext->channels, inputFrame->nb_samples, m_pSysCodecContext->sample_fmt, 0)) < 0)
    {
        av_freep(&(*m_convertedSysSamples)[0]);
        free(*m_convertedSysSamples);
        return ret;
    }

    /**
    * Convert the input samples to the desired output sample format.
    * This requires a temporary storage provided by converted_input_samples.
    */
    /** Convert the samples using the resampler. */
    if ((ret = swr_convert(m_pSysAudioSwrContext,m_convertedSysSamples, inputFrame->nb_samples,(const uint8_t**)inputFrame->extended_data, inputFrame->nb_samples)) < 0)
    {
        return ret;
    }
    AVRational rational = {1, AV_TIME_BASE };
    int audioSize = audioFifoSize(m_sysAudioFifo);
    int64_t timeshift = (int64_t)audioSize * AV_TIME_BASE /(int64_t)(stream->codec->sample_rate);
    m_sysAudioFrame += inputFrame->nb_samples;
    /** Add the converted input samples to the FIFO buffer for later processing. */
    /**
    * Make the FIFO as large as it needs to be to hold both,
    * the old and the new samples.
    */
    if ((ret = audioFifoRealloc(m_sysAudioFifo, audioFifoSize(m_sysAudioFifo) + inputFrame->nb_samples)) < 0)
    {
        printf("Could not reallocate FIFO\n");
        return ret;
    }

    /** Store the new samples in the FIFO buffer. */
    if (audioWrite(m_sysAudioFifo, (void **)m_convertedSysSamples, inputFrame->nb_samples) < inputFrame->nb_samples)
    {
        printf("Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    int64_t timeinc = (int64_t)m_pSysCodecContext->frame_size * AV_TIME_BASE /(int64_t)(stream->codec->sample_rate);
    //当前帧的时间戳不能小于上一帧的值
    if(lTimeStamp - timeshift > m_nLastAudioCardPresentationTime )
    {
        m_nLastAudioCardPresentationTime = lTimeStamp - timeshift;
    }
    /**
        * Take one frame worth of audio samples from the FIFO buffer,
        * encode it and write it to the output file.
        */
    while (audioFifoSize(m_sysAudioFifo) >= output_frame_size)
    {
        /** Temporary storage of the output samples of the frame written to the file. */
        AVFrame *output_frame = av_frame_alloc();
        if (!output_frame)
        {
            return AVERROR(ENOMEM);
        }
        /**
            * Use the maximum number of possible samples per frame.
            * If there is less than the maximum possible frame size in the FIFO
            * buffer use this number. Otherwise, use the maximum possible frame size
            */
        const int frame_size = FFMIN(audioFifoSize(m_sysAudioFifo), m_pSysCodecContext->frame_size);
        /** Initialize temporary storage for one output frame. */
        /**
            * Set the frame's parameters, especially its size and format.
            * av_frame_get_buffer needs this to allocate memory for the
            * audio samples of the frame.
            * Default channel layouts based on the number of channels
            * are assumed for simplicity.
            */
        output_frame->nb_samples = frame_size;
        output_frame->channel_layout = m_pSysCodecContext->channel_layout;
        output_frame->format = m_pSysCodecContext->sample_fmt;
        output_frame->sample_rate = m_pSysCodecContext->sample_rate;
        output_frame->pts = av_rescale_q(m_nLastAudioCardPresentationTime, rational, m_sysAudioStream->time_base);
        /**
            * Allocate the samples of the created frame. This call will make
            * sure that the audio frame can hold as many samples as specified.
            */
        if ((ret = av_frame_get_buffer(output_frame, 0)) < 0)
        {
            printf("Could not allocate output frame samples\n");
            av_frame_free(&output_frame);
            return ret;
        }

        /**
            * Read as many samples from the FIFO buffer as required to fill the frame.
            * The samples are stored in the frame temporarily.
            */
        if (audioRead(m_sysAudioFifo, (void **)output_frame->data, frame_size) < frame_size)
        {
            printf("Could not read data from FIFO\n");
            return AVERROR_EXIT;
        }
        /** Encode one frame worth of audio samples. */
        /** Packet used for temporary storage. */
        AVPacket outputPacket;
        av_init_packet(&outputPacket);
        outputPacket.data = nullptr;
        outputPacket.size = 0;
        int enc_got_frame_a = 0;
        /**
                * Encode the audio frame and store it in the temporary packet.
                * The output audio stream encoder is used to do this.
                */
        if ((ret = avcodec_encode_audio2(m_pSysCodecContext, &outputPacket, output_frame, &enc_got_frame_a)) < 0)
        {
            printf("Could not encode frame\n");
            av_packet_unref(&outputPacket);
            return ret;
        }
        /** Write one audio frame from the temporary packet to the output file. */
        if (enc_got_frame_a)
        {
            outputPacket.stream_index = m_sysAudioStream->index;
            outputPacket.pts = av_rescale_q(m_nLastAudioCardPresentationTime, rational, m_sysAudioStream->time_base);
            if ((ret = writeFrame(m_videoFormatContext, &outputPacket)) < 0)
            {
                char tmpErrString[128] = {0};
                printf("Could not write audio frame, error: %s\n", av_make_error_string(tmpErrString, AV_ERROR_MAX_STRING_SIZE, ret));
                av_packet_unref(&outputPacket);
                return ret;
            }
            av_packet_unref(&outputPacket);
        }//if (enc_got_frame_a)
        m_nb_samples += output_frame->nb_samples;
        m_nLastAudioCardPresentationTime += timeinc;
        av_frame_free(&output_frame);
    }// -- while end
    return 0;
}

int CAVOutputStream::writeSysToMixAudioFrame(AVStream *stream, AVFrame *inputFrame, int64_t lTimeStamp)
{
    int ret;
    if(nullptr == m_pSysAudioSwrContext)
    {
        m_pSysAudioSwrContext = swr_alloc_set_opts(nullptr,
                                                   av_get_default_channel_layout(m_pSysCodecContext->channels),
                                                   m_pSysCodecContext->sample_fmt,
                                                   m_pSysCodecContext->sample_rate,
                                                   av_get_default_channel_layout(stream->codec->channels),
                                                   stream->codec->sample_fmt,
                                                   stream->codec->sample_rate,
                                                   0,
                                                   nullptr);
        assert(m_pSysCodecContext->sample_rate == stream->codec->sample_rate);
        swr_init(m_pSysAudioSwrContext);
        if(nullptr == m_sysAudioFifo)
        {
            m_sysAudioFifo = audioFifoAlloc(m_pSysCodecContext->sample_fmt, m_pSysCodecContext->channels, 20*inputFrame->nb_samples);
        }
        is_fifo_scardinit ++;
    }
    if ((ret = av_samples_alloc(m_convertedSysSamples, nullptr, m_pSysCodecContext->channels, inputFrame->nb_samples, m_pSysCodecContext->sample_fmt, 0)) < 0)
    {
        av_freep(&(*m_convertedSysSamples)[0]);
        free(*m_convertedSysSamples);
        return ret;
    }
    if ((ret = swr_convert(m_pSysAudioSwrContext,m_convertedSysSamples, inputFrame->nb_samples,(const uint8_t**)inputFrame->extended_data, inputFrame->nb_samples)) < 0)
    {
        return ret;
    }
    int fifoSpace = audioFifoSpace(m_sysAudioFifo);
    int checkTime = 0;
    while(fifoSpace < inputFrame->nb_samples
          && isWriteFrame()
          && checkTime < 1000)
    {
        fifoSpace = audioFifoSpace(m_sysAudioFifo);
        usleep(10*1000);
        checkTime ++;
        //printf("_fifo_spk full m_fifo_scard!\n");
    }
    if (fifoSpace >= inputFrame->nb_samples)
    {
        if (audioWrite(m_sysAudioFifo, (void **)m_convertedSysSamples, inputFrame->nb_samples) < inputFrame->nb_samples)
        {
            printf("Could not write data to FIFO\n");
            return AVERROR_EXIT;
        }
    }
    return 0;
}

void  CAVOutputStream::close()
{
    QThread::msleep(500);
    if(nullptr != m_videoFormatContext
            || nullptr != m_videoStream
            || nullptr != m_micAudioStream
            || nullptr != m_sysAudioStream
            || nullptr != audio_amix_st)
    {
        qDebug() << Q_FUNC_INFO << "开始写文件尾";
        //Write file trailer
        writeTrailer(m_videoFormatContext);
        qDebug() << Q_FUNC_INFO << "写文件尾完成";
    }

    if (m_videoStream)
    {
        avcodec_close(m_videoStream->codec);
        m_videoStream = nullptr;
    }
    if (m_micAudioStream)
    {
        avcodec_close(m_micAudioStream->codec);
        m_micAudioStream = nullptr;
    }
    //    if (audio_scard_st)
    //    {
    //        avcodec_close(audio_scard_st->codec);
    //        audio_scard_st = nullptr;
    //    }
    if (audio_amix_st)
    {
        avcodec_close(audio_amix_st->codec);
        audio_amix_st = nullptr;
    }

    if(m_out_buffer)
    {
        av_free(m_out_buffer);
        m_out_buffer = nullptr;
    }
    if (m_convertedMicSamples)
    {
        av_freep(&m_convertedMicSamples[0]);
        m_convertedMicSamples = nullptr;
    }
    if (m_convertedSysSamples)
    {
        av_freep(&m_convertedSysSamples[0]);
        m_convertedSysSamples = nullptr;
    }
    is_fifo_scardinit = 0;
    if(m_videoFormatContext)
    {
        avio_close(m_videoFormatContext->pb);
    }
    avformat_free_context(m_videoFormatContext);
    m_videoFormatContext = nullptr;
    m_videoCodecID  = AV_CODEC_ID_NONE;
    m_micAudioCodecID = AV_CODEC_ID_NONE;
    m_sysAudioCodecID = AV_CODEC_ID_NONE;
    if(m_bMix)
    {
        av_frame_free(&mMic_frame);
        av_frame_free(&mSpeaker_frame);
        avfilter_graph_free(&filter_graph);
    }
}
void CAVOutputStream::setIsOverWrite(bool isCOntinue)
{
    m_isOverWrite = isCOntinue;
}

int CAVOutputStream::audioRead(AVAudioFifo *af, void **data, int nb_samples)
{
    QMutexLocker locker(&m_audioReadWriteMutex);
    //qDebug() << "+++++++++++++++++++++++ 读音频帧 +++++++++++++++++++++++";
    return av_audio_fifo_read(af, data, nb_samples);
}

int CAVOutputStream::audioWrite(AVAudioFifo *af, void **data, int nb_samples)
{
    QMutexLocker locker(&m_audioReadWriteMutex);
    //qDebug() << "----------------------- 写音频帧";
    return av_audio_fifo_write(af, data, nb_samples);
}

int CAVOutputStream::writeFrame(AVFormatContext *s, AVPacket *pkt)
{
    QMutexLocker locker(&m_writeFrameMutex);
    qDebug() << "+++++++++++++++++++++++ 写视频帧";
    return av_interleaved_write_frame(s, pkt);
}

int CAVOutputStream::writeTrailer(AVFormatContext *s)
{
    qDebug() << "+++++++++++++++++++++++ 写视频尾";
    QMutexLocker locker(&m_writeFrameMutex);
    return av_write_trailer(s);
}

int CAVOutputStream::audioFifoSpace(AVAudioFifo *af)
{
    qDebug() << "+++++++++++++++++++++++ 1";
    QMutexLocker locker(&m_audioReadWriteMutex);
    return av_audio_fifo_space(af);
}

int CAVOutputStream::audioFifoSize(AVAudioFifo *af)
{
    qDebug() << "+++++++++++++++++++++++ 2";
    QMutexLocker locker(&m_audioReadWriteMutex);
    return av_audio_fifo_size(af);
}

int CAVOutputStream::audioFifoRealloc(AVAudioFifo *af, int nb_samples)
{
    qDebug() << "+++++++++++++++++++++++ 3";
    QMutexLocker locker(&m_audioReadWriteMutex);
    return av_audio_fifo_realloc(af,nb_samples);
}

AVAudioFifo *CAVOutputStream::audioFifoAlloc(AVSampleFormat sample_fmt, int channels, int nb_samples)
{
    qDebug() << "+++++++++++++++++++++++ 4";
    QMutexLocker locker(&m_audioReadWriteMutex);
    return av_audio_fifo_alloc(sample_fmt,channels,nb_samples);
}

void CAVOutputStream::audioFifoFree(AVAudioFifo *af)
{
    qDebug() << "+++++++++++++++++++++++ 5";
    QMutexLocker locker(&m_audioReadWriteMutex);
    av_audio_fifo_free(af);
}

bool CAVOutputStream::isWriteFrame()
{
    QMutexLocker locker(&m_isWriteFrameMutex);
    return m_isWriteFrame;
}

void CAVOutputStream::setIsWriteFrame(bool isWriteFrame)
{
    QMutexLocker locker(&m_isWriteFrameMutex);
    m_isWriteFrame = isWriteFrame;
}

