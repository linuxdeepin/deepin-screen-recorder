#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include "settings.h"

Settings::Settings(QObject *parent) : QObject(parent)
{
    settings = new QSettings(QDir(configPath()).filePath("config.conf"), QSettings::IniFormat);

    groupName = "fileformat";
}

QString Settings::configPath()
{
    return QDir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()).filePath(qApp->organizationName())).filePath(qApp->applicationName());
}

QVariant Settings::getOption(const QString &key)
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
