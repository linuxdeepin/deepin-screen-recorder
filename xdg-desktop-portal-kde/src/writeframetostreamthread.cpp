#include "writeframetostreamthread.h"
#include "waylandintegration.h"
#include "waylandintegration_p.h"
#include <qdebug.h>
#include <qimage.h>
#include "MainLoop.h"
WaylandIntegration::WriteFrameToStreamThread::WriteFrameToStreamThread(WaylandIntegrationPrivate* waylandIntegration,QObject *parent) : QThread(parent)
{
    m_waylandIntegration = waylandIntegration;
}
void WaylandIntegration::WriteFrameToStreamThread::run(){
    //    using namespace WaylandIntegration;
    //   WaylandIntegration::WaylandIntegrationPrivate* waylandIntegra = WaylandIntegration::waylandIntegration();
    msleep(300);
    if(m_waylandIntegration==NULL){
        qDebug()<<"WriteFrameToStreamThread get WaylandIntegration = null!!";
        return;
    }

    WaylandIntegrationPrivate::waylandFrame frame;
    while (m_waylandIntegration->m_isStreamObjCreat)
    {
        if(m_waylandIntegration->getFrame(frame))
        {
            m_waylandIntegration->m_recordStreamObjPtr->addImage(frame);
        }
    }
}
