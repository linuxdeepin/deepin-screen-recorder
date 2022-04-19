/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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


#include "shotstartplugin.h"
#include <DApplication>
#include <QDesktopWidget>


#define ShotShartPlugin "shot-shart-plugin"
#define ShotShartApp "dde-shot-start"

ShotStartPlugin::ShotStartPlugin(QObject *parent)
    : QObject(parent), m_iconWidget(nullptr), m_tipsWidget(nullptr)

{
}

ShotStartPlugin::~ShotStartPlugin()
{
    if (nullptr != m_iconWidget)
        m_iconWidget->deleteLater();

    if (nullptr != m_tipsWidget)
        m_tipsWidget->deleteLater();
}

const QString ShotStartPlugin::pluginName() const
{
    return ShotShartPlugin;
}

const QString ShotStartPlugin::pluginDisplayName() const
{
    return tr("deepin-screen-recorder");
}

void ShotStartPlugin::init(PluginProxyInterface *proxyInter)
{
    // 加载翻译
    QString appName = qApp->applicationName();
    qApp->setApplicationName(ShotShartApp);
    qApp->loadTranslator();
    qApp->setApplicationName(appName);

    m_proxyInter = proxyInter;


    if (m_iconWidget.isNull())
        m_iconWidget.reset(new IconWidget);

    if (m_tipsWidget.isNull())
        m_tipsWidget.reset(new TipsWidget);

    if (!pluginIsDisable()) {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

bool ShotStartPlugin::pluginIsDisable()
{
    return m_proxyInter->getValue(this, "disabled", false).toBool();
}

void ShotStartPlugin::pluginStateSwitched()
{
    const bool disabledNew = !pluginIsDisable();
    m_proxyInter->saveValue(this, "disabled", disabledNew);
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

QWidget *ShotStartPlugin::itemWidget(const QString &itemKey)
{
    if (itemKey != ShotShartPlugin) return nullptr;

    return m_iconWidget.data();
}

QWidget *ShotStartPlugin::itemTipsWidget(const QString &itemKey)
{
    if (itemKey != ShotShartPlugin) return nullptr;

    m_tipsWidget->setText("deepin-screen-recorder Ctrl+Alt+A");
    return m_tipsWidget.data();
}

const QString ShotStartPlugin::itemCommand(const QString &itemKey)
{
    if (itemKey != ShotShartPlugin) return QString();

    return "dbus-send --print-reply --dest=com.deepin.Screenshot /com/deepin/Screenshot com.deepin.Screenshot.StartScreenshot";
}

const QString ShotStartPlugin::itemContextMenu(const QString &itemKey)
{
    if (itemKey != ShotShartPlugin) return QString();

    return  m_iconWidget->itemContextMenu();

}

void ShotStartPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{

    Q_UNUSED(checked);
    if (itemKey != ShotShartPlugin) return;

    m_iconWidget->invokedMenuItem(menuId);
}
