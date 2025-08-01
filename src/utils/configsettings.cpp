// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configsettings.h"
#include "saveutils.h"
#include "../utils.h"
#include "../utils/log.h"

#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QTemporaryFile>
#include <QDebug>

ConfigSettings::ConfigSettings(QObject *parent)
    : QObject(parent)
{
    qCDebug(dsrApp) << "ConfigSettings constructor called.";
    m_settings = new QSettings("deepin/deepin-screen-recorder", "deepin-screen-recorder");
    setValue("effect", "is_blur", false);
    setValue("effect", "is_mosaic", false);
    qCDebug(dsrApp) << "Config file path: " << m_settings->fileName();
}

ConfigSettings *ConfigSettings::m_configSettings = nullptr;
const QMap<QString, QMap<QString, QVariant>> ConfigSettings::m_defaultConfig = {
    {"rectangle", {{"color_index", 3}, {"line_width", 0}}},
    {"oval", {{"color_index", 3}, {"line_width", 0}}},
    {"line", {{"color_index", 3}, {"line_width", 0}}},
    {"arrow", {{"color_index", 3}, {"line_width", 0}}},
    {"pen", {{"color_index", 3}, {"line_width", 0}}},
    // 模糊类型， 模糊、马赛克；模糊形状，矩形，圆形；模糊强度
    {"effect", {{"isBlur", false}, {"isOval", 0}, {"radius", 0}, {"line_width", 1}}},
    {"text", {{"color_index", 3}, {"fontsize", 0}}},
    // 几何图形类型，矩形或椭圆
    {"shape", {{"current", "rectangle"}}},
    // 截图保存选项，格式，保存位置选项
    //save_dir: 普通路径
    //save_dir_change: 指定路径是否需要改变
    {"shot", {{"format", 0}, {"save_op", 0}, {"save_cursor", 0}, {"save_dir", ""},  {"save_dir_change", false}, {"border_index", 0}, {"save_ways", 1}}},
    // 录屏保存选项
    // curor 0 不录制鼠标，及不录制鼠标点击,1 录制鼠标,2 录制鼠标点击,3 录制鼠标，及录制鼠标点击,
    // audio 0 不录制任何音频,1 麦克风音频, 2 录制系统音频,3 录制混音,
    // save_op 保存位置视频目录 0, 桌面 1
    {"recorder", {{"format", 1}, {"frame_rate", 24}, {"save_op", 0}, {"save_dir", ""}, {"cursor", 0}, {"audio", 0}}},
};

ConfigSettings *ConfigSettings::instance()
{
    qCDebug(dsrApp) << "Getting ConfigSettings instance.";
    if (!m_configSettings) {
        qCDebug(dsrApp) << "Creating new ConfigSettings instance.";
        m_configSettings = new ConfigSettings();
    }

    qCDebug(dsrApp) << "Returning ConfigSettings instance.";
    return m_configSettings;
}

void ConfigSettings::setValue(const QString &group, const QString &key, const QVariant &val)
{
    qCDebug(dsrApp) << "Setting value for group: " << group << ", key: " << key << ", value: " << val;
    if (!m_defaultConfig.contains(group) || !m_defaultConfig[group].contains(key)) {
        qCDebug(dsrApp) << "Default config does not contain group or key. Group: " << group << ", Key: " << key;
        return;
    }

    if (val.type() == QVariant::Int) {
        qCDebug(dsrApp) << "Value type is Int. Emitting shapeConfigChanged.";
        emit shapeConfigChanged(group, key, val.toInt());
    }

    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup(group);
    m_settings->setValue(key, val);
    m_settings->endGroup();
    m_settings->sync();

    if (val.type() == QVariant::Int) {
        qCDebug(dsrApp) << "Config changed. Emitting shapeConfigChanged again.";
        emit shapeConfigChanged(group, key, val.toInt());
    }
    qCDebug(dsrApp) << "Value set successfully.";
}

QVariant ConfigSettings::getValue(const QString &group, const QString &key)
{
    qCDebug(dsrApp) << "Getting value for group: " << group << ", key: " << key;
    QMutexLocker locker(&m_mutex);
    QVariant value;
    m_settings->beginGroup(group);

    if (m_settings->contains(key)) {
        qCDebug(dsrApp) << "Settings contains key. Getting value from settings.";
        value = m_settings->value(key);
    } else {
        qCDebug(dsrApp) << "Settings does not contain key. Getting default value.";
        value = getDefaultValue(group, key);
    }
    m_settings->endGroup();
    qCDebug(dsrApp) << "Returning value: " << value;
    return value;
}

QVariant ConfigSettings::getDefaultValue(const QString &group, const QString &key)
{
    qCDebug(dsrApp) << "Getting default value for group: " << group << ", key: " << key;
    QVariant value;
    if (m_defaultConfig.contains(group) && m_defaultConfig[group].contains(key)) {
        qCDebug(dsrApp) << "Default config contains group and key. Setting value.";
        value.setValue(m_defaultConfig[group][key]);
    } else {
        qCDebug(dsrApp) << "ERROR! Default config does not contain group or key. Group: " << group << ", Key: " << key;
    }
    qCDebug(dsrApp) << "Returning default value: " << value;
    return  value;
}

QStringList ConfigSettings::keys(const QString &group)
{
    qCDebug(dsrApp) << "Getting keys for group: " << group;
    QStringList v;
    m_settings->beginGroup(group);
    v = m_settings->childKeys();
    m_settings->endGroup();
    qCDebug(dsrApp) << "Returning keys: " << v;
    return v;
}

ConfigSettings::~ConfigSettings()
{
    qCDebug(dsrApp) << "ConfigSettings destructor called.";
}
