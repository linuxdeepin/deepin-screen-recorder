#include "writeFrameThread.h"
#include "waylandintegration.h"
#include "waylandintegration_p.h"
#include <qdebug.h>
#include <qimage.h>
#include "recordAdmin.h"

WaylandIntegration::WriteFrameThread::WriteFrameThread(WaylandIntegrationPrivate* context, QObject *parent) :
    QThread(parent),
    m_bWriteFrame(false)
{
    m_context = context;
}

int test = 0;
void WaylandIntegration::WriteFrameThread::run()
{
    if(nullptr == m_context)
        return;
    WaylandIntegrationPrivate::waylandFrame frame;
    while (bWriteFrame())
    {
        if(m_context->getFrame(frame))
        {
            qDebug() << Q_FUNC_INFO << "++++++++++++++++++++++++" << test++ << frame._width <<  frame._height;
            m_context->m_recordAdmin->m_pOutputStream->writeVideoFrame(frame);
        }
    }
}

bool WaylandIntegration::WriteFrameThread::bWriteFrame()
{
    QMutexLocker locker(&m_writeFrameMutex);
    return m_bWriteFrame;
}

void WaylandIntegration::WriteFrameThread::setBWriteFrame(bool bWriteFrame)
{
    QMutexLocker locker(&m_writeFrameMutex);
    m_bWriteFrame = bWriteFrame;
}
