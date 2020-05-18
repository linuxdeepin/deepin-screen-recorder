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
   while (m_waylandIntegration->m_isStreamObjCreat) {

       WaylandIntegrationPrivate::waylandFrame wf;
       bool isSeccess = m_waylandIntegration->getFrame(wf);
       if(isSeccess){
//           QString name = QString::fromUtf8("test1111111111.jpg");
           auto capture = new QImage(wf.frame, wf.width, wf.height, QImage::Format_RGB32);
//           capture->save(name);
           m_waylandIntegration->m_recordStreamObjPtr->addImage(capture);
           delete capture;
       }
   }
}
