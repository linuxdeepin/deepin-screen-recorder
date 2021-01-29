#ifndef CAMERAWATCHER_H
#define CAMERAWATCHER_H

#include <QObject>
#include <QThread>
#include <QMutex>

class CameraWatcher : public QThread
{
    Q_OBJECT
public:
    explicit CameraWatcher(QObject *parent = nullptr);
    ~CameraWatcher();
    void setWatch(const bool &is);
    bool isWatch();
    //void setIsRecoding(bool value);
    void run();

signals:
    void sigCameraState(bool couldUse);


private:
    bool m_loopwatch;
    //bool m_isRecoding;
    bool m_coulduse;
    //多线程加锁
    QMutex m_mutex;
};

#endif // CAMERAWATCHER_H

