#ifndef WRITEFRAMETHREAD_H
#define WRITEFRAMETHREAD_H
#include <QMutex>
#include <QObject>
#include <QThread>

namespace WaylandIntegration
{
class WaylandIntegrationPrivate;
class WriteFrameThread : public QThread
{
    Q_OBJECT

public:
    explicit WriteFrameThread(WaylandIntegrationPrivate* context,QObject *parent = nullptr);
    void run();

    bool bWriteFrame();
    void setBWriteFrame(bool bWriteFrame);

private:
    WaylandIntegrationPrivate * m_context;
    //是否写视频帧
    bool m_bWriteFrame;
    QMutex m_writeFrameMutex;
};

}

#endif // WRITEFRAMETHREAD_H
