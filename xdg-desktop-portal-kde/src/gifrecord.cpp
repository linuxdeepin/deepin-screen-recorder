#include "gifrecord.h"
#include <QImage>
#include <qdebug.h>
//#include "gifwrite.h"
#include "recordAdmin.h"

GifRecord::GifRecord(WaylandIntegration::WaylandIntegrationPrivate *context, QObject *parent) : QThread(parent),
    m_context(context)
//    m_index(index)
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
    m_delay = 30;
    QByteArray pathArry = m_filePath.toLocal8Bit();
    char *pathCh = new char[strlen(pathArry.data())+1];
    strcpy(pathCh,pathArry.data());
    GifBegin(&m_gitWrite,pathCh,static_cast<uint32_t>(m_selectWidth),static_cast<uint32_t>(m_selectHeight),static_cast<uint32_t>(m_delay));
}


void GifRecord::run()
{
    if(nullptr == m_context)
        return;
    WaylandIntegration::WaylandIntegrationPrivate::waylandFrame frame;
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

void GifRecord::stop()
{
    GifEnd(&m_gitWrite);
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
