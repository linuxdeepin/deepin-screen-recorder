#include <QApplication>
#include <QStandardPaths>
#include <QObject>
#include "settings.h"

Settings::Settings(QObject *parent) : QObject(parent) 
{
    settings = new QSettings(configPath() + "/config.ini", QSettings::IniFormat);
    groupName = "fileformat";
}

QString Settings::configPath() 
{
    auto userConfigPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    return userConfigPath + "/" + qApp->organizationName() + "/" + qApp->applicationName();
}

QVariant Settings::option(const QString &key) 
{
    settings->beginGroup(groupName);
    QVariant result;
    if (settings->contains(key)) {
        result = settings->value(key);
    } else {
        result = true;
    }
    settings->endGroup();
    
    return result;
}

void Settings::setOption(const QString &key, const QVariant &value) {
    settings->beginGroup(groupName);
    settings->setValue(key, value);
    settings->endGroup();
    
    settings->sync();
}
