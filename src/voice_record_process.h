#ifndef VOICE_RECORD_PROCESS_H
#define VOICE_RECORD_PROCESS_H
#include "settings.h"
#include <QProcess>
#include <QThread>
#include <QRect>
#include <proc/readproc.h>
#include <proc/sysinfo.h>
#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDateTime>
#include <QTimer>
class VoiceRecordProcess : public QThread
{
    Q_OBJECT

public:
    VoiceRecordProcess(QObject *parent = 0);

    QString generateRecordingFilepath();
    QString getRecordingFilepath();

    void exitRecord();
    void pauseRecord();
    void renderLevel(const QAudioBuffer &buffer);
    void resumeRecord();
    void startRecord();
    void stopRecord();
protected:
    void run();
private:
    QAudioProbe *audioProbe;
    QAudioRecorder *audioRecorder;
    QString recordPath;
    QDateTime lastUpdateTime;
    QTimer *tickerTimer;
    float recordingTime;
};
#endif // VOICE_RECORDER_PROCESS_H
