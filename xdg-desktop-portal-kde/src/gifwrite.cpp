#include "gifwrite.h"
#include "recordAdmin.h"
#include <QDebug>

GifWrite::GifWrite(WaylandIntegration::WaylandIntegrationPrivate* context,QObject *parent): QThread(parent),
    m_context(context)
{

}

GifWrite::~GifWrite()
{

}
void GifWrite::run()
{
    //int test =0;
    GifFrame frame;
    while (getBCache() || getCacheSize()>0)
    {
        frame.f = nullptr;
        frame.width = 0;
        frame.height = 0;
        frame.delay = 0;
        frame.palette = nullptr;
        frame.data = nullptr;
        frame.time = 0;
        frame = getFrame();
        if(nullptr != frame.data && nullptr != frame.palette && nullptr != frame.f)
        {
            //qDebug() << "-------------------------------------------" << test++ << "" << frame.time;
            m_context->m_recordAdmin->m_pGifCreator->GifWriteLzwImage(frame.f, frame.data, 0, 0, frame.width, frame.height, frame.delay, frame.palette);
        }
    }
    m_context->m_recordAdmin->m_pGifCreator->GifEnd(m_context->m_recordAdmin->m_pGifCreator->m_pGifWriter);
    qDebug() << "写gif文件尾";
}

void GifWrite::insertFrame(GifFrame frame)
{
    QMutexLocker locker(&m_cacheMutex);
    m_cacheMap.insert(frame.time,frame);
}

GifFrame GifWrite::getFrame()
{
    QMutexLocker locker(&m_cacheMutex);
    if((m_cacheMap.size() < 30 && getBCache()) || 0 == m_cacheMap.size())
    {
        GifFrame frame;
        frame.data = nullptr;
        return frame;
    }
    else
    {
        GifFrame frame = m_cacheMap.first();
        m_cacheMap.remove(m_cacheMap.firstKey());
        return frame;
    }
}

int GifWrite::getCacheSize()
{
    QMutexLocker locker(&m_cacheMutex);
    return m_cacheMap.size();
}

bool GifWrite::getBCache()
{
    QMutexLocker locker(&m_bCacheMutex);
    return m_bCache;
}

void GifWrite::setBCache(bool bCache)
{
    QMutexLocker locker(&m_bCacheMutex);
    m_bCache = bCache;
}



