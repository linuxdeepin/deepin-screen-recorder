// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "voicevolumewatcher.h"

#include "utils.h"

static const QString kV20AudioService = "com.deepin.daemon.Audio";
static const QString kV20AudioPath = "/com/deepin/daemon/Audio";

voiceVolumeWatcher::voiceVolumeWatcher(QObject *parent)
    : QObject(parent)
    , m_coulduse(false)
    , m_isExistSystemAudio(false)
{
    m_version2 = Utils::isSysGreatEqualV23();
    if (m_version2) {
        qInfo() << "Using version 2.0 audio service";

        m_audioUtils = new AudioUtils(this);
        // check if there is an audio service
        if (m_audioUtils != nullptr) {
            QString cards = m_audioUtils->cards();
            if (!cards.isEmpty()) {
                // initialize all active ports
                initAvailInputPorts(cards);
            }
            connect(m_audioUtils, &AudioUtils::cardsChanged, this, &voiceVolumeWatcher::onCardsChanged);
            m_isExistSystemAudio = true;
            m_watchTimer = new QTimer(this);
            // the timer detects if the microphone is present
            connect(m_watchTimer, &QTimer::timeout, this, &voiceVolumeWatcher::slotVoiceVolumeWatcher);
        } else {
            qWarning() << "Failed to start the audio monitoring service!";
        }

    } else {
        qInfo() << "Using version 1.0 audio service";

        // initialize the DUS interface, check whether an audio interface exists com.deepin.daemon.Audio
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered(kV20AudioService).value()) {
            initV20DeviceWatcher();
            m_isExistSystemAudio = true;
        }
        m_watchTimer = new QTimer(this);
        connect(m_watchTimer, &QTimer::timeout, this, &voiceVolumeWatcher::slotVoiceVolumeWatcher);
    }
}

voiceVolumeWatcher::~voiceVolumeWatcher() {}

void voiceVolumeWatcher::setWatch(const bool isWatcher)
{
    if (isWatcher) {
        m_watchTimer->start(1000);
    } else {
        m_watchTimer->stop();
    }
}

/**
 * @brief Replace the original run method with slotVoiceVolumeWatcher.
 *  Fixed the issue that screenshot recording was slow to exit.
 */
void voiceVolumeWatcher::slotVoiceVolumeWatcher()
{
    double currentMicrophoneVolume = 0.0;
    if (m_version2) {
        if (nullptr != m_audioUtils) {
            if (isMicrophoneAvail(m_audioUtils->defaultSourceActivePort().name)) {
                currentMicrophoneVolume = m_audioUtils->defaultSourceVolume();
            }
        }
    } else {
        // link: https://pms.uniontech.com/zentao/bug-view-52019.html
        if (!m_defaultSource.isNull()) {
            AudioPort activePort = m_defaultSource->activePort();
            if (isMicrophoneAvail(activePort.name)) {
                currentMicrophoneVolume = m_defaultSource->volume();
            }
        }
    }

    static const double DBL_EPSILON = 0.000001;
    bool couldUse = bool(currentMicrophoneVolume > DBL_EPSILON);
    if (couldUse != m_coulduse) {
        // Send log information to the UI
        m_coulduse = couldUse;
        emit sigRecodeState(couldUse);
    }
}

/**
 * @brief Returns whether there is a system sound card or not
 */
bool voiceVolumeWatcher::getystemAudioState()
{
    return m_isExistSystemAudio;
}

void voiceVolumeWatcher::onCardsChanged(const QString &value)
{
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
        const int cardId = jCard["Id"].toInt();

        QJsonArray jPorts = jCard["Ports"].toArray();

        for (QJsonValue portVar : jPorts) {
            Port port;

            QJsonObject jPort = portVar.toObject();
            port.available = jPort["Available"].toInt();
            port.isActive = false;

            // 0 Unknow 1 Not available 2 Available
            if (port.available == 2 || port.available == 0) {
                port.portId = jPort["Name"].toString();
                port.portName = jPort["Description"].toString();
                port.cardId = cardId;
                port.cardName = cardName;
                // Only input ports are added
                if (port.isInputPort()) {
                    m_availableInputPorts.insert(port.portId, port);
                }
            }
        }
    }
}

/**
 * @brief Microphone sound detection
 */
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

/**
   @brief Create audio device watcher with DBus interface
        This function for V20 or older system edition.
 */
void voiceVolumeWatcher::initV20DeviceWatcher()
{
    m_audioInterface.reset(new com::deepin::daemon::Audio(kV20AudioService, kV20AudioPath, QDBusConnection::sessionBus(), this));
    m_defaultSource.reset(new com::deepin::daemon::audio::Source(
        kV20AudioService, m_audioInterface->defaultSource().path(), QDBusConnection::sessionBus(), this));

    connect(
        m_audioInterface.get(), &com::deepin::daemon::Audio::DefaultSourceChanged, this, [this](const QDBusObjectPath &value) {
            qInfo() << "Input device change from:"
                    << "\nactive port:" << m_defaultSource->activePort().name << "\ndevice name:" << m_defaultSource->name();

            m_defaultSource.reset(
                new com::deepin::daemon::audio::Source(kV20AudioService, value.path(), QDBusConnection::sessionBus(), this));

            qInfo() << "\nTo:"
                    << "\nactive port:" << m_defaultSource->activePort().name << "\ndevice name:" << m_defaultSource->name();
        });

    connect(m_audioInterface.get(), &com::deepin::daemon::Audio::CardsChanged, this, &voiceVolumeWatcher::onCardsChanged);

    // init current card info
    onCardsChanged(m_audioInterface->cards());
}

/**
 * @brief As long as the port name does not contain an output field, the port is attributed to the input port
 */
bool voiceVolumeWatcher::Port::isInputPort() const
{
    const QString inputPortFingerprint("output");
    return !portId.contains(inputPortFingerprint, Qt::CaseInsensitive);
}

bool voiceVolumeWatcher::Port::isLoopback() const
{
    const QString loopbackFingerprint("Loopback");

    return cardName.contains(loopbackFingerprint, Qt::CaseInsensitive);
}

QDebug &operator<<(QDebug &out, const voiceVolumeWatcher::Port &port)
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
