#ifndef WRITEFRAMETOSTREAMTHREAD_H
#define WRITEFRAMETOSTREAMTHREAD_H
#include <QObject>
#include <QThread>
namespace WaylandIntegration{
class WaylandIntegrationPrivate;
class WriteFrameToStreamThread : public QThread
{
    Q_OBJECT
public:
    explicit WriteFrameToStreamThread(WaylandIntegrationPrivate* waylandIntegration,QObject *parent = nullptr);
    void run();
    //signals:

    //public slots:
private:
    WaylandIntegrationPrivate * m_waylandIntegration;
};

}

#endif // WRITEFRAMETOSTREAMTHREAD_H
