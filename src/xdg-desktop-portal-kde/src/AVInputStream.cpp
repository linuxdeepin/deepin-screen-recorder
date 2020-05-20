#include "AVInputStream.h"
#include <unistd.h>
#include <stdio.h>
#include <qbuffer.h>
#include <QProcess>
#include <QTime>
#include <QDebug>

CAVInputStream::CAVInputStream(void)
{
    m_hCapAudioThread = NULL;
    m_exit_thread = false;
    m_outPutType = Nomal;
    //    m_pVidFmtCtx = NULL;
    m_pAudFmtCtx = NULL;
    m_pAudFmtCtx_scard = NULL;
    //    m_pInputFormat = NULL;
    m_pAudioInputFormat = NULL;

    dec_pkt = NULL;

    m_pVideoCBFunc = NULL;
    m_pAudioCBFunc = NULL;
    m_pAudioScardCBFunc = NULL;
    m_audio_device = "default";
    m_audio_device_scard = "default";
    //    m_videoindex = -1;
    m_audioindex = -1;
    m_audioindex_scard = -1;
    m_start_time = 0;
}

CAVInputStream::~CAVInputStream(void)
{
    printf("Desctruction Input!\n");
    CloseInputStream();
}


void  CAVInputStream::SetVideoCaptureCB(VideoCaptureCB pFuncCB)
{
    m_pVideoCBFunc = pFuncCB;
}

void  CAVInputStream::SetAudioCaptureCB(AudioCaptureCB pFuncCB)
{
    m_pAudioCBFunc = pFuncCB;
}
void  CAVInputStream::SetAudioScardCaptureCB(AudioCaptureCB pFuncCB)
{
    m_pAudioScardCBFunc = pFuncCB;
}
void  CAVInputStream::SetWirteAmixtCB(AudioMixCB pFuncCB)
{
    m_mixCBFunc = pFuncCB;
}
//void  CAVInputStream::SetVideoCaptureDevice(string device_name)
//{
//    m_video_device = device_name;
//}

//void  CAVInputStream::SetAudioCaptureDevice(string device_name)
//{
//    m_audio_device_mic = device_name;
//}
void  CAVInputStream::setRecordAudioMic(bool isrecord)
{
    if(isrecord){
        m_audio_device = "default";
    }else{
        m_audio_device = "";
    }

}
void  CAVInputStream::setRecordAudioSCard(bool isrecord)
{
    if(isrecord){
        m_audio_device_scard = "default";
    }else{
        m_audio_device_scard = "";
    }
}
bool  CAVInputStream::OpenInputStream()
{
    //    m_videoindex = 999;
    if(m_outPutType==Gif){
        m_audio_device.clear();
        m_audio_device_scard.clear();
        return true;
    }
    AVDictionary *device_param = 0;
    int i;
    m_isMerge = false;

    ///音频...
    m_pAudioInputFormat = av_find_input_format("pulse"); //alsa
    assert(m_pAudioInputFormat != NULL);
    if(m_pAudioInputFormat == NULL)
    {
        printf("did not find this audio input devices\n");
    }

    if(!m_audio_device.empty())
    {
        string device_name = m_audio_device;

        //Set own audio device's name
        if (avformat_open_input(&m_pAudFmtCtx, device_name.c_str(), m_pAudioInputFormat, &device_param) != 0){

            printf("Couldn't open input audio stream.（无法打开输入流）\n");
            return false;
        }

        //input audio initialize
        if (avformat_find_stream_info(m_pAudFmtCtx, NULL) < 0)
        {
            printf("Couldn't find audio stream information.（无法获取流信息）\n");
            return false;
        }
        m_audioindex = -1;
        for (i = 0; i < m_pAudFmtCtx->nb_streams; i++)
        {
            if (m_pAudFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                m_audioindex = i;
                break;
            }
        }
        if (m_audioindex == -1)
        {
            printf("Couldn't find a audio stream.（没有找到音频流）\n");
            return false;
        }
        ///Caution, m_pAudFmtCtx->streams[m_audioindex]->codec->codec_id =14, AV_CODEC_ID_RAWVIDEO
        if (avcodec_open2(m_pAudFmtCtx->streams[m_audioindex]->codec, avcodec_find_decoder(m_pAudFmtCtx->streams[m_audioindex]->codec->codec_id), NULL) < 0)
        {
            printf("Could not open audio codec.（无法打开解码器）\n");
            return false;
        }

        /* print Video device information*/
        av_dump_format(m_pAudFmtCtx, 0, "default", 0);
    }
    m_pAudioCardInputFormat = av_find_input_format("pulse"); //alsa
    assert(m_pAudioCardInputFormat != NULL);
    if(m_pAudioCardInputFormat == NULL)
    {
        printf("did not find this card audio input devices\n");
    }

    if(!m_audio_device_scard.empty())
    {
        string device_name;
        //        m_audio_device_scard;
        if(m_audio_device_scard.compare("default")==0){
            QString device = currentAudioChannel();
            if(device.length()>0){
                device_name = device.toLatin1().data()[0];
            }else{
                printf("did not find this card AudioChannel \n");
            }

        }else{
            device_name = m_audio_device_scard;
        }

        if (avformat_open_input(&m_pAudFmtCtx_scard, device_name.c_str(), m_pAudioCardInputFormat, &device_param) != 0){
            printf("Couldn't open input audio stream.（无法打开输入流）\n");
            return false;
        }
        if (avformat_find_stream_info(m_pAudFmtCtx_scard, NULL) < 0)
        {
            printf("Couldn't find audio stream information.（无法获取流信息）\n");
            return false;
        }
        fflush(stdout);
        m_audioindex_scard = -1;
        for (i = 0; i < m_pAudFmtCtx_scard->nb_streams; i++)
        {
            if (m_pAudFmtCtx_scard->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                m_audioindex_scard = i;
                break;
            }
        }
        if (m_audioindex_scard == -1)
        {
            printf("Couldn't find a audio stream.（没有找到音频流）\n");
            return false;
        }
        ///Caution, m_pAudFmtCtx->streams[m_audioindex]->codec->codec_id =14, AV_CODEC_ID_RAWVIDEO
        if (avcodec_open2(m_pAudFmtCtx_scard->streams[m_audioindex_scard]->codec, avcodec_find_decoder(m_pAudFmtCtx_scard->streams[m_audioindex_scard]->codec->codec_id), NULL) < 0)
        {
            printf("Could not open audio codec.（无法打开解码器）\n");
            return false;
        }

        /* print Video device information*/
        av_dump_format(m_pAudFmtCtx_scard, 0, device_name.c_str(), 0);
    }
    if(!m_audio_device_scard.empty() && !m_audio_device.empty()){
        m_isMerge = true;
    }
    return true;
}

bool  CAVInputStream::StartCapture()
{
    //    if(m_outPutType!=Gif){
    //        if (m_videoindex == -1)
    //        {
    //            printf("错误：你没有打开设备 \n");
    //            return false;
    //        }
    //    }
    m_start_time = av_gettime();

    m_exit_thread = false;


    if(!m_audio_device.empty())
    {
        int rc = pthread_create(&m_hCapAudioThread, NULL, CaptureAudioThreadFunc, (void *)this);
    }
    if(!m_audio_device_scard.empty())
    {
        int rc = pthread_create(&m_hCapAudioScardThread, NULL, CaptureAudioSCardThreadFunc, (void *)this);
    }
    beginWriteMixAudio();

    if(!m_audio_device.empty())
    {
        pthread_join(m_hCapAudioThread,NULL);
    }
    if(!m_audio_device_scard.empty()){
        pthread_join(m_hCapAudioScardThread,NULL);
    }
    if(m_isMerge)
        pthread_join(m_hReadMixThread,NULL);
    onsFinisheStream();

    return true;
}
void CAVInputStream::writeToFrame(QImage *img, int64_t time){
    if (m_exit_thread)
        return;
    //    if(dec_pkt == NULL)
    //    {
    //        return;
    //    }
    if(m_start_time<=0){
        return ;
    }
    int encode_video = 1;
    int ret;
    AVFrame * pframe = NULL;
    pframe = av_frame_alloc();
    pframe->width = m_screenDW;
    pframe->height = m_screenDH;
    pframe->format =AV_PIX_FMT_BGR32;
    ret = av_frame_get_buffer(pframe,32);
    if(ret==0)
    {

        //pthread_mutex_lock(&mutexScreenD);
        QImage* imageTempt = img;
        int64_t timeStamp1 =av_gettime();

        //pthread_mutex_unlock(&mutexScreenD);

        pframe->width = imageTempt->width();
        pframe->height = imageTempt->height();
        pframe->format = m_ipix_fmt;
        pframe->pts = 0;
        pframe->crop_left = m_cl;
        pframe->crop_top = m_ct;
        pframe->crop_right = m_cr;
        pframe->crop_bottom = m_cb;
        pframe->data[0] = imageTempt->bits();
        //int dec_got_frame = 0;
        if(m_pVideoCBFunc)
        {
            int64_t timeStamp = timeStamp1 - m_start_time;
            //if(timeStamp>=0){
            //pthread_mutex_lock(&mutex);
            static int s_flag = 0;
            if (s_flag < time)
            {
                s_flag = time;
            }
            else {
                qDebug() << "error=================";
            }
            //qDebug() << "+++++++++++++++++++++++++++++++++++:时间戳："  << time;
            m_pVideoCBFunc(nullptr, m_ipix_fmt, pframe,time);
            //pthread_mutex_unlock(&mutex);
            //}
        }
        av_frame_free(&pframe);
    }
}

void CAVInputStream::writeToFrame(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame)
{
    if (m_exit_thread || m_start_time<=0)
        return;
    AVFrame * pframe = av_frame_alloc();
    pframe->width = m_screenDW;
    pframe->height = m_screenDH;
    pframe->format =AV_PIX_FMT_BGR32;
    if(0 == av_frame_get_buffer(pframe,32))
    {
        pframe->width = frame._width;
        pframe->height = frame._height;
        pframe->format = m_ipix_fmt;
        pframe->crop_left = m_cl;
        pframe->crop_top = m_ct;
        pframe->crop_right = m_cr;
        pframe->crop_bottom = m_cb;
        pframe->linesize[0] = frame._stride;
        pframe->data[0] = frame._frame;
        if(m_pVideoCBFunc)
        {
            //qDebug() << "+++++++++++++++++++++++++++++++++++:时间戳："  << time;
            m_pVideoCBFunc(nullptr, m_ipix_fmt, pframe,frame._time);
        }
        av_frame_free(&pframe);
    }
}
void CAVInputStream::initScreenData(){

}
void CAVInputStream::CloseInputStream()
{
    m_exit_thread = true;
    m_isWriting = false;
    usleep(200*1000);

    //    usleep(300000);
}
void  CAVInputStream::onsFinisheStream()
{
    //    av_free_packet(dec_pkt);
    //   m_videoindex = -1;
    //关闭线程
    if(m_hCapAudioThread)
    {
        m_hCapAudioThread = NULL;
    }
    if(m_hCapAudioScardThread)
    {
        m_hCapAudioScardThread = NULL;
    }
    if(m_hReadMixThread)
    {
        m_hReadMixThread = NULL;
    }
}
void CAVInputStream::onFInishCleanImage(){
}

void  *CAVInputStream::writeAmixThreadFunc(void* lParam)
{
    CAVInputStream * pThis = (CAVInputStream*)lParam;

    printf("CaptureAudioThread\n");
    pThis->doWritAmixAudio();
    return NULL;
}
int CAVInputStream::beginWriteMixAudio(){
    if(m_isMerge)
    {
        int rc = pthread_create(&m_hReadMixThread, NULL, writeAmixThreadFunc, (void *)this);
        return rc;
    }
    return 0;
}
void CAVInputStream::doWritAmixAudio(){
    if(!m_isMerge){
        return;
    }
    if(m_outPutType==Gif){
        return ;
    }
    m_isWriting = true;
    int ret;
    while (m_isWriting){
        //pthread_mutex_lock(&mutex);
        m_mixCBFunc();
        //pthread_mutex_unlock(&mutex);

    }
}
void* CAVInputStream::CaptureAudioThreadFunc(void* lParam)
{
    CAVInputStream * pThis = (CAVInputStream*)lParam;

    printf("CaptureAudioThread\n");
    pThis->ReadAudioPackets();
    return NULL;
}
//ReadAudioPackets + ReadVideoPackets
int CAVInputStream::ReadAudioPackets()
{
    //audio trancoding here
    int ret;
    if(m_outPutType==Gif){
        return 1;
    }
    if(m_audio_device.empty()){
        return 1;
    }
    int encode_audio = 1;
    int dec_got_frame_a = 0;

    //start decode and encode
    while (encode_audio)
    {
        if (m_exit_thread)
        {
            break;
        }

        QTime timer12;
        timer12.start();
        AVFrame *input_frame = av_frame_alloc();
        if (!input_frame)
        {
            ret = AVERROR(ENOMEM);
            return ret;
        }

        /** Decode one frame worth of audio samples. */
        /** Packet used for temporary storage. */
        AVPacket input_packet;
        av_init_packet(&input_packet);
        input_packet.data = NULL;
        input_packet.size = 0;

        /** Read one audio frame from the input file into a temporary packet. */
        if ((ret = av_read_frame(m_pAudFmtCtx, &input_packet)) < 0)
        {
            /** If we are at the end of the file, flush the decoder below. */
            if (ret == AVERROR_EOF)
            {
                encode_audio = 0;
            }
            else
            {
                //printf("Could not read audio frame\n");
                //                return ret;
                //added by flq
                continue;
                //added end
            }
        }

        if ((ret = avcodec_decode_audio4(m_pAudFmtCtx->streams[m_audioindex]->codec, input_frame, &dec_got_frame_a, &input_packet)) < 0)
        {
            printf("Could not decode audio frame\n");
            return ret;
        }
        av_packet_unref(&input_packet);
        /** If there is decoded data, convert and store it */
        if (dec_got_frame_a)
        {
            if(m_pAudioCBFunc)
            {
                if(m_isMerge){

                    //                     pthread_mutex_lock(&mutexAMix);
                    m_pAudioCBFunc(m_pAudFmtCtx->streams[m_audioindex], input_frame, av_gettime() - m_start_time);
                    //                     pthread_mutex_unlock(&mutexAMix);
                }else{
                    //pthread_mutex_lock(&mutex);
                    ///音频输出的回调,param3:打时间戳 //tqq
                    m_pAudioCBFunc(m_pAudFmtCtx->streams[m_audioindex], input_frame, av_gettime() - m_start_time);
                    //pthread_mutex_unlock(&mutex);
                }

            }
        }
        av_frame_free(&input_frame);
        int elapsed = timer12.elapsed();
        printf("----fram audio elapsed=%d\n",elapsed) ;
        fflush(stdout);
    }//while
    if (m_pAudFmtCtx != NULL)
    {
        avformat_close_input(&m_pAudFmtCtx);
    }


    if(m_pAudFmtCtx)
        avformat_free_context(m_pAudFmtCtx);

    m_pAudFmtCtx = NULL;
    m_audioindex = -1;
    return 0;
}
void* CAVInputStream::CaptureAudioSCardThreadFunc(void* lParam)
{
    CAVInputStream * pThis = (CAVInputStream*)lParam;

    printf("CaptureAudioThread\n");
    pThis->ReadAudioSCardPackets();
    return NULL;
}

//ReadAudioPackets + ReadVideoPackets
int CAVInputStream::ReadAudioSCardPackets()
{
    //audio trancoding here
    int ret;
    if(m_outPutType==Gif){
        return 1;
    }
    if(m_audio_device_scard.empty()){
        return 1;
    }
    int encode_audio = 1;
    int dec_got_frame_a = 0;

    //start decode and encode
    while (encode_audio)
    {
        if (m_exit_thread)
        {
            break;
        }

        AVFrame *input_frame = av_frame_alloc();
        if (!input_frame)
        {
            ret = AVERROR(ENOMEM);
            return ret;
        }
        AVPacket input_packet;
        av_init_packet(&input_packet);
        input_packet.data = NULL;
        input_packet.size = 0;

        /** Read one audio frame from the input file into a temporary packet. */
        if ((ret = av_read_frame(m_pAudFmtCtx_scard, &input_packet)) < 0)
        {
            /** If we are at the end of the file, flush the decoder below. */
            if (ret == AVERROR_EOF)
            {
                encode_audio = 0;
            }
            else
            {
                //printf("Could not read audio frame\n");
                //                return ret;
                //added by flq
                continue;
                //added end
            }
        }

        if ((ret = avcodec_decode_audio4(m_pAudFmtCtx_scard->streams[m_audioindex_scard]->codec, input_frame, &dec_got_frame_a, &input_packet)) < 0)
        {
            printf("Could not decode audio frame\n");
            return ret;
        }
        av_packet_unref(&input_packet);
        /** If there is decoded data, convert and store it */
        if (dec_got_frame_a)
        {
            if(m_pAudioCBFunc)
            {
                if(m_isMerge){
                    //                    pthread_mutex_lock(&mutexAMix);
                    m_pAudioScardCBFunc(m_pAudFmtCtx_scard->streams[m_audioindex_scard], input_frame, av_gettime() - m_start_time);
                    //                     pthread_mutex_unlock(&mutexAMix);
                }else{
                    //pthread_mutex_lock(&mutex);
                    ///音频输出的回调,param3:打时间戳 //tqq
                    m_pAudioScardCBFunc(m_pAudFmtCtx_scard->streams[m_audioindex_scard], input_frame, av_gettime() - m_start_time);
                    //pthread_mutex_unlock(&mutex);
                }

            }
        }

        av_frame_free(&input_frame);


    }//while
    if (m_pAudFmtCtx_scard != NULL)
    {
        avformat_close_input(&m_pAudFmtCtx_scard);
    }


    if(m_pAudFmtCtx_scard)
        avformat_free_context(m_pAudFmtCtx_scard);

    m_pAudFmtCtx_scard = NULL;
    m_audioindex_scard = -1;
    return 0;
}
bool CAVInputStream::GetVideoInputInfo(int & width, int & height, int & frame_rate, AVPixelFormat & pixFmt)
{
    //    if( m_outPutType == Gif)
    {
        width =m_screenDW-m_cl-m_cr;
        height = m_screenDH-m_ct-m_cb;
        pixFmt = m_ipix_fmt;
        frame_rate = m_fps;

        return true;
    }

    //    return false;
}

bool  CAVInputStream::GetAudioInputInfo(AVSampleFormat & sample_fmt, int & sample_rate, int & channels,int &layout)
{
    if(m_outPutType==Gif){
        return false;
    }
    if(m_audioindex != -1)
    {
        sample_fmt = m_pAudFmtCtx->streams[m_audioindex]->codec->sample_fmt;
        sample_rate = m_pAudFmtCtx->streams[m_audioindex]->codec->sample_rate;
        channels = m_pAudFmtCtx->streams[m_audioindex]->codec->channels;
        layout = m_pAudFmtCtx->streams[m_audioindex]->codec->channel_layout;
        return true;
    }

    return false;
}
bool  CAVInputStream::GetAudioSCardInputInfo(AVSampleFormat & sample_fmt, int & sample_rate, int & channels,int& layout)
{
    if(m_outPutType==Gif){
        return false;
    }

    if(m_audioindex_scard != -1)
    {
        sample_fmt = m_pAudFmtCtx_scard->streams[m_audioindex_scard]->codec->sample_fmt;
        sample_rate = m_pAudFmtCtx_scard->streams[m_audioindex_scard]->codec->sample_rate;
        channels =m_pAudFmtCtx_scard->streams[m_audioindex_scard]->codec->channels;
        layout = m_pAudFmtCtx_scard->streams[m_audioindex_scard]->codec->channel_layout;
        return true;
    }


    return false;
}

void  CAVInputStream::setVidioOutPutType(OUTPUT_TYPE outType)
{
    m_outPutType = outType;
}
OUTPUT_TYPE CAVInputStream::getVidioOutPutType()
{
    return m_outPutType;
}
QString CAVInputStream::currentAudioChannel()
{
    QStringList options;
    options << QString(QStringLiteral("-c"));
    options << QString(QStringLiteral("pacmd list-sources | grep -PB 1 'analog.*monitor>' | head -n 1 | perl -pe 's/.* //g'"));
    QProcess process;
    process.start(QString(QStringLiteral("bash")), options);
    process.waitForFinished();
    process.waitForReadyRead();
    QByteArray tempArray =  process.readAllStandardOutput();
    char * charTemp = tempArray.data();
    QString str_output = QString(QLatin1String(charTemp));
    process.close();
    return str_output;
}
