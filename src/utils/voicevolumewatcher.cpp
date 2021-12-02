/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "voicevolumewatcher.h"
#include "audioutils.h"

#include <QDebug>
#include <QThread>

voiceVolumeWatcher::voiceVolumeWatcher(QObject *parent)
    : QObject(parent)
    , m_coulduse(false)
    , m_isExistSystemAudio(false)
{
    //m_isRecoding = false;

    // 初始化Dus接口

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("com.deepin.daemon.Audio").value()) {
        initDeviceWatcher();
        initConnections();
        m_isExistSystemAudio = true;
    }
    m_watchTimer = new QTimer(this);
    connect(m_watchTimer, &QTimer::timeout, this, &voiceVolumeWatcher::slotvoiceVolumeWatcher); //新增定时器检测麦克风
}

voiceVolumeWatcher::~voiceVolumeWatcher()
{

}

//停止log循环读取
void voiceVolumeWatcher::setWatch(const bool isWatcher)
{
    if (isWatcher) {
        m_watchTimer->start(1000);
    } else {
        m_watchTimer->stop();
    }
}

// 将原有的run方法替换为slotvoiceVolumeWatcher，解决截图录屏退出时缓慢的问题
void voiceVolumeWatcher::slotvoiceVolumeWatcher()
{
    static const double DBL_EPSILON = 0.000001;
    bool couldUse = false;
    double currentMicrophoneVolume = 0.0;
    if (nullptr != m_defaultSource) {
        //https://pms.uniontech.com/zentao/bug-view-52019.html
        AudioPort activePort = m_defaultSource->activePort();
        //qDebug() << "=========" << activePort.name << activePort.description << activePort.availability << "--------";
        couldUse = false;
        if (isMicrophoneAvail(activePort.name)) {
            currentMicrophoneVolume = m_defaultSource->volume();
            if (currentMicrophoneVolume > DBL_EPSILON) {
                couldUse = true;
            }
        }
        if (couldUse != m_coulduse) {
            //发送log信息到UI
            m_coulduse = couldUse;
            emit sigRecodeState(couldUse);
        }
    } else {
        if (couldUse != m_coulduse) {
            //发送log信息到UI
            m_coulduse = couldUse;
            emit sigRecodeState(couldUse);
        }
    }
}

bool voiceVolumeWatcher::getystemAudioState()
{
    return m_isExistSystemAudio;
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
            this)
    );

    m_defaultSource.reset(
        new com::deepin::daemon::audio::Source(
            m_serviceName,
            m_audioInterface->defaultSource().path(),
            QDBusConnection::sessionBus(),
            this)
    );
    onCardsChanged(m_audioInterface->cards());
}

void voiceVolumeWatcher::onCardsChanged(const QString &value)
{
    //qDebug() << "Cards changed:" << value;
    if (value.isEmpty()) {
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
            , this, &voiceVolumeWatcher::onCardsChanged);
}

void voiceVolumeWatcher::onDefaultSourceChanaged(const QDBusObjectPath &value)
{
    //TODO:
    //    When default source changed, need recreate
    // the source Object
    qDebug() << "Source change-->from:" << m_defaultSource->path()
             << " name:" << m_defaultSource->name();

    m_defaultSource.reset(
        new com::deepin::daemon::audio::Source(
            m_serviceName,
            value.path(),
            QDBusConnection::sessionBus(),
            this)
    );

    /*
    AudioPort activePort = m_defaultSource->activePort();

    qDebug() << "Source change:-->to:" << value.path()
            << " name:" << m_defaultSource->name()
            << " activePort:" << activePort.name;

    emit inputSourceChanged(activePort.description);
    */
}

QDebug &operator <<(QDebug &out, const voiceVolumeWatcher::Port &port)
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
