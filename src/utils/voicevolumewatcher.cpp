#include <QThread>
#include "voicevolumewatcher.h"
//#include "uiutil.h"
#include "audioutils.h"
#include <QDebug>

voiceVolumeWatcher::voiceVolumeWatcher(QObject *parent) : QThread(parent)
{
    m_loopwatch = true;
    //m_isRecoding = false;
    m_coulduse = true;

    // 初始化Dus接口
    initDeviceWatcher();
    initConnections();
}

voiceVolumeWatcher::~voiceVolumeWatcher()
{

}

//停止log循环读取
void voiceVolumeWatcher::stopWatch()
{
    m_loopwatch = false;
}
/*
void voiceVolumeWatcher::setIsRecoding(bool value)
{
    m_isRecoding = value;
}
*/
void voiceVolumeWatcher::run()
{
    m_loopwatch = true;
    //    QThread::currentThread()->msleep(200);

    static const double DBL_EPSILON = 0.000001;
    //static const double volumeLowMark = 0.2; //20% volume

    while (m_loopwatch) {
        AudioPort activePort = m_defaultSource->activePort();
        //qDebug() << "=========" << activePort.name << activePort.description << activePort.availability << "--------";
        bool couldUse = false;
        if (isMicrophoneAvail(activePort.name)) {
            double currentMicrophoneVolume = m_defaultSource->volume();
            if(currentMicrophoneVolume > DBL_EPSILON) {
                couldUse = true;
            }
        }
        if (couldUse != m_coulduse) {
            //发送log信息到UI
            m_coulduse = couldUse;
            emit sigRecodeState(couldUse);
        }
        QThread::currentThread()->msleep(1000);
    }
}

// 麦克风声音检测
bool voiceVolumeWatcher::isMicrophoneAvail(const QString &activePort) const
{
    bool available = false;
    QMap<QString, Port>::const_iterator iter = m_availableInputPorts.find(activePort);
    if (iter != m_availableInputPorts.end()) {
        if (!iter->isLoopback()) {
            available = true;
        }
    }
    return available;
}
void voiceVolumeWatcher::initDeviceWatcher()
{
    m_audioInterface.reset(
                new com::deepin::daemon::Audio(
                    m_serviceName,
                    "/com/deepin/daemon/Audio",
                    QDBusConnection::sessionBus(),
                    this )
                );

    m_defaultSource.reset(
                new com::deepin::daemon::audio::Source (
                    m_serviceName,
                    m_audioInterface->defaultSource().path(),
                    QDBusConnection::sessionBus(),
                    this )
                );
    onCardsChanged(m_audioInterface->cards());
}

void voiceVolumeWatcher::onCardsChanged(const QString &value)
{
    //qDebug() << "Cards changed:" << value;
    if(value.isEmpty()){
        return;
    }
    initAvailInputPorts(value);
}

void voiceVolumeWatcher::initAvailInputPorts(const QString &cards)
{
    m_availableInputPorts.clear();

    QJsonDocument doc = QJsonDocument::fromJson(cards.toUtf8());
    QJsonArray jCards = doc.array();

    for (QJsonValue cardVar : jCards) {
        QJsonObject jCard = cardVar.toObject();
        const QString cardName = jCard["Name"].toString();
        const int     cardId   = jCard["Id"].toInt();

        QJsonArray jPorts = jCard["Ports"].toArray();

        for (QJsonValue portVar : jPorts) {
            Port port;

            QJsonObject jPort = portVar.toObject();
            port.available = jPort["Available"].toInt();

            // 0 Unknow 1 Not available 2 Available
            if (port.available == 2 || port.available == 0) {
                port.portId   = jPort["Name"].toString();
                port.portName = jPort["Description"].toString();
                port.cardId   = cardId;
                port.cardName = cardName;
                // 只添加输入port
                if (port.isInputPort()) {
                    m_availableInputPorts.insert(port.portId, port);
                    //qDebug() << " " << port;
                }
            }
        }
    }
}

bool voiceVolumeWatcher::Port::isInputPort() const
{
    const QString inputPortFingerprint("input");

    return portId.contains(inputPortFingerprint, Qt::CaseInsensitive);
}

bool voiceVolumeWatcher::Port::isLoopback() const
{
    const QString loopbackFingerprint("Loopback");

    return cardName.contains(loopbackFingerprint,  Qt::CaseInsensitive);
}



void voiceVolumeWatcher::initConnections()
{
    //Default source change event
    connect(m_audioInterface.get(), &com::deepin::daemon::Audio::DefaultSourceChanged,
            this, &voiceVolumeWatcher::onDefaultSourceChanaged);

    connect(m_audioInterface.get(), &com::deepin::daemon::Audio::CardsChanged
            ,this, &voiceVolumeWatcher::onCardsChanged);
}

void voiceVolumeWatcher::onDefaultSourceChanaged(const QDBusObjectPath &value)
{
    //TODO:
    //    When default source changed, need recreate
    // the source Object
    qDebug() << "Source change-->from:" << m_defaultSource->path()
             << " name:" << m_defaultSource->name();

    m_defaultSource.reset(
                new com::deepin::daemon::audio::Source (
                    m_serviceName,
                    value.path(),
                    QDBusConnection::sessionBus(),
                    this )
                );

    /*
    AudioPort activePort = m_defaultSource->activePort();

    qDebug() << "Source change:-->to:" << value.path()
            << " name:" << m_defaultSource->name()
            << " activePort:" << activePort.name;

    emit inputSourceChanged(activePort.description);
    */
}

QDebug & operator <<(QDebug &out, const voiceVolumeWatcher::Port &port)
{
    out << "\n Port { "
        << "portId=" << port.portId << ","
        << "portName=" << port.portName << ","
        << "available=" << port.available << ","
        << "isActive=" << port.isActive << ","
        << "cardName=" << port.cardName << ","
        << "cardId=" << port.cardId << ","
        << " }\n";

    return out;
}
