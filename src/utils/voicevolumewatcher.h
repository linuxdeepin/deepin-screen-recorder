#ifndef VOICEVOLUMEWATCHER_H
#define VOICEVOLUMEWATCHER_H

#include <com_deepin_daemon_audio.h>
#include <com_deepin_daemon_audio_source.h>

#include <QObject>
#include <QThread>

class voiceVolumeWatcher : public QThread
{
    Q_OBJECT
public:
    explicit voiceVolumeWatcher(QObject *parent = nullptr);
    ~voiceVolumeWatcher();
    void setWatch(const bool &is);
    bool isWatch();
    void setIsRecoding(bool value);
    void run();
protected:
    void initDeviceWatcher();
    void onCardsChanged(const QString &value);
    void initAvailInputPorts(const QString &cards);
    void initConnections();
    void onDefaultSourceChanaged(const QDBusObjectPath &value);
    bool isMicrophoneAvail(const QString &activePort) const;

signals:
    void sigRecodeState(bool couldUse);
    /* void inputSourceChanged(const QString &name); */

private:
    const QString m_serviceName {"com.deepin.daemon.Audio"};

    QScopedPointer<com::deepin::daemon::Audio> m_audioInterface;
    QScopedPointer<com::deepin::daemon::audio::Source> m_defaultSource;

    struct Port {
        QString  portId;
        QString portName;
        QString cardName;
        int     available {1};
        int     cardId;
        bool    isActive {false};

        bool isInputPort() const;
        bool isLoopback() const;
    };
    friend QDebug& operator << (QDebug& out, const Port &port);

    //All available input ports.except loopback port.
    QMap<QString, Port> m_availableInputPorts;

    bool m_loopwatch;
    //bool m_isRecoding;
    bool m_coulduse;
    //多线程加锁
    QMutex m_mutex;
};


#endif // VOICEVOLUMEWATCHER_H
