#include "xgifrecord.h"

XGifRecord::XGifRecord(QRect rect,QString savePath,QObject *parent) :QThread(parent),
    m_recordRect(rect),
    m_bWriteFrame(false),
    m_savePath(savePath),
    m_pixelRatio(-1)
{
    m_bufferSize = 60;
    m_delay = 15;
    m_pixelRatio = qApp->primaryScreen()->devicePixelRatio();
}

void XGifRecord::run()
{
    setBWriteFrame(true);
    QtConcurrent::run(this,&XGifRecord::screenshots);
    QByteArray pathArry = m_savePath.toLocal8Bit();
    char *pathCh = new char[strlen(pathArry.data())+1];
    strcpy(pathCh,pathArry.data());
    bool bFirst = true;
    while (bWriteFrame())
    {
        QImage img;
        if(getFrame(img)){
            if(bFirst){
                bFirst = false;
                GifBegin(&m_gifWrite,
                         pathCh,
                         static_cast<uint32_t>(img.width()),
                         static_cast<uint32_t>(img.height()),
                         static_cast<uint32_t>(m_delay));
            }
            GifWriteFrame(&m_gifWrite,
                          img.convertToFormat(QImage::Format_RGBA8888).bits(),
                          static_cast<uint32_t>(img.width()),
                          static_cast<uint32_t>(img.height()),
                          static_cast<uint32_t>(m_delay));
        }
    }
    delete [] pathCh;
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
                                        static_cast<int>(m_recordRect.x()/m_pixelRatio),
                                        static_cast<int>(m_recordRect.y()/m_pixelRatio),
                                        static_cast<int>(m_recordRect.width()/m_pixelRatio),
                                        static_cast<int>(m_recordRect.height()/m_pixelRatio)).toImage());
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
