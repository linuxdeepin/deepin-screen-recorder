// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"
#include <QDebug>
#include "../utils/log.h"

Settings *Settings::m_settingsInstance = nullptr;
Settings::Settings(QObject *parent) : QObject(parent)
{
    qCDebug(dsrApp) << "Settings constructor called.";
    m_settings = new QSettings("deepin/deepin-screen-recorder", "deepin-pin-screenshots", this);
    qCDebug(dsrApp) << "QSettings object created.";
}


Settings *Settings::instance()
{
    if (m_settingsInstance == nullptr) {
        qCDebug(dsrApp) << "Creating new Settings instance.";
        m_settingsInstance = new Settings;
    } else {
        qCDebug(dsrApp) << "Returning existing Settings instance.";
    }
    return m_settingsInstance;
}

void Settings::release()
{
    if (m_settingsInstance != nullptr) {
        qCDebug(dsrApp) << "Releasing Settings instance.";
        delete m_settingsInstance;
        m_settingsInstance = nullptr;
    } else {
        qCDebug(dsrApp) << "Settings instance is already null, no release needed.";
    }
}

bool Settings::setSaveOption(const QPair<int, int> &value)
{
    qCDebug(dsrApp) << "Setting save options - path type:" << value.first << "format:" << value.second;
    m_settings->setValue("savePath", value.first);
    m_settings->setValue("saveFormat", value.second);
    qCDebug(dsrApp) << "Save options set.";
    return true;
}

QPair<int, int> Settings::getSaveOption()
{
    QPair<int, int> pair;
    pair.first = m_settings->value("savePath").toInt();
    pair.second = m_settings->value("saveFormat").toInt();
    qCDebug(dsrApp) << "Getting save options - path type:" << pair.first << "format:" << pair.second;
    return pair;
}

void Settings::setSavePath(const QString savePathDir)
{
    qCDebug(dsrApp) << "Setting save path directory:" << savePathDir;
    m_settings->setValue("savePathDir", savePathDir);
    qCDebug(dsrApp) << "Save path directory set.";
}

QString Settings::getSavePath()
{
    QString path = m_settings->value("savePathDir").toString();
    qCDebug(dsrApp) << "Getting save path directory:" << path;
    return path;
}

void Settings::setIsChangeSavePath(const bool isChange)
{
    qCDebug(dsrApp) << "Setting isChangeSavePath to:" << isChange;
    m_settings->setValue("isChangeSavePath", isChange);
    qCDebug(dsrApp) << "isChangeSavePath set.";
}

bool Settings::getIsChangeSavePath()
{
    bool isChange = m_settings->value("isChangeSavePath").toBool();
    qCDebug(dsrApp) << "Getting isChangeSavePath:" << isChange;
    return isChange;
}
