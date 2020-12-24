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
    const QString pluginName() const override;
    //cppcheck误报：此函数从未被使用，其实这个函数由dde-dock框架调用
    const QString pluginDisplayName() const override;
    //cppcheck误报：此函数从未被使用，其实这个函数由dde-dock框架调用
    void init(PluginProxyInterface *proxyInter) override;
    //cppcheck误报：此函数从未被使用，其实这个函数由dde-dock框架调用
    bool pluginIsAllowDisable() override { return true; }
    QWidget *itemWidget(const QString &itemKey) override;

public slots:
    void onStart();
    void onStop();

private:
    void refresh();

private:
    QTimer *m_timer;
    QPointer<TimeWidget> m_timeWidget;
    QPointer<DBusService> m_dBusService;
    bool m_bshow;
};

#endif // RECORDTIME_H
