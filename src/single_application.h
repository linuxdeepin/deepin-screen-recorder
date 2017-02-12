#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include <QtNetwork/QLocalServer>

class SingleApplication : public QApplication {
    Q_OBJECT

public:
    SingleApplication(int &argc, char **argv);
    ~SingleApplication() {
        delete localServer;
    };

    bool isRunning();

private slots:
    void newLocalConnection();
    
signals:
    void secondInstanceStart();

private:
    void initLocalConnection();
    void newLocalServer();

    bool appIsRunning;
    QLocalServer *localServer;
    QString serverName;
};

#endif // SINGLEAPPLICATION_H
