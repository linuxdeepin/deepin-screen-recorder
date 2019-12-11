#include <QThread>
#include <QCameraInfo>
#include "camerawatcher.h"
#include "audioutils.h"
#include <QDebug>

CameraWatcher::CameraWatcher(QObject *parent) : QThread(parent)
{
    m_loopwatch = true;
    m_isRecoding = false;
    m_coulduse = true;
}

CameraWatcher::~CameraWatcher()
{

}

//停止log循环读取
void CameraWatcher::stopWatch()
{
    m_loopwatch = false;
}

void CameraWatcher::setIsRecoding(bool value)
{
    m_isRecoding = value;
}

void CameraWatcher::run()
{
    m_loopwatch = true;
//    QThread::currentThread()->msleep(1000);
    while (m_loopwatch) {
        //log缓存被更新并且仍进行loop循环
        if (!m_isRecoding && m_loopwatch) {
            bool couldUse = false;
//            qDebug() << "QCameraInfo::availableCameras()" << QCameraInfo::defaultCamera().deviceName();
            if (QCameraInfo::availableCameras().count() > 0) {
                couldUse = true;
            }
//            if (couldUse != m_coulduse) {
//                //发送log信息到UI
//                m_coulduse = couldUse;
            emit sigCameraState(couldUse);
//            }
        }
        QThread::currentThread()->msleep(200);
    }
}

