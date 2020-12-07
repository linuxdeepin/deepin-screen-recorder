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
    ~RecordTimePlugin();
    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;
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
};

#endif // RECORDTIME_H
