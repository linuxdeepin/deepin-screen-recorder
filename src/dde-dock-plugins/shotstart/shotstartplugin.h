// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECORDTIME_H
#define RECORDTIME_H

#include "iconwidget.h"
#include "tipswidget.h"
#include <QtDBus/QtDBus>
#include <dde-dock/pluginsiteminterface.h>

class ShotStartPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "shotstart.json")

public:
    explicit ShotStartPlugin(QObject *parent = nullptr);
    ~ShotStartPlugin() override;
    /**
     * @brief pluginName:返回插件名称
     * @return
     */
    const QString pluginName() const override;
    /**
     * @brief pluginDisplayName:返回插件名称，用于界面显示
     * @return
     */
    const QString pluginDisplayName() const override;
    /**
     * @brief init:插件初始化入口函数
     * @param proxyInter:主程序进程，上下文指针
     */
    void init(PluginProxyInterface *proxyInter) override;
    QIcon icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType()) override;
    /**
     * @brief pluginIsAllowDisable:返回插件是否允许被禁用
     * @return
     */
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;
    void pluginStateSwitched() override;

    PluginSizePolicy pluginSizePolicy() const override {return  PluginsItemInterface::Custom;}
    /**
     * @brief itemWidget:返回插件主控件，用于dde-dock面板上显示
     * @param itemKey:控件名称
     * @return
     */
    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;

    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, const int order) override;

    // 鼠标单击执行命令
    const QString itemCommand(const QString &itemKey) override;

    // 右键菜单
    const QString itemContextMenu(const QString &itemKey) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

private:
    QScopedPointer<IconWidget> m_iconWidget;
    QScopedPointer<TipsWidget> m_tipsWidget;
};

#endif // RECORDTIME_H
