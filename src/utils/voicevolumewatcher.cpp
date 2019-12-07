#include <QThread>
#include "voicevolumewatcher.h"
//#include "uiutil.h"
#include "audioutils.h"
#include <QDebug>

voiceVolumeWatcher::voiceVolumeWatcher(QObject *parent) : QThread(parent)
{
    m_loopwatch = true;
    m_isRecoding = false;
    m_coulduse = true;
}

voiceVolumeWatcher::~voiceVolumeWatcher()
{

}

//停止log循环读取
void voiceVolumeWatcher::stopWatch()
{
    m_loopwatch = false;
}

void voiceVolumeWatcher::setIsRecoding(bool value)
{
    m_isRecoding = value;
}

void voiceVolumeWatcher::run()
{
    m_loopwatch = true;
//    QThread::currentThread()->msleep(200);
    while (m_loopwatch) {

        //log缓存被更新并且仍进行loop循环
        if (!m_isRecoding && m_loopwatch) {
            bool couldUse = false;
            couldUse = AudioUtils::canMicrophoneInput();
            if (couldUse != m_coulduse) {
                //发送log信息到UI
                m_coulduse = couldUse;
                emit sigRecodeState(couldUse);
            }
        }
        QThread::currentThread()->msleep(1000);
    }
}
