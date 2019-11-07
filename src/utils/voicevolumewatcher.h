#ifndef VOICEVOLUMEWATCHER_H
#define VOICEVOLUMEWATCHER_H

#include <QObject>
#include <QThread>

class voiceVolumeWatcher : public QThread
{
    Q_OBJECT
public:
    explicit voiceVolumeWatcher(QObject *parent = nullptr);
    ~voiceVolumeWatcher();
    void stopWatch();
    void setIsRecoding(bool value);
    void run();

signals:
    void sigRecodeState(bool couldUse);


private:
    bool m_loopwatch;
    bool m_isRecoding;
    bool m_coulduse;
};


#endif // VOICEVOLUMEWATCHER_H
