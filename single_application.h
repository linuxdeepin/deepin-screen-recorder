#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QObject>
#include <QApplication>
#include <QtNetwork/QLocalServer>
#include <QWidget>

class SingleApplication : public QApplication {
    Q_OBJECT

public:
    SingleApplication(int &argc, char **argv);

    bool isRunning();
    QWidget *w;

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
