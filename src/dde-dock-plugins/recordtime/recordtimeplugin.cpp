#include <QDesktopWidget>
#include "recordtimeplugin.h"

RecordTimePlugin::RecordTimePlugin(QObject *parent)
    : QObject(parent)
    , m_bshow(false)
{
    m_timer = new QTimer(this);
    m_timeWidget = new TimeWidget();
}

RecordTimePlugin::~RecordTimePlugin()
{
    if(nullptr != m_timer)
        m_timer->deleteLater();
    if(nullptr != m_timeWidget)
        m_timeWidget->deleteLater();
}

const QString RecordTimePlugin::pluginName() const
{
    return QString("deepin-screen-recorder-plugin");
}

const QString RecordTimePlugin::pluginDisplayName() const
{
    return QString("deepin-screen-recorder");
}

void RecordTimePlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;
    m_dBusService = new DBusService(this);
    connect(m_dBusService,SIGNAL(start()),this,SLOT(onStart()));
    connect(m_dBusService,SIGNAL(stop()),this,SLOT(onStop()));
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.ScreenRecorder.time")
            && sessionBus.registerObject("/com/deepin/ScreenRecorder/time",this, QDBusConnection::ExportAdaptors)){
        qDebug() << "dbus service registration failed!";
    }

    m_timer->start(600);
    connect(m_timer, &QTimer::timeout, this, &RecordTimePlugin::refresh);
    //test
    //onStart();
}

QWidget *RecordTimePlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_timeWidget;
}

void RecordTimePlugin::onStart()
{
    if (m_timeWidget->enabled()){
        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemAdded(this, pluginName());
        m_bshow = true;
        m_timeWidget->start();
    }
}

void RecordTimePlugin::onStop()
{
    if (m_timeWidget->enabled()){
        m_proxyInter->itemRemoved(this, pluginName());
        m_bshow = false;
        m_timeWidget->stop();
    }
}

void RecordTimePlugin::refresh()
{
    QSize size = m_timeWidget->sizeHint();
    if(size.width() > m_timeWidget->width()
            && 1 != position()
            && 3 != position()
            && m_bshow){
        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemAdded(this, pluginName());
    }
}








