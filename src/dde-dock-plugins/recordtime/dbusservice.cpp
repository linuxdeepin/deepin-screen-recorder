#include "dbusservice.h"
#include <QDebug>
DBusService::DBusService(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

bool DBusService::onStart()
{
    emit start();
    return true;
}

bool DBusService::onStop()
{
    emit stop();
    return true;
}
