#include "gifrecord.h"
#include <QImage>
#include <qdebug.h>
#include "gifwrite.h"
#include "recordAdmin.h"

GifRecord::GifRecord(WaylandIntegration::WaylandIntegrationPrivate *context, int index, QObject *parent) : QThread(parent),
    m_context(context),
    m_index(index)
{
}

GifRecord::~GifRecord()
{

}

void GifRecord::init(int screenWidth,
                     int screenHeight,
                     int x,
                     int y,
                     int selectWidth,
                     int selectHeight,
                     int fps,
                     QString filePath)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_x = x;
    m_y = y;
    m_selectWidth = selectWidth;
    m_selectHeight = selectHeight;
    m_fps = fps;
    m_filePath = filePath;
    m_delay = 15;
}


//int GifRecord::test =0;
void GifRecord::run()
{
    if(nullptr == m_context)
        return;

    uint8_t *m_cacheOldImage = new uint8_t[m_selectWidth*m_selectHeight*4];

    WaylandIntegration::WaylandIntegrationPrivate::waylandFrame inFrame;
    GifFrame outFrame;
    while (bWriteFrame())
    {
        if(m_context->getFrame(inFrame))
        {
            outFrame = m_context->m_recordAdmin->m_pGifCreator->GifWriteFrame(m_context->m_recordAdmin->m_pGifCreator->m_pGifWriter,
                                                                              m_cacheOldImage,
                                                                              QImage(inFrame._frame,inFrame._width,inFrame._height,QImage::Format_RGB32).copy(m_x,m_y,m_selectWidth,m_selectHeight).
                                                                              convertToFormat(QImage::Format_RGBA8888).bits(),
                                                                              static_cast<uint32_t>(inFrame._time),
                                                                              static_cast<uint32_t>(m_selectWidth),
                                                                              static_cast<uint32_t>(m_selectHeight),
                                                                              static_cast<uint32_t>(m_delay));

            if(nullptr != outFrame.data)
            {
                //qDebug() << "+++++++++++++++++++++++++++++++++++++++++" << test++;
                m_context->m_recordAdmin->m_pGifWrite->insertFrame(outFrame);
            }
        }
    }
    delete[] m_cacheOldImage;
}

void GifRecord::stop()
{
    //GifEnd(m_context->m_recordAdmin->m_pGifWriter);
    qDebug() << "写gif文件尾";
}

bool GifRecord::bWriteFrame()
{
    QMutexLocker locker(&m_writeFrameMutex);
    return m_bWriteFrame;
}

void GifRecord::setBWriteFrame(bool bWriteFrame)
{
    QMutexLocker locker(&m_writeFrameMutex);
    m_bWriteFrame = bWriteFrame;
}
