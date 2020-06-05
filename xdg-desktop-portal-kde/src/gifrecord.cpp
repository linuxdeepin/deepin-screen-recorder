#include "gifrecord.h"
#include <QImage>
#include <qdebug.h>

WaylandIntegration::GifRecord::GifRecord(WaylandIntegrationPrivate *context, QObject *parent) : QThread(parent),
    m_context(context)
{

}

WaylandIntegration::GifRecord::~GifRecord()
{

}

void WaylandIntegration::GifRecord::init(int screenWidth,
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
    m_delay = 30;
    GifBegin(&m_gitWrite,m_filePath.toLatin1(),static_cast<uint32_t>(m_selectWidth),static_cast<uint32_t>(m_selectHeight),static_cast<uint32_t>(m_delay));
}

void WaylandIntegration::GifRecord::run()
{
    if(nullptr == m_context)
        return;
    WaylandIntegrationPrivate::waylandFrame frame;
    while (bWriteFrame())
    {
        if(m_context->getFrame(frame))
        {
            GifWriteFrame(&m_gitWrite,
                          QImage(frame._frame,frame._width,frame._height,QImage::Format_RGB32).copy(m_x,m_y,m_selectWidth,m_selectHeight).
                          convertToFormat(QImage::Format_RGBA8888).bits(),
                          static_cast<uint32_t>(m_selectWidth),
                          static_cast<uint32_t>(m_selectHeight),
                          static_cast<uint32_t>(m_delay));
        }
    }
    stop();
}

void WaylandIntegration::GifRecord::stop()
{
    GifEnd(&m_gitWrite);
    qDebug() << "写gif文件尾";
}

bool WaylandIntegration::GifRecord::bWriteFrame()
{
    QMutexLocker locker(&m_writeFrameMutex);
    return m_bWriteFrame;
}

void WaylandIntegration::GifRecord::setBWriteFrame(bool bWriteFrame)
{
    QMutexLocker locker(&m_writeFrameMutex);
    m_bWriteFrame = bWriteFrame;
}
