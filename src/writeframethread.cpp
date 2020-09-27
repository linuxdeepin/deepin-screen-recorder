#include "writeframethread.h"
#include "waylandintegration.h"
#include "waylandintegration_p.h"
#include <qdebug.h>
#include <qimage.h>
#include "recordadmin.h"

WriteFrameThread::WriteFrameThread(WaylandIntegration::WaylandIntegrationPrivate* context, QObject *parent) :
    QThread(parent),
    m_bWriteFrame(false)
{
    m_context = context;
}

//int test = 0;
void WriteFrameThread::run()
{
    if(nullptr == m_context)
        return;
    WaylandIntegration::WaylandIntegrationPrivate::waylandFrame frame;
    while (bWriteFrame())
    {
        if(m_context->getFrame(frame))
        {
            //qDebug() << Q_FUNC_INFO << "++++++++++++++++++++++++" << test++ << frame._width <<  frame._height;
            m_context->m_recordAdmin->m_pOutputStream->writeVideoFrame(frame);
        }
    }
}

bool WriteFrameThread::bWriteFrame()
{
    QMutexLocker locker(&m_writeFrameMutex);
    return m_bWriteFrame;
}

void WriteFrameThread::setBWriteFrame(bool bWriteFrame)
{
    QMutexLocker locker(&m_writeFrameMutex);
    m_bWriteFrame = bWriteFrame;
}
