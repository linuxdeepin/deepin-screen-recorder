// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configsettings.h"
#include "saveutils.h"
#include "../utils.h"

#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QTemporaryFile>
#include <QDebug>

//extern int g_configThemeType;

ConfigSettings::ConfigSettings(QObject *parent)
    : QObject(parent)
{
    m_settings = new QSettings("deepin", "deepin-screen-recorder");
    setValue("effect", "is_blur", false);
    setValue("effect", "is_mosaic", false);
    // 日志隐私，配置文件路径会打印用户名，安全问题。
    //qDebug() << "Setting file:" << m_settings->fileName();
}

ConfigSettings *ConfigSettings::m_configSettings = nullptr;
ConfigSettings *ConfigSettings::instance()
{
    if (!m_configSettings) {
        m_configSettings = new ConfigSettings();
    }

    return m_configSettings;
}

// TODO(justforlxz): use qApp to check shift
/*
void ConfigSettings::setTemporarySaveAction(const std::pair<bool, SaveAction> temporarySaveAction)
{
    m_temporarySaveOp = temporarySaveAction;
}
*/
void ConfigSettings::setValue(const QString &group, const QString &key,
                              QVariant val)
{
    m_settings->beginGroup(group);
    m_settings->setValue(key, val);
    m_settings->endGroup();
    m_settings->sync();

    if (val.type() == QVariant::Int) {
        qDebug() << "config changed";
        emit shapeConfigChanged(group, key, val.toInt());
    }

    if (group == "arrow" && key == "is_straight") {
        emit straightLineConfigChanged(val.toBool());
    }

    qDebug() << "ConfigSettings:" << group << key << val;
}

QVariant ConfigSettings::value(const QString &group, const QString &key,
                               const QVariant &defaultValue)
{
    Q_UNUSED(defaultValue);
    QMutexLocker locker(&m_mutex);

    QVariant value;
    m_settings->beginGroup(group);

    if (m_settings->contains(key)) {
        value = m_settings->value(key);
    } else {
        value = getDefaultValue(group, key);
    }
    m_settings->endGroup();

    return value;
}

QVariant ConfigSettings::getDefaultValue(const QString &group, const QString &key)
{
    Q_UNUSED(group);
    QVariant value;
    // QVariant 初始化默认值
    // toInt() == 0, toString() == "", toBool() == false
    if (Utils::isTabletEnvironment && group == "recordConfig") {
        // 平板默认录屏参数，MP4,24帧
        if (key == "save_as_gif") {
            return QVariant(false);
        } else if (key == "lossless_recording") {
            return QVariant(false);
        } else if (key == "mkv_framerate") {
            return QVariant(24);
        }
    }
    if (key == "save_op_record") {
        value.setValue(SaveAction::SaveToVideo);
    }
    if (key == "fontsize") {
        value.setValue(12);
    } else if (key == "saveClip") {
        value.setValue(1);
    } else if (key == "save_op") {
        if (Utils::isTabletEnvironment) {
            value.setValue(SaveAction::PadDefaultPath);
        } else {
            value.setValue(SaveAction::SaveToImage);
        }
    }
    return  value;
}

QStringList ConfigSettings::keys(const QString &group)
{
    QStringList v;
    m_settings->beginGroup(group);
    v = m_settings->childKeys();
    m_settings->endGroup();

    return v;
}

ConfigSettings::~ConfigSettings()
{
}
