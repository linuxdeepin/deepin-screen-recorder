/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
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

#ifndef RECORDTIME_H
#define RECORDTIME_H

#include <QtDBus/QtDBus>
#include <dde-dock/pluginsiteminterface.h>
#include "timewidget.h"
#include "dbusservice.h"

class RecordTimePlugin : public QObject,PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "recordtime.json")

public:
    explicit RecordTimePlugin(QObject *parent = nullptr);
    ~RecordTimePlugin() override;
    /**
     * @brief pluginName:返回插件名称
     * @return
     */
    const QString pluginName() const override;
    //cppcheck误报：此函数从未被使用，其实这个函数由dde-dock框架调用
    /**
     * @brief pluginDisplayName:返回插件名称，用于界面显示
     * @return
     */
    const QString pluginDisplayName() const override;
    //cppcheck误报：此函数从未被使用，其实这个函数由dde-dock框架调用
    /**
     * @brief init:插件初始化入口函数
     * @param proxyInter:主程序进程，上下文指针
     */
    void init(PluginProxyInterface *proxyInter) override;
    //cppcheck误报：此函数从未被使用，其实这个函数由dde-dock框架调用
    /**
     * @brief pluginIsAllowDisable:返回插件是否允许被禁用
     * @return
     */
    bool pluginIsAllowDisable() override { return true; }
    /**
     * @brief itemWidget:返回插件主控件，用于dde-dock面板上显示
     * @param itemKey:控件名称
     * @return
     */
    QWidget *itemWidget(const QString &itemKey) override;

public slots:
    /**
     * @brief onStart:启动计时服务
     */
    void onStart();

    /**
     * @brief onStop:停止计时服务
     */
    void onStop();

private:
    /**
     * @brief refresh:绕过dde-dock 2020.12版本对插件的控件大小的限制
     */
    void refresh();

private:
    QTimer *m_timer;
    QPointer<TimeWidget> m_timeWidget;
    QPointer<DBusService> m_dBusService;
    bool m_bshow;
};

#endif // RECORDTIME_H
