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
    explicit DBusService(QObject *parent = nullptr);

public slots:
    /**
     * @brief onStart:主程序通知插件开始计时
     * @return
     */
    Q_SCRIPTABLE bool onStart();

    /**
     * @brief onStop:主程序通知插件结束计时
     * @return
     */
    Q_SCRIPTABLE bool onStop();

signals:
    void start();
    void stop();
};

#endif // DBUSSERVICE_H
