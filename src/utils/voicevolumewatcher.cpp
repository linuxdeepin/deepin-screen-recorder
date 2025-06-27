// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "voicevolumewatcher.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "com_deepin_daemon_audio.h"
#include "../dbus/com_deepin_daemon_audio_source.h"
#endif

#include "utils.h"
#include "log.h"

static const QString kV20AudioService = "com.deepin.daemon.Audio";
static const QString kV20AudioPath = "/com/deepin/daemon/Audio";

voiceVolumeWatcher::voiceVolumeWatcher(QObject *parent)
    : voiceVolumeWatcher(kV20AudioService, kV20AudioPath, QDBusConnection::sessionBus(), parent)
{
    qCDebug(dsrApp) << "Initializing voiceVolumeWatcher with default audio service.";
}

voiceVolumeWatcher::voiceVolumeWatcher(const QString &service, const QString &path, 
                                     const QDBusConnection &connection, QObject *parent)
    : voicevolumewatcher_interface(service, path, connection, parent)
    , m_coulduse(false)
    , m_isExistSystemAudio(false)
{
    qCDebug(dsrApp) << "Initializing voiceVolumeWatcher with service:" << service << ", path:" << path;
    m_version2 = Utils::isSysGreatEqualV23();
    if (m_version2) {
        qCInfo(dsrApp) << "Using version 2.0 audio service";
        qCDebug(dsrApp) << "System version >= V23, using AudioUtils.";

        m_audioUtils = new AudioUtils(this);
        // check if there is an audio service
        if (m_audioUtils != nullptr) {
            qCDebug(dsrApp) << "AudioUtils initialized.";
            QString cards = m_audioUtils->cards();
            if (!cards.isEmpty()) {
                // initialize all active ports
                initAvailInputPorts(cards);
                qCDebug(dsrApp) << "Initialized available input ports from cards.";
            }
            connect(m_audioUtils, &AudioUtils::cardsChanged, this, &voiceVolumeWatcher::onCardsChanged);
            m_isExistSystemAudio = true;
            m_watchTimer = new QTimer(this);
            // the timer detects if the microphone is present
            connect(m_watchTimer, &QTimer::timeout, this, &voiceVolumeWatcher::slotVoiceVolumeWatcher);
            qCDebug(dsrApp) << "Audio service exists and watch timer connected.";
        } else {
            qWarning() << "Failed to start the audio monitoring service!";
        }

    } else {
        qCInfo(dsrApp) << "Using version 1.0 audio service";

        // initialize the DUS interface, check whether an audio interface exists com.deepin.daemon.Audio
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered(kV20AudioService).value()) {
            initV20DeviceWatcher();
            m_isExistSystemAudio = true;
            qCDebug(dsrApp) << "V20 audio service registered and device watcher initialized.";
        }
        m_watchTimer = new QTimer(this);
        connect(m_watchTimer, &QTimer::timeout, this, &voiceVolumeWatcher::slotVoiceVolumeWatcher);
        qCDebug(dsrApp) << "Watch timer connected for V20 audio service.";
    }
}

voiceVolumeWatcher::~voiceVolumeWatcher()
{
    qCDebug(dsrApp) << "voiceVolumeWatcher destructor called.";
}

void voiceVolumeWatcher::setWatch(const bool isWatcher)
{
    qCDebug(dsrApp) << "Setting watch state to:" << isWatcher;
    if (isWatcher) {
        m_watchTimer->start(1000);
        qCDebug(dsrApp) << "Watch timer started.";
    } else {
        m_watchTimer->stop();
        qCDebug(dsrApp) << "Watch timer stopped.";
    }
}

/**
 * @brief Replace the original run method with slotVoiceVolumeWatcher.
 *  Fixed the issue that screenshot recording was slow to exit.
 */
void voiceVolumeWatcher::slotVoiceVolumeWatcher()
{
    qCDebug(dsrApp) << "Starting slotVoiceVolumeWatcher.";
    double currentMicrophoneVolume = 0.0;
    if (m_version2) {
        qCDebug(dsrApp) << "Checking microphone volume for version 2.0.";
        if (nullptr != m_audioUtils) {
            AudioPort activePort = m_audioUtils->defaultSourceActivePort();
            if (isMicrophoneAvail(activePort.name)) {
                currentMicrophoneVolume = m_audioUtils->defaultSourceVolume();
                qCDebug(dsrApp) << "Microphone available, volume:" << currentMicrophoneVolume;
            }
        }
    } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        qCDebug(dsrApp) << "Checking microphone volume for version 1.0.";
        // link: https://pms.uniontech.com/zentao/bug-view-52019.html
        if (!m_defaultSource.isNull()) {
            AudioPort activePort = m_defaultSource->activePort();
            if (isMicrophoneAvail(activePort.name)) {
                currentMicrophoneVolume = m_defaultSource->volume();
                qCDebug(dsrApp) << "Microphone available, volume:" << currentMicrophoneVolume;
            }
        }
#endif
    }

    static const double DBL_EPSILON = 0.000001;
    bool couldUse = bool(currentMicrophoneVolume > DBL_EPSILON);
    if (couldUse != m_coulduse) {
        // Send log information to the UI
        m_coulduse = couldUse;
        emit sigRecodeState(couldUse);
        qCDebug(dsrApp) << "Microphone usability changed to:" << couldUse;
    }
    qCDebug(dsrApp) << "Slot voice volume watcher finished.";
}

/**
 * @brief Returns whether there is a system sound card or not
 */
bool voiceVolumeWatcher::getystemAudioState()
{
    qCDebug(dsrApp) << "Returning system audio state:" << m_isExistSystemAudio;
    return m_isExistSystemAudio;
}

void voiceVolumeWatcher::onCardsChanged(const QString &value)
{
    qCDebug(dsrApp) << "Cards changed, new value length:" << value.length();
    if (value.isEmpty()) {
        qCWarning(dsrApp) << "Cards changed value is empty, returning.";
        return;
    }
    initAvailInputPorts(value);
    qCDebug(dsrApp) << "Initialized available input ports after cards changed.";
}

void voiceVolumeWatcher::initAvailInputPorts(const QString &cards)
{
    qCDebug(dsrApp) << "Initializing available input ports.";
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
                    qCDebug(dsrApp) << "Added input port:" << port.portName << "(" << port.portId << ") to available list.";
                }
            }
        }
    }
    qCDebug(dsrApp) << "Finished initializing available input ports. Total:" << m_availableInputPorts.size();
}

/**
 * @brief Microphone sound detection
 */
bool voiceVolumeWatcher::isMicrophoneAvail(const QString &activePort) const
{
    qCDebug(dsrApp) << "Checking if microphone is available for port:" << activePort;
    bool available = false;
    QMap<QString, Port>::const_iterator iter = m_availableInputPorts.find(activePort);
    if (iter != m_availableInputPorts.end()) {
        qCDebug(dsrApp) << "Active port found in available input ports.";
        if (!iter->isLoopback()) {
            available = true;
            qCDebug(dsrApp) << "Port is not loopback, microphone available.";
        }
    }
    qCDebug(dsrApp) << "Microphone availability for port " << activePort << ":" << available;
    return available;
}

/**
   @brief Create audio device watcher with DBus interface
        This function for V20 or older system edition.
 */
void voiceVolumeWatcher::initV20DeviceWatcher()
{
    qCDebug(dsrApp) << "Initializing V20 device watcher.";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_audioInterface.reset(new com::deepin::daemon::Audio::Audio(kV20AudioService, kV20AudioPath, QDBusConnection::sessionBus(), this));
    m_defaultSource.reset(new com::deepin::daemon::AudioSource::Source(
        kV20AudioService, m_audioInterface->defaultSource().path(), QDBusConnection::sessionBus(), this));
    qCDebug(dsrApp) << "DBus interfaces for V20 audio initialized.";

    connect(
        m_audioInterface.get(), &com::deepin::daemon::Audio::DefaultSourceChanged, this, [this](const QDBusObjectPath &value) {
            qCInfo(dsrApp) << "Input device change from:"
                    << "\nactive port:" << m_defaultSource->activePort().name << "\ndevice name:" << m_defaultSource->name();
            qCDebug(dsrApp) << "Default source changed signal received. New path:" << value.path();

            m_defaultSource.reset(
                new com::deepin::daemon::AudioSource::Source(kV20AudioService, value.path(), QDBusConnection::sessionBus(), this));
            qCDebug(dsrApp) << "Default source updated to new path.";

            qCInfo(dsrApp) << "\nTo:"
                    << "\nactive port:" << m_defaultSource->activePort().name << "\ndevice name:" << m_defaultSource->name();
        });

    connect(m_audioInterface.get(), &com::deepin::daemon::Audio::CardsChanged, this, &voiceVolumeWatcher::onCardsChanged);
    qCDebug(dsrApp) << "Connected CardsChanged signal.";

    // init current card info
    onCardsChanged(m_audioInterface->cards());
    qCDebug(dsrApp) << "Initialized current card info.";
#else
    qCCritical(dsrApp) << "V20 interface initialization triggered without Qt5 support.";
#endif
}

/**
 * @brief As long as the port name does not contain an output field, the port is attributed to the input port
 */
bool voiceVolumeWatcher::Port::isInputPort() const
{
    qCDebug(dsrApp) << "Checking if port is an input port:" << portId;
    const QString inputPortFingerprint("output");
    return !portId.contains(inputPortFingerprint, Qt::CaseInsensitive);
}

bool voiceVolumeWatcher::Port::isLoopback() const
{
    qCDebug(dsrApp) << "Checking if card is a loopback device:" << cardName;
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
