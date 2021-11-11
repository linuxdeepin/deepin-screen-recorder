#include "waylandscrollmonitor.h"


WaylandScrollMonitor::WaylandScrollMonitor(QObject *parent) : QObject(parent)
    , m_connection(nullptr)
    , m_queue(nullptr)
    , m_registry(nullptr)
    , m_fakeinput(nullptr)
{
    initWaylandScrollThread();
}

WaylandScrollMonitor::~WaylandScrollMonitor()
{
    releaseWaylandScrollThread();
}
void WaylandScrollMonitor::slotManualScroll(float direction)
{
    if (m_fakeinput != nullptr) {
        m_fakeinput->requestPointerAxisForCapture(Qt::Vertical, direction);
    }
}

void WaylandScrollMonitor::setupRegistry()
{
    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);
    qDebug() << "==========setupRegistry=====";
    connect(m_registry, &KWayland::Client::Registry::fakeInputAnnounced, this, &WaylandScrollMonitor::setupFakeinput);
    //connect(m_registry, &KWayland::Client::Registry::fakeInputRemoved, this, &WaylandMonitor::removeOutput);
//    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, this, [this] {
//        m_registryInitialized = true;
//    });

    m_registry->create(m_connection);
    m_registry->setEventQueue(m_queue);
    m_registry->setup();
}

void WaylandScrollMonitor::setupFakeinput(quint32 name, quint32 version)
{
    if (m_fakeinput == nullptr) {
        m_fakeinput = new KWayland::Client::FakeInput(this);
        m_fakeinput->setup(m_registry->bindFakeInput(name, version));
        m_fakeinput->authenticate(qAppName(), "wayland scroll monitor");
    }
}

void WaylandScrollMonitor::initWaylandScrollThread()
{

    m_connection = new KWayland::Client::ConnectionThread(this);
    connect(m_connection, &KWayland::Client::ConnectionThread::connected, this, &WaylandScrollMonitor::setupRegistry, Qt::QueuedConnection);
    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied, this, [this] {
        if (m_queue)
        {
            delete m_queue;
            m_queue = nullptr;
        }

        m_connection->deleteLater();
        m_connection = nullptr;
    });
    connect(m_connection, &KWayland::Client::ConnectionThread::failed, this, [this] {
        m_connection->deleteLater();
        m_connection = nullptr;
    });
    m_connection->initConnection();
}

void WaylandScrollMonitor::releaseWaylandScrollThread()
{

    if (m_fakeinput) {
        m_fakeinput->release();
        delete m_fakeinput;
        m_fakeinput = nullptr;
    }

    if (m_registry) {
        m_registry->release();
        delete  m_registry;
        m_registry = nullptr;
    }

    if (m_queue) {
        m_queue->release();
        delete m_queue;
        m_queue = nullptr;
    }

    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }
}

void WaylandScrollMonitor::doWaylandAutoScroll()
{
    if (m_fakeinput != nullptr) {
        m_fakeinput->requestPointerAxisForCapture(Qt::Vertical, 15.0);
    }
}
