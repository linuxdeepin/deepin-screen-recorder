#ifndef CAMERAWATCHER_H
#define CAMERAWATCHER_H

#include <QObject>
#include <QThread>

class CameraWatcher : public QThread
{
    Q_OBJECT
public:
    explicit CameraWatcher(QObject *parent = nullptr);
    ~CameraWatcher();
    void stopWatch();
    void setIsRecoding(bool value);
    void run();

signals:
    void sigCameraState(bool couldUse);


private:
    bool m_loopwatch;
    bool m_isRecoding;
    bool m_coulduse;
};

#endif // CAMERAWATCHER_H

