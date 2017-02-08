#include <QtNetwork/QLocalSocket>
#include <QFileInfo>
#include <QDebug>
#include "single_application.h"

#define TIME_OUT                (500)    // 500ms

SingleApplication::SingleApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , w(NULL)
    , appIsRunning(false)
    , localServer(NULL) {

    // Use application name as LocalServer's name.
    serverName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();

    initLocalConnection();
}


bool SingleApplication::isRunning()
{
    return appIsRunning;
}

void SingleApplication::newLocalConnection()
{
    QLocalSocket *socket = localServer->nextPendingConnection();
    if(socket) {
        socket->waitForReadyRead(2*TIME_OUT);
        delete socket;

        emit secondInstanceStart();
    }
}

void SingleApplication::initLocalConnection()
{
    appIsRunning = false;

    QLocalSocket socket;
    socket.connectToServer(serverName);
    if(socket.waitForConnected(TIME_OUT)) {
        fprintf(stderr, "%s already running.\n",
                serverName.toLocal8Bit().constData());
        appIsRunning = true;
        return;
    }

    newLocalServer();
}

void SingleApplication::newLocalServer()
{
    localServer = new QLocalServer(this);
    connect(localServer, SIGNAL(newConnection()), this, SLOT(newLocalConnection()));
    if(!localServer->listen(serverName)) {
        // If monitor failed (such as program crahs), remove it.
        if(localServer->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer(serverName);
            localServer->listen(serverName);
        }
    }
}
