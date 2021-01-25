/*
 * Maintainer: Peng Hui<penghui@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    /*
    if (m_settings->allKeys().isEmpty()) {
        setValue("common", "color_index", 0);
        setValue ("common", "default_savepath", "");
        setValue("common", "themeType", 0);

        setValue("arrow", "color_index", 0);
        setValue("arrow", "arrow_linewidth_index", 1);
        setValue("arrow", "straightline_linewidth_index", 1);
        setValue("arrow", "is_straight", false);
        setValue("oval", "is_blur", false);
        setValue("oval", "is_mosaic", false);
        setValue("oval", "color_index", 0);
        setValue("oval", "linewidth_index", 1);
        setValue("line", "color_index", 0);
        setValue("line", "linewidth_index", 1);
        setValue("rectangle", "is_blur", false);
        setValue("rectangle", "is_mosaic", false);
        setValue("rectangle", "color_index", 0);
        setValue("rectangle", "linewidth_index", 1);
        setValue("text", "color_index", 0);
        setValue("text", "fontsize", 12);
        setValue("text", "prev_color", 0);

        setValue("save", "save_op", SaveAction::SaveToClipboard);
        setValue("save", "save_quality", 100);
        setValue("save", "format", 0);
        setValue("save", "saveClip", 1);
        setValue("save", "saveCursor", 0);
    }
*/
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
void ConfigSettings::setTemporarySaveAction(const std::pair<bool, SaveAction> temporarySaveAction)
{
    m_temporarySaveOp = temporarySaveAction;
}

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
    if(Utils::isTabletEnvironment && group == "recordConfig") {
        // 平板默认录屏参数，MP4,24帧
        if(key == "save_as_gif") {
            return QVariant(false);
        } else if(key == "lossless_recording") {
            return QVariant(false);
        } else if (key == "mkv_framerate") {
            return QVariant(24);
        }
    }

    if(key == "fontsize"){
        value.setValue(12);
    }else if (key == "linewidth_index"
              || key == "saveClip"
              || key == "arrow_linewidth_index"
              || key == "straightline_linewidth_index") {
        value.setValue(1);
    } else if (key == "save_op") {
        if (Utils::isTabletEnvironment) {
            value.setValue(SaveAction::PadDefaultPath);
        } else {
            value.setValue(SaveAction::SaveToClipboard);
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
