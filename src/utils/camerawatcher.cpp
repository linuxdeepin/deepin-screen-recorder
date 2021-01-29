#include "camerawatcher.h"
#include "audioutils.h"

#include <QThread>
#include <QCameraInfo>
#include <QDebug>

CameraWatcher::CameraWatcher(QObject *parent)
    : QThread(parent)
    , m_loopwatch(true)
    , m_coulduse(true)
{
    //m_isRecoding = false;
}

CameraWatcher::~CameraWatcher()
{

}

void CameraWatcher::setWatch(const bool &is)
{
    QMutexLocker locker(&m_mutex);
    m_loopwatch = is;
}

bool CameraWatcher::isWatch()
{
    QMutexLocker locker(&m_mutex);
    return m_loopwatch;
}

/*
 * never used
void CameraWatcher::setIsRecoding(bool value)
{
    m_isRecoding = value;
}
*/

void CameraWatcher::run()
{
    setWatch(true);
    //    QThread::currentThread()->msleep(1000);
    //防止反复申请和释放内存，减少内存碎片
    bool couldUse = false;
    while (isWatch()) {
        couldUse = false;
        //qDebug() << "QCameraInfo::availableCameras()" << QCameraInfo::defaultCamera().deviceName();
        if (QCameraInfo::availableCameras().count() > 0) {
            couldUse = true;
        }

        if (couldUse != m_coulduse) {
            //发送log信息到UI
            m_coulduse = couldUse;
            emit sigCameraState(couldUse);
        }

        QThread::currentThread()->msleep(1000);
    }
}

