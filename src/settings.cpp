// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include "settings.h"
#include "utils/log.h"

Settings::Settings(QObject *parent) : QObject(parent)
{
    qCDebug(dsrApp) << "Settings constructor called.";
    settings = new QSettings(QDir(configPath()).filePath("config.conf"), QSettings::IniFormat);
    qCDebug(dsrApp) << "QSettings object created with config path:" << configPath() << ".";

    groupName = "fileformat";
    qCDebug(dsrApp) << "Group name set to:" << groupName << ".";
}

Settings::~Settings()
{
    qCDebug(dsrApp) << "Settings destructor called.";
    if (settings) {
        delete settings;
        settings = nullptr;
        qCDebug(dsrApp) << "QSettings object deleted.";
    }
}

QString Settings::configPath()
{
    QString path = QDir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()).filePath(qApp->organizationName())).filePath(qApp->applicationName());
    qCDebug(dsrApp) << "Config path returned:" << path << ".";
    return path;
}

QVariant Settings::getOption(const QString &key)
{
    qCDebug(dsrApp) << "getOption() called for key:" << key << ".";
    settings->beginGroup(groupName);
    QVariant result;
    if (settings->contains(key)) {
        result = settings->value(key);
        qCDebug(dsrApp) << "Option found for key:" << key << ", value:" << result << ".";
    } else {
        result = QVariant();
        qCDebug(dsrApp) << "Option not found for key:" << key << ", returning empty QVariant.";
    }
    settings->endGroup();

    return result;
}

void Settings::setOption(const QString &key, const QVariant &value)
{
    qCDebug(dsrApp) << "setOption() called for key:" << key << ", value:" << value << ".";
    settings->beginGroup(groupName);
    settings->setValue(key, value);
    settings->endGroup();

    settings->sync();
    qCDebug(dsrApp) << "Settings synced after setting option for key:" << key << ".";
}
