// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shotstartplugin.h"
#include <DApplication>
#include <QDesktopWidget>


#define ShotShartPlugin "shot-start-plugin"
#define ShotShartApp "deepin-screen-recorder" // 使用截图录屏的翻译

ShotStartPlugin::ShotStartPlugin(QObject *parent)
    : QObject(parent), m_iconWidget(nullptr), m_tipsWidget(nullptr)

{
}

const QString ShotStartPlugin::pluginName() const
{
    return ShotShartPlugin;
}

const QString ShotStartPlugin::pluginDisplayName() const
{
    return tr("Screen Capture");
}

void ShotStartPlugin::init(PluginProxyInterface *proxyInter)
{
#ifndef UNIT_TEST
    // 加载翻译
    QString appName = qApp->applicationName();
    qApp->setApplicationName(ShotShartApp);
    qApp->loadTranslator();
    qApp->setApplicationName(appName);
#endif

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
    return m_proxyInter->getValue(this, "disabled", true).toBool();
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

    m_tipsWidget->setText(tr("Screenshot") + m_iconWidget->getSysShortcuts("screenshot"));
    return m_tipsWidget.data();
}

int ShotStartPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    return m_proxyInter->getValue(this, key, 1).toInt();
}

void ShotStartPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
}

const QString ShotStartPlugin::itemCommand(const QString &itemKey)
{
    if (itemKey != ShotShartPlugin) return QString();

    return "dbus-send --print-reply --dest=com.deepin.Screenshot /com/deepin/Screenshot com.deepin.Screenshot.StartScreenshot";
}

const QString ShotStartPlugin::itemContextMenu(const QString &itemKey)
{
    if (itemKey != ShotShartPlugin)
        return QString();

    return  m_iconWidget->itemContextMenu();
}

void ShotStartPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{

    Q_UNUSED(checked);
    if (itemKey != ShotShartPlugin) return;

    m_iconWidget->invokedMenuItem(menuId);
}

ShotStartPlugin::~ShotStartPlugin()
{
    if (nullptr != m_iconWidget)
        m_iconWidget->deleteLater();

    if (nullptr != m_tipsWidget)
        m_tipsWidget->deleteLater();
}
