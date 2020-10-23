#include "xgifrecord.h"

XGifRecord::XGifRecord(QRect rect,QString savePath,QObject *parent) :QThread(parent),
    m_recordRect(rect),
    m_savePath(savePath)
{
    m_bufferSize = 60;
    m_delay = 15;
}

void XGifRecord::run()
{
    setBWriteFrame(true);
    QtConcurrent::run(this,&XGifRecord::screenshots);
    QByteArray pathArry = m_savePath.toLocal8Bit();
    char *pathCh = new char[strlen(pathArry.data())+1];
    strcpy(pathCh,pathArry.data());
    GifBegin(&m_gifWrite,
             pathCh,
             static_cast<uint32_t>(m_recordRect.width()),
             static_cast<uint32_t>(m_recordRect.height()),
             static_cast<uint32_t>(m_delay));
    while (bWriteFrame())
    {
        QImage img;
        if(getFrame(img)){
            GifWriteFrame(&m_gifWrite,
                          img.convertToFormat(QImage::Format_RGBA8888).bits(),
                          static_cast<uint32_t>(m_recordRect.width()),
                          static_cast<uint32_t>(m_recordRect.height()),
                          static_cast<uint32_t>(m_delay));
        }
    }

    GifEnd(&m_gifWrite);
}

void XGifRecord::appendBuffer(QImage img)
{
    QMutexLocker locker(&m_mutex);
    if(m_imageList.size() >= m_bufferSize){
        m_imageList.removeFirst();
        m_imageList.append(img);
    }
    else if(0 <= m_imageList.size() && m_imageList.size() < m_bufferSize){
        m_imageList.append(img);
    }
}

void XGifRecord::screenshots()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    while (bWriteFrame()) {
        appendBuffer(screen->grabWindow(QApplication::desktop()->winId(),
                                        m_recordRect.x(),
                                        m_recordRect.y(),
                                        m_recordRect.width(),
                                        m_recordRect.height()).toImage());
    }
}

bool XGifRecord::getFrame(QImage &img)
{
    QMutexLocker locker(&m_mutex);
    if(m_imageList.size() <= 0){
        img = QImage();
        return false;
    }
    else {
        img = m_imageList.first();
        m_imageList.removeFirst();
        return true;
    }
}

void XGifRecord::setBWriteFrame(bool bWriteFrame)
{
    QMutexLocker locker(&m_writeFrameMutex);
    m_bWriteFrame = bWriteFrame;
}

bool XGifRecord::bWriteFrame()
{
    QMutexLocker locker(&m_writeFrameMutex);
    return m_bWriteFrame;
}

void XGifRecord::stop()
{
    setBWriteFrame(false);
}
