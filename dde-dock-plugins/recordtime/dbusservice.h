#ifndef DBUSSERVICE_H
#define DBUSSERVICE_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class DBusService : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","com.deepin.ScreenRecorder.time")

public:
    DBusService(QObject *parent);

public slots:
    Q_SCRIPTABLE bool onStart();
    Q_SCRIPTABLE bool onStop();

signals:
    void start();
    void stop();
};

#endif // DBUSSERVICE_H
